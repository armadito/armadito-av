#ifndef _LIBUHURU_MODULEP_H_
#define _LIBUHURU_MODULEP_H_

#include <libuhuru/module.h>

#include <stdio.h>
#include <glib.h>

struct module_manager;

struct module_manager *module_manager_new(void);

void module_manager_add(struct module_manager *mm, struct uhuru_module *module);

void module_manager_load_path(struct module_manager *mm, const char *path);

struct uhuru_module **module_manager_get_modules(struct module_manager *mm);

#endif
