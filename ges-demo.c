/*

  Author: Lubosz Sarnecki
  2013

*/

#include <stdlib.h>

#ifdef PLATTFORM_WINDOWS
#include <windows.h>
#endif

#include "ges-demo.h"
#include "ges-renderer.h"

GESTimeline *
testPatternTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;
  GESTestClip *srca, *srcb;

  timeline = ges_timeline_new_pal();

  layer = ges_layer_new ();
  g_object_set (layer, "auto-transition", TRUE, NULL);

  ges_timeline_add_layer (timeline, layer);

  srca = ges_test_clip_new ();
  srcb = ges_test_clip_new ();

  g_object_set (srca,
      "vpattern", GES_VIDEO_TEST_PATTERN_SMPTE,
      "duration", 3 * GST_SECOND,
      "start", 0, NULL);

  g_object_set (srcb,
      "vpattern", GES_VIDEO_TEST_PATTERN_CIRCULAR,
      "duration", 3 * GST_SECOND,
      "start", 2 * GST_SECOND, NULL);
  ges_test_clip_set_frequency (srcb, 800);

  ges_layer_add_clip (layer, GES_CLIP (srca));
  ges_layer_add_clip (layer, GES_CLIP (srcb));


  ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
effectTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;
  GESClip *clip1, *clip2;
  GESEffect *effect1, *effect2;

  timeline = ges_timeline_new_pal();
  layer = ges_layer_new ();

  ges_timeline_add_layer (timeline, layer);

  clip1 = ges_clip_unknown_from_rel_path ("sd/trailer_400p.ogg", layer, 0, 0, 10);
  clip2 = ges_clip_unknown_from_rel_path ("sd/sintel_trailer-480p.ogv", layer, 10, 5, 10);

  effect1 = ges_effect_new ("agingtv");
  ges_container_add (GES_CONTAINER (clip1), GES_TIMELINE_ELEMENT (effect1));

  effect2 = ges_effect_new ("rippletv");
  ges_container_add (GES_CONTAINER (clip2), GES_TIMELINE_ELEMENT (effect2));

  ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
transitionTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;

  timeline = ges_timeline_new_pal();
  layer = ges_layer_new ();

  g_object_set (layer, "auto-transition", TRUE, NULL);

  ges_timeline_add_layer (timeline, layer);

  ges_clip_unknown_from_rel_path ("sd/Mandelbox.mp4", layer, 0, 0, 10);
  ges_clip_unknown_from_rel_path ("sd/trailer_400p.ogg", layer, 7, 5, 10);

  ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
testTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;

  timeline = ges_timeline_new_pal();
  layer = ges_layer_new ();

  ges_timeline_add_layer (timeline, layer);

  GESClip *src = GES_CLIP (ges_test_clip_new ());

  g_object_set (src,
      "vpattern", GES_VIDEO_TEST_PATTERN_SMPTE,
      "duration", 5 * GST_SECOND, NULL);

  ges_layer_add_clip (layer, src);

  ges_timeline_commit (timeline);

  return timeline;
}


GESTimeline *
minuteTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;

  timeline = ges_timeline_new_pal();
  layer = ges_layer_new ();

  ges_timeline_add_layer (timeline, layer);

  ges_clip_unknown_from_rel_path ("sd/Black Ink and Water Test - A Place in Time Song.mp4", layer, 0, 0, 15);
  ges_clip_unknown_from_rel_path ("sd/trailer_400p.ogg", layer, 15, 2, 15);
  ges_clip_unknown_from_rel_path ("sd/sintel_trailer-480p.mp4", layer, 30, 4, 15);
  ges_clip_unknown_from_rel_path ("hd/fluidsimulation.mp4", layer, 45, 0, 15);

  ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
imageTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;

  timeline = ges_timeline_new_pal();

  layer = ges_layer_new ();
  g_object_set (layer, "auto-transition", TRUE, NULL);

  ges_timeline_add_layer (timeline, layer);

  ges_clip_unknown_from_rel_path ("images/LAMP_720_576.jpg",layer, 0, 0, 6);
  ges_clip_unknown_from_rel_path ("images/wallpaper-1946968.jpg", layer, 3, 0, 6);

  ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
hdTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;

  VideoSize hd = { 1280, 720, 30 };
  timeline = ges_timeline_audio_video_from_videosize(&hd);
  layer = ges_layer_new ();

  ges_timeline_add_layer (timeline, layer);

  ges_clip_unknown_from_rel_path ("hd/BlenderFluid.webm", layer, 0, 4, 5);
  ges_clip_unknown_from_rel_path ("hd/fluidsimulation.mp4", layer, 5, 7, 5);

  ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
overlayTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer, *layer2;

  timeline = ges_timeline_new_pal();
  layer = ges_layer_new ();
  layer2 = ges_layer_new();

  ges_layer_set_priority(layer2, 1);

  guint prio1 = ges_layer_get_priority(layer);
  guint prio2 = ges_layer_get_priority(layer2);

  g_object_set (layer, "auto-transition", TRUE, NULL);

  g_print("prios %d %d\n", prio1, prio2);

  ges_timeline_add_layer (timeline, layer);
  ges_timeline_add_layer (timeline, layer2);

  ges_clip_unknown_from_rel_path ("images/PNG_transparency_demonstration_1.png", layer, 0, 0, 10);
  ges_clip_unknown_from_rel_path ("hd/fluidsimulation.mp4", layer2, 0, 0, 10);

  ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
musicTL (void)
{
  GESTimeline *timeline;

  timeline = ges_timeline_new_audio_video ();

  GESLayer *layer = ges_layer_new ();
  GESLayer *audiolayer1 = ges_layer_new ();
  GESLayer *audiolayer2 = ges_layer_new ();

  ges_timeline_add_layer (timeline, layer);
  ges_timeline_add_layer (timeline, audiolayer1);
  ges_timeline_add_layer (timeline, audiolayer2);

  g_object_set (layer, "priority", 0, NULL);
  g_object_set (audiolayer1, "priority", 1, NULL);
  g_object_set (audiolayer2, "priority", 2, NULL);

  ges_clip_from_rel_path ("hd/fluidsimulation.mp4", layer, 0, 20, 10,
      GES_TRACK_TYPE_VIDEO);
  ges_clip_from_rel_path ("audio/prof.ogg", audiolayer1, 0, 0, 10,
      GES_TRACK_TYPE_AUDIO);
  ges_clip_from_rel_path ("audio/vask.wav", audiolayer2, 2, 0, 7,
      GES_TRACK_TYPE_AUDIO);

  ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
pngTransparencyTL (void)
{
  GESTimeline *timeline;
  GESTrack *trackv;
  timeline = ges_timeline_new ();

  trackv = GES_TRACK (ges_video_track_new ());

  ges_timeline_add_track (timeline, trackv);

  GstCaps *caps;
//  const gchar * capsstring = "video/x-raw,width=720,height=576,framerate=25/1,format=I420";
  const gchar * capsstring = "video/x-raw,width=720,height=576,framerate=25/1";

  caps = gst_caps_from_string (capsstring);
  gchar * capstring = gst_caps_to_string(caps);
  g_print("caps: %s\n", capstring);

  ges_track_set_restriction_caps(trackv, caps);

  GESLayer *layer1 = ges_layer_new ();
  ges_timeline_add_layer (timeline, layer1);
  g_object_set (layer1, "priority", 0, NULL);

  ges_clip_from_rel_path ("transparent/bokeeh.mov", layer1, 0, 0, 10, GES_TRACK_TYPE_VIDEO);

  GESLayer *layer2 = ges_layer_new ();
  ges_timeline_add_layer (timeline, layer2);
  g_object_set (layer2, "priority", 1, NULL);
  ges_clip_from_rel_path ("hd/fluidsimulation.mp4", layer2, 0, 20, 10,
      GES_TRACK_TYPE_VIDEO);

  ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
alphaTestTL (void)
{
  GESTimeline *timeline;

  GESTrack *trackv;
  timeline = ges_timeline_new ();

  trackv = GES_TRACK (ges_video_track_new ());

  if (!ges_timeline_add_track (timeline, trackv)) {
    gst_object_unref (timeline);
    timeline = NULL;
  }

  VideoSize pal = { 720, 576, 25 };
  GstCaps * caps = gst_caps_from_videosize(&pal);
  ges_track_set_restriction_caps(trackv, caps);

  GESLayer *layer1 = ges_layer_new ();
  GESLayer *layer2 = ges_layer_new ();

  ges_timeline_add_layer (timeline, layer1);
  ges_timeline_add_layer (timeline, layer2);

  g_object_set (layer1, "priority", 0, NULL);
  g_object_set (layer2, "priority", 1, NULL);

  GESClip * png = ges_clip_from_rel_path ("image/Fish.png", layer1, 0, 0, 10,
      GES_TRACK_TYPE_VIDEO);

  GESTrackElement* elem = ges_clip_find_track_element(png, trackv, G_TYPE_NONE);

  ges_track_element_set_child_properties (elem, "alpha", 0.5, NULL);

  ges_clip_from_rel_path ("hd/fluidsimulation.mp4", layer2, 0, 20, 10,
      GES_TRACK_TYPE_VIDEO);

  ges_timeline_commit (timeline);

  return timeline;
}

GESTimeline *
volumeTestTL (void)
{
  GESTimeline *timeline;

  GESTrack *tracka;
  timeline = ges_timeline_new ();

  tracka = GES_TRACK (ges_audio_track_new ());

  if (!ges_timeline_add_track (timeline, tracka)) {
    gst_object_unref (timeline);
    timeline = NULL;
  }

  GESLayer *layer1 = ges_layer_new ();
  GESLayer *layer2 = ges_layer_new ();

  ges_timeline_add_layer (timeline, layer1);
  ges_timeline_add_layer (timeline, layer2);

  g_object_set (layer1, "priority", 0, NULL);
  g_object_set (layer2, "priority", 1, NULL);

  GESClip * music1 = ges_clip_from_rel_path ("audio/02_Oliver_Huntemann_-_Rikarda.flac", layer1, 0, 0, 10,
      GES_TRACK_TYPE_AUDIO);
  ges_clip_from_rel_path ("audio/prof.ogg", layer2, 0, 0, 10, GES_TRACK_TYPE_AUDIO);

  GESTrackElement* elem = ges_clip_find_track_element(music1, tracka, G_TYPE_NONE);

  ges_track_element_set_child_properties (elem, "volume", 2.1, NULL);

  ges_timeline_commit (timeline);

  return timeline;
}


GESTimeline * compTL (void)
{
  GESTimeline *timeline;
  GESTrack *trackv;

  timeline = ges_timeline_new ();
  trackv = GES_TRACK (ges_video_track_new ());
  ges_timeline_add_track (timeline, trackv);

  const gchar * capsstring = "video/x-raw,width=1920,height=1080,framerate=25/1";
  GstCaps * caps = gst_caps_from_string (capsstring);
  gchar * capstring = gst_caps_to_string(caps);
  g_print("caps: %s\n", capstring);
//  ges_track_set_restriction_caps(trackv, caps);

  const gchar* assets[] = {"image/vieh.png",
                    "image/PNG_transparency_demonstration_1.png",
                    "image/Ice_Cream.png",
                    "image/Fish.png"};

  guint asset_count = 4;

  for (int i = 1; i <= asset_count; i++) {
      GESLayer *layer = ges_layer_new ();
      ges_timeline_add_layer (timeline, layer);
      g_object_set (layer, "priority", i - 1, NULL);

      g_print("asset %s\n", assets[i-1]);

      GESClip * vieh = ges_clip_from_rel_path (assets[i-1], layer, 0, 0, 10,
          GES_TRACK_TYPE_VIDEO);

      GESTrackElement* elem = ges_clip_find_track_element(
                  vieh, trackv, G_TYPE_NONE);

      ges_track_element_set_child_properties (elem,
                                              "posx", i * 100,
                                              "posy", i * 100,
                                              "width", i * 100,
                                              "height", i * 100,
                                               NULL);
  }

  GESLayer *backgroud_layer = ges_layer_new ();
  ges_timeline_add_layer (timeline, backgroud_layer);
  g_object_set (backgroud_layer, "priority", asset_count, NULL);
  ges_clip_from_rel_path ("image/wallpaper-2597248.jpg", backgroud_layer, 0, 0, 10,
      GES_TRACK_TYPE_VIDEO);

  ges_timeline_commit (timeline);

  return timeline;
}


GESTimeline * positionTestTL (void)
{
  GESTimeline *timeline;
  GESTrack *trackv;
  GError **error = NULL;
  GESAsset *asset;
  GESClip * clip;

  timeline = ges_timeline_new ();
  trackv = GES_TRACK (ges_video_track_new ());
  ges_timeline_add_track (timeline, trackv);

  const gchar * capsstring = "video/x-raw,width=720,height=576,framerate=25/1";
  GstCaps * caps = gst_caps_from_string (capsstring);
  gchar * capstring = gst_caps_to_string(caps);
  g_print("caps: %s\n", capstring);
  ges_track_set_restriction_caps(trackv, caps);

  GESLayer *layer = ges_layer_new ();
  ges_timeline_add_layer (timeline, layer);

  asset = GES_ASSET (ges_uri_clip_asset_request_sync (
                          ges_renderer_get_absolute_path("image/wallpaper720p.jpg"),
                         error));

  clip = ges_layer_add_asset (layer, asset,
      0, 0, 2 * GST_SECOND, GES_TRACK_TYPE_VIDEO);

  GESTrackElement* elem = ges_clip_find_track_element(clip, trackv, G_TYPE_NONE);

  ges_track_element_set_child_properties (elem, "posx", 100, "width", 100, NULL);

  ges_timeline_commit (timeline);

  return timeline;
}

void playTests(void) {
    ges_renderer_play(hdTL());
    ges_renderer_play(musicTL());
    ges_renderer_play(testTL());
    ges_renderer_play(transitionTL());
    ges_renderer_play(effectTL());
    ges_renderer_play(minuteTL());
}

void formatTests(void) {
    ges_renderer_render_pal(testTL(), "formats", PROFILE_VORBIS_VP8_WEBM);
    ges_renderer_render_pal(testTL(), "formats", PROFILE_VORBIS_THEORA_OGG);
    ges_renderer_render_pal(testTL(), "formats", PROFILE_AAC_H264_QUICKTIME);
    ges_renderer_render_pal(testTL(), "formats", PROFILE_VORBIS_H264_MATROSKA);
}

void renderTests(void) {
    ges_renderer_render_pal(effectTL(), "effect", PROFILE_AAC_H264_QUICKTIME);
    ges_renderer_render_pal(minuteTL(), "1minute", PROFILE_AAC_H264_QUICKTIME);

    VideoSize hd = { 1280, 720, 30 };
    ges_renderer_render(hdTL(), "hd", PROFILE_AAC_H264_QUICKTIME, &hd);
    ges_renderer_render_pal(musicTL(), "audio", PROFILE_AAC_H264_QUICKTIME);
    ges_renderer_render_pal(imageTL(), "image", PROFILE_AAC_H264_QUICKTIME);
    ges_renderer_render_pal(transitionTL(), "transition", PROFILE_AAC_H264_QUICKTIME);
}

void newTests(void) {
  ges_renderer_play(compTL());
  ges_renderer_play(volumeTestTL());
  ges_renderer_play(alphaTestTL());
  ges_renderer_play(pngTransparencyTL());
}

int
main (int argc, char **argv)
{
#ifdef PLATTFORM_WINDOWS
  LoadLibrary("exchndl.dll");
#endif
  gst_init (&argc, &argv);
  ges_init ();

  ges_renderer_init_path();

  formatTests();
  renderTests();
  newTests();

  return 0;
}
