#ifndef _LIBUHURU_CONF_H_
#define _LIBUHURU_CONF_H_

#include "modulep.h"
#include "uhurup.h"

void conf_load(struct uhuru_module *mod);

void conf_set(struct uhuru *uhuru, const char *mod_name, const char *key, const char *value);

char *conf_get(struct uhuru *uhuru, const char *mod_name, const char *key);

#endif
