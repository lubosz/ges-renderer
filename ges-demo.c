#include <gst/gst.h>
#include <ges/ges.h>

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

GESTimeline * transitionTL () {
  GESTimeline *timeline;
  GESLayer *layer1;
  GESClip *srca, *srcb;
  guint64 aduration, bduration, tduration, tstart, ainpoint, binpoint;
  GESTransitionClip *tr = NULL;

  gchar * patha = path("sd/Black Ink and Water Test - A Place in Time Song.mp4");
  gchar * pathb = path("sd/Sesame Street- Kermit and Joey Say the Alphabet.mp4");
  
  gchar *nick = "crossfade";
  gdouble adur, bdur, tdur, ainp, binp;
  
  adur = 10;
  bdur = 10;
  ainp = 10;
  binp = 10;
  tdur = 5;

  timeline = ges_timeline_new_audio_video();

  layer1 = GES_LAYER (ges_layer_new ());
  g_object_set (layer1, "priority", (gint32) 0, NULL);

  if (!ges_timeline_add_layer (timeline, layer1))
    exit (-1);

  aduration = (guint64) (adur * GST_SECOND);
  bduration = (guint64) (bdur * GST_SECOND);
  tduration = (guint64) (tdur * GST_SECOND);
  ainpoint = (guint64) (ainp * GST_SECOND);
  binpoint = (guint64) (binp * GST_SECOND);
  tstart = aduration - tduration;
  
  srca = placeAsset(layer1, patha, 0, ainp, adur);
  srcb = placeAsset(layer1, pathb, tstart / GST_SECOND, binp, bdur);
  
  g_object_set (srca, "priority", (guint32) 1,  NULL);
  g_object_set (srcb, "priority", (guint32) 2,  NULL);
  
  if (tduration != 0) {
    if (!(tr = ges_transition_clip_new_for_nick (nick)))
      g_error ("invalid transition type %s\n", nick);

    g_object_set (tr,
        "start", (guint64) tstart,
        "duration", (guint64) tduration, "in-point", (guint64) 0, NULL);
    ges_layer_add_clip (layer1, GES_CLIP (tr));
  }
  
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
  clip2 = placeAsset(layer, path("sd/Sintel_Trailer1.480p.DivX_Plus_HD.mkv"), 
    10, 5, 10);
  
  effect1 = ges_effect_new("agingtv");
  ges_container_add(clip1, effect1);
  
  effect2 = ges_effect_new("rippletv");
  ges_container_add(clip2, effect2);
  
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
    0, 0, 5);
  placeAsset(layer, path("sd/trailer_400p.ogg"), 
    5, 2, 15);
  placeAsset(layer, path("sd/sintel_trailer-480p.mp4"), 
    20, 4, 10);
  placeAsset(layer, path("sd/Sesame Street- Kermit and Joey Say the Alphabet.mp4"), 
    30, 0, 30);
  
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

GstEncodingProfile* encoderProfile(EncodingProfile type) {
  GstEncodingContainerProfile *prof;
  GstCaps *caps;
  GstCaps *settings;
  
  g_print("writing %s file: %s: (%s + %s) \n", 
    profiles[type][3], 
    profiles[type][0], 
    profiles[type][2], 
    profiles[type][1]);

  caps = gst_caps_from_string(profiles[type][0]);
  prof = gst_encoding_container_profile_new("Profile", "A web video profile", caps, NULL);
  gst_caps_unref (caps);

  caps = gst_caps_from_string(profiles[type][2]);
  settings = gst_caps_from_string("video/x-raw,width=720,height=576,framerate=25/1");
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

void renderPipeline(GESPipeline *pipeline, EncodingProfile prof) {
    gchar * fileName = g_strconcat(dataPath, "export/render.", profiles[prof][3], NULL);
    g_print("filename %s \n", fileName);
     
    GstEncodingProfile* profile = encoderProfile(prof);
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
  runJob(timeline, NULL, FALSE);
}

void render(GESTimeline *timeline, EncodingProfile prof) {
  runJob(timeline, prof, TRUE);
}

void runJob(GESTimeline *timeline, EncodingProfile prof, gboolean render) {
  GMainLoop *mainloop;
  mainloop = g_main_loop_new (NULL, FALSE);
  
  DurationPipeline dpipeline = newPipeline(timeline);

  if (render) {
    renderPipeline(dpipeline.pipeline, prof);
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

void main() {
  GESTimeline *timeline;

  gst_init (NULL, NULL);
  ges_init ();
  
  //timeline = transitionTL();
  //timeline = minuteTL();
  //timeline = effectTL();
  
  //render(timeline, PROFILE_VORBIS_VP8_WEBM);
  //render(timeline, PROFILE_VORBIS_H264_MATROSKA);
  //render(timeline, PROFILE_AAC_H264_QUICKTIME);
  render(transitionTL(), PROFILE_AAC_H264_QUICKTIME);

}

