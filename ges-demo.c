#include <gst/gst.h>
#include <ges/ges.h>


#include <unistd.h>

static const char * const profiles[][4] = {
  { "application/ogg", "audio/x-vorbis", "video/x-theora", "ogv" },
  { "video/webm", "audio/x-vorbis", "video/x-vp8", "webm"},
  { "video/quicktime,variant=iso", "audio/mpeg,mpegversion=1,layer=3", "video/x-h264",  "mp4"},
  { "video/x-matroska", "audio/x-vorbis", "video/x-h264", "mkv"},
};

typedef struct DurationPipeline DurationPipeline;
struct DurationPipeline {
  GESPipeline * pipeline;
  GstClockTime duration;
};

typedef enum
{
  PROFILE_NONE = -1,
  PROFILE_VORBIS_THEORA_OGG,
  PROFILE_VORBIS_VP8_WEBM,
  PROFILE_AAC_H264_QUICKTIME,
  PROFILE_VORBIS_H264_MATROSKA,
} EncodingProfile;

gchar * dataPath = "file:///home/bmonkey/workspace/ges/data/";

char * path(char * filenName) {
  return g_strconcat(dataPath, filenName, NULL);
}

GESClip * placeAsset(GESLayer * layer, gchar * path, gint start, gint in, gint dur) {
  GError **error = NULL;
  GESAsset * asset;

  asset = ges_uri_clip_asset_request_sync(path, error);

  return ges_layer_add_asset(layer, asset, 
            start * GST_SECOND, 
            in * GST_SECOND, 
            dur * GST_SECOND, 
            GES_TRACK_TYPE_UNKNOWN);
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

GstEncodingProfile* encoderProfile(EncodingProfile type, int width, int height, int fps) {
  GstEncodingContainerProfile *prof;
  GstCaps *caps;
  GstCaps *settings;
  
  g_print("Format: %s\n  Container: %s\n  Video:     %s \n  Audio:     %s\n", 
    profiles[type][3], 
    profiles[type][0], 
    profiles[type][2], 
    profiles[type][1]);

  caps = gst_caps_from_string(profiles[type][0]);
  prof = gst_encoding_container_profile_new("Profile", "A web video profile", caps, NULL);
  gst_caps_unref (caps);

  caps = gst_caps_from_string(profiles[type][2]);
  
  char capsstring [50];
  sprintf (capsstring, "video/x-raw,width=%d,height=%d,framerate=%d/1", width, height, fps);
  
  settings = gst_caps_from_string(capsstring);
  gst_encoding_container_profile_add_profile(prof,
      (GstEncodingProfile*) gst_encoding_video_profile_new(caps, NULL, settings, 0));
  gst_caps_unref (caps);
  gst_caps_unref (settings);

  caps = gst_caps_from_string(profiles[type][1]);
  settings = gst_caps_from_string("audio/x-raw");
  gst_encoding_container_profile_add_profile(prof,
      (GstEncodingProfile*) gst_encoding_audio_profile_new(caps, NULL, settings, 0));
  gst_caps_unref (caps);
  gst_caps_unref (settings);

  return (GstEncodingProfile*) prof;
}

gboolean durationQuerier(DurationPipeline *dpipeline) {
  gint64 position;
  gst_element_query_position (GST_ELEMENT (dpipeline->pipeline), GST_FORMAT_TIME,
      &position);

  float percent = (float) position * 100 / (float) dpipeline->duration;
  
  float positionSec = (float) position / GST_SECOND;
  float durationSec = (float) dpipeline->duration / GST_SECOND;

  if (position > 0)
    g_print ("\r%.2f%% %.2f/%.2fs", percent, positionSec, durationSec);
    
  return TRUE;
}

void renderPipeline(GESPipeline *pipeline, EncodingProfile prof, gchar * name) {
    gchar * fileName = g_strconcat(dataPath, "export/", name, ".", profiles[prof][3], NULL);
    g_print("Rendering %s\n", fileName);
     
    GstEncodingProfile* profile = encoderProfile(prof, 720, 576, 25);
    ges_pipeline_set_render_settings(pipeline, fileName, profile);
    ges_pipeline_set_mode (pipeline, TIMELINE_MODE_RENDER);
}

DurationPipeline newPipeline(GESTimeline *timeline) {
  GESPipeline *pipeline;
  pipeline = ges_pipeline_new();
  ges_pipeline_add_timeline (pipeline, timeline);
  
  DurationPipeline dpipeline;
  dpipeline.pipeline = pipeline;
  dpipeline.duration = ges_timeline_get_duration(timeline);
  
  return dpipeline;
}

void play(GESTimeline *timeline) {
  runJob(timeline, NULL, NULL);
}

void render(GESTimeline *timeline, gchar * name, EncodingProfile prof) {
  g_print("\n====\n");
  float now = (float) g_get_monotonic_time() / (float) GST_MSECOND;

  runJob(timeline, name, prof);

  float then = (float) g_get_monotonic_time() / (float) GST_MSECOND;
  float dur = then - now;
  g_print("\n====\n");
  g_print("Rendering took %.2fs\n", dur);
  g_print("====\n");
}

void runJob(GESTimeline *timeline, gchar * name, EncodingProfile prof) {
  GMainLoop *mainloop;
  mainloop = g_main_loop_new (NULL, FALSE);
  
  DurationPipeline dpipeline = newPipeline(timeline);

  if (name != NULL) {
    renderPipeline(dpipeline.pipeline, prof, name);
  } else {
    ges_pipeline_set_mode (dpipeline.pipeline, TIMELINE_MODE_PREVIEW_VIDEO);
    g_timeout_add_seconds (dpipeline.duration, (GSourceFunc) g_main_loop_quit, mainloop);
  }

  GstBus *bus;
  bus = gst_pipeline_get_bus(dpipeline.pipeline);
  g_signal_connect(bus, "message", busMessageCb, mainloop);
  g_timeout_add (100, (GSourceFunc) durationQuerier, &dpipeline);
  gst_bus_add_signal_watch (bus);

  gst_element_set_state (dpipeline.pipeline, GST_STATE_PLAYING);
  
  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);
}


