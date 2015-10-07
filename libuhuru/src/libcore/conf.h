#ifndef _LIBUHURU_CONF_H_
#define _LIBUHURU_CONF_H_

#include <libuhuru/module.h>
#include <libuhuru/scan.h>

int conf_set_value(struct uhuru *uhuru, const char *mod_name, const char *key, const char **argv);

const char **conf_get_value(struct uhuru *uhuru, const char *mod_name, const char *key);

void conf_load_file(struct uhuru *uhuru, const char *filename);

void conf_load_path(struct uhuru *uhuru, const char *path);

#endif
