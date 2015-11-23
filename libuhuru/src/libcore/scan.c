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

struct callback_entry {
  uhuru_scan_callback_t callback;
  void *callback_data;
};

struct uhuru_scan {
  struct uhuru *uhuru;                /* pointer to uhuru handle */
  int scan_id;                        /* scan id for GUI */
  int to_scan_count;                  /* files to scan counter, to compute progress */
  int scanned_count;                  /* already scanned counter, to compute progress */
  GThread *to_scan_count_thread;      /* thread used to count the files to compute progress */
  const char *path;                   /* root path of the scan */
  enum uhuru_scan_flags flags;        /* scan flags (recursive, threaded, etc) */
  GThreadPool *thread_pool;           /* the thread pool if multi-threaded */
  GArray *callbacks;                  /* array of struct callback_entry */
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

struct uhuru_scan *uhuru_scan_new(struct uhuru *uhuru, int scan_id, const char *path, enum uhuru_scan_flags flags)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)malloc(sizeof(struct uhuru_scan));

  scan->uhuru = uhuru;
  scan->scan_id = scan_id;
  scan->to_scan_count = 0;
  scan->scanned_count = 0;
  scan->to_scan_count_thread = NULL;

#ifdef HAVE_REALPATH
  scan->path = (const char *)realpath(path, NULL);
  if (scan->path == NULL) {
    perror("realpath");
    free(scan);
    return NULL;
  }
#else
  scan->path = os_strdup(path);
#endif

  scan->flags = flags;
  scan->thread_pool = NULL;

  /* use a GArray, and not a GPtrArray, because GArray can contain structs instead of pointers to struct */
  scan->callbacks = g_array_new(FALSE, FALSE, sizeof(struct callback_entry));
  uhuru_scan_add_builtin_callbacks(scan);

  return scan;
}

void uhuru_scan_add_callback(struct uhuru_scan *scan, uhuru_scan_callback_t callback, void *callback_data)
{
  struct callback_entry entry;

  entry.callback = callback;
  entry.callback_data = callback_data;

  /* this function copies the structure that is passed as argument */
  g_array_append_val(scan->callbacks, entry);
}

/* call all the registered callbacks */
static void uhuru_scan_call_callbacks(struct uhuru_scan *scan, struct uhuru_report *report)
{
  int i;

  /* just iterate over the array of structures */
  for(i = 0; i < scan->callbacks->len; i++) {
    struct callback_entry *entry = &g_array_index(scan->callbacks, struct callback_entry, i);
    uhuru_scan_callback_t callback = entry->callback;

    (*callback)(report, entry->callback_data);
  }
}

/* apply the modules contained in 'modules' in order to compute the scan status of 'path' */
/* 'modules' is a NULL-terminated array of pointers to struct uhuru_module */
/* 'mime_type' is the mime-type of the file */
static enum uhuru_file_status scan_apply_modules(const char *path, const char *mime_type, struct uhuru_module **modules,  struct uhuru_report *report)
{
  enum uhuru_file_status current_status = UHURU_UNDECIDED;

  /* iterate over the modules */
  for (; *modules != NULL; modules++) {
    struct uhuru_module *mod = *modules;
    enum uhuru_file_status mod_status;
    char *mod_report = NULL;

	// Added by ulrich for debug.	
	//printf("[ii] Debug :: Applying module %s :: report = %d\n",mod->name,report);

    /* if module status is not OK, don't call it */
    if (mod->status != UHURU_MOD_OK)
      continue;

    /* call the scan function of the module */
    mod_status = (*mod->scan_fun)(mod, path, mime_type, &mod_report);

    /* then compare the status that was returned by the module with current status */
    /* if current status is 'less than' (see status.c for comparison meaning) */
    /* adopt the module returned status as current status */
    /* for instance, if current_status is UNDECIDED and module status is MALWARE, */
    /* current_status become MALWARE */
    if (uhuru_file_status_cmp(current_status, mod_status) < 0) {
      current_status = mod_status;
      if (report != NULL)
	uhuru_report_change(report, mod_status, (char *)mod->name, mod_report);
    } else if (mod_report != NULL)
      free(mod_report);

    /* if module has returned an authoritative status, no need to go on */
    if (current_status == UHURU_WHITE_LISTED || current_status == UHURU_MALWARE)
      break;
  }

