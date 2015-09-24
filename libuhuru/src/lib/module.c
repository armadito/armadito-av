#include <libuhuru/module.h>
#include "modulep.h"
#include "libuhuru-config.h"

#include <assert.h>
#include <gmodule.h>
#include <stdlib.h>
#include <string.h>

struct module_manager {
  /* a GArray and not a GPtrArray because GArray can be automatically NULL terminated */
  GArray *modules;

  struct uhuru *uhuru;
};

/*
 * module 
 */

/*
  is module copy really needed?
  module management modifies the uhuru_module structure, namely the fields
  'status' and 'data'
  but this is safe even if the structure is in a dynamically loaded object 
  (this has been checked in a small program).
  problem may arise if there are 2 instances of struct uhuru, but this 
  should not happen
  so for now, we copy
 */
static struct uhuru_module *module_new(struct uhuru_module *src, struct uhuru *uhuru)
{
  struct uhuru_module *mod = g_new0(struct uhuru_module, 1);

  mod->init_fun = src->init_fun;
  mod->conf_table = src->conf_table;
  mod->post_init_fun = src->post_init_fun;
  mod->scan_fun = src->scan_fun;
  mod->close_fun = src->close_fun;
  mod->info_fun = src->info_fun;

	#ifdef WIN32
		mod->name = _strdup(src->name);
	#else
		mod->name = strdup(src->name);
	#endif

  mod->size = src->size;

  mod->status = UHURU_MOD_OK;
  mod->data = NULL;

  mod->uhuru = uhuru;

  if (mod->init_fun != NULL) {
    if (mod->size > 0)
      mod->data = g_malloc(mod->size);

    mod->status = (*mod->init_fun)(mod);

    if (mod->status != UHURU_MOD_OK)
      g_log(NULL, G_LOG_LEVEL_CRITICAL, "initialization error for module '%s'\n", mod->name);
  }

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

/*
 * module manager 
 */
struct module_manager *module_manager_new(struct uhuru *uhuru)
{
  struct module_manager *mm = g_new(struct module_manager, 1);

  mm->modules = g_array_new(TRUE, TRUE, sizeof(struct uhuru_module *));
  mm->uhuru = uhuru;

  return mm;
}

void module_manager_add(struct module_manager *mm, struct uhuru_module *module)
{
  struct uhuru_module *clone = module_new(module, mm->uhuru);

  g_array_append_val(mm->modules, clone);
}

void module_manager_load_path(struct module_manager *mm, const char *path)
{
  GDir *dir;
  const char *filename;
  GString *full_path = g_string_new("");
  GError *err = NULL;

  dir = g_dir_open(path, 0, &err);

  while((filename = g_dir_read_name(dir)) != NULL) {
    struct uhuru_module *mod_loaded;

    g_string_printf(full_path, "%s%s%s", path, G_DIR_SEPARATOR_S, filename);

    mod_loaded = module_load(full_path->str);

    if (mod_loaded != NULL)
      module_manager_add(mm, mod_loaded);
  }

  g_dir_close(dir);

  g_string_free(full_path, TRUE);
}

void module_manager_post_init_all(struct module_manager *mm)
{
  struct uhuru_module **modv;

  for (modv = module_manager_get_modules(mm); *modv != NULL; modv++) {
    struct uhuru_module *mod = *modv;

    if (mod->post_init_fun != NULL && mod->status == UHURU_MOD_OK)
      mod->status = (*mod->post_init_fun)(mod);
  }
}

struct uhuru_module **module_manager_get_modules(struct module_manager *mm)
{
  return (struct uhuru_module **)mm->modules->data;
}

void module_manager_close_all(struct module_manager *mm)
{
  struct uhuru_module **modv;

  for (modv = module_manager_get_modules(mm); *modv != NULL; modv++) {
    struct uhuru_module *mod = *modv;

    if (mod->close_fun != NULL && mod->status == UHURU_MOD_OK)
      mod->status = (*mod->close_fun)(mod);
  }
}

#ifdef DEBUG
const char *module_debug(struct uhuru_module *module)
{
  GString *s = g_string_new("");
  const char *ret;

  g_string_append_printf(s, "Module '%s':\n", module->name);
  g_string_append_printf(s, "  status     %d\n", module->status);
  g_string_append_printf(s, "  data       %p\n", module->data);
  g_string_append_printf(s, "  init       %p\n", module->init_fun);
  g_string_append_printf(s, "  post_init  %p\n", module->post_init_fun);
  g_string_append_printf(s, "  scan       %p\n", module->scan_fun);
  g_string_append_printf(s, "  close      %p\n", module->close_fun);

  if (module->conf_table != NULL) {
    struct uhuru_conf_entry *p;

    g_string_append_printf(s, "  configuration:\n");
    for(p = module->conf_table; p->directive != NULL; p++)
      g_string_append_printf(s, "    directive %-20s conf %p\n", p->directive, p->conf_fun);
  }

  ret = s->str;
  g_string_free(s, FALSE);

  return ret;
}
#endif
