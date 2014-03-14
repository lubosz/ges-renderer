#include <stdlib.h>
#include <gst/gst.h>
#include <ges/ges.h>

GESTimeline *pngTL (void);
void message_cb (GstBus * bus, GstMessage * message, GMainLoop * mainloop);

GESPipeline *pipeline;

void
message_cb (GstBus * bus, GstMessage * message, GMainLoop * mainloop)
{
  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:{
      GError *err = NULL;
      gchar *dbg_info = NULL;

      gst_message_parse_error (message, &err, &dbg_info);
      g_printerr ("\n\nERROR from element %s: %s\n",
          GST_OBJECT_NAME (message->src), err->message);
      g_printerr ("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
      g_error_free (err);
      g_free (dbg_info);

      GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (pipeline),
          GST_DEBUG_GRAPH_SHOW_ALL, "png-bug-error");

      g_main_loop_quit (mainloop);
      break;
    }
    case GST_MESSAGE_EOS:{
      GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (pipeline),
          GST_DEBUG_GRAPH_SHOW_ALL, "png-bug-eos");
      g_print ("\nDone\n");
      g_main_loop_quit (mainloop);
      break;
    }
    default:
      break;
  }
}

GESTimeline *
pngTL (void)
{
  GESTimeline *timeline;
  GError **error = NULL;
  GESUriClipAsset *asset;
  const gchar *path =
      "file:///home/bmonkey/workspace/ges/ges-renderer/data/image/test1.png";
  GESTrack *trackv;

  timeline = ges_timeline_new ();

  trackv = GES_TRACK (ges_video_track_new ());

  const gchar *capsstring = "video/x-raw,width=720,height=576,framerate=25/1";
  GstCaps *caps = gst_caps_from_string (capsstring);
  ges_track_set_restriction_caps (trackv, caps);

  ges_timeline_add_track (timeline, trackv);

  GESLayer *layer = ges_layer_new ();

  ges_timeline_add_layer (timeline, layer);

  asset = ges_uri_clip_asset_request_sync (path, error);

  ges_layer_add_asset (layer, GES_ASSET (asset),
      0, 0, GST_SECOND, GES_TRACK_TYPE_VIDEO);

  ges_timeline_commit (timeline);

  return timeline;
}

int
main (int argc, char **argv)
{
  GstClockTime duration;

  gst_init (&argc, &argv);
  ges_init ();

  GMainLoop *mainloop;
  mainloop = g_main_loop_new (NULL, FALSE);

  GESTimeline *timeline = pngTL ();

  pipeline = ges_pipeline_new ();
  ges_pipeline_set_timeline (pipeline, timeline);

  duration = ges_timeline_get_duration (timeline);

  ges_pipeline_set_mode (pipeline, GES_PIPELINE_MODE_PREVIEW_VIDEO);
  g_timeout_add_seconds (duration, (GSourceFunc) g_main_loop_quit, mainloop);

  GstBus *bus;
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  g_signal_connect (bus, "message", (GCallback) message_cb, mainloop);
  gst_bus_add_signal_watch (bus);
  gst_object_unref (bus);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (pipeline),
      GST_DEBUG_GRAPH_SHOW_ALL, "png-bug-preloop");

  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);

  gst_object_unref (pipeline);

  return 0;
}
