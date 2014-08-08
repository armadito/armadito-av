#include <libumwsu/module.h>
#include <libumwsu/scan.h>
#include "dir.h"
#include "modulep.h"

#include <assert.h>
#include <glib.h>
#include <magic.h>
#include <stdlib.h>
#include <stdio.h>

struct umw {
  GHashTable *mime_types_table;
  GPtrArray *modules;
  magic_t magic;
};

static struct umw *umw_new(void)
{
  struct umw *u = (struct umw *)malloc(sizeof(struct umw));

  assert(u != NULL);

  u->magic = magic_open(MAGIC_MIME_TYPE);
  magic_load(u->magic, NULL);

  u->mime_types_table = g_hash_table_new(g_str_hash, g_str_equal);
  u->modules = g_ptr_array_new();

  return u;
}

void umw_add_module(struct umw *u, struct umw_module *mod)
{
  g_ptr_array_add(u->modules, mod);
}

static void load_entry(const char *full_path, void *data)
{
  struct umw *u = (struct umw *)data;
  const char *t = magic_file(u->magic, full_path);
  struct umw_module *mod;
  
  if (strncmp("application/x-sharedlib", t))
    return;

  printf("UMW: loading module object: %s\n", full_path);

  mod = module_new(full_path);

  umw_add_module(u, mod);
}

static int umw_module_load_directory(struct umw *u, const char *directory)
{
  return dir_map(directory, 0, load_entry, u);
}

static void umw_add_mime_type(struct umw *u, const char *mime_type, struct umw_module *mod)
{
  GPtrArray *mod_array;

  mod_array = (GPtrArray *)g_hash_table_lookup(u->mime_types_table, mime_type);

  if (mod_array == NULL) {
    mod_array = g_ptr_array_new();

    g_hash_table_insert(u->mime_types_table, (gpointer)mime_type, mod_array);
  }

  g_ptr_array_add(mod_array, mod);
}

static void uwm_module_init_all(struct umw *u)
{
  int i;

  for (i = 0; i < u->modules->len; i++) {
    struct umw_module *mod = (struct umw_module *)g_ptr_array_index(u->modules, i);

    if (mod->init != NULL)
      (*mod->init)(&mod->data);

    if (mod->mime_types != NULL) {
      const char **p;

      for(p = mod->mime_types; *p != NULL; p++)
	umw_add_mime_type(u, *p, mod);
    }
  }
}

static void mod_print(gpointer data, gpointer user_data)
{
  module_print((struct umw_module *)data);
}

static void uwm_module_print_all(struct umw *u)
{
  g_ptr_array_foreach(u->modules, mod_print, NULL);
}

struct umw *umw_open(void)
{
  struct umw *u = umw_new();

  umw_module_load_directory(u, LIBUMWSU_MODULES_PATH);

  /* module_conf_all(); */

  uwm_module_init_all(u);

  /* uwm_module_print_all(u); */

  return u;
}

static void mod_print_name(gpointer data, gpointer user_data)
{
  struct umw_module *mod = (struct umw_module *)data;
  printf("%s ", mod->name);
}

void print_mime_type_entry(gpointer key, gpointer value, gpointer user_data)
{
  printf("UMW: MIME type: %s handled by module: ", (char *)key);
  g_ptr_array_foreach((GPtrArray *)value, mod_print_name, NULL);
  printf("\n");
}

void umw_print(struct umw *u)
{
  printf("UMW: modules loaded: ");
  g_ptr_array_foreach(u->modules, mod_print_name, NULL);
  printf("\n");

  g_hash_table_foreach(u->mime_types_table, print_mime_type_entry, NULL);
}

void umw_close(struct umw *u)
{
  magic_close(u->magic);
}

enum umw_status umw_scan_file(struct umw *u, const char *path)
{
  const char *mime_type = magic_file(u->magic, path);
  GPtrArray *mod_array;
  enum umw_status status = UMW_CLEAN;

  mod_array = (GPtrArray *)g_hash_table_lookup(u->mime_types_table, mime_type);
  
  if (mod_array == NULL)
    status = UMW_UNKNOWN_FILE_TYPE;
  else {
    int i;

    for (i = 0; i < mod_array->len; i++) {
      struct umw_module *mod = (struct umw_module *)g_ptr_array_index(mod_array, i);

      printf("UMW: module %s: scanning %s\n", mod->name, path);

      status = (*mod->scan)(path, mod->data);

      if (status != UMW_CLEAN)
	break;
    }
  }

  fprintf(stderr, "%s: %s\n", path, umw_status_str(status));

  return status;
}

static void scan_entry(const char *full_path, void *data)
{
  struct umw *u = (struct umw *)data;

  umw_scan_file(u, full_path);
}

enum umw_status umw_scan_dir(struct umw *u, const char *path, int recurse)
{
  dir_map(path, recurse, scan_entry, u);

  return UMW_CLEAN;
}

const char *umw_status_str(enum umw_status status)
{
  switch(status) {
#define M(S) case S: return #S
    M(UMW_CLEAN);
    M(UMW_MALWARE);
    M(UMW_EINVAL);
    M(UMW_IERROR);
    M(UMW_UNKNOWN_FILE_TYPE);
  }

  return "UNKNOWN STATUS";
}
