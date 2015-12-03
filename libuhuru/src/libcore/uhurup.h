#ifndef _LIBUHURU_UHURUP_H_
#define _LIBUHURU_UHURUP_H_

#include <glib.h>

#include <libuhuru/core.h>

struct uhuru_module **uhuru_get_modules(struct uhuru *u);

struct uhuru_module *uhuru_get_module_by_name(struct uhuru *u, const char *name);

#ifdef DEBUG
const char *uhuru_debug(struct uhuru *u);
#endif

#endif
