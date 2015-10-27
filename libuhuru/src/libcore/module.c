#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "modulep.h"
#include "os/dir.h"
#include "os/string.h"

#include <assert.h>
#include <gmodule.h>
#include <glib.h>
#include <stdio.h>
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

static void module_free(struct uhuru_module *mod);

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
static struct uhuru_module *module_new(struct uhuru_module *src, struct uhuru *uhuru, uhuru_error **error)
{
  struct uhuru_module *mod = g_new0(struct uhuru_module, 1);

  mod->init_fun = src->init_fun;
  mod->conf_table = src->conf_table;
  mod->post_init_fun = src->post_init_fun;
  mod->scan_fun = src->scan_fun;
  mod->close_fun = src->close_fun;
  mod->info_fun = src->info_fun;
  mod->name = os_strdup(src->name);
  mod->size = src->size;
  mod->status = UHURU_MOD_OK;
  mod->data = NULL;
  mod->uhuru = uhuru;

  /* module has no init_fun, nothing else to do */
  if (mod->init_fun == NULL)
    return mod;

  if (mod->size > 0)
    mod->data = g_malloc(mod->size);

  /* call the init function */
  mod->status = (*mod->init_fun)(mod);

  /* everything's ok */
  if (mod->status == UHURU_MOD_OK)
    return mod;

  /* module init failed, set error and return NULL */
  g_log(NULL, G_LOG_LEVEL_WARNING, "initialization error for module '%s'\n", mod->name);

  uhuru_error_set(error, UHURU_ERROR_MODULE_INIT_FAILED, "initialization error for module");

  module_free(mod);

  return NULL;
}

static void module_free(struct uhuru_module *mod)
{
  free((void *)mod->name);

  free(mod);
}

static struct uhuru_module *module_load(const char *filename)
{
  struct uhuru_module *mod_loaded;
  GModule *g_mod;

  g_mod = g_module_open(filename, G_MODULE_BIND_LAZY);

  if (!g_mod)
    return NULL;

  if (!g_module_symbol(g_mod, "module", (gpointer *)&mod_loaded)) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "symbol %s not found in file %s", "module", filename);

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

int module_manager_add(struct module_manager *mm, struct uhuru_module *module, uhuru_error **error)
{
  /* FIXME: dirty without error checking */
  struct uhuru_module *clone = module_new(module, mm->uhuru, error);

  if (clone != NULL)
    g_array_append_val(mm->modules, clone);

  return 0;
}

static void module_load_dirent_cb(const char *full_path, enum os_file_flag flags, int entry_errno, void *data)
{
  if (flags & FILE_FLAG_IS_PLAIN_FILE) {
    struct uhuru_module *mod_loaded;

    mod_loaded = module_load(full_path);

    if (mod_loaded != NULL)
      module_manager_add((struct module_manager *)data, mod_loaded, NULL);
  }
}

int module_manager_load_path(struct module_manager *mm, const char *path, uhuru_error **error)
{
  /* FIXME: for now, dirty stuff, do nothing with error */

  os_dir_map(path, 0, module_load_dirent_cb, mm);

  return 0;
}

int module_manager_post_init_all(struct module_manager *mm, uhuru_error **error)
{
  struct uhuru_module **modv;

  /* iterate over all modules */
  for (modv = module_manager_get_modules(mm); *modv != NULL; modv++) {
    struct uhuru_module *mod = *modv;

    /* module has no post_init_fun, do nothing */
    if (mod->post_init_fun == NULL)
      continue;

    /* module is not ok, do nothing */
    if (mod->status != UHURU_MOD_OK)
      continue;

    /* call the post_init function */
    mod->status = (*mod->post_init_fun)(mod);

    /* if post_init failed, return an error */
    if (mod->status != UHURU_MOD_OK) {
      g_log(NULL, G_LOG_LEVEL_WARNING, "post_init error for module '%s'\n", mod->name);

      uhuru_error_set(error, UHURU_ERROR_MODULE_POST_INIT_FAILED, "post_init error for module");

      return UHURU_ERROR_MODULE_POST_INIT_FAILED;
    }
  }

  return 0;
}

struct uhuru_module **module_manager_get_modules(struct module_manager *mm)
{
  return (struct uhuru_module **)mm->modules->data;
}

int module_manager_close_all(struct module_manager *mm, uhuru_error **error)
{
  struct uhuru_module **modv;

  for (modv = module_manager_get_modules(mm); *modv != NULL; modv++) {
    struct uhuru_module *mod = *modv;

    /* module has no close_fun, do nothing */
    if (mod->close_fun == NULL)
      continue;

    /* module is not ok, do nothing */
    if (mod->status != UHURU_MOD_OK)
      continue;

    /* call the close function */
    mod->status = (*mod->close_fun)(mod);

    /* if close failed, return an error */
    if (mod->status != UHURU_MOD_OK) {
      g_log(NULL, G_LOG_LEVEL_WARNING, "close error for module '%s'\n", mod->name);

      uhuru_error_set(error, UHURU_ERROR_MODULE_POST_INIT_FAILED, "close error for module");

      return UHURU_ERROR_MODULE_CLOSE_FAILED;
    }
  }

  return 0;
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