  return current_status;
}

static int compute_progress(struct uhuru_scan *scan)
{
  int progress;

  if (scan->to_scan_count == 0)
    return REPORT_PROGRESS_UNKNOWN;

  progress = (int)((100.0 * scan->scanned_count) / scan->to_scan_count);

  if (progress > 100)
    progress = 100;

  return progress;
}

/* scan a file: */
/* - find its mime type */
/* - get the modules that apply to this mime type */
/* - apply them to scan the file */
/* - and finally call the callbacks  */
static enum uhuru_file_status scan_file(struct uhuru_scan *scan, const char *path)
{
  struct uhuru_report report;
  struct uhuru_module **modules = NULL;
  const char *mime_type;
  enum uhuru_file_status status;

  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "scan_file - %s", path);

  if (os_file_do_not_scan(path))
    return UHURU_CLEAN;

  /* initializes the structure passed to callbacks */
  uhuru_report_init(&report, scan->scan_id, path, REPORT_PROGRESS_UNKNOWN);

  /* find the mime type using OS specific function (magic_* on linux, FindMimeFromData on windows */
  mime_type = os_mime_type_guess(path);

  /* ask uhuru handle for the array of modules */
  if (mime_type != NULL)
    modules = uhuru_get_applicable_modules(scan->uhuru, mime_type);

  /* if no modules apply, then file is not handled */
  if (modules == NULL || mime_type == NULL) {
    status = UHURU_UNKNOWN_FILE_TYPE;
    uhuru_report_change(&report, status, NULL, NULL);
  } else
    /* otherwise we scan it by applying the modules */
    status = scan_apply_modules(path, mime_type, modules, &report);

  /* update the progress */
  /* may be not thread safe, but who cares about precise values? */
  scan->scanned_count++;

  report.progress = compute_progress(scan);

  /* once done, call the callbacks */
  uhuru_scan_call_callbacks(scan, &report);

  /* and free the report (it may contain a strdup'ed string) */
  uhuru_report_destroy(&report);

  if (mime_type != NULL)
    free((void *)mime_type);

  return status;
}

/* the thread function called by the thread pool, in case of threaded scan */
static void scan_entry_thread_fun(gpointer data, gpointer user_data)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)user_data;
  char *path = (char *)data;

  scan_file(scan, path);

  /* path was strdup'ed, so free it */
  free(path);
}

/* this function is called when an error is found during directory traversal */
static void process_error(struct uhuru_scan *scan, const char *full_path, int entry_errno)
{
  struct uhuru_report report;

  uhuru_report_init(&report, scan->scan_id, full_path, REPORT_PROGRESS_UNKNOWN);

  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "local_scan_entry: Error - %s", full_path);

  report.status = UHURU_IERROR;
  report.mod_report = os_strdup(os_strerror(entry_errno));
  uhuru_scan_call_callbacks(scan, &report);

  uhuru_report_destroy(&report);
}

/* scan one entry of the directory traversal */
/* entry can be either a directory, a file or anything else */
/* we scan only plain files, but also signal errors */
static void scan_entry(const char *full_path, enum os_file_flag flags, int entry_errno, void *data)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)data;

  if (flags & FILE_FLAG_IS_ERROR) {
    process_error(scan, full_path, entry_errno);
    return;
  }

  if (!(flags & FILE_FLAG_IS_PLAIN_FILE))
    return;

  /* if scan is multi thread, just queue the scan to the thread pool, otherwise do it here */
  if (scan->flags & UHURU_SCAN_THREADED)
    g_thread_pool_push(scan->thread_pool, (gpointer)os_strdup(full_path), NULL);
  else
    scan_file(scan, full_path);
}

/* dummy function, should use platform (# of cores) or configuration data */
static int get_max_threads(void)
{
  return 8;
}

static void count_entry(const char *full_path, enum os_file_flag flags, int entry_errno, void *data)
{
  int *pcount = (int *)data;

  if (!(flags & FILE_FLAG_IS_PLAIN_FILE))
    return;

  (*pcount)++;
}

