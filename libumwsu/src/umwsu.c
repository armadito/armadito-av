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
  GSList *scan_callbacks;
};

#define MAX_CALLBACKS 4

struct umwsu_scan {
  struct umwsu *u;
  GThreadPool *thread_pool;  
  int callback_count;
  struct {
    umwsu_scan_callback_t callback;
    void *callback_data;
  } callbacks[MAX_CALLBACKS];
};

void scan_entry_thread(gpointer data, gpointer user_data);

static void umwsu_scan_add_callback(struct umwsu_scan *sd, umwsu_scan_callback_t callback, void **callback_data)
{
  assert(sd->callback_count < MAX_CALLBACKS);

  sd->callbacks[sd->callback_count].callback = callback;
  sd->callbacks[sd->callback_count].callback_data = callback_data;

  sd->callback_count++;
}

static int get_max_threads(void)
{
  return 8;
}

static struct umwsu_scan *umwsu_scan_new(struct umwsu *u, int threaded, umwsu_scan_callback_t callback, void *callback_data)
{
  struct umwsu_scan *sd;

  sd = (struct umwsu_scan *)malloc(sizeof(struct umwsu_scan));

  sd->u = u;
  if (threaded)
    sd->thread_pool = g_thread_pool_new(scan_entry_thread, u, get_max_threads(), FALSE, NULL);
  else
    sd->thread_pool = NULL;

  sd->callback_count = 0;
  
#if 0
  umwsu_scan_add_callback(sd, scan_print_callback, NULL);
  umwsu_scan_add_callback(sd, xml_report_callback, umwsu_report_xml_new());
#endif
  umwsu_scan_add_callback(sd, callback, callback_data);

  return sd;
}

static void umwsu_scan_call_callbacks(struct umwsu_scan *sd, struct umwsu_report *report)
{
  int i;

  for(i = 0; i < sd->callback_count; i++) {
    umwsu_scan_callback_t callback = sd->callbacks[i].callback;

    (*callback)(sd->u, report, sd->callbacks[i].callback_data);
  }
}

static void umwsu_scan_free(struct umwsu_scan *sd)
{
  if (sd->thread_pool != NULL)
    g_thread_pool_free(sd->thread_pool, FALSE, TRUE);

  free(sd);
}


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

enum umwsu_status umwsu_scan_file(struct umwsu *u, magic_t magic, const char *path)
{
  GPtrArray *mod_array = get_applicable_modules(u, magic, path);
  enum umwsu_status current_status = UMWSU_CLEAN;
  struct umwsu_report report;

  umwsu_report_init(&report, path);

  if (mod_array == NULL) {
    current_status = UMWSU_UNKNOWN_FILE_TYPE;
    report.status = current_status;
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
	umwsu_report_change(&report, mod_status, (char *)mod->name, mod_report);
      } else if (mod_report != NULL)
	free(mod_report);

      if (current_status == UMWSU_MALWARE)
	break;
    }
  }

  if (u->verbosity >= 3)
    printf("%s: %s\n", path, umwsu_status_str(current_status));

  umwsu_report_print(&report, stdout);

  /* call the callbacks here */
  /* ... */

  umwsu_report_destroy(&report);

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

  umwsu_scan_file(u, get_private_magic(), path);

  free(path);
}

static void scan_entry_threaded(const char *full_path, const struct dirent *dir_entry, void *data)
{
  struct umwsu_scan *sd = (struct umwsu_scan *)data;

  if (dir_entry->d_type == DT_DIR)
    return;

  g_thread_pool_push(sd->thread_pool, (gpointer)strdup(full_path), NULL);
}

static void scan_entry_non_threaded(const char *full_path, const struct dirent *dir_entry, void *data)
{
  struct umwsu_scan *sd = (struct umwsu_scan *)data;

  if (dir_entry->d_type == DT_DIR)
    return;

  umwsu_scan_file(sd->u, NULL, full_path);
}

enum umwsu_status umwsu_scan_dir(struct umwsu *u, const char *path, int recurse, int threaded, umwsu_scan_callback_t callback, void **callback_data )
{
  struct umwsu_scan *sd;

  sd = umwsu_scan_new(u, threaded, callback, callback_data);

  dir_map(path, recurse, (threaded) ? scan_entry_threaded : scan_entry_non_threaded, &sd);

  umwsu_scan_free(sd);

  return UMWSU_CLEAN;
}

