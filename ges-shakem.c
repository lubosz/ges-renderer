#include <gst/gst.h>
#include <ges/ges.h>

static const char * const profile_specs[][4] = {
  { "application/ogg", "audio/x-vorbis", "video/x-theora", "ogv" },
  { "video/webm", "audio/x-vorbis", "video/x-vp8", "webm"},
  { "video/quicktime,variant=iso", "audio/mpeg,mpegversion=1,layer=3", "video/x-h264",  "mov"},
  { "video/x-matroska", "audio/x-vorbis", "video/x-h264", "mkv"},
};

typedef enum
{
  PROFILE_NONE = -1,
  PROFILE_VORBIS_THEORA_OGG,
  PROFILE_VORBIS_VP8_WEBM,
  PROFILE_AAC_H264_QUICKTIME,
  PROFILE_VORBIS_H264_MATROSKA,
} EncodingProfileName;

//gchar * videoFile1 = "file:///home/bmonkey/workspace/ges/data/trailer_400p.ogg";
gchar * videoFile1 = "file:///home/bmonkey/workspace/ges/data/BlenderFluid.webm";
//gchar * videoFile1 = "file:///home/bmonkey/workspace/ges/data/trailer_480p.mov";
gchar * videoFile2 = "file:///home/bmonkey/workspace/ges/data/sintel_trailer-720p.ogv";
//gchar * videoFile2 = "file:///home/bmonkey/workspace/ges/data/sintel_trailer-480p.mp4";
gchar * videoFile3 = "file:///home/bmonkey/workspace/ges/data/Sesame Street- Kermit and Joey Say the Alphabet.mp4";
gchar * videoFile4 = "file:///home/bmonkey/workspace/ges/data/BlenderFluid.webm";
//gchar * videoFile4 = "file:///home/bmonkey/workspace/ges/data/Blender Physics Animation HD.flv";

gchar * outfile = "file:///home/bmonkey/workspace/ges/export/crender.";

static GstClockTime duration;
static GESPipeline *pipeline;

GESTimeline * simplerTimeline() {
  GESTimeline *timeline;
  GError **error = NULL;
  GESLayer *layer;
  GESAsset *asset1,*asset2;
  GESClip * clip1, *clip2;
  GESEffect * effect1, *effect2;

  timeline = ges_timeline_new_audio_video();
  layer = ges_layer_new();

  ges_timeline_add_layer (timeline, layer);
  asset1 = ges_uri_clip_asset_request_sync(videoFile1, error);
  //asset2 = ges_uri_clip_asset_request_sync(videoFile2, error);
  
  clip1 = ges_layer_add_asset(layer, asset1, 0 * GST_SECOND, 2 * GST_SECOND, 10 * GST_SECOND, GES_TRACK_TYPE_VIDEO);
  //clip2 = ges_layer_add_asset(layer, asset2, 10 * GST_SECOND, 2 * GST_SECOND, 10 * GST_SECOND, GES_TRACK_TYPE_VIDEO);
  
  effect1 = ges_effect_new("agingtv");
  ges_container_add(clip1, effect1);
  
  //effect2 = ges_effect_new("videoscale");
  //ges_container_add(clip2, effect2);
  
  ges_timeline_commit(timeline);

  return timeline;
}

GESTimeline * simpleTimeline() {
  GESTimeline *timeline;
  GError **error = NULL;
  GESLayer *layer;
  GESAsset *asset1,*asset2,*asset3,*asset4;
  GESClip * clip1;
  GESEffect * effect1;

  timeline = ges_timeline_new_audio_video();
  layer = ges_layer_new();

  ges_timeline_add_layer (timeline, layer);
  asset1 = ges_uri_clip_asset_request_sync(videoFile1, error);
  asset2 = ges_uri_clip_asset_request_sync(videoFile2, error);
  asset3 = ges_uri_clip_asset_request_sync(videoFile3, error);
  asset4 = ges_uri_clip_asset_request_sync(videoFile4, error);
  
  clip1 = ges_layer_add_asset(layer, asset1, 1 * GST_SECOND, 1 * GST_SECOND, 9 * GST_SECOND, GES_TRACK_TYPE_UNKNOWN);
  ges_layer_add_asset(layer, asset2, 10 * GST_SECOND, 2 * GST_SECOND, 10 * GST_SECOND, GES_TRACK_TYPE_UNKNOWN);
  ges_layer_add_asset(layer, asset3, 20 * GST_SECOND, 4 * GST_SECOND, 10 * GST_SECOND, GES_TRACK_TYPE_UNKNOWN);
  ges_layer_add_asset(layer, asset4, 30 * GST_SECOND, 0 * GST_SECOND, 10 * GST_SECOND, GES_TRACK_TYPE_UNKNOWN);
  
  effect1 = ges_effect_new("agingtv");
  ges_container_add(clip1, effect1);
  
  ges_timeline_commit(timeline);

  return timeline;
}

