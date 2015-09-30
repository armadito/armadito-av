#include "libuhuru-config.h"

#include <libuhuru/module.h>
#include <libuhuru/scan.h>

#include "conf.h"
#include "modulep.h"
#include "statusp.h"
#include "uhurup.h"
#include "os/mimetype.h"
#include "builtin-modules/alert.h"
#include "builtin-modules/quarantine.h"
#include "builtin-modules/remote.h"
#include "builtin-modules/mimetype.h"

#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct uhuru {
  int is_remote;
  struct module_manager *module_manager;
  GHashTable *mime_type_table;
};

static struct uhuru *uhuru_new(int is_remote)
{
  struct uhuru *u = g_new(struct uhuru, 1);

  u->is_remote = is_remote;

  u->module_manager = module_manager_new(u);

  u->mime_type_table = g_hash_table_new(g_str_hash, g_str_equal);

  return u;
}

struct uhuru *uhuru_open(int is_remote)
{
  struct uhuru *u;

  os_mime_type_init();

  u = uhuru_new(is_remote);

  module_manager_add(u->module_manager, &remote_module);

  if (!u->is_remote) {
    module_manager_add(u->module_manager, &mimetype_module);
    module_manager_add(u->module_manager, &alert_module);
    module_manager_add(u->module_manager, &quarantine_module);

    module_manager_load_path(u->module_manager, LIBUHURU_MODULES_PATH);
  }

  conf_load_file(u, LIBUHURU_CONF_DIR "/uhuru.conf");
  conf_load_path(u, LIBUHURU_CONF_DIR "/conf.d");

  module_manager_post_init_all(u->module_manager);

#ifdef DEBUG
  g_log(NULL, G_LOG_LEVEL_DEBUG, "after post_init:\n%s\n", uhuru_debug(u));
#endif

  return u;
}

int uhuru_is_remote(struct uhuru *u)
{
  return u->is_remote;
}

const char *uhuru_get_remote_url(struct uhuru *u)
{
  struct uhuru_module *remote_module;
  const char *remote_url;

  remote_module = uhuru_get_module_by_name(u, "remote");
  assert(remote_module != NULL);

  remote_url = remote_module_get_sock_dir(remote_module);
  assert(remote_url != NULL);

  return remote_url;
}

struct uhuru_module **uhuru_get_modules(struct uhuru *u)
{
  return module_manager_get_modules(u->module_manager);
}

struct uhuru_module *uhuru_get_module_by_name(struct uhuru *u, const char *module_name)
{
  struct uhuru_module **modv;

  for (modv = module_manager_get_modules(u->module_manager); *modv != NULL; modv++)
    if (!strcmp((*modv)->name, module_name))
      return *modv;

  return NULL;
}

void uhuru_add_mime_type(struct uhuru *u, const char *mime_type, struct uhuru_module *module)
{
  /* a GArray and not a GPtrArray because GArray can be automatically NULL terminated */
  GArray *modules;

  modules = (GArray *)g_hash_table_lookup(u->mime_type_table, mime_type);

  if (modules == NULL) {
    modules = g_array_new(TRUE, TRUE, sizeof(struct uhuru_module *));

    g_hash_table_insert(u->mime_type_table, (gpointer)(strdup(mime_type)), modules);
  }

  g_array_append_val(modules, module);
}

struct uhuru_module **uhuru_get_applicable_modules(struct uhuru *u, const char *mime_type)
{
  GArray *modules;

  modules = (GArray *)g_hash_table_lookup(u->mime_type_table, mime_type);

  if (modules != NULL)
    return (struct uhuru_module **)modules->data;

  modules = (GArray *)g_hash_table_lookup(u->mime_type_table, "*");

  if (modules != NULL)
    return (struct uhuru_module **)modules->data;

  return NULL;
}

void uhuru_close(struct uhuru *u)
{
  module_manager_close_all(u->module_manager);
}

#ifdef DEBUG
static void mod_print_name(gpointer data, gpointer user_data)
{
  struct uhuru_module *mod = (struct uhuru_module *)data;

  printf("%s ", mod->name);
}

static void print_mime_type_entry(gpointer key, gpointer value, gpointer user_data)
{
  GString *s = (GString *)user_data;
  GArray *modules = (GArray *)value;
  struct uhuru_module **modv;

  g_string_append_printf(s, "    mimetype: %s handled by modules:", (char *)key);
 
  for (modv = (struct uhuru_module **)modules->data; *modv != NULL; modv++)
    g_string_append_printf(s, " %s", (*modv)->name);

  g_string_append_printf(s, "\n");
}

const char *uhuru_debug(struct uhuru *u)
{
  struct uhuru_module **modv;
  GString *s = g_string_new("");
  const char *ret;
  g_string_append_printf(s, "Uhuru:\n");

  for (modv = module_manager_get_modules(u->module_manager); *modv != NULL; modv++)
    g_string_append_printf(s, "%s\n", module_debug(*modv));

  g_string_append_printf(s, "  mime types:\n");
  g_hash_table_foreach(u->mime_type_table, print_mime_type_entry, s);

  ret = s->str;
  g_string_free(s, FALSE);

  return ret;
}
#endif
