#include <libumwsu/scan.h>
#include "dir.h"
#include "modulep.h"
#include "umwsup.h"

#include <assert.h>
#include <glib.h>
#include <magic.h>
#include <stdlib.h>
#include <stdio.h>

struct umw {
  GHashTable *magic_desc_table;
  GPtrArray *modules;
};

static struct umw *umw_new(void)
{
  struct umw *u = (struct umw *)malloc(sizeof(struct umw));

  assert(u != NULL);

  u->magic_desc_table = g_hash_table_new(g_str_hash, g_str_equal);
  u->modules = g_ptr_array_new();

  return u;
}

void umw_add_module(struct umw *u, struct umw_module *mod)
{
  g_ptr_array_add(u->modules, mod);
}

struct load_data {
  magic_t magic;
  struct umw *u;
};

static void load_entry(const char *full_path, void *data)
{
  struct load_data *l = (struct load_data *)data;
  const char *t = magic_file(l->magic, full_path);
  struct umw_module *mod;
  
  if (strncmp("ELF 64-bit LSB shared object", t, 28))
    return;

  printf("loading module: %s\n", full_path);

  mod = module_new(full_path);

  umw_add_module(l->u, mod);
}

static int umw_module_load_directory(struct umw *u, const char *directory)
{
  struct load_data data;
  int r;

  data.magic = magic_open(MAGIC_NONE);
  data.u = u;

  magic_load(data.magic, NULL);

  r = dir_map(directory, 0, load_entry, &data);

  magic_close(data.magic);

  return r;
}

void umw_add_magic_desc(struct umw *u, const char *magic_desc, struct umw_module *mod)
{
  GPtrArray *mod_array;

  mod_array = (GPtrArray *)g_hash_table_lookup(u->magic_desc_table, magic_desc);

  if (mod_array == NULL) {
    mod_array = g_ptr_array_new();

    g_hash_table_insert(u->magic_desc_table, (gpointer)magic_desc, mod_array);
  }

  g_ptr_array_add(mod_array, mod);
}

static void uwm_module_init_all(struct umw *u)
{
  int i;

  for (i = 0; i < u->modules->len; i++) {
    struct umw_module *mod = (struct umw_module *)g_ptr_array_index(u->modules, i);
    const char **pdesc;

    for(pdesc = mod->files; *pdesc != NULL; pdesc++)
      umw_add_magic_desc(u, *pdesc, mod);
  }
}

struct umw *umw_open(void)
{
  struct umw *u = umw_new();

  umw_module_load_directory(u, LIBUMWSU_MODULES_PATH);

  /* module_conf_all(); */

  uwm_module_init_all(u);

  return NULL;
}

enum umw_status umw_scan_file(struct umw *umw_handle, const char *path)
{
  return UMW_MALWARE;
}

enum umw_status umw_scan_dir(struct umw *umw_handle, const char *path, int recurse)
{
  return UMW_MALWARE;
}
