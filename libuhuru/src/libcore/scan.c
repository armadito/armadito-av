#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "conf.h"
#include "os/dir.h"
#include "os/file.h"
#include "os/io.h"
#include "os/mimetype.h"
#include "os/string.h"
#include "modulep.h"
#include "statusp.h"
#include "reportp.h"
#include "uhurup.h"
#ifdef HAVE_ALERT_MODULE
#include "builtin-modules/alert.h"
#endif
#ifdef HAVE_QUARANTINE_MODULE
#include "builtin-modules/quarantine.h"
#endif

#include <assert.h>
#include <errno.h>
#include <glib.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
/* for now, dirty while debugging on-access on linux */
#ifdef linux
#include <unistd.h>
#endif


struct callback_entry {
  uhuru_scan_callback_t callback;
  void *callback_data;
};

struct uhuru_scan {
  struct uhuru *uhuru;
  const char *path;
  enum uhuru_scan_flags flags;
  GThreadPool *thread_pool;  
  GArray *callbacks;
};

/* later, these modules will be loaded dynamically and modules will export  */

/* a 'post_scan_fun' that will be called automatically after scanning a file */
/* so that there will be no need to add the callbacks by hand */
static void uhuru_scan_add_builtin_callbacks(struct uhuru_scan *scan)
{
#ifdef HAVE_ALERT_MODULE
  struct uhuru_module *alert_module;
#endif
#ifdef HAVE_QUARANTINE_MODULE
  struct uhuru_module *quarantine_module;
#endif

#ifdef HAVE_ALERT_MODULE
  alert_module = uhuru_get_module_by_name(scan->uhuru, "alert");
  uhuru_scan_add_callback(scan, alert_callback, alert_module->data);
#endif

#ifdef HAVE_QUARANTINE_MODULE
  quarantine_module = uhuru_get_module_by_name(scan->uhuru, "quarantine");
  uhuru_scan_add_callback(scan, quarantine_callback, quarantine_module->data);
#endif
}

struct uhuru_scan *uhuru_scan_new(struct uhuru *uhuru, const char *path, enum uhuru_scan_flags flags)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)malloc(sizeof(struct uhuru_scan));

  scan->uhuru = uhuru;

  scan->path = NULL;

#ifdef HAVE_REALPATH
  if (path != NULL) {
    scan->path = (const char *)realpath(path, NULL);
    if (scan->path == NULL) {
      perror("realpath");
      free(scan);
      return NULL;
    }
  }
#else
  if (path != NULL)
    scan->path = os_strdup(path);
#endif

  scan->flags = flags;
  scan->thread_pool = NULL;

  scan->callbacks = g_array_new(FALSE, FALSE, sizeof(struct callback_entry));
  uhuru_scan_add_builtin_callbacks(scan);

  return scan;
}

void uhuru_scan_add_callback(struct uhuru_scan *scan, uhuru_scan_callback_t callback, void *callback_data)
{
  struct callback_entry entry;

  entry.callback = callback;
  entry.callback_data = callback_data;

  g_array_append_val(scan->callbacks, entry);
}

static void uhuru_scan_call_callbacks(struct uhuru_scan *scan, struct uhuru_report *report)
{
  int i;

  for(i = 0; i < scan->callbacks->len; i++) {
    struct callback_entry *entry = &g_array_index(scan->callbacks, struct callback_entry, i);
    uhuru_scan_callback_t callback = entry->callback;

    (*callback)(report, entry->callback_data);
  }
}

#ifdef linux
static const char *get_access_mode(int access_mode)
{
  switch(access_mode) {
  case O_RDONLY:
    return "O_RDONLY";
  case O_WRONLY:
    return "O_WRONLY";
  case O_RDWR:
	 return "O_RDWR";
  }

  return "UNKNOWN";
}

static void check_fd(int fd)
{
  int flags = fcntl(fd, F_GETFL);

  if (flags < 0) {
    g_log(NULL, G_LOG_LEVEL_DEBUG, "cannot get file flags for fd %d (%s)", fd, os_strerror(errno));
  } else {
    g_log(NULL, G_LOG_LEVEL_DEBUG, "file fd %d access_mode %s", fd, get_access_mode(flags & O_ACCMODE));
  }
}
#endif
#ifdef WIN32
static void check_fd(int fd)
{
}
#endif

static enum uhuru_file_status scan_apply_modules(int fd, const char *path, const char *mime_type, struct uhuru_module **modules,  struct uhuru_report *report)
{
  enum uhuru_file_status current_status = UHURU_UNDECIDED;

