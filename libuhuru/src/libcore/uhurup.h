#ifndef _LIBUHURU_UHURUP_H_
#define _LIBUHURU_UHURUP_H_

#include <glib.h>

#include <libuhuru/core.h>

struct uhuru_module **uhuru_get_modules(struct uhuru *u);

struct uhuru_module *uhuru_get_module_by_name(struct uhuru *u, const char *name);

void uhuru_add_mime_type(struct uhuru *u, const char *mime_type, struct uhuru_module *module);

/* NULL terminated array of struct uhuru_module */
struct uhuru_module **uhuru_get_applicable_modules(struct uhuru *u, const char *mime_type);

#ifdef DEBUG
const char *uhuru_debug(struct uhuru *u);
#endif

#endif
