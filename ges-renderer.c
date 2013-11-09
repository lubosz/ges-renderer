/*

  Author: Lubosz Sarnecki
  2013

*/

#include "ges-renderer.h"

#include <stdlib.h>

static gchar *data_path;
static GESPipeline *pipeline = NULL;
static GstClockTime duration;
static gboolean wasError = FALSE;

char *replace(char *s, char old, char replacement) {
    char *p = s;

    while(*p) {
        if(*p == old)
            *p = replacement;
        ++p;
    }
    return s;
}

void ges_renderer_init(void) {
    duration = 0;
    ges_renderer_init_path();
}

void ges_renderer_init_path(void) {
    duration = 0;
    char directory[1024];
    getcwd (directory, 1024);
  #ifdef PLATTFORM_WINDOWS
    char * replaced = replace (directory, '\\', '/');
    data_path = g_strconcat ("file:///", replaced, "/data/", NULL);
  #else
    data_path = g_strconcat ("file://", &directory, "/data/", NULL);
  #endif
}

char *
ges_renderer_get_absolute_path (const char *rel_path)
{
  return g_strconcat (data_path, rel_path, NULL);
}



gint ges_asset_get_structure_int(GESUriClipAsset * asset, const char * name) {
    GstDiscovererInfo * info = ges_uri_clip_asset_get_info(asset);
    GstDiscovererStreamInfo* stream_info = gst_discoverer_info_get_stream_info(info);
    GstCaps* caps = gst_discoverer_stream_info_get_caps(stream_info);
    GstStructure * structure = gst_caps_get_structure(caps, 0);
    gint value;
    gst_structure_get_int (structure, name, &value);
    return value;
}

gint ges_asset_get_width(GESUriClipAsset * asset) {
    return ges_asset_get_structure_int(asset, "width");
}

gint ges_asset_get_height(GESUriClipAsset *asset) {
    return ges_asset_get_structure_int(asset, "height");
}

GESClip *
ges_clip_unknown_from_rel_path (const gchar * path, GESLayer * layer, gint start, gint in, gint dur)
{
  return ges_clip_from_rel_path (path, layer, start, in, dur, GES_TRACK_TYPE_UNKNOWN);
}

GESClip *
ges_clip_from_rel_path (const gchar * rel_path, GESLayer * layer, gint start, gint in, gint dur,
    GESTrackType tt)
{
  return ges_clip_from_path (ges_renderer_get_absolute_path(rel_path), layer, start, in, dur, tt);
}

GESClip *
ges_clip_from_path (const gchar * path, GESLayer * layer, gint start, gint in, gint dur,
    GESTrackType tt)
{
  GError **error = NULL;
  GESUriClipAsset * asset;
  GESClip * clip;

  asset = ges_uri_clip_asset_request_sync (path, error);

  guint width = ges_asset_get_width(asset);
  guint height = ges_asset_get_height(asset);

  g_print("Size %dx%d\n", width, height);

  clip = ges_layer_add_asset (layer, GES_ASSET(asset),
      start * GST_SECOND, in * GST_SECOND, dur * GST_SECOND, tt);

  gst_object_unref(asset);

  return clip;
}

void
bus_message_cb (GstBus * bus, GstMessage * message, GMainLoop * mainloop)
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

GstCaps * gst_caps_from_videosize(VideoSize * size) {
    GstCaps *caps;
    char capsstring[50];
    sprintf (capsstring,
        "video/x-raw,width=%d,height=%d,framerate=%d/1,format=I420",
        size->width, size->height, size->fps);

    caps = gst_caps_from_string (capsstring);

    return caps;
}

GstEncodingProfile *
gst_encoding_profile_from_type (EncodingProfile type, VideoSize * size)
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

  settings = gst_caps_from_videosize(size);

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
ges_renderer_print_progress (void)
{
  gint64 position = 0;

  gst_element_query_position (GST_ELEMENT (pipeline),
      GST_FORMAT_TIME, &position);

  float percent = (float) position * 100 / (float) duration;

  //TODO: error when pos > dur

  float positionSec = (float) position / GST_SECOND;
  float durationSec = (float) duration / GST_SECOND;

  if (position > 0)
    g_print ("\r%.2f%% %.2f/%.2fs", percent, positionSec, durationSec);

  return TRUE;
}

