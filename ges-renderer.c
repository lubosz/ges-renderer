/*

  Author: Lubosz Sarnecki
  2013

*/

#include "ges-renderer.h"

#include <stdlib.h>

static gchar *dataPath;
static GESPipeline *pipeline = NULL;
static GstClockTime duration;
static gboolean wasError = FALSE;

void setPath(gchar * path) {
    duration = 0;
    dataPath = path;
    g_print ("data path: %s\n", dataPath);
}

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
      //TODO: Log error message to file
      wasError = TRUE;
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
    g_print ("\r%.2f%% %.2f/%.2fs                                ", percent, positionSec, durationSec);

  return TRUE;
}

void
renderPipeline (GESPipeline * pipeline, EncodingProfile prof,
    const gchar * name, VideoSize * size)
{
    g_print ("data path %s\n", dataPath);
    g_print ("name %s\n", name);

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
  VideoSize pal = { 720, 576, 25 };
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

  const gchar * exitStatus = "Rendering";
  if (wasError)
      exitStatus = "Error";

  g_print ("%s took %.2fs\n", exitStatus, dur);
  g_print ("====\n");
}

void printTarget(GstEncodingTarget *target) {
	const gchar * name = gst_encoding_target_get_name(target);
	const gchar * cat = gst_encoding_target_get_category(target);
	const gchar * desc = gst_encoding_target_get_description(target);
	
	g_print ("target: %s\n%s\n%s\n\n", name, cat, desc);
}

void listProfiles(void) {
	GList *categories, *tmpc;
    GList * targets;

	categories = gst_encoding_list_available_categories();
	g_print("bar\n");

	for (tmpc = categories; tmpc; tmpc = tmpc->next) {
	  g_print("foo\n");
	  gchar *category = (gchar *) tmpc->data;

	  targets = gst_encoding_list_all_targets (category);

	  g_list_foreach (targets, (GFunc) printTarget, NULL);
	  g_list_free (targets);
	}

	g_list_foreach (categories, (GFunc) g_free, NULL);
	g_list_free (categories);

}
