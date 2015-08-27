#ifndef _LIBUHURU_UHURUP_H_
#define _LIBUHURU_UHURUP_H_

#include <libuhuru/scan.h>
#include "libuhuru-config.h"

#include <glib.h>
#include <magic.h>

struct uhuru_module *uhuru_get_module_by_name(struct uhuru *u, const char *name);

void uhuru_add_mime_type(struct uhuru *u, const char *mime_type, struct uhuru_module *module);

/* NULL terminated array of struct uhuru_module */
struct uhuru_module **uhuru_get_applicable_modules(struct uhuru *u, magic_t magic, const char *path, const char **p_mime_type);

int uhuru_is_remote(struct uhuru *u);

#ifdef DEBUG
const char *uhuru_debug(struct uhuru *u);
#endif

#endif
