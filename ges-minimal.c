#ifdef PLATTFORM_WINDOWS
#include <windows.h>
#endif
#include <ges/ges.h>

int
main (int argc, char **argv)
{
#ifdef PLATTFORM_WINDOWS
    LoadLibrary("exchndl.dll");
#endif

  GESTimeline *timeline;
  GESLayer *layer;
  GESPipeline *pipeline;

  gst_init (&argc, &argv);
  ges_init ();

  timeline = ges_timeline_new_audio_video ();

  layer = ges_layer_new ();
  ges_timeline_add_layer (timeline, layer);

  GESAsset *src_asset = ges_asset_request (GES_TYPE_TEST_CLIP, NULL, NULL);
  ges_layer_add_asset (layer, src_asset, 0, 0, GST_SECOND, GES_TRACK_TYPE_UNKNOWN);

  ges_timeline_commit (timeline);
  
  pipeline = ges_pipeline_new ();
  ges_pipeline_add_timeline (pipeline, timeline);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  GMainLoop *mainloop;
  mainloop = g_main_loop_new (NULL, FALSE);

  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);

  return 0;
}
