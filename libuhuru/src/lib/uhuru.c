#include <libuhuru/module.h>
#include <libuhuru/scan.h>
#include "alert.h"
#include "dir.h"
#include "modulep.h"
#include "quarantine.h"
#include "remote.h"
#include "statusp.h"
#include "watchp.h"

#include <assert.h>
#include <glib.h>
#include <magic.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct uhuru {
  int verbosity;
  int is_remote;
  GPtrArray *modules;

  GHashTable *mime_types_table;
  magic_t magic;
  /* for now... */
  struct uhuru_watch *watch;
};

static struct uhuru *uhuru_new(int is_remote)
{
  struct uhuru *u = (struct uhuru *)malloc(sizeof(struct uhuru));

  assert(u != NULL);

  u->verbosity = 0;
  u->is_remote = is_remote;

  u->modules = g_ptr_array_new();

  u->magic = magic_open(MAGIC_MIME_TYPE);
  magic_load(u->magic, NULL);

  u->mime_types_table = g_hash_table_new(g_str_hash, g_str_equal);

  u->watch = NULL;

  return u;
}

static void uhuru_add_module(struct uhuru *u, struct uhuru_module *mod)
{
  g_ptr_array_add(u->modules, mod);
}

static void load_entry(const char *full_path, enum dir_entry_flag flags, int errno, void *data)
{
  struct uhuru *u = (struct uhuru *)data;
  const char *t = magic_file(u->magic, full_path);
  struct uhuru_module *mod;

  if (strcmp("application/x-sharedlib", t))
    return;

  if (u->verbosity >= 1)
    printf("UHURU: loading module object: %s\n", full_path);

  mod = module_new(full_path);

  uhuru_add_module(u, mod);
}

static int uhuru_module_load_directory(struct uhuru *u, const char *directory)
{
  return dir_map(directory, 0, load_entry, u);
}

static void uhuru_module_load_all(struct uhuru *u)
{
  if (!u->is_remote)
    uhuru_module_load_directory(u, LIBUHURU_MODULES_PATH);

  if (!u->is_remote) {
    uhuru_add_module(u, &uhuru_mod_alert);
    uhuru_add_module(u, &uhuru_mod_quarantine);
  }

  uhuru_add_module(u, &uhuru_mod_remote);
}

static void uhuru_module_conf_all(struct uhuru *u)
{
  int i;

  for (i = 0; i < u->modules->len; i++) {
    struct uhuru_module *mod = (struct uhuru_module *)g_ptr_array_index(u->modules, i);

    conf_load(mod);
  }
}

static void uhuru_add_mime_type(struct uhuru *u, const char *mime_type, struct uhuru_module *mod)
{
  GPtrArray *mod_array;

  mod_array = (GPtrArray *)g_hash_table_lookup(u->mime_types_table, mime_type);

  if (mod_array == NULL) {
    mod_array = g_ptr_array_new();

    g_hash_table_insert(u->mime_types_table, (gpointer)mime_type, mod_array);
  }

  g_ptr_array_add(mod_array, mod);
}

static void uwmsu_module_init_all(struct uhuru *u)
{
  int i;

  for (i = 0; i < u->modules->len; i++) {
    struct uhuru_module *mod = (struct uhuru_module *)g_ptr_array_index(u->modules, i);

    if (mod->init != NULL)
      (*mod->init)(&mod->data);

    if (mod->mime_types != NULL) {
      const char **p;

      for(p = mod->mime_types; *p != NULL; p++)
	uhuru_add_mime_type(u, *p, mod);
    }
  }
}

static void mod_print(gpointer data, gpointer user_data)
{
  module_print((struct uhuru_module *)data, stderr);
}

static void uwm_module_print_all(struct uhuru *u)
{
  g_ptr_array_foreach(u->modules, mod_print, NULL);
}

struct uhuru *uhuru_open(int is_remote)
{
  struct uhuru *u = uhuru_new(is_remote);

  uhuru_module_load_all(u);

  uhuru_module_conf_all(u);

  uwmsu_module_init_all(u);

  /* uwm_module_print_all(u); */

  return u;
}

int uhuru_is_remote(struct uhuru *u)
{
  return u->is_remote;
}

void uhuru_set_verbose(struct uhuru *u, int verbosity)
{
  u->verbosity = verbosity;
}

int uhuru_get_verbose(struct uhuru *u)
{
  return u->verbosity;
}

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
  g_ptr_array_foreach(u->modules, mod_print_name, NULL);
  printf("\n");

  g_hash_table_foreach(u->mime_types_table, print_mime_type_entry, NULL);
}

struct uhuru_module *uhuru_get_module_by_name(struct uhuru *u, const char *name)
{
  int i;

  for (i = 0; i < u->modules->len; i++) {
    struct uhuru_module *mod = (struct uhuru_module *)g_ptr_array_index(u->modules, i);

    if (!strcmp(mod->name, name))
      return mod;
  }

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

void uhuru_close(struct uhuru *u)
{
  magic_close(u->magic);

  /* must close all modules */
}

void uhuru_watch(struct uhuru *u, const char *dir)
{
  if (u->watch == NULL)
    u->watch = uhuru_watch_new();

  uhuru_watch_add(u->watch, dir);
}

int uhuru_watch_next_event(struct uhuru *u, struct uhuru_watch_event *event)
{
  return uhuru_watch_wait(u->watch, event);
}

