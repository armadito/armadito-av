#ifndef _LIBARMADITO_MODULEP_H_
#define _LIBARMADITO_MODULEP_H_

#include <libarmadito.h>

struct module_manager;

struct module_manager *module_manager_new(struct armadito *armadito);

void module_manager_free(struct module_manager *mm);

void module_manager_add(struct module_manager *mm, struct a6o_module *module);

int module_manager_load_path(struct module_manager *mm, const char *path, a6o_error **error);

int module_manager_init_all(struct module_manager *mm, a6o_error **error);

int module_manager_configure_all(struct module_manager *mm, struct a6o_conf *conf, a6o_error **error);

int module_manager_post_init_all(struct module_manager *mm, a6o_error **error);

int module_manager_close_all(struct module_manager *mm, a6o_error **error);

/* returns a NULL-terminated array of struct a6o_module */
struct a6o_module **module_manager_get_modules(struct module_manager *mm);

struct a6o_module *module_manager_get_module_by_name(struct module_manager *mm, const char *name);

#ifdef DEBUG
const char *module_debug(struct a6o_module *module);
#endif

#endif
