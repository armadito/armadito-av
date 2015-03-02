#ifndef _LIBUMWSU_CONF_H_
#define _LIBUMWSU_CONF_H_

#include "modulep.h"
#include "umwsup.h"

void conf_load(struct umwsu_module *mod);

void conf_set(struct umwsu *umwsu, const char *mod_name, const char *key, const char *value);

char *conf_get(struct umwsu *umwsu, const char *mod_name, const char *key);

#endif
