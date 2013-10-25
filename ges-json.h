#ifndef GESJSON_H
#define GESJSON_H

#include <glib-object.h>
#include <json-glib/json-glib.h>

const gchar * getString(JsonReader *reader, const gchar *member_name);
const int getInt(JsonReader *reader, const gchar *member_name);
const gboolean getBool(JsonReader *reader, const gchar *member_name);
void getAssets(JsonReader *reader, const gchar *member_name);

#endif // GESJSON_H
