#ifndef _LIBUHURU_UHURUP_H_
#define _LIBUHURU_UHURUP_H_

#include <libuhuru/scan.h>

#include <glib.h>
#include <magic.h>

GPtrArray *uhuru_get_applicable_modules(struct uhuru *u, magic_t magic, const char *path, char **p_mime_type);

struct uhuru_module *uhuru_get_module_by_name(struct uhuru *u, const char *name);

int uhuru_is_remote(struct uhuru *u);

#endif
