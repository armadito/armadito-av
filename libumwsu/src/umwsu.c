#include <libumwsu/module.h>
#include <libumwsu/scan.h>
#include "dir.h"
#include "modulep.h"

#include <assert.h>
#include <glib.h>
#include <magic.h>
#include <stdlib.h>
#include <stdio.h>

struct umwsu {
  GHashTable *mime_types_table;
  GPtrArray *modules;
  magic_t magic;
};

static struct umwsu *umwsu_new(void)
{
  struct umwsu *u = (struct umwsu *)malloc(sizeof(struct umwsu));

  assert(u != NULL);

  u->magic = magic_open(MAGIC_MIME_TYPE);
  magic_load(u->magic, NULL);

  u->mime_types_table = g_hash_table_new(g_str_hash, g_str_equal);
  u->modules = g_ptr_array_new();

  return u;
}

void umwsu_add_module(struct umwsu *u, struct umwsu_module *mod)
{
  g_ptr_array_add(u->modules, mod);
}

static void load_entry(const char *full_path, void *data)
{
  struct umwsu *u = (struct umwsu *)data;
  const char *t = magic_file(u->magic, full_path);
  struct umwsu_module *mod;
  
  if (strncmp("application/x-sharedlib", t))
    return;

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
  module_print((struct umwsu_module *)data);
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

static void mod_print_name(gpointer data, gpointer user_data)
{
  struct umwsu_module *mod = (struct umwsu_module *)data;
  printf("%s ", mod->name);
}

void print_mime_type_entry(gpointer key, gpointer value, gpointer user_data)
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

void umwsu_close(struct umwsu *u)
{
  magic_close(u->magic);
}

enum umwsu_status umwsu_scan_file(struct umwsu *u, const char *path)
{
  const char *mime_type = magic_file(u->magic, path);
  GPtrArray *mod_array;
  enum umwsu_status status = UMWSU_CLEAN;

  mod_array = (GPtrArray *)g_hash_table_lookup(u->mime_types_table, mime_type);
  
  if (mod_array == NULL)
    status = UMWSU_UNKNOWN_FILE_TYPE;
  else {
    int i;

    for (i = 0; i < mod_array->len; i++) {
      struct umwsu_module *mod = (struct umwsu_module *)g_ptr_array_index(mod_array, i);

      printf("UMWSU: module %s: scanning %s\n", mod->name, path);

      status = (*mod->scan)(path, mod->data);

      if (status != UMWSU_CLEAN)
	break;
    }
  }

  fprintf(stderr, "%s: %s\n", path, umwsu_status_str(status));

  return status;
}

static void scan_entry(const char *full_path, void *data)
{
  struct umwsu *u = (struct umwsu *)data;

  umwsu_scan_file(u, full_path);
}

enum umwsu_status umwsu_scan_dir(struct umwsu *u, const char *path, int recurse)
{
  dir_map(path, recurse, scan_entry, u);

  return UMWSU_CLEAN;
}

const char *umwsu_status_str(enum umwsu_status status)
{
  switch(status) {
#define M(S) case S: return #S
    M(UMWSU_CLEAN);
    M(UMWSU_MALWARE);
    M(UMWSU_EINVAL);
    M(UMWSU_IERROR);
    M(UMWSU_UNKNOWN_FILE_TYPE);
  }

  return "UNKNOWN STATUS";
}
