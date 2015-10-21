#ifndef _UHURU_JSON_
#define _UHURU_JSON_

#include "linkhash.h"
#include "json_util.h"

void print_json_value(json_object *jobj);
void json_parse_array(json_object *jobj, char *key);
void json_parse(json_object * jobj);
json_object * create_json_obj();

#endif