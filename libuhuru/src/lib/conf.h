#ifndef _LIBUHURU_CONF_H_
#define _LIBUHURU_CONF_H_

#include <libuhuru/module.h>
#include <libuhuru/scan.h>

struct uhuru_conf_parser;

struct uhuru_conf_parser *uhuru_conf_parser_new(const char *filename);

int uhuru_conf_parser_parse(struct uhuru_conf_parser *cp);

void uhuru_conf_parser_free(struct uhuru_conf_parser *cp);



void conf_load(struct uhuru_module *mod);

void conf_set(struct uhuru *uhuru, const char *mod_name, const char *key, const char *value);

char *conf_get(struct uhuru *uhuru, const char *mod_name, const char *key);

#endif
