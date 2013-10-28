/*

  Author: Lubosz Sarnecki
  2013

*/

#ifndef GES_RENDERER_H
#define GES_RENDERER_H

#include <unistd.h>
#include <stdio.h>
#include <gst/gst.h>
#include <ges/ges.h>
#include <gst/pbutils/encoding-profile.h>
#include <gst/pbutils/encoding-target.h>

static const char *const profiles[][4] = {
  {"application/ogg", "audio/x-vorbis", "video/x-theora", "ogv"},
  {"video/webm", "audio/x-vorbis", "video/x-vp8", "webm"},
  {"video/quicktime,variant=iso", "audio/mpeg,mpegversion=1,layer=3", "video/x-h264", "mp4"},
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

void setPath(gchar * path);

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

void printTarget(GstEncodingTarget *target);
void listProfiles(void);

void init_path(void);

GstCaps * makeCaps(VideoSize *size);

GESTimeline * palTimeline(void);
GESTimeline * newTimeline(VideoSize * size);

#endif // GES_RENDERER_H
