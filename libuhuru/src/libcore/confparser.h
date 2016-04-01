#ifndef _LIBUHURU_CONFPARSER_H_
#define _LIBUHURU_CONFPARSER_H_

#include <stddef.h>

struct uhuru_conf_parser;

enum conf_parser_value_type {
	CP_VALUE_NONE    = 0,
	CP_VALUE_INT     = 1,
	CP_VALUE_STRING  = 2,
	CP_VALUE_LIST    = 3,
};

typedef int (*conf_parser_callback_t)(const char *section, const char *key, struct uhuru_conf_value *value, void *user_data);

struct uhuru_conf_parser *uhuru_conf_parser_new(const char *filename, conf_parser_callback_t callback, void *user_data);

int uhuru_conf_parser_parse(struct uhuru_conf_parser *cp);

void uhuru_conf_parser_free(struct uhuru_conf_parser *cp);

#endif

