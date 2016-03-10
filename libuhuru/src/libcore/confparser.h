#ifndef _LIBUHURU_CONFPARSER_H_
#define _LIBUHURU_CONFPARSER_H_

struct uhuru_conf_parser;

typedef void (*conf_parser_callback_t)(const char *section, const char *key, const char **argv, size_t length, void *user_data);

struct uhuru_conf_parser *uhuru_conf_parser_new(const char *filename, conf_parser_callback_t callback, void *user_data);

int uhuru_conf_parser_parse(struct uhuru_conf_parser *cp);

void uhuru_conf_parser_free(struct uhuru_conf_parser *cp);

#endif
