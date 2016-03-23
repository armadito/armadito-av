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

/* was: typedef int (*conf_parser_callback_t)(const char *section, const char *key, const char **argv, size_t length, void *user_data); */

/* depending on parser current value type, callback will be called like this: */
/*  - if INT, (*cb)(user_data, section, key, type, int) */
/*  - if STRING, (*cb)(user_data, section, key, type, const char *) */
/*  - if LIST, (*cb)(user_data, section, key, type, const char **, size_t) */

typedef int (*conf_parser_callback_t)(void *user_data, const char *section, const char *key, enum conf_parser_value_type type, ...);

struct uhuru_conf_parser *uhuru_conf_parser_new(const char *filename, conf_parser_callback_t callback, void *user_data);

int uhuru_conf_parser_parse(struct uhuru_conf_parser *cp);

void uhuru_conf_parser_free(struct uhuru_conf_parser *cp);

#endif
