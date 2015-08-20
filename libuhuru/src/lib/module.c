#include <libuhuru/module.h>
#include "modulep.h"

#include <assert.h>
#include <gmodule.h>
#include <stdlib.h>
#include <string.h>

struct uhuru_module *module_load(const char *filename)
{
  struct uhuru_module *mod, *mod_loaded;
  GModule *g_mod;

  g_mod = g_module_open(filename, G_MODULE_BIND_LAZY);

  if (!g_mod)
    return NULL;

  if (!g_module_symbol(g_mod, "module", (gpointer *)&mod_loaded)) {
    g_log(NULL, G_LOG_LEVEL_CRITICAL, "symbol %s not found in file %s", "module", filename);

    return NULL;
  }

  mod = (struct uhuru_module *)malloc(sizeof(struct uhuru_module));
  assert(mod != NULL);

  mod->name = strdup(mod_loaded->name);

  mod->init_fun = mod_loaded->init_fun;
  mod->conf_set = mod_loaded->conf_set;
  mod->conf_get = mod_loaded->conf_get;
  mod->post_init_fun = mod_loaded->post_init_fun;
  mod->scan_fun = mod_loaded->scan_fun;
  mod->close_fun = mod_loaded->close_fun;

  g_log(NULL, G_LOG_LEVEL_DEBUG, "module %s loaded from file %s\n", mod->name, filename);

  return mod;
}

void uhuru_module_manager_init(struct uhuru_module_manager *mm)
{
  mm->modules = g_ptr_array_new();
}

void uhuru_module_manager_add(struct uhuru_module_manager *mm, struct uhuru_module *module)
{
  g_ptr_array_add(mm->modules, module);
}

void uhuru_module_manager_load_path(struct uhuru_module_manager *mm, const char *path)
{
  GDir *dir;
  const char *filename;
  GError *err = NULL;

  dir = g_dir_open(path, 0, &err);

  while((filename = g_dir_read_name(dir)) != NULL) {
    struct uhuru_module *mod = module_load(filename);

    if (mod != NULL)
      uhuru_module_manager_add(mm, mod);
  }

  g_dir_close(dir);
}

void uhuru_module_manager_init_all(struct uhuru_module_manager *mm)
{
  int i;
  struct uhuru_module *mod;

  for (i = 0; i < mm->modules->len; i++) {
    mod = (struct uhuru_module *)g_ptr_array_index(mm->modules, i);

    mod->mod_status = UHURU_MOD_OK;
    mod->data = NULL;

    if (mod->init_fun != NULL)
      mod->mod_status = (*mod->init_fun)(&mod->data);
  }

  for (i = 0; i < mm->modules->len; i++) {
    mod = (struct uhuru_module *)g_ptr_array_index(mm->modules, i);

    if (mod->mod_status == UHURU_MOD_OK)
      conf_load(mod);
  }

  for (i = 0; i < mm->modules->len; i++) {
    mod = (struct uhuru_module *)g_ptr_array_index(mm->modules, i);

    if (mod->post_init_fun != NULL && mod->mod_status == UHURU_MOD_OK)
      mod->mod_status = (*mod->post_init_fun)(mod->data);
  }
}

struct uhuru_module *uhuru_module_manager_get_by_name(struct uhuru_module_manager *mm, const char *module_name)
{
  int i;

  for (i = 0; i < mm->modules->len; i++) {
    struct uhuru_module *mod = (struct uhuru_module *)g_ptr_array_index(mm->modules, i);

    if (!strcmp(mod->name, module_name))
      return mod;
  }

  return NULL;
}

void uhuru_module_manager_close_all(struct uhuru_module_manager *mm)
{
  int i;

  for (i = 0; i < mm->modules->len; i++) {
    struct uhuru_module *mod = (struct uhuru_module *)g_ptr_array_index(mm->modules, i);

    if (mod->close_fun != NULL && mod->mod_status == UHURU_MOD_OK)
      mod->mod_status = (*mod->close_fun)(mod->data);
  }
}


#if 0

struct uhuru_module *module_load(const char *path);

void module_init(struct uhuru_module *mod);

void module_debug(struct uhuru_module *mod);

void module_debug(struct uhuru_module *mod)
{
  g_log(NULL, G_LOG_LEVEL_DEBUG, "name: %s", mod->name);
  g_log(NULL, G_LOG_LEVEL_DEBUG, "init: %p", mod->init_fun);
  g_log(NULL, G_LOG_LEVEL_DEBUG, "scan: %p", mod->scan_fun);
  g_log(NULL, G_LOG_LEVEL_DEBUG, "close: %p", mod->close_fun);
}

static void mod_debug(gpointer data, gpointer user_data)
{
  module_debug((struct uhuru_module *)data);
}

static void uwm_module_print_all(struct uhuru *u)
{
  g_ptr_array_foreach(u->modules, mod_debug, NULL);
}

#endif
