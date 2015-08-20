#ifndef _LIBUHURU_MODULEP_H_
#define _LIBUHURU_MODULEP_H_

#include <libuhuru/module.h>

#include <stdio.h>
#include <glib.h>

struct uhuru_module_manager {
  GPtrArray *modules;
};

void uhuru_module_manager_init(struct uhuru_module_manager *mm);

void uhuru_module_manager_add(struct uhuru_module_manager *mm, struct uhuru_module *module);

void uhuru_module_manager_load_path(struct uhuru_module_manager *mm, const char *path);

void uhuru_module_manager_init_all(struct uhuru_module_manager *mm);

struct uhuru_module *uhuru_module_manager_get_by_name(struct uhuru_module_manager *mm, const char *module_name);

void uhuru_module_manager_close_all(struct uhuru_module_manager *mm);

#endif