GESTimeline * transitionTL () {
  GESTimeline *timeline;
  GESLayer *layer;
  GESClip *srca, *srcb;
  
  timeline = ges_timeline_new_audio_video();

  layer = ges_layer_new();
  g_object_set (layer, "auto-transition", TRUE, NULL);
  
  ges_timeline_add_layer (timeline, layer);
  
  srca = ges_test_clip_new();
  srcb = ges_test_clip_new();
  
  g_object_set (srca, 
    "vpattern", GES_VIDEO_TEST_PATTERN_SMPTE, 
    "duration", 5 * GST_SECOND, 
    NULL);

  g_object_set (srcb, 
    "vpattern", GES_VIDEO_TEST_PATTERN_CIRCULAR,
    "duration", 5 * GST_SECOND,
    "start", 2 * GST_SECOND,
  NULL);
  
  ges_layer_add_clip(layer, srca);
  ges_layer_add_clip(layer, srcb);
  
  /*
  tr = ges_transition_clip_new_for_nick ("crossfade");
  g_object_set (tr,
        "start", tdur * GST_SECOND,
        "duration", tst * GST_SECOND, 
        "in-point", 0, 
        NULL);
  ges_layer_add_clip (layer1, GES_CLIP (tr));
  */
  ges_timeline_commit(timeline);

  return timeline;
}

GESTimeline * effectTL() {
  GESTimeline *timeline;
  GError **error = NULL;
  GESLayer *layer;
  GESAsset *asset1,*asset2;
  GESClip * clip1, *clip2;
  GESEffect * effect1, *effect2;

  timeline = ges_timeline_new_audio_video();
  layer = ges_layer_new();

  ges_timeline_add_layer (timeline, layer);
  
  clip1 = placeAsset(layer, path("sd/trailer_400p.ogg"),  
    0, 0, 10);
  clip2 = placeAsset(layer, path("sd/sintel_trailer-480p.ogv"), 
    10, 5, 10);

  effect1 = ges_effect_new("agingtv");
  ges_container_add(clip1, effect1);
  
  effect2 = ges_effect_new("rippletv");
  ges_container_add(clip2, effect2);
  
  ges_timeline_commit(timeline);

  return timeline;
}


