/*

  Author: Lubosz Sarnecki
  2013

*/

#ifndef GESDEMO_H
#define GESDEMO_H

#include <unistd.h>
#include <stdio.h>
#include <gst/gst.h>
#include <ges/ges.h>

static const char *const profiles[][4] = {
  {"application/ogg", "audio/x-vorbis", "video/x-theora", "ogv"},
  {"video/webm", "audio/x-vorbis", "video/x-vp8", "webm"},
  {"video/quicktime,variant=iso", "audio/mpeg,mpegversion=1,layer=3",
      "video/x-h264", "mp4"},
  {"video/x-matroska", "audio/x-vorbis", "video/x-h264", "mkv"},
};

typedef enum
{
  PROFILE_NONE = -1,
  PROFILE_VORBIS_THEORA_OGG,
  PROFILE_VORBIS_VP8_WEBM,
  PROFILE_AAC_H264_QUICKTIME,
  PROFILE_VORBIS_H264_MATROSKA
} EncodingProfile;

typedef struct VideoSize VideoSize;
struct VideoSize
{
  gint width;
  gint height;
  gint fps;
};

static gchar *dataPath;

char *path (const char *filenName);

GESClip *placeAsset (GESLayer * layer, gchar * get_path, gint start, gint in,
    gint dur);
GESClip *placeAssetType (GESLayer * layer, gchar * path, gint start, gint in,
    gint dur, GESTrackType tt);

void busMessageCb (GstBus * bus, GstMessage * message, GMainLoop * mainloop);

GstEncodingProfile *encoderProfile (EncodingProfile type, VideoSize * size);

gboolean durationQuerier (void);

void renderPipeline (GESPipeline * pipeline, EncodingProfile prof,
    const gchar * name, VideoSize * size);

GESPipeline *newPipeline (GESTimeline * timeline);
void runJob (GESTimeline * timeline, const gchar * name, EncodingProfile prof,
    VideoSize * size);

void play (GESTimeline * timeline);
void render (GESTimeline * timeline, const gchar * name, EncodingProfile prof);
void renderWithSize (GESTimeline * timeline, const gchar * name,
    EncodingProfile prof, VideoSize * size);

GESTimeline *transitionTL (void);
GESTimeline *effectTL (void);
GESTimeline *testTL (void);
GESTimeline *minuteTL (void);
GESTimeline *imageTL (void);
GESTimeline *oneTL (void);
GESTimeline *musicTL (void);
GESTimeline *hdTL (void);

static GESPipeline *pipeline = NULL;
static GstClockTime duration;

#endif // GESDEMO_H
