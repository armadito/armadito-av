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

void umwsu_scan_add_callback(struct umwsu_scan *scan, umwsu_scan_callback_t callback, void *callback_data)
{
#if 0
  assert(scan->callback_count < MAX_CALLBACKS);

  scan->callbacks[sd->callback_count].callback = callback;
  scan->callbacks[sd->callback_count].callback_data = callback_data;

  sd->callback_count++;
#endif
}

static int get_max_threads(void)
{
  return 8;
}

struct umwsu_scan umwsu_scan_new(struct umwsu *umwsu_handle, const char *path, enum umwsu_scan_flags flags)
{
  struct umwsu_scan *sd;

  sd = (struct umwsu_scan *)malloc(sizeof(struct umwsu_scan));

  sd->u = u;
  if (flags & UMWSU_SCAN_THREADED)
    sd->thread_pool = g_thread_pool_new(scan_entry_thread, u, get_max_threads(), FALSE, NULL);
  else
    sd->thread_pool = NULL;

  sd->callback_count = 0;
  
#if 0
  umwsu_scan_add_callback(sd, scan_print_callback, NULL);
  umwsu_scan_add_callback(sd, xml_report_callback, umwsu_report_xml_new());
#endif

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

void umwsu_scan_free(struct umwsu_scan *scan)
{
  if (scan->thread_pool != NULL)
    g_thread_pool_free(scan->thread_pool, FALSE, TRUE);

  free(scan);
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

