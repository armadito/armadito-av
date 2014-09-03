#include <libumwsu/module.h>
#include <libumwsu/scan.h>
#include "dir.h"
#include "modulep.h"
#include "statusp.h"

#include <assert.h>
#include <glib.h>
#include <magic.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct umwsu {
  int verbosity;
  GHashTable *mime_types_table;
  GPtrArray *modules;
  magic_t magic;
};

static struct umwsu *umwsu_new(void)
{
  struct umwsu *u = (struct umwsu *)malloc(sizeof(struct umwsu));

  assert(u != NULL);

  u->verbosity = 0;

  u->magic = magic_open(MAGIC_MIME_TYPE);
  magic_load(u->magic, NULL);

  u->mime_types_table = g_hash_table_new(g_str_hash, g_str_equal);
  u->modules = g_ptr_array_new();

  return u;
}

static void umwsu_add_module(struct umwsu *u, struct umwsu_module *mod)
{
  g_ptr_array_add(u->modules, mod);
}

static void load_entry(const char *full_path, const struct dirent *dir_entry, void *data)
{
  struct umwsu *u = (struct umwsu *)data;
  const char *t = magic_file(u->magic, full_path);
  struct umwsu_module *mod;

  if (strcmp("application/x-sharedlib", t))
    return;

  if (u->verbosity >= 1)
    printf("UMWSU: loading module object: %s\n", full_path);

  mod = module_new(full_path);

  umwsu_add_module(u, mod);
}

static int umwsu_module_load_directory(struct umwsu *u, const char *directory)
{
  return dir_map(directory, 0, load_entry, u);
}

static void umwsu_add_mime_type(struct umwsu *u, const char *mime_type, struct umwsu_module *mod)
{
  GPtrArray *mod_array;

  mod_array = (GPtrArray *)g_hash_table_lookup(u->mime_types_table, mime_type);

  if (mod_array == NULL) {
    mod_array = g_ptr_array_new();

    g_hash_table_insert(u->mime_types_table, (gpointer)mime_type, mod_array);
  }

  g_ptr_array_add(mod_array, mod);
}

static void uwm_module_init_all(struct umwsu *u)
{
  int i;

  for (i = 0; i < u->modules->len; i++) {
    struct umwsu_module *mod = (struct umwsu_module *)g_ptr_array_index(u->modules, i);

    if (mod->init != NULL)
      (*mod->init)(&mod->data);

    if (mod->mime_types != NULL) {
      const char **p;

      for(p = mod->mime_types; *p != NULL; p++)
	umwsu_add_mime_type(u, *p, mod);
    }
  }
}

static void mod_print(gpointer data, gpointer user_data)
{
  module_print((struct umwsu_module *)data, stderr);
}

static void uwm_module_print_all(struct umwsu *u)
{
  g_ptr_array_foreach(u->modules, mod_print, NULL);
}

struct umwsu *umwsu_open(void)
{
  struct umwsu *u = umwsu_new();

  umwsu_module_load_directory(u, LIBUMWSU_MODULES_PATH);

  /* module_conf_all(); */

  uwm_module_init_all(u);

  /* uwm_module_print_all(u); */

  return u;
}

void umwsu_set_verbose(struct umwsu *u, int verbosity)
{
  u->verbosity = verbosity;
}

int umwsu_get_verbose(struct umwsu *u)
{
  return u->verbosity;
}

static void mod_print_name(gpointer data, gpointer user_data)
{
  struct umwsu_module *mod = (struct umwsu_module *)data;
  printf("%s ", mod->name);
}

static void print_mime_type_entry(gpointer key, gpointer value, gpointer user_data)
{
  printf("UMWSU: MIME type: %s handled by module: ", (char *)key);
  g_ptr_array_foreach((GPtrArray *)value, mod_print_name, NULL);
  printf("\n");
}

void umwsu_print(struct umwsu *u)
{
  printf("UMWSU: modules loaded: ");
  g_ptr_array_foreach(u->modules, mod_print_name, NULL);
  printf("\n");

  g_hash_table_foreach(u->mime_types_table, print_mime_type_entry, NULL);
}

GPtrArray *umwsu_get_applicable_modules(struct umwsu *u, magic_t magic, const char *path)
{
  const char *mime_type;

  if (magic == NULL)
    magic = u->magic;

  mime_type = magic_file(magic, path);

  return (GPtrArray *)g_hash_table_lookup(u->mime_types_table, mime_type);
}

void umwsu_close(struct umwsu *u)
{
  magic_close(u->magic);
}

