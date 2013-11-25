
#ifdef PLATTFORM_WINDOWS
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ges-json.h"
#include "ges-renderer.h"

const gchar *
getString (JsonReader * reader, const gchar * member_name)
{
  json_reader_read_member (reader, member_name);
  const char *value = json_reader_get_string_value (reader);
  json_reader_end_member (reader);
  return value;
}

const int
getInt (JsonReader * reader, const gchar * member_name)
{
  json_reader_read_member (reader, member_name);
  int value = json_reader_get_int_value (reader);
  json_reader_end_member (reader);
  return value;
}

const gboolean
getBool (JsonReader * reader, const gchar * member_name)
{
  json_reader_read_member (reader, member_name);
  gboolean value = json_reader_get_boolean_value (reader);
  json_reader_end_member (reader);
  return value;
}

void
getAssets (JsonReader * reader, const gchar * member_name,
    GESTimeline * timeline, GESTrackType type, gboolean transitions)
{

  if (!is_in_members (reader, member_name))
    return;

  GESLayer *layer = ges_layer_new ();

  if (transitions)
    g_object_set (layer, "auto-transition", TRUE, NULL);

  ges_timeline_add_layer (timeline, layer);

  int i;
  json_reader_read_member (reader, member_name);

  g_print ("= %s clips =\n", member_name);

  for (i = 0; i < json_reader_count_elements (reader); i++) {
    json_reader_read_element (reader, i);
    const char *src = getString (reader, "src");
    int start = getInt (reader, "start");
    int in = getInt (reader, "in");
    int dur = getInt (reader, "dur");
    g_print ("Clip: %s (start: %d, in: %d, dur: %d)\n", src, start, in, dur);
    GESClip *clip =
        ges_clip_from_path (ges_renderer_get_absolute_path (src), layer, start,
        in, dur, type);

    if (is_in_members (reader, "effect")) {
      const char *effect_str = getString (reader, "effect");
      g_print ("Using effect %s", effect_str);
      GESEffect *effect = ges_effect_new (effect_str);
      ges_container_add (GES_CONTAINER (clip), GES_TIMELINE_ELEMENT (effect));
    }

    json_reader_end_element (reader);
  }
  json_reader_end_member (reader);
}

gboolean
is_in_members (JsonReader * reader, const char *member)
{
  gchar **members = json_reader_list_members (reader);
  int member_size = json_reader_count_members (reader);

  for (int i = 0; i < member_size; i++) {
    if (strcmp (members[i], member) == 0) {
      //g_print("found member: %s = %s\n", members[i], member);
      return TRUE;
    }

  }
  return FALSE;
}

void
render_json (JsonNode * root)
{
  JsonReader *reader = json_reader_new (root);
  GESTimeline *jsonTimeline;

  json_reader_read_member (reader, "composition");

  // comp strings
  const char *name = getString (reader, "name");
  gboolean autotransition = getBool (reader, "autotransition");
  const char *src_dir = getString (reader, "src-dir");

  g_print ("Source Directory: %s\nName: %s\n", src_dir, name);

  if (autotransition)
    g_print ("Auto Transitions on.\n");

  // comp ints
  int width = getInt (reader, "width");
  int height = getInt (reader, "height");
  int fps = getInt (reader, "fps");

  g_print ("Resolution: %dx%d, FPS: %d\n", width, height, fps);

  GESRendererProfile res = { width, height, fps };
  jsonTimeline = ges_timeline_audio_video_from_videosize (&res);

  // videos
  getAssets (reader, "video", jsonTimeline, GES_TRACK_TYPE_UNKNOWN,
      autotransition);
  getAssets (reader, "music", jsonTimeline, GES_TRACK_TYPE_AUDIO,
      autotransition);
  getAssets (reader, "image", jsonTimeline, GES_TRACK_TYPE_VIDEO,
      autotransition);
  getAssets (reader, "voice", jsonTimeline, GES_TRACK_TYPE_AUDIO, FALSE);
  getAssets (reader, "sound", jsonTimeline, GES_TRACK_TYPE_AUDIO, FALSE);

  ges_timeline_commit (jsonTimeline);

  // formats
  json_reader_read_member (reader, "formats");
  int i;
  for (i = 0; i < json_reader_count_elements (reader); i++) {
    json_reader_read_element (reader, i);
    const char *format = json_reader_get_string_value (reader);
    json_reader_end_element (reader);
    g_print ("format: %s\n", format);
    EncodingProfile prof = PROFILE_AAC_H264_QUICKTIME;
    if (strcmp (format, "webm") == 0) {
      prof = PROFILE_VORBIS_VP8_WEBM;
    } else if (strcmp (format, "mkv") == 0) {
      prof = PROFILE_VORBIS_H264_MATROSKA;
    } else if (strcmp (format, "mp4") == 0) {
      prof = PROFILE_AAC_H264_QUICKTIME;
    } else if (strcmp (format, "ogg") == 0) {
      prof = PROFILE_VORBIS_THEORA_OGG;
    }
    res.profile = prof;
    ges_renderer_render (jsonTimeline, name, &res);
  }
  json_reader_end_member (reader);

  json_reader_end_member (reader);

  g_object_unref (reader);
}

int
main (int argc, char *argv[])
{
#ifdef PLATTFORM_WINDOWS
  LoadLibrary ("exchndl.dll");
#endif
  JsonParser *parser;
  JsonNode *root;
  GError *error;

  if (argc < 2) {
    g_print ("Usage: ./ges-json.exe <filename.json>\n");
    return EXIT_FAILURE;
  }

  parser = json_parser_new ();

  error = NULL;
  json_parser_load_from_file (parser, argv[1], &error);
  if (error) {
    g_print ("Unable to parse `%s': %s", argv[1], error->message);
    g_error_free (error);
    g_object_unref (parser);
    return EXIT_FAILURE;
  }

  root = json_parser_get_root (parser);


  gst_init (&argc, &argv);
  ges_init ();

  ges_renderer_init_path ();

  render_json (root);

  g_object_unref (parser);

  return EXIT_SUCCESS;
}
