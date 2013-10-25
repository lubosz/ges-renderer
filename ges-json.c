
#ifdef PLATTFORM_WINDOWS
#include <windows.h>
#endif

#include <stdlib.h>

#include "ges-json.h"

#include <ges/ges.h>

/*
int
main (int argc, char **argv)
{
    LoadLibrary("exchndl.dll");


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
*/

const gchar * getString(JsonReader *reader, const gchar *member_name) {
    json_reader_read_member (reader, member_name);
    const char *value = json_reader_get_string_value (reader);
    json_reader_end_member (reader);
    return value;
}

const int getInt(JsonReader *reader, const gchar *member_name) {
    json_reader_read_member (reader, member_name);
    int value = json_reader_get_int_value (reader);
    json_reader_end_member (reader);
    return value;
}

const gboolean getBool(JsonReader *reader, const gchar *member_name) {
    json_reader_read_member (reader, member_name);
    gboolean value = json_reader_get_boolean_value (reader);
    json_reader_end_member (reader);
    return value;
}

void getAssets(JsonReader *reader, const gchar *member_name) {
    int i;
    g_print("= %s clips =\n", member_name);
    json_reader_read_member (reader, member_name);
    for(i = 0; i < json_reader_count_elements(reader); i++) {
        json_reader_read_element (reader, i);
        const char *src = getString(reader, "src");
        int start = getInt(reader, "start");
        int in = getInt(reader, "in");
        int dur = getInt(reader, "dur");
        g_print("Clip: %s (start: %d, in: %d, dur: %d)\n", src, start, in, dur);
        json_reader_end_element (reader);
    }
    json_reader_end_member (reader);
}

int main (int argc, char *argv[]) {
#ifdef PLATTFORM_WINDOWS
  LoadLibrary("exchndl.dll");
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

  // manipulate the object tree and then exit

  JsonReader *reader = json_reader_new (root);

  json_reader_read_member (reader, "composition");

  // comp strings
  const char *name = getString(reader, "name");
  gboolean autotransition = getBool(reader, "autotransition");
  const char *src_dir = getString(reader, "src-dir");

  g_print("Source Directory: %s\nName: %s\n", src_dir, name);

  if (autotransition)
      g_print("Auto Transitions on.\n");

  // comp ints
  int width = getInt(reader, "width");
  int height = getInt(reader, "height");
  int fps = getInt(reader, "fps");

  g_print("Resolution: %dx%d, FPS: %d\n", width, height, fps);

  // formats
  json_reader_read_member (reader, "formats");
  int i;
  for(i = 0; i < json_reader_count_elements(reader); i++) {
      json_reader_read_element (reader, i);
      const char *format = json_reader_get_string_value (reader);
      json_reader_end_element (reader);
      g_print("format: %s\n", format);
  }
  json_reader_end_member (reader);

  // videos
  getAssets(reader, "video");
  getAssets(reader, "music");
  getAssets(reader, "sound");
  json_reader_end_member (reader);


  g_object_unref (reader);
  g_object_unref (parser);

  return EXIT_SUCCESS;
}
