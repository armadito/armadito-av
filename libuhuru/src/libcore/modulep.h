#ifndef _LIBUHURU_MODULEP_H_
#define _LIBUHURU_MODULEP_H_

#include <libuhuru/core.h>

struct module_manager;

struct module_manager *module_manager_new(struct uhuru *uhuru);

void module_manager_add(struct module_manager *mm, struct uhuru_module *module);

void module_manager_load_path(struct module_manager *mm, const char *path);

/* NULL terminated array of struct uhuru_module */
struct uhuru_module **module_manager_get_modules(struct module_manager *mm);

void module_manager_post_init_all(struct module_manager *mm);

void module_manager_close_all(struct module_manager *mm);

#ifdef DEBUG
const char *module_debug(struct uhuru_module *module);
#endif

#endif