  for (; *modules != NULL; modules++) {
    struct uhuru_module *mod = *modules;
    enum uhuru_file_status mod_status;
    char *mod_report = NULL;

    if (mod->status != UHURU_MOD_OK)
      continue;

    check_fd(fd);
    if (os_lseek(fd, 0L, SEEK_SET) < 0) {
      g_log(NULL, G_LOG_LEVEL_WARNING, "cannot lseek file %s Error - %s", path, os_strerror(errno));
      return UHURU_IERROR;
    }

    mod_status = (*mod->scan_fun)(mod, fd, mime_type, &mod_report);

    if (uhuru_file_status_cmp(current_status, mod_status) < 0) {
      current_status = mod_status;
      uhuru_report_change(report, mod_status, (char *)mod->name, mod_report);
    } else if (mod_report != NULL)
      free(mod_report);

    if (current_status == UHURU_WHITE_LISTED || current_status == UHURU_MALWARE)
      break;
  }

  return current_status;
}

static enum uhuru_file_status scan_fd(struct uhuru_scan *scan, int fd, const char *path)
{
  struct uhuru_report report;
  struct uhuru_module **modules;
  const char *mime_type;
  enum uhuru_file_status status;

  g_log(NULL, G_LOG_LEVEL_DEBUG, "uhuru_scan_fd - %d (path %s)", fd, (path != NULL) ? path : "unknown");

  uhuru_report_init(&report, path);

  check_fd(fd);
  mime_type = os_mime_type_guess_fd(fd);
  check_fd(fd);

  modules = uhuru_get_applicable_modules(scan->uhuru, mime_type);

  if (modules == NULL)
    status = UHURU_UNKNOWN_FILE_TYPE;
  else
    status = scan_apply_modules(fd, path, mime_type, modules, &report);

  uhuru_scan_call_callbacks(scan, &report);

  uhuru_report_destroy(&report);

  return status;
}

static enum uhuru_file_status scan_file_path(struct uhuru_scan *scan, const char *path)
{
	int fd = -1;

  g_log(NULL, G_LOG_LEVEL_DEBUG, "scan_path - %s", path);

#ifdef linux
  /* FIXME: on windows, must find a way to OR the flag with _O_BINARY */
  fd = os_open(path, O_RDONLY);
#endif

  if (fd < 0) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "cannot open file %s Error - %d", path, errno);
    return UHURU_IERROR;
  }

  return scan_fd(scan, fd, path);
}

static void scan_entry_thread_fun(gpointer data, gpointer user_data)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)user_data;
  char *path = (char *)data;

  scan_file_path(scan, path);

  free(path);
}

static void process_error(struct uhuru_scan *scan, const char *full_path, int entry_errno)
{
  struct uhuru_report report;

  uhuru_report_init(&report, full_path);

  g_log(NULL, G_LOG_LEVEL_WARNING, "local_scan_entry: Error - %s", full_path);

  report.status = UHURU_IERROR;
  report.mod_report = os_strdup(os_strerror(entry_errno));
  uhuru_scan_call_callbacks(scan, &report);

  uhuru_report_destroy(&report);
}

static void scan_entry(const char *full_path, enum os_file_flag flags, int entry_errno, void *data)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)data;

  if (flags & FILE_FLAG_IS_ERROR) {
    process_error(scan, full_path, entry_errno);
    return;
  }

  if (!(flags & FILE_FLAG_IS_PLAIN_FILE))
    return;

  if (scan->flags & UHURU_SCAN_THREADED)
    g_thread_pool_push(scan->thread_pool, (gpointer)os_strdup(full_path), NULL);
  else
    scan_file_path(scan, full_path);
}

static int get_max_threads(void)
{
  return 8;
}

void uhuru_scan_run(struct uhuru_scan *scan)
{
  struct os_file_stat stat_buf;
  int stat_errno;

  if (scan->flags & UHURU_SCAN_THREADED)
    scan->thread_pool = g_thread_pool_new(scan_entry_thread_fun, scan, get_max_threads(), FALSE, NULL);

  os_file_stat(scan->path, &stat_buf, &stat_errno);

  if (stat_buf.flags & FILE_FLAG_IS_PLAIN_FILE) {
    if (scan->flags & UHURU_SCAN_THREADED)
      g_thread_pool_push(scan->thread_pool, (gpointer)os_strdup(scan->path), NULL);
    else
      scan_file_path(scan, scan->path);
  } else if (stat_buf.flags & FILE_FLAG_IS_DIRECTORY) {
    int recurse = scan->flags & UHURU_SCAN_RECURSE;

    os_dir_map(scan->path, recurse, scan_entry, scan);
  }

  if (scan->flags & UHURU_SCAN_THREADED)
    g_thread_pool_free(scan->thread_pool, FALSE, TRUE);
}

void uhuru_scan_free(struct uhuru_scan *scan)
{
  free((char *)scan->path);

 g_array_free(scan->callbacks, TRUE);

  free(scan);
}

enum uhuru_file_status uhuru_scan_fd(struct uhuru *uhuru, int fd, const char *path)
{
  struct uhuru_scan *scan = uhuru_scan_new(uhuru, NULL, 0);
  enum uhuru_file_status status;

  status = scan_fd(scan, fd, path);

  uhuru_scan_free(scan);

  return status;
}