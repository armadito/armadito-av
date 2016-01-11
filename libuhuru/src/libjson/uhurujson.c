#include "libuhuru-config.h"

#include "uhurujson.h"

void foo(struct json_object *obj)
{
  json_object_to_file("/var/tmp/json-debug", obj);
}

