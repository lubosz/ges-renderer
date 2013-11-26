/*

  Author: Lubosz Sarnecki
  2013

*/

#include <stdlib.h>

#ifdef PLATTFORM_WINDOWS
#include <windows.h>
#endif

#include "ges-renderer.h"

GESTimeline *testTL (void);

GESTimeline *
testTL (void)
{
  GESTimeline *timeline;
  GESLayer *layer;

  timeline = ges_timeline_new_video ();
  layer = ges_layer_new ();

  ges_timeline_add_layer (timeline, layer);

  GESClip *src = GES_CLIP (ges_test_clip_new ());

  g_object_set (src,
      "vpattern", GES_VIDEO_TEST_PATTERN_SMPTE,
      "duration", 5 * GST_SECOND, NULL);

  ges_layer_add_clip (layer, src);

  ges_timeline_commit (timeline);

  return timeline;
}

int
main (int argc, char **argv)
{
#ifdef PLATTFORM_WINDOWS
  LoadLibrary ("exchndl.dll");
#endif
  gst_init (&argc, &argv);
  ges_init ();

  ges_renderer_init ();

  GESRendererProfile webm = { 720, 576, 25, PROFILE_VORBIS_VP8_WEBM };
  GESRendererProfile ogg = { 720, 576, 25, PROFILE_VORBIS_THEORA_OGG };
  GESRendererProfile qt = { 720, 576, 25, PROFILE_AAC_H264_QUICKTIME };
  GESRendererProfile mkv = { 720, 576, 25, PROFILE_VORBIS_H264_MATROSKA };

  ges_renderer_render (testTL (), "formats", &webm);
  ges_renderer_render (testTL (), "formats", &ogg);
  ges_renderer_render (testTL (), "formats", &qt);
  ges_renderer_render (testTL (), "formats", &mkv);

  return 0;
}
