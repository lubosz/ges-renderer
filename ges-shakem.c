#include <gst/gst.h>
#include <ges/ges.h>

gchar *videoFile1 = "file:///home/bmonkey/workspace/ges/data/trailer_400p.ogg";

GESTimeline * simpleTimeline() {
  GESTimeline *timeline;
  GError **error = NULL;
  GESLayer *layer;
  GESAsset * asset1;
  GESClip * clip1;
  GESEffect * effect1;

  timeline = ges_timeline_new_audio_video();
  layer = ges_layer_new();

  ges_timeline_add_layer (timeline, layer);
  asset1 = ges_uri_clip_asset_request_sync(videoFile1, error);
  
  clip1 = ges_layer_add_asset(layer, asset1, 1 * GST_SECOND, 1 * GST_SECOND, 9 * GST_SECOND, GES_TRACK_TYPE_UNKNOWN);
  
  effect1 = ges_effect_new("agingtv");
  ges_container_add(clip1, effect1);
  
  ges_timeline_commit(timeline);

  return timeline;
}

void main() {
  GMainLoop *mainloop;
  GESPipeline *pipeline;
  GESTimeline *timeline;

  gst_init (NULL, NULL);
  ges_init ();

  timeline = simpleTimeline();
  pipeline = ges_pipeline_new();
  ges_pipeline_add_timeline (pipeline, timeline);
  
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  
  mainloop = g_main_loop_new (NULL, FALSE);
  g_timeout_add_seconds (3, (GSourceFunc) g_main_loop_quit, mainloop);

  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);
}
