#ifndef _LIBUHURU_CONF_H_
#define _LIBUHURU_CONF_H_

#include <libuhuru/module.h>
#include <libuhuru/scan.h>

void conf_load_dir(struct uhuru *uhuru, const char *path);

int conf_set_value(struct uhuru *uhuru, const char *mod_name, const char *key, const char **argv);

const char **conf_get_value(struct uhuru *uhuru, const char *mod_name, const char *key);

#endif
