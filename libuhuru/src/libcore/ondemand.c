#include <libuhuru/core.h>
#include "libuhuru-config.h"

#include "ondemandp.h"
#include "scanp.h"
#include "os/file.h"
#include "os/io.h"
#include "os/string.h"

#include <errno.h>
#include <glib.h>
#include <stdlib.h>

struct uhuru_on_demand *uhuru_on_demand_new(struct uhuru *uhuru, int scan_id, const char *root_path, enum uhuru_scan_flags flags)
{
  struct uhuru_on_demand *on_demand = (struct uhuru_on_demand *)malloc(sizeof(struct uhuru_on_demand));

  /* in future, can have many scan configurations */
  on_demand->scan_conf = uhuru_scan_conf_on_demand();

  on_demand->scan = uhuru_scan_new(uhuru, scan_id);

  on_demand->count_thread = NULL;

#ifdef HAVE_REALPATH
  on_demand->root_path = (const char *)realpath(root_path, NULL);
  if (on_demand->root_path == NULL) {
    perror("realpath");
    free(on_demand);
    return NULL;
  }
#else
  on_demand->root_path = os_strdup(root_path);
#endif

  on_demand->flags = flags;

  on_demand->thread_pool = NULL;

  return on_demand;
}

struct uhuru_scan *uhuru_on_demand_get_scan(struct uhuru_on_demand *on_demand)
{
  return on_demand->scan;
}

/* the thread function called by the thread pool, in case of threaded scan */
static void scan_entry_thread_fun(gpointer data, gpointer user_data)
{
  struct uhuru_file_context *file_context = (struct uhuru_file_context *)data;
  struct uhuru_scan *scan = (struct uhuru_scan *)user_data;
  enum uhuru_file_status status;
	
  status = uhuru_scan_context(scan, file_context);

  uhuru_file_context_free(file_context);
}

/* this function is called when an error is found during directory traversal */
static void process_error(struct uhuru_scan *scan, const char *full_path, int entry_errno)
{
  struct uhuru_report report;

  uhuru_report_init(&report, scan->scan_id, full_path, REPORT_PROGRESS_UNKNOWN);

  uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "local_scan_entry: Error - %s", full_path);

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
  struct uhuru_on_demand *on_demand = (struct uhuru_on_demand *)data;
  struct uhuru_file_context file_context;
  enum uhuru_file_context_status context_status;
  int fd;

  if (flags & FILE_FLAG_IS_ERROR) {
    process_error(on_demand->scan, full_path, entry_errno);
    return;
  }

  if (!(flags & FILE_FLAG_IS_PLAIN_FILE))
    return;

  /* open the file */
  fd = os_open(full_path, O_RDONLY);
  if (fd < 0) {
    process_error(on_demand->scan, full_path, errno);
    return;
  }

  /* get file scan context */
  context_status = uhuru_file_context_get(&file_context, fd, full_path, on_demand->scan_conf);

  if (context_status)    /* means file must not be scanned */
    return;

  g_thread_pool_push(on_demand->thread_pool, uhuru_file_context_clone(&file_context), NULL);
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

static gpointer count_thread_fun(gpointer data)
{
  struct uhuru_on_demand *on_demand = (struct uhuru_on_demand *)data;
  int recurse = on_demand->flags & UHURU_SCAN_RECURSE;
  int count = 0;

  os_dir_map(on_demand->root_path, recurse, count_entry, &count);

  /* set the counter inside the uhuru_scan struct only at the end, so */
  /* that the scan function does not see the intermediate values, only the last one */
  on_demand->scan->to_scan_count = count;

  return NULL;
}

static void count_to_scan(struct uhuru_on_demand *on_demand)
{
#if defined(HAVE_GTHREAD_NEW)
  on_demand->count_thread = g_thread_new("to_scan_count_thread", count_thread_fun, on_demand);
#elif defined(HAVE_GTHREAD_CREATE)
  on_demand->count_thread = g_thread_create(count_thread_fun, on_demand, TRUE, NULL);
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
void uhuru_on_demand_run(struct uhuru_on_demand *on_demand)
{
  struct os_file_stat stat_buf;
  int stat_errno;

  /* create the thread pool now */
  on_demand->thread_pool = g_thread_pool_new(scan_entry_thread_fun, on_demand->scan, get_max_threads(), FALSE, NULL);

  /* what is scan root_path? a file or a directory? */
  os_file_stat(on_demand->root_path, &stat_buf, &stat_errno);

  /* it is a file, scan it, in a thread if scan is threaded */
  /* otherwise, walk through the directory and apply 'scan_entry' function to each entry (either file or directory) */
  if (stat_buf.flags & FILE_FLAG_IS_PLAIN_FILE) {
    on_demand->scan->to_scan_count = 1;

    g_thread_pool_push(on_demand->thread_pool, (gpointer)os_strdup(on_demand->root_path), NULL);

  } else if (stat_buf.flags & FILE_FLAG_IS_DIRECTORY) {
    int recurse = on_demand->flags & UHURU_SCAN_RECURSE;

    count_to_scan(on_demand);

    os_dir_map(on_demand->root_path, recurse, scan_entry, on_demand);
  }

  /* if threaded, free the thread_pool */
  /* this has a side effect to wait for completion of *all* the scans queue'd in the thread pool */
  g_thread_pool_free(on_demand->thread_pool, FALSE, TRUE);

  /* send the final progress (100%) */
  final_progress(on_demand->scan);

  if (on_demand->count_thread != NULL)
    g_thread_join(on_demand->count_thread);
}

void uhuru_on_demand_free(struct uhuru_on_demand *on_demand)
{
}