GESTimeline * testTL() {
  GESTimeline *timeline;
  GESLayer *layer;

  timeline = ges_timeline_new_audio_video();
  layer = ges_layer_new();

  ges_timeline_add_layer (timeline, layer);
  
  GESClip * src = ges_test_clip_new();
  
  g_object_set (src, 
    "vpattern", GES_VIDEO_TEST_PATTERN_SMPTE,
    "duration", 5 * GST_SECOND,
  NULL);
  
  ges_layer_add_clip(layer, src);
  
  ges_timeline_commit(timeline);

  return timeline;
}


GESTimeline * minuteTL() {
  GESTimeline *timeline;
  GESLayer *layer;

  timeline = ges_timeline_new_audio_video();
  layer = ges_layer_new();

  ges_timeline_add_layer (timeline, layer);
  
  placeAsset(layer, path("sd/Black Ink and Water Test - A Place in Time Song.mp4"),  
    0, 0, 15);
  placeAsset(layer, path("sd/trailer_400p.ogg"), 
    15, 2, 15);
  placeAsset(layer, path("sd/sintel_trailer-480p.mp4"), 
    30, 4, 15);
  placeAsset(layer, path("sd/Sesame Street- Kermit and Joey Say the Alphabet.mp4"), 
    45, 0, 15);
  
  ges_timeline_commit(timeline);

  return timeline;
}

GESTimeline * imageTL() {
  GESTimeline *timeline;
  GESLayer *layer;

  timeline = ges_timeline_new_audio_video();
  layer = ges_layer_new();
  //g_object_set (layer, "auto-transition", TRUE, NULL);

  ges_timeline_add_layer (timeline, layer);
  
  placeAsset(layer, path("images/LAMP_720_576.jpg"), 
    0, 0, 2);
  placeAsset(layer, path("images/Fish.png"),  
    2, 0, 2);
  placeAsset(layer, path("images/PNG_transparency_demonstration_1.png"), 
    4, 0, 2);
  placeAsset(layer, path("images/wallpaper-1946968.jpg"), 
    6, 0, 2);

  ges_timeline_commit(timeline);

  return timeline;
}

GESTimeline * oneTL() {
  GESTimeline *timeline;
  GESLayer *layer;

  timeline = ges_timeline_new_audio_video();
  layer = ges_layer_new();

  ges_timeline_add_layer (timeline, layer);
  
  placeAsset(layer, path("sd/trailer_400p.ogg"), 0, 0, 5);
  
  ges_timeline_commit(timeline);

  return timeline;
}


void main() {
  GESTimeline *timeline;

  gst_init (NULL, NULL);
  ges_init ();
  
  char * dir = get_current_dir_name();
  g_print("dir: %s\n", dir);

  render(testTL(), "test", PROFILE_VORBIS_VP8_WEBM);

  /*
  render(testTL(), "test", PROFILE_VORBIS_THEORA_OGG);
  render(testTL(), "test", PROFILE_AAC_H264_QUICKTIME);
  render(testTL(), "test", PROFILE_VORBIS_H264_MATROSKA);
  
  render(effectTL(), "effect", PROFILE_AAC_H264_QUICKTIME);
  
  
  render(minuteTL(), "1-minute-sd", PROFILE_VORBIS_VP8_WEBM);
  render(minuteTL(), "1-minute-sd", PROFILE_VORBIS_THEORA_OGG);
  render(minuteTL(), "1-minute-sd", PROFILE_AAC_H264_QUICKTIME);
  
  render(effectTL(), "transition", PROFILE_AAC_H264_QUICKTIME);
  */
  
  /*
  play(imageTL());
  play(testTL());
  play(transitionTL());
  play(effectTL());
  play(minuteTL());
  */
}

