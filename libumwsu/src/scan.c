#include <libumwsu/module.h>
#include <libumwsu/scan.h>
#include "alert.h"
#include "dir.h"
#include "modulep.h"
#include "umwsup.h"
#include "statusp.h"

#include <assert.h>
#include <glib.h>
#include <magic.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

struct callback_entry {
  umwsu_scan_callback_t callback;
  void *callback_data;
};

struct umwsu_scan {
  struct umwsu *u;
  const char *path;
  enum umwsu_scan_flags flags;
  GThreadPool *thread_pool;  
  GPrivate *private_magic_key;
  GArray *callbacks;
};

static void scan_entry_thread(gpointer data, gpointer user_data);

static int get_max_threads(void)
{
  return 8;
}

/* Unfortunately, libmagic is not thread-safe. */
/* We create a new magic_t for each thread, and keep it  */
/* in thread's private data, so that it is created only once. */
static void magic_destroy_notify(gpointer data)
{
  magic_close((magic_t)data);
}

struct umwsu_scan *umwsu_scan_new(struct umwsu *umwsu_handle, const char *path, enum umwsu_scan_flags flags)
{
  struct umwsu_scan *scan;

  scan = (struct umwsu_scan *)malloc(sizeof(struct umwsu_scan));

  scan->u = umwsu_handle;
  scan->path = (const char *)strdup(path);

  scan->flags = flags;

  if (scan->flags & UMWSU_SCAN_THREADED) {
    scan->thread_pool = g_thread_pool_new(scan_entry_thread, scan, get_max_threads(), FALSE, NULL);
    scan->private_magic_key = g_private_new(magic_destroy_notify);
  } else {
    scan->thread_pool = NULL;
    scan->private_magic_key = NULL;
  }

  scan->callbacks = g_array_new(FALSE, FALSE, sizeof(struct callback_entry));

  umwsu_scan_add_callback(scan, alert_callback, NULL);

  return scan;
}

void umwsu_scan_add_callback(struct umwsu_scan *scan, umwsu_scan_callback_t callback, void *callback_data)
{
  struct callback_entry entry;

  entry.callback = callback;
  entry.callback_data = callback_data;

  g_array_append_val(scan->callbacks, entry);
}

static void umwsu_scan_call_callbacks(struct umwsu_scan *scan, struct umwsu_report *report)
{
  int i;

  for(i = 0; i < scan->callbacks->len; i++) {
    struct callback_entry *entry = &g_array_index(scan->callbacks, struct callback_entry, i);
    umwsu_scan_callback_t callback = entry->callback;

    (*callback)(report, entry->callback_data);
  }
}

static enum umwsu_status umwsu_scan_apply_modules(const char *path, GPtrArray *mod_array,  struct umwsu_report *report)
{
  enum umwsu_status current_status = UMWSU_UNDECIDED;

  if (mod_array == NULL) {
    current_status = UMWSU_UNKNOWN_FILE_TYPE;
    report->status = current_status;
  } else {
    int i;

    for (i = 0; i < mod_array->len; i++) {
      struct umwsu_module *mod = (struct umwsu_module *)g_ptr_array_index(mod_array, i);
      enum umwsu_status mod_status;
      char *mod_report = NULL;

#if 0
      if (umwsu_get_verbose(u) >= 2)
	printf("UMWSU: module %s: scanning %s\n", mod->name, path);
#endif

      mod_status = (*mod->scan)(path, mod->data, &mod_report);

#if 0
      printf("UMWSU: module %s: scanning %s -> %s\n", mod->name, path, umwsu_status_str(mod_status));
#endif

      if (umwsu_status_cmp(current_status, mod_status) < 0) {
	current_status = mod_status;
	umwsu_report_change(report, mod_status, (char *)mod->name, mod_report);
      } else if (mod_report != NULL)
	free(mod_report);

#if 0
      printf("UMWSU: current status %s\n", umwsu_status_str(current_status));
#endif

      if (current_status == UMWSU_WHITE_LISTED || current_status == UMWSU_MALWARE)
	break;
    }
  }

  return current_status;
}

static enum umwsu_status umwsu_scan_file(struct umwsu_scan *scan, magic_t magic, const char *path)
{
  enum umwsu_status status;
  struct umwsu_report report;

  umwsu_report_init(&report, path);

  status = umwsu_scan_apply_modules(path, umwsu_get_applicable_modules(scan->u, magic, path), &report);

  if (umwsu_get_verbose(scan->u) >= 3)
    printf("%s: %s\n", path, umwsu_status_str(status));

  umwsu_scan_call_callbacks(scan, &report);

  umwsu_report_destroy(&report);

  return status;
}

static magic_t get_private_magic(struct umwsu_scan *scan)
{
  magic_t m = (magic_t)g_private_get(scan->private_magic_key);

  if (m == NULL) {
    m = magic_open(MAGIC_MIME_TYPE);
    magic_load(m, NULL);

    g_private_set(scan->private_magic_key, (gpointer)m);
  }

  return m;
}

static void scan_entry_thread(gpointer data, gpointer user_data)
{
  struct umwsu_scan *scan = (struct umwsu_scan *)user_data;
  char *path = (char *)data;

  umwsu_scan_file(scan, get_private_magic(scan), path);

  free(path);
}

static void scan_entry_threaded(const char *full_path, const struct dirent *dir_entry, void *data)
{
  struct umwsu_scan *scan = (struct umwsu_scan *)data;

  if (dir_entry->d_type == DT_DIR)
    return;

  g_thread_pool_push(scan->thread_pool, (gpointer)strdup(full_path), NULL);
}

static void scan_entry_non_threaded(const char *full_path, const struct dirent *dir_entry, void *data)
{
  struct umwsu_scan *scan = (struct umwsu_scan *)data;

  if (dir_entry->d_type == DT_DIR)
    return;

  umwsu_scan_file(scan, NULL, full_path);
}

enum umwsu_status umwsu_scan_start(struct umwsu_scan *scan)
{
  struct stat sb;

  if (stat(scan->path, &sb) == -1) {
    perror("stat");
    /* exit(EXIT_FAILURE); */
  }

  if (S_ISREG(sb.st_mode))
    return umwsu_scan_file(scan, NULL, scan->path);

  if (S_ISDIR(sb.st_mode)) {
    int threaded = scan->flags  & UMWSU_SCAN_THREADED;
    int recurse = scan->flags  & UMWSU_SCAN_RECURSE;

    dir_map(scan->path, recurse, (threaded) ? scan_entry_threaded : scan_entry_non_threaded, scan);

    return UMWSU_CLEAN;
  }

  return UMWSU_EINVAL;
}

void umwsu_scan_finish(struct umwsu_scan *scan)
{
  if (scan->thread_pool != NULL)
    g_thread_pool_free(scan->thread_pool, FALSE, TRUE);
}

void umwsu_scan_free(struct umwsu_scan *scan)
{
  free((char *)scan->path);

  g_array_free(scan->callbacks, TRUE);

  free(scan);
}