static gpointer to_scan_count_thread_fun(gpointer data)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)data;
  int recurse = scan->flags & UHURU_SCAN_RECURSE;
  int count = 0;

  os_dir_map(scan->path, recurse, count_entry, &count);

  /* set the counter inside the uhuru_scan struct only at the end, so */
  /* that the scan function does not see the intermediate values, only the last one */
  scan->to_scan_count = count;

  return NULL;
}

static void to_scan_count(struct uhuru_scan *scan)
{
#if defined(HAVE_GTHREAD_NEW)
  scan->to_scan_count_thread = g_thread_new("to_scan_count_thread", to_scan_count_thread_fun, scan);
#elif defined(HAVE_GTHREAD_CREATE)
  scan->to_scan_count_thread = g_thread_create(to_scan_count_thread_fun, scan, TRUE, NULL);
#endif
}

/* this function is called at the end of a scan, to send the 100% progress */
static void final_progress(struct uhuru_scan *scan)
{
  struct uhuru_report report;

  uhuru_report_init(&report, scan->scan_id, NULL, 100);

  uhuru_scan_call_callbacks(scan, &report);

  uhuru_report_destroy(&report);
}

/* NOTE: this function has several shortcomings: */
/* - it should return also a file status for directories (but how to compute it?) */
/* - it should be made simpler by separating the file case and the directory case */
/* - it should use a thread pool also for traversing directories */
/* run a scan by traversing the directory (if scan root_path is a directory) */
/* or scanning the file (if not) */
/* blocks until scan is finished, even if scan is multi-threaded */
void uhuru_scan_run(struct uhuru_scan *scan)
{
  struct os_file_stat stat_buf;
  int stat_errno;

  /* create the thread pool now */
  if (scan->flags & UHURU_SCAN_THREADED)
    scan->thread_pool = g_thread_pool_new(scan_entry_thread_fun, scan, get_max_threads(), FALSE, NULL);

  /* what is scan root_path? a file or a directory? */
  os_file_stat(scan->path, &stat_buf, &stat_errno);

  /* it is a file, scan it, in a thread if scan is threaded */
  /* otherwise, walk through the directory and apply 'scan_entry' function to each entry (either file or directory) */
  if (stat_buf.flags & FILE_FLAG_IS_PLAIN_FILE) {
    scan->to_scan_count = 1;

    if (scan->flags & UHURU_SCAN_THREADED)
      g_thread_pool_push(scan->thread_pool, (gpointer)os_strdup(scan->path), NULL);
    else
      scan_file(scan, scan->path);
  } else if (stat_buf.flags & FILE_FLAG_IS_DIRECTORY) {
    int recurse = scan->flags & UHURU_SCAN_RECURSE;

    to_scan_count(scan);

    os_dir_map(scan->path, recurse, scan_entry, scan);
  }

  /* if threaded, free the thread_pool */
  /* this has a side effect to wait for completion of *all* the scans queue'd in the thread pool */
  if (scan->flags & UHURU_SCAN_THREADED)
    g_thread_pool_free(scan->thread_pool, FALSE, TRUE);

  /* send the final progress (100%) */
  final_progress(scan);

  if (scan->to_scan_count_thread != NULL)
    g_thread_join(scan->to_scan_count_thread);
}

/* just free the structure */
void uhuru_scan_free(struct uhuru_scan *scan)
{
  free((char *)scan->path);

  g_array_free(scan->callbacks, TRUE);

  free(scan);
}

/* the simple version, for on-access scan: */
/* no callbacks */
/* no threads */
enum uhuru_file_status uhuru_scan_simple(struct uhuru *uhuru, const char *path,  struct uhuru_report *report)
{
  struct uhuru_module **modules = NULL;
  const char *mime_type;
  enum uhuru_file_status status;

  if (os_file_do_not_scan(path))
    return UHURU_CLEAN;

  // Initialize the scan report structure.
  if (report != NULL)
	  uhuru_report_init(report, 1, path, REPORT_PROGRESS_UNKNOWN);

  /* find the mime type, as in scan_file fun */
  mime_type = os_mime_type_guess(path);

  if (mime_type != NULL)
    modules = uhuru_get_applicable_modules(uhuru, mime_type);
  
  if (modules == NULL || mime_type == NULL)
    return UHURU_UNKNOWN_FILE_TYPE;

  status = scan_apply_modules(path, mime_type, modules, report);

  free((void *)mime_type);

  return status;
}
