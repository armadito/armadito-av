#ifndef _LIBUMWSU_UMWSUP_H_
#define _LIBUMWSU_UMWSUP_H_

#include <libumwsu/scan.h>

#include <glib.h>
#include <magic.h>

GPtrArray *umwsu_get_applicable_modules(struct umwsu *u, magic_t magic, const char *path, char **p_mime_type);

struct umwsu_module *umwsu_get_module_by_name(struct umwsu *u, const char *name);

#endif
