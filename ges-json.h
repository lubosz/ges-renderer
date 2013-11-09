#ifndef GESJSON_H
#define GESJSON_H

#include <glib-object.h>
#include <json-glib/json-glib.h>
#include <ges/ges.h>

const gchar *getString (JsonReader * reader, const gchar * member_name);
const int getInt (JsonReader * reader, const gchar * member_name);
const gboolean getBool (JsonReader * reader, const gchar * member_name);
void getAssets (JsonReader * reader, const gchar * member_name,
    GESTimeline * timeline, GESTrackType type, gboolean transitions);
void render_json (JsonNode * root);
gboolean is_in_members (JsonReader * reader, const char *member);

#endif // GESJSON_H