void busMessageCb(GstBus *bus, GstMessage *message, GMainLoop *mainloop) {
  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR: {
      GError *err = NULL;
      gchar *dbg_info = NULL;
      
      gst_message_parse_error (message, &err, &dbg_info);
      g_printerr ("ERROR from element %s: %s\n", GST_OBJECT_NAME (message->src), err->message);
      g_printerr ("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
      g_error_free (err);
      g_free (dbg_info);
      break;
    }
    case GST_MESSAGE_EOS: {
      g_print("\nDone\n");
      g_main_loop_quit (mainloop);
      break;
    }
  }
}

GstEncodingProfile* encoderProfile(EncodingProfileName type) {
  GstEncodingContainerProfile *prof;
  GstCaps *caps;
  GstCaps *settings;
  
  g_print("writing %s file: %s: (%s + %s) \n", 
    profile_specs[type][3], 
    profile_specs[type][0], 
    profile_specs[type][2], 
    profile_specs[type][1]);

  caps = gst_caps_from_string(profile_specs[type][0]);
  prof = gst_encoding_container_profile_new("ShakeM Profile", "A Web Video Profile", caps, NULL);
  gst_caps_unref (caps);

  caps = gst_caps_from_string(profile_specs[type][2]);
  settings = gst_caps_from_string("video/x-raw,width=720,height=576,framerate=25/1");
  gst_encoding_container_profile_add_profile(prof,
      (GstEncodingProfile*) gst_encoding_video_profile_new(caps, NULL, settings, 0));
  gst_caps_unref (caps);
  gst_caps_unref (settings);

  caps = gst_caps_from_string(profile_specs[type][1]);
  settings = gst_caps_from_string("audio/x-raw");
  gst_encoding_container_profile_add_profile(prof,
      (GstEncodingProfile*) gst_encoding_audio_profile_new(caps, NULL, settings, 0));
  gst_caps_unref (caps);
  gst_caps_unref (settings);

  return (GstEncodingProfile*) prof;
}

gboolean  durationQuerier() {
  gint64 position;
  gst_element_query_position (GST_ELEMENT (pipeline), GST_FORMAT_TIME,
      &position);

  float percent = (float) position * 100 / (float) duration;
  
  float positionSec = (float) position / GST_SECOND;
  float durationSec = (float) duration / GST_SECOND;

  if (position > 0)
    g_print ("\r%.2f%% %.2f/%.2fs", percent, positionSec, durationSec);
  
/*
  progress = "\r%s: %.2f%%\t%.2f/%.2f secs" % (
            self.preset, 
            curTime / self.duration * 100, 
            curTime / Gst.SECOND, 
            self.duration / Gst.SECOND);
  stdout.write(progress);
  stdout.flush();
*/
  return TRUE;
}

void main() {
  GESTimeline *timeline;

  gst_init (NULL, NULL);
  ges_init ();

  //timeline = simpleTimeline();
  timeline = simplerTimeline();
  pipeline = ges_pipeline_new();

  ges_pipeline_add_timeline (pipeline, timeline);
  
  GMainLoop *mainloop;
  mainloop = g_main_loop_new (NULL, FALSE);

  duration = ges_timeline_get_duration(timeline);

  int render = 1;
  
  if (render) {
    unsigned prof = PROFILE_AAC_H264_QUICKTIME;
     /*
      PROFILE_VORBIS_THEORA_OGG,
      PROFILE_VORBIS_VP8_WEBM,
      PROFILE_AAC_H264_QUICKTIME,
      PROFILE_VORBIS_H264_MATROSKA,
     */
     
    gchar * fileName = g_strconcat(outfile, profile_specs[prof][3], NULL);
    g_print("filename %s \n", fileName);
     
    GstEncodingProfile* profile = encoderProfile(prof);
    ges_pipeline_set_render_settings(pipeline, fileName, profile);
    ges_pipeline_set_mode (pipeline, TIMELINE_MODE_RENDER);
  } else {
    g_timeout_add_seconds (duration, (GSourceFunc) g_main_loop_quit, mainloop);
  }
      
  GstBus *bus;
  bus = gst_pipeline_get_bus(pipeline);
  g_signal_connect(bus, "message", busMessageCb, mainloop);
  g_timeout_add (100, (GSourceFunc) durationQuerier, pipeline);
  gst_bus_add_signal_watch (bus);

  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);
}

