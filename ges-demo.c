/*

  Author: Lubosz Sarnecki
  2013

*/

#include "ges-demo.h"

#include <stdlib.h>

static VideoSize pal = { 720, 576, 25 };

char *
path (const char *filenName)
{
  return g_strconcat (dataPath, filenName, NULL);
}

GESClip *
placeAsset (GESLayer * layer, gchar * path, gint start, gint in, gint dur)
{
  return placeAssetType (layer, path, start, in, dur, GES_TRACK_TYPE_UNKNOWN);
}

GESClip *
placeAssetType (GESLayer * layer, gchar * path, gint start, gint in, gint dur,
    GESTrackType tt)
{
  GError **error = NULL;
  GESAsset *asset;

  asset = GES_ASSET (ges_uri_clip_asset_request_sync (path, error));

  return ges_layer_add_asset (layer, asset,
      start * GST_SECOND, in * GST_SECOND, dur * GST_SECOND, tt);
}

void
busMessageCb (GstBus * bus, GstMessage * message, GMainLoop * mainloop)
{
  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:{
      GError *err = NULL;
      gchar *dbg_info = NULL;

      gst_message_parse_error (message, &err, &dbg_info);
      g_printerr ("ERROR from element %s: %s\n", GST_OBJECT_NAME (message->src),
          err->message);
      g_printerr ("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
      g_error_free (err);
      g_free (dbg_info);
      break;
    }
    case GST_MESSAGE_EOS:{
      g_print ("\nDone\n");
      g_main_loop_quit (mainloop);
      break;
    }
    default:
      break;
  }
}

GstEncodingProfile *
encoderProfile (EncodingProfile type, VideoSize * size)
{
  GstEncodingContainerProfile *prof;
  GstCaps *caps;
  GstCaps *settings;

  g_print ("Format: %s\n  Container: %s\n  Video:     %s \n  Audio:     %s\n",
      profiles[type][3],
      profiles[type][0], profiles[type][2], profiles[type][1]);

  caps = gst_caps_from_string (profiles[type][0]);
  prof =
      gst_encoding_container_profile_new ("Profile", "A web video profile",
      caps, NULL);
  gst_caps_unref (caps);

  caps = gst_caps_from_string (profiles[type][2]);

  char capsstring[50];
  sprintf (capsstring, "video/x-raw,width=%d,height=%d,framerate=%d/1",
      size->width, size->height, size->fps);

  settings = gst_caps_from_string (capsstring);
  gst_encoding_container_profile_add_profile (prof,
      (GstEncodingProfile *) gst_encoding_video_profile_new (caps, NULL,
          settings, 0));
  gst_caps_unref (caps);
  gst_caps_unref (settings);

  caps = gst_caps_from_string (profiles[type][1]);
  settings = gst_caps_from_string ("audio/x-raw");
  gst_encoding_container_profile_add_profile (prof,
      (GstEncodingProfile *) gst_encoding_audio_profile_new (caps, NULL,
          settings, 0));
  gst_caps_unref (caps);
  gst_caps_unref (settings);

  return (GstEncodingProfile *) prof;
}

gboolean
durationQuerier (void)
{
  gint64 position;

  gst_element_query_position (GST_ELEMENT (pipeline),
      GST_FORMAT_TIME, &position);

  float percent = (float) position * 100 / (float) duration;

  float positionSec = (float) position / GST_SECOND;
  float durationSec = (float) duration / GST_SECOND;

  if (position > 0)
    g_print ("\r%.2f%% %.2f/%.2fs", percent, positionSec, durationSec);

  return TRUE;
}

void
renderPipeline (GESPipeline * pipeline, EncodingProfile prof,
    const gchar * name, VideoSize * size)
{
  gchar *fileName =
      g_strconcat (dataPath, "export/", name, ".", profiles[prof][3], NULL);
  g_print ("Rendering %s\n", fileName);

  GstEncodingProfile *profile = encoderProfile (prof, size);
  ges_pipeline_set_render_settings (pipeline, fileName, profile);
  ges_pipeline_set_mode (pipeline, TIMELINE_MODE_RENDER);
}

GESPipeline *
newPipeline (GESTimeline * timeline)
{
  GESPipeline *pipeline;
  pipeline = ges_pipeline_new ();
  ges_pipeline_add_timeline (pipeline, timeline);

  duration = ges_timeline_get_duration (timeline);

  return pipeline;
}

