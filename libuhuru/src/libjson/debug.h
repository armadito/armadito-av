#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <json.h>

const char *jobj_str(struct json_object *obj);

void jobj_debug(struct json_object *obj, const char *obj_name);

#endif
