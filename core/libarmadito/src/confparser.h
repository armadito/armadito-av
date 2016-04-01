#ifndef _LIBARMADITO_CONFPARSER_H_
#define _LIBARMADITO_CONFPARSER_H_

#include <stddef.h>

struct a6o_conf_parser;

enum conf_parser_value_type {
	CP_VALUE_NONE    = 0,
	CP_VALUE_INT     = 1,
	CP_VALUE_STRING  = 2,
	CP_VALUE_LIST    = 3,
};

typedef int (*conf_parser_callback_t)(const char *section, const char *key, struct a6o_conf_value *value, void *user_data);

struct a6o_conf_parser *a6o_conf_parser_new(const char *filename, conf_parser_callback_t callback, void *user_data);

int a6o_conf_parser_parse(struct a6o_conf_parser *cp);

void a6o_conf_parser_free(struct a6o_conf_parser *cp);

#endif

