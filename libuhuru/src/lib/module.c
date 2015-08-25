#include <libuhuru/module.h>
#include "modulep.h"

#include <assert.h>
#include <gmodule.h>
#include <stdlib.h>
#include <string.h>

struct module_manager {
  /* a GArray and not a GPtrArray because GArray can be automatically NULL terminated */
  GArray *modules;
};

/*
  is module copy really needed?
  module management modifies the uhuru_module structure, namely the fields
  'status' and 'data'
  but this is safe even if the structure is in a dynamically loaded object 
  (this has been checked in a small program).
  problem may arise if there are 2 instances of struct uhuru, but this 
  should not happen
  so for now, we don't copy, but keep the code around in case of
 */
static struct uhuru_module *module_new(struct uhuru_module *src)
{
  struct uhuru_module *mod = g_new(struct uhuru_module, 1);

  mod->init_fun = src->init_fun;
  mod->conf_table = src->conf_table;
  mod->post_init_fun = src->post_init_fun;
  mod->scan_fun = src->scan_fun;
  mod->close_fun = src->close_fun;

  mod->name = strdup(src->name);

  mod->mod_status = UHURU_MOD_OK;
  mod->mod_data = NULL;

  return mod;
}

static struct uhuru_module *module_load(const char *filename)
{
  struct uhuru_module *mod_loaded;
  GModule *g_mod;

  g_mod = g_module_open(filename, G_MODULE_BIND_LAZY);

  if (!g_mod)
    return NULL;

  if (!g_module_symbol(g_mod, "module", (gpointer *)&mod_loaded)) {
    g_log(NULL, G_LOG_LEVEL_CRITICAL, "symbol %s not found in file %s", "module", filename);

    return NULL;
  }

  g_log(NULL, G_LOG_LEVEL_DEBUG, "module %s loaded from file %s\n", mod_loaded->name, filename);

  return mod_loaded;
}

struct module_manager *module_manager_new(void)
{
  struct module_manager *mm = g_new(struct module_manager, 1);

  mm->modules = g_array_new(TRUE, TRUE, sizeof(struct uhuru_module *));

  return mm;
}

void module_manager_add(struct module_manager *mm, struct uhuru_module *module)
{
  g_array_append_val(mm->modules, module);
}

void module_manager_load_path(struct module_manager *mm, const char *path)
{
  GDir *dir;
  const char *filename;
  GError *err = NULL;

  dir = g_dir_open(path, 0, &err);

  while((filename = g_dir_read_name(dir)) != NULL) {
    struct uhuru_module *mod_loaded = module_load(filename);

    if (mod_loaded != NULL)
      module_manager_add(mm, mod_loaded);
  }

  g_dir_close(dir);
}

struct uhuru_module **module_manager_get_modules(struct module_manager *mm)
{
  return (struct uhuru_module **)mm->modules->data;
}
