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
  GSList *scan_callbacks;
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
  u->scan_callbacks = NULL;

  return u;
}

void umwsu_set_verbose(struct umwsu *u, int verbosity)
{
  u->verbosity = verbosity;
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

static GPtrArray *get_applicable_modules(struct umwsu *u, magic_t magic, const char *path)
{
  const char *mime_type;

  if (magic == NULL)
    magic = u->magic;

  mime_type = magic_file(magic, path);

  return (GPtrArray *)g_hash_table_lookup(u->mime_types_table, mime_type);
}

enum umwsu_status umwsu_scan_file(struct umwsu *u, magic_t magic, const char *path, struct umwsu_report *report)
{
  GPtrArray *mod_array = get_applicable_modules(u, magic, path);
  enum umwsu_status current_status = UMWSU_CLEAN;

  umwsu_report_init(report, path);

  if (mod_array == NULL) {
    current_status = UMWSU_UNKNOWN_FILE_TYPE;
    report->status = current_status;
  } else {
    int i;

    for (i = 0; i < mod_array->len; i++) {
      struct umwsu_module *mod = (struct umwsu_module *)g_ptr_array_index(mod_array, i);
      enum umwsu_status mod_status;
      char *mod_report = NULL;

      if (u->verbosity >= 2)
	printf("UMWSU: module %s: scanning %s\n", mod->name, path);

      mod_status = (*mod->scan)(path, mod->data, &mod_report);

      if (umwsu_status_cmp(current_status, mod_status) < 0) {
	current_status = mod_status;
	umwsu_report_change(report, mod_status, (char *)mod->name, mod_report);
      } else if (mod_report != NULL)
	free(mod_report);

      if (current_status == UMWSU_MALWARE)
	break;
    }
  }

  if (u->verbosity >= 3)
    printf("%s: %s\n", path, umwsu_status_str(current_status));

  return current_status;
}

/*
  Unfortunately, libmagic is not thread-safe.
  We create a new magic_t for each thread, and keep it 
  in thread's private data, so that it is created only once.
 */

static void magic_destroy_notify(gpointer data)
{
  magic_close((magic_t)data);
}

static GPrivate private_magic = G_PRIVATE_INIT(magic_destroy_notify);

static magic_t get_private_magic(void)
{
  magic_t m;

  m = (magic_t)g_private_get(&private_magic);

  if (m == NULL) {
    m = magic_open(MAGIC_MIME_TYPE);
    magic_load(m, NULL);

    g_private_set(&private_magic, (gpointer)m);
  }

  return m;
}

void scan_entry_thread(gpointer data, gpointer user_data)
{
  struct umwsu *u = (struct umwsu *)user_data;
  char *path = (char *)data;
  struct umwsu_report report;

  umwsu_scan_file(u, get_private_magic(), path, &report);
  umwsu_report_print(&report, stdout);
  umwsu_report_destroy(&report);

  free(path);
}

struct scan_data {
  struct umwsu *u;
  GThreadPool *thread_pool;  
};

static void scan_entry_threaded(const char *full_path, const struct dirent *dir_entry, void *data)
{
  struct scan_data *sd = (struct scan_data *)data;

  if (dir_entry->d_type == DT_DIR)
    return;

  g_thread_pool_push(sd->thread_pool, (gpointer)strdup(full_path), NULL);
}

static void scan_entry_non_threaded(const char *full_path, const struct dirent *dir_entry, void *data)
{
  struct scan_data *sd = (struct scan_data *)data;
  struct umwsu_report report;

  if (dir_entry->d_type == DT_DIR)
    return;

  umwsu_scan_file(sd->u, NULL, full_path, &report);
  umwsu_report_print(&report, stdout);
  umwsu_report_destroy(&report);
}

static int get_max_threads(void)
{
  return 8;
}

enum umwsu_status umwsu_scan_dir(struct umwsu *u, const char *path, int recurse, int threaded)
{
  struct scan_data sd;

  sd.u = u;

  if (threaded) {
    sd.thread_pool = g_thread_pool_new(scan_entry_thread, u, get_max_threads(), FALSE, NULL);

    dir_map(path, recurse, scan_entry_threaded, &sd);

    g_thread_pool_free(sd.thread_pool, FALSE, TRUE);
  }
  else
    dir_map(path, recurse, scan_entry_non_threaded, &sd);

  return UMWSU_CLEAN;
}

