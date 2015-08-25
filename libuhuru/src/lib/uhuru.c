#include <libuhuru/module.h>
#include <libuhuru/scan.h>
#include "conf.h"
#include "modulep.h"
#include "statusp.h"
#include "builtin-modules/alert.h"
#include "builtin-modules/quarantine.h"
#include "builtin-modules/remote.h"
#include "builtin-modules/uhurumod.h"

#include <assert.h>
#include <glib.h>
#include <magic.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct uhuru {
  int is_remote;
  struct module_manager *module_manager;
  GHashTable *mime_types_table;

  /* FIXME: to be removed */
  magic_t magic;
};

static struct uhuru *uhuru_new(int is_remote)
{
  struct uhuru *u = (struct uhuru *)malloc(sizeof(struct uhuru));

  assert(u != NULL);

  u->is_remote = is_remote;

  u->module_manager = module_manager_new();

  u->mime_types_table = g_hash_table_new(g_str_hash, g_str_equal);

  u->magic = magic_open(MAGIC_MIME_TYPE);
  magic_load(u->magic, NULL);

  return u;
}

static void uhuru_add_modules(struct uhuru *u)
{
  module_manager_add(u->module_manager, &uhurumod_module);

  if (!u->is_remote) {
    module_manager_add(u->module_manager, &alert_module);
    module_manager_add(u->module_manager, &quarantine_module);
  }

  module_manager_add(u->module_manager, &remote_module);

  if (!u->is_remote)
    module_manager_load_path(u->module_manager, LIBUHURU_MODULES_PATH);
}

static void uhuru_all_modules_init(struct uhuru *u)
{
  struct uhuru_module **modv;

  for (modv = module_manager_get_modules(u->module_manager); *modv != NULL; modv++) {
    struct uhuru_module *mod = *modv;

    if (mod->init_fun != NULL)
      mod->mod_status = (*mod->init_fun)(&mod->mod_data);
    else if (!strcmp(mod->name, "uhuru")) {
      /* 
	 very special treatment for the "uhuru" module, which has its mod_data pointing to 
	 the struct uhuru *
	 this is done to allow the configuration directives to access the uhuru structure
       */
      mod->mod_status = UHURU_MOD_OK;
      mod->mod_data = u;
    }
  }
}

static void uhuru_all_modules_post_init(struct uhuru *u)
{
  struct uhuru_module **modv;

  for (modv = module_manager_get_modules(u->module_manager); *modv != NULL; modv++) {
    struct uhuru_module *mod = *modv;

    if (mod->post_init_fun != NULL && mod->mod_status == UHURU_MOD_OK)
      mod->mod_status = (*mod->post_init_fun)(mod->mod_data);
  }
}

struct uhuru *uhuru_open(int is_remote)
{
  struct uhuru *u = uhuru_new(is_remote);

  uhuru_add_modules(u);

  uhuru_all_modules_init(u);

  conf_load_file(u, LIBUHURU_CONF_DIR "/uhuru.conf");
  conf_load_path(u, LIBUHURU_CONF_DIR "/conf.d");

  uhuru_all_modules_post_init(u);

  return u;
}

int uhuru_is_remote(struct uhuru *u)
{
  return u->is_remote;
}

void uhuru_add_mime_type(struct uhuru *u, const char *mime_type, struct uhuru_module *mod)
{
  GPtrArray *mod_array;

  mod_array = (GPtrArray *)g_hash_table_lookup(u->mime_types_table, mime_type);

  if (mod_array == NULL) {
    mod_array = g_ptr_array_new();

    g_hash_table_insert(u->mime_types_table, (gpointer)mime_type, mod_array);
  }

  g_ptr_array_add(mod_array, mod);
}

struct uhuru_module *uhuru_get_module_by_name(struct uhuru *u, const char *module_name)
{
  struct uhuru_module **modv;

  for (modv = module_manager_get_modules(u->module_manager); *modv != NULL; modv++)
    if (!strcmp((*modv)->name, module_name))
      return *modv;

  return NULL;
}

GPtrArray *uhuru_get_applicable_modules(struct uhuru *u, magic_t magic, const char *path, char **p_mime_type)
{
  GPtrArray *modules;

  if (magic == NULL)
    magic = u->magic;

  *p_mime_type = strdup(magic_file(magic, path));

  modules = (GPtrArray *)g_hash_table_lookup(u->mime_types_table, *p_mime_type);

  if (modules != NULL)
    return modules;

  modules = (GPtrArray *)g_hash_table_lookup(u->mime_types_table, "*");

  return modules;
}

static void uhuru_all_modules_close(struct uhuru *u)
{
  struct uhuru_module **modv;

  for (modv = module_manager_get_modules(u->module_manager); *modv != NULL; modv++) {
    struct uhuru_module *mod = *modv;

    if (mod->close_fun != NULL && mod->mod_status == UHURU_MOD_OK)
      mod->mod_status = (*mod->close_fun)(mod->mod_data);
  }
}

void uhuru_close(struct uhuru *u)
{
  magic_close(u->magic);

  uhuru_all_modules_close(u);
}


/*
 * debug functions
 * must probably be updated
 */

static void mod_print_name(gpointer data, gpointer user_data)
{
  struct uhuru_module *mod = (struct uhuru_module *)data;

  printf("%s ", mod->name);
}

static void print_mime_type_entry(gpointer key, gpointer value, gpointer user_data)
{
  printf("UHURU: MIME type: %s handled by module: ", (char *)key);
  g_ptr_array_foreach((GPtrArray *)value, mod_print_name, NULL);
  printf("\n");
}

void uhuru_print(struct uhuru *u)
{
  printf("UHURU: modules loaded: ");
  /* g_ptr_array_foreach(u->modules, mod_print_name, NULL); */
  printf("\n");

  g_hash_table_foreach(u->mime_types_table, print_mime_type_entry, NULL);
}

