#include "print.h"

static void json_print_object(struct json_object *obj, FILE *out, int level);
static void json_print_array(struct json_object *obj, FILE *out, int level);
static void json_print_value(struct json_object *obj, FILE *out, int level);

static void space(FILE *out, int level)
{
  int i;

  for(i = 0; i < level; i++)
    fprintf(out, " ");
}

static void json_print_object(struct json_object *obj, FILE *out, int level)
{
  int first = 1;

  fprintf(out, "{\n");

  json_object_object_foreach(obj, key, val) {
    if (!first) {
      fprintf(out, ",\n");
    } else
      first = 0;

    space(out, level + 2);

    fprintf(out, "\"%s\": ", key);
    json_print_value(val, out, level + 2);
  }

  fprintf(out, "\n");

  space(out, level);

  fprintf(out, "}");
}

static void json_print_array(struct json_object *obj, FILE *out, int level)
{
  int i;
  struct json_object *val;
  int first = 1;

  fprintf(out, "[\n");

  for(i = 0; i < json_object_array_length(obj); i++) {
    if (!first) {
      fprintf(out, ",\n");
    } else
      first = 0;

    space(out, level + 2);

    val = json_object_array_get_idx(obj, i);
    json_print_value(val, out, level + 2);
  }

  fprintf(out, "\n");

  space(out, level);

  fprintf(out, "]");
}

static void json_print_value(struct json_object *obj, FILE *out, int level)
{
  int i;

  switch(json_object_get_type(obj)) {
  case json_type_null:
    fprintf(out, "(null)");
    break;
  case json_type_boolean:
    fprintf(out, "%s", json_object_get_boolean(obj) ? "true" : "false");
    break;
  case json_type_double:
    fprintf(out, "%f", json_object_get_double(obj));
    break;
  case json_type_int:
    fprintf(out, "%d", json_object_get_int(obj));
    break;
  case json_type_string:
    fprintf(out, "\"%s\"", json_object_get_string(obj));
    break;
  case json_type_object:
    json_print_object(obj, out, level);
    break;
  case json_type_array:
    json_print_array(obj, out, level);
  }
}

void uhuru_json_print(struct json_object *obj, FILE *out)
{
  json_print_value(obj, out, 0);
  fprintf(out, "\n");
}