void
ges_pipeline_setup_rendering (GESPipeline * pipeline, EncodingProfile prof,
    const gchar * name, VideoSize * size)
{
  gchar *fileName =
      g_strconcat (data_path, "export/", name, ".", profiles[prof][3], NULL);
  g_print ("Rendering %s\n", fileName);

  GstEncodingProfile *profile = gst_encoding_profile_from_type (prof, size);
  ges_pipeline_set_render_settings (pipeline, fileName, profile);
  ges_pipeline_set_mode (pipeline, TIMELINE_MODE_RENDER);
}

GESPipeline *
ges_pipeline_from_timeline (GESTimeline * timeline)
{
  GESPipeline *pipeline;
  pipeline = ges_pipeline_new ();
  ges_pipeline_add_timeline (pipeline, timeline);

  duration = ges_timeline_get_duration (timeline);

  return pipeline;
}

void
ges_renderer_run_job (GESTimeline * timeline, const gchar * name, EncodingProfile prof,
    VideoSize * size)
{
  GMainLoop *mainloop;
  mainloop = g_main_loop_new (NULL, FALSE);

  pipeline = ges_pipeline_from_timeline (timeline);

  if (name != NULL) {
    ges_pipeline_setup_rendering (pipeline, prof, name, size);
  } else {
    ges_pipeline_set_mode (pipeline, TIMELINE_MODE_PREVIEW_VIDEO);
    g_timeout_add_seconds (duration, (GSourceFunc) g_main_loop_quit, mainloop);
  }

  GstBus *bus;
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  g_signal_connect (bus, "message", (GCallback) bus_message_cb, mainloop);
  g_timeout_add (100, (GSourceFunc) ges_renderer_print_progress, NULL);
  gst_bus_add_signal_watch (bus);
  gst_object_unref(bus);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);

  gst_object_unref(pipeline);
}

void
ges_renderer_play (GESTimeline * timeline)
{
  gchar *name = NULL;
  ges_renderer_run_job (timeline, name, PROFILE_NONE, NULL);
}

void
ges_renderer_render_pal (GESTimeline * timeline, const gchar * name, EncodingProfile prof)
{
  VideoSize pal = { 720, 576, 25 };
  ges_renderer_render (timeline, name, prof, &pal);
}

void
ges_renderer_render (GESTimeline * timeline, const gchar * name,
    EncodingProfile prof, VideoSize * size)
{
  g_print ("\n====\n");
  float now = (float) g_get_monotonic_time () / (float) GST_MSECOND;

  ges_renderer_run_job (timeline, name, prof, size);

  float then = (float) g_get_monotonic_time () / (float) GST_MSECOND;
  float dur = then - now;
  g_print ("\n====\n");

  const gchar * exitStatus = "Rendering";
  if (wasError)
      exitStatus = "Error";

  g_print ("%s took %.2fs\n", exitStatus, dur);
  g_print ("====\n");
}

GESTimeline * ges_timeline_new_pal(void) {
    GESTimeline *timeline;
    VideoSize pal = { 720, 576, 25 };
    timeline = ges_timeline_audio_video_from_videosize(&pal);
    return timeline;
}

GESTimeline * ges_timeline_audio_video_from_videosize(VideoSize * size) {
    GESTimeline *timeline;
    GESTrack *tracka, *trackv;
    timeline = ges_timeline_new ();

    tracka = GES_TRACK (ges_audio_track_new ());
    trackv = GES_TRACK (ges_video_track_new ());

    if (!ges_timeline_add_track (timeline, trackv) ||
        !ges_timeline_add_track (timeline, tracka)) {
      gst_object_unref (timeline);
      timeline = NULL;
    }

    GstCaps * caps = gst_caps_from_videosize(size);

    ges_track_set_restriction_caps(trackv, caps);

    return timeline;
}