void
runJob (GESTimeline * timeline, const gchar * name, EncodingProfile prof,
    VideoSize * size)
{
  GMainLoop *mainloop;
  mainloop = g_main_loop_new (NULL, FALSE);

  pipeline = newPipeline (timeline);

  if (name != NULL) {
    renderPipeline (pipeline, prof, name, size);
  } else {
    ges_pipeline_set_mode (pipeline, TIMELINE_MODE_PREVIEW_VIDEO);
    g_timeout_add_seconds (duration, (GSourceFunc) g_main_loop_quit, mainloop);
  }

  GstBus *bus;
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  g_signal_connect (bus, "message", (GCallback) busMessageCb, mainloop);
  g_timeout_add (100, (GSourceFunc) durationQuerier, NULL);
  gst_bus_add_signal_watch (bus);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);
}

void
play (GESTimeline * timeline)
{
  gchar *name = NULL;
  runJob (timeline, name, PROFILE_NONE, NULL);
}

void
render (GESTimeline * timeline, const gchar * name, EncodingProfile prof)
{
  renderWithSize (timeline, name, prof, &pal);
}

void
renderWithSize (GESTimeline * timeline, const gchar * name,
    EncodingProfile prof, VideoSize * size)
{
  g_print ("\n====\n");
  float now = (float) g_get_monotonic_time () / (float) GST_MSECOND;

  runJob (timeline, name, prof, size);

  float then = (float) g_get_monotonic_time () / (float) GST_MSECOND;
  float dur = then - now;
  g_print ("\n====\n");
  g_print ("Rendering took %.2fs\n", dur);
  g_print ("====\n");
}

GESTimeline *
transitionTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;
  GESTestClip *srca, *srcb;

  timeline = ges_timeline_new_audio_video ();

  layer = ges_layer_new ();
  g_object_set (layer, "auto-transition", TRUE, NULL);

  ges_timeline_add_layer (timeline, layer);

  srca = ges_test_clip_new ();
  srcb = ges_test_clip_new ();

  g_object_set (srca,
      "vpattern", GES_VIDEO_TEST_PATTERN_SMPTE,
      "duration", 5 * GST_SECOND, NULL);

  g_object_set (srcb,
      "vpattern", GES_VIDEO_TEST_PATTERN_CIRCULAR,
      "duration", 5 * GST_SECOND, "start", 2 * GST_SECOND, NULL);

  ges_test_clip_set_frequency (srcb, 800);

  ges_layer_add_clip (layer, GES_CLIP (srca));
  ges_layer_add_clip (layer, GES_CLIP (srcb));

  /*
     tr = ges_transition_clip_new_for_nick ("crossfade");
     g_object_set (tr,
     "start", tdur * GST_SECOND,
     "duration", tst * GST_SECOND, 
     "in-point", 0, 
     NULL);
     ges_layer_add_clip (layer1, GES_CLIP (tr));
   */
  //ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
effectTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;
  GESClip *clip1, *clip2;
  GESEffect *effect1, *effect2;

  timeline = ges_timeline_new_audio_video ();
  layer = ges_layer_new ();

  ges_timeline_add_layer (timeline, layer);

  clip1 = placeAsset (layer, path ("sd/trailer_400p.ogg"), 0, 0, 10);
  clip2 = placeAsset (layer, path ("sd/sintel_trailer-480p.ogv"), 10, 5, 10);

  effect1 = ges_effect_new ("agingtv");
  ges_container_add (GES_CONTAINER (clip1), GES_TIMELINE_ELEMENT (effect1));

  effect2 = ges_effect_new ("rippletv");
  ges_container_add (GES_CONTAINER (clip2), GES_TIMELINE_ELEMENT (effect2));

  //ges_timeline_commit (timeline);

  return timeline;
}


GESTimeline *
testTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;

  timeline = ges_timeline_new_audio_video ();
  layer = ges_layer_new ();

  ges_timeline_add_layer (timeline, layer);

  GESClip *src = GES_CLIP (ges_test_clip_new ());

  g_object_set (src,
      "vpattern", GES_VIDEO_TEST_PATTERN_SMPTE,
      "duration", 5 * GST_SECOND, NULL);

  ges_layer_add_clip (layer, src);

  //ges_timeline_commit (timeline);

  return timeline;
}


