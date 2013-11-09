#include <ges/ges.h>
#include <stdio.h>

GstCaps * gst_caps_from_videosize(int w, int h, int fps);
GstEncodingProfile * gst_encoding_profile_from_type (GstCaps * settings);
gboolean ges_renderer_print_progress (void);
void bus_message_cb (GstBus * bus, GstMessage * message, GMainLoop * mainloop);

static GstClockTime duration;
GESPipeline *pipeline;

static const char *const profile[4] = {
    "video/quicktime,variant=iso",
    "video/x-h264",
    "audio/mpeg,mpegversion=1,layer=3"
};

GstCaps * gst_caps_from_videosize(int w, int h, int fps) {
    GstCaps *caps;
    char capsstring[50];
    sprintf (capsstring,
        "video/x-raw,width=%d,height=%d,framerate=%d/1",
        w, h, fps);

    caps = gst_caps_from_string (capsstring);

    return caps;
}

GstEncodingProfile *
gst_encoding_profile_from_type (GstCaps *settings)
{
  GstEncodingContainerProfile *prof;
  GstCaps *caps;

  caps = gst_caps_from_string (profile[0]);
  prof =
      gst_encoding_container_profile_new ("Profile", "A web video profile",
      caps, NULL);
  gst_caps_unref (caps);

  caps = gst_caps_from_string (profile[1]);

  gst_encoding_container_profile_add_profile (prof,
      (GstEncodingProfile *) gst_encoding_video_profile_new (caps, NULL,
          settings, 0));
  gst_caps_unref (caps);
  gst_caps_unref (settings);

  caps = gst_caps_from_string (profile[2]);
  settings = gst_caps_from_string ("audio/x-raw");
  gst_encoding_container_profile_add_profile (prof,
      (GstEncodingProfile *) gst_encoding_audio_profile_new (caps, NULL,
          settings, 0));
  gst_caps_unref (caps);
  gst_caps_unref (settings);

  return (GstEncodingProfile *) prof;
}

gboolean
ges_renderer_print_progress (void)
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
  GESTimeline *timeline;
  GESLayer *layer;
  GError **error = NULL;
  GESAsset *asset;
  const gchar * url = "file:///home/bmonkey/workspace/ges/ges-demos/data/sd/sintel_trailer-480p.mp4";
  const gchar *exportURL = "file:///home/bmonkey/workspace/ges/ges-demos/transition.mp4";

  gst_init (&argc, &argv);
  ges_init ();

  timeline = ges_timeline_new_audio_video ();

  layer = ges_layer_new ();

  g_object_set (layer, "auto-transition", TRUE, NULL);

  ges_timeline_add_layer (timeline, layer);

  asset = GES_ASSET (ges_uri_clip_asset_request_sync (url, error));

  ges_layer_add_asset (layer, asset,
      0 * GST_SECOND, 0 * GST_SECOND, 10 * GST_SECOND, GES_TRACK_TYPE_VIDEO);

  ges_layer_add_asset (layer, asset,
      5 * GST_SECOND, 20 * GST_SECOND, 10 * GST_SECOND, GES_TRACK_TYPE_VIDEO);

  ges_timeline_commit (timeline);

  duration = ges_timeline_get_duration (timeline);
  
  pipeline = ges_pipeline_new ();
  ges_pipeline_add_timeline (pipeline, timeline);

  GstCaps* settings = gst_caps_from_videosize(720, 576, 25);
  GstEncodingProfile *profile = gst_encoding_profile_from_type (settings);
  ges_pipeline_set_render_settings (pipeline, exportURL, profile);
  ges_pipeline_set_mode (pipeline, TIMELINE_MODE_RENDER);

  GMainLoop *mainloop;
  mainloop = g_main_loop_new (NULL, FALSE);

  GstBus *bus;
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  g_signal_connect (bus, "message", (GCallback) bus_message_cb, mainloop);
  g_timeout_add (100, (GSourceFunc) ges_renderer_print_progress, NULL);
  gst_bus_add_signal_watch (bus);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);

  return 0;
}
