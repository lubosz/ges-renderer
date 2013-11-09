#ifdef PLATTFORM_WINDOWS
#include <windows.h>
#endif
#include <ges/ges.h>

void bus_message_cb (GstBus * bus, GstMessage * message, GMainLoop * mainloop);

void
bus_message_cb (GstBus * bus, GstMessage * message, GMainLoop * mainloop)
{
  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:{
      GError *err = NULL;
      gchar *dbg_info = NULL;

      gst_message_parse_error (message, &err, &dbg_info);
      g_printerr ("\n\nERROR from element %s: %s\n", GST_OBJECT_NAME (message->src),
          err->message);
      g_printerr ("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
      g_error_free (err);
      g_free (dbg_info);
      g_main_loop_quit (mainloop);
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

int
main (int argc, char **argv)
{
#ifdef PLATTFORM_WINDOWS
    LoadLibrary("exchndl.dll");
#endif

  GESTimeline *timeline;
  GESLayer *layer;
  GESPipeline *pipeline;
  GESTrack *track;
  GMainLoop *mainloop;
  GstBus *bus;
  GESAsset *asset;

  gst_init (&argc, &argv);
  ges_init ();

  timeline = ges_timeline_new ();
  track = GES_TRACK (ges_video_track_new ());
  ges_timeline_add_track (timeline, track);

  layer = ges_layer_new ();
  ges_timeline_add_layer (timeline, layer);

  asset = ges_asset_request (GES_TYPE_TEST_CLIP, NULL, NULL);
  ges_layer_add_asset (layer, asset, 0, 0, GST_SECOND, GES_TRACK_TYPE_VIDEO);
  gst_object_unref (asset);

  ges_timeline_commit (timeline);
  
  pipeline = ges_pipeline_new ();
  ges_pipeline_add_timeline (pipeline, timeline);

  mainloop = g_main_loop_new (NULL, FALSE);

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  g_signal_connect (bus, "message", (GCallback) bus_message_cb, mainloop);
  gst_bus_add_signal_watch (bus);
  gst_object_unref (bus);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  g_main_loop_run (mainloop);
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);

  gst_object_unref (track);
  gst_object_unref (layer);
  gst_object_unref (timeline);
  gst_object_unref (pipeline);
  g_main_loop_unref (mainloop);

  return 0;
}