GESTimeline *
minuteTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;

  timeline = ges_timeline_new_audio_video ();
  layer = ges_layer_new ();

  ges_timeline_add_layer (timeline, layer);

  placeAsset (layer,
      path ("sd/Black Ink and Water Test - A Place in Time Song.mp4"),
      0, 0, 15);
  placeAsset (layer, path ("sd/trailer_400p.ogg"), 15, 2, 15);
  placeAsset (layer, path ("sd/sintel_trailer-480p.mp4"), 30, 4, 15);
  placeAsset (layer, path ("sd/Mandelbox.mp4"), 45, 0, 15);

  //ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
imageTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;

  timeline = ges_timeline_new_audio_video ();
  layer = ges_layer_new ();
  //g_object_set (layer, "auto-transition", TRUE, NULL);

  ges_timeline_add_layer (timeline, layer);

  placeAsset (layer, path ("images/LAMP_720_576.jpg"), 0, 0, 2);
  placeAsset (layer, path ("images/Fish.png"), 2, 0, 2);
  placeAsset (layer, path ("images/PNG_transparency_demonstration_1.png"),
      4, 0, 2);
  placeAsset (layer, path ("images/wallpaper-1946968.jpg"), 6, 0, 2);

  //ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
hdTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;

  timeline = ges_timeline_new_audio_video ();
  layer = ges_layer_new ();

  ges_timeline_add_layer (timeline, layer);

  placeAsset (layer, path ("hd/BlenderFluid.webm"), 0, 4, 5);
  placeAsset (layer, path ("hd/fluidsimulation.mp4"), 5, 7, 5);

  //ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
oneTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;

  timeline = ges_timeline_new_audio_video ();
  layer = ges_layer_new ();

  ges_timeline_add_layer (timeline, layer);

  placeAsset (layer, path ("sd/trailer_400p.ogg"), 0, 0, 5);

  ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
musicTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer = ges_layer_new ();
  GESLayer *audiolayer1 = ges_layer_new ();
  GESLayer *audiolayer2 = ges_layer_new ();

  timeline = ges_timeline_new_audio_video ();

  ges_timeline_add_layer (timeline, layer);
  ges_timeline_add_layer (timeline, audiolayer1);
  ges_timeline_add_layer (timeline, audiolayer2);

  placeAssetType (layer, path ("sd/Mandelbox.mp4"), 0, 20, 10,
      GES_TRACK_TYPE_VIDEO);
  placeAssetType (audiolayer1, path ("audio/prof.ogg"), 0, 0, 10,
      GES_TRACK_TYPE_AUDIO);
  placeAssetType (audiolayer2, path ("audio/vask.wav"), 3, 0, 7,
      GES_TRACK_TYPE_AUDIO);

  ges_timeline_commit (timeline);

  return timeline;
}

int
main (int argc, char **argv)
{
  gst_init (NULL, NULL);
  ges_init ();

  char directory[1024];
  getcwd (directory, 1024);

  //dataPath = g_strconcat ("file://", &directory, "/data/", NULL);
  dataPath = g_strconcat ("file:///C:/Users/bmonkey/cerbero/dist/windows_x86/bin/data/", NULL);

  g_print ("data path: %s\n", dataPath);

  render (testTL (), "formats", PROFILE_VORBIS_VP8_WEBM);
  render (testTL (), "formats", PROFILE_VORBIS_THEORA_OGG);
  render (testTL (), "formats", PROFILE_AAC_H264_QUICKTIME);
  render (testTL (), "formats", PROFILE_VORBIS_H264_MATROSKA);

  render (effectTL (), "effect", PROFILE_AAC_H264_QUICKTIME);
  render (minuteTL (), "1minute", PROFILE_AAC_H264_QUICKTIME);
  render (transitionTL (), "transition", PROFILE_AAC_H264_QUICKTIME);
  render (musicTL (), "audio", PROFILE_AAC_H264_QUICKTIME);

  render (imageTL (), "image", PROFILE_VORBIS_VP8_WEBM);

  VideoSize hd = { 1280, 720, 30 };
  renderWithSize (hdTL (), "hd", PROFILE_AAC_H264_QUICKTIME, &hd);

  /*
     play(hdTL());
     play(musicTL());
     play(testTL());
     play(transitionTL());
     play(effectTL());
     play(minuteTL());
   */
  return 0;
}
