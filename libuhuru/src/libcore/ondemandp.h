#ifndef _LIBUHURU_ONDEMANDP_H_
#define _LIBUHURU_ONDEMANDP_H_

#include <glib.h>

struct uhuru_on_demand {
  struct uhuru_scan_conf *scan_conf;
  struct uhuru_scan *scan;
  GThread *count_thread;              /* thread used to count the files to compute progress */
  const char *root_path;              /* root path of the scan */
  enum uhuru_scan_flags flags;        /* scan flags (recursive, threaded, etc) */
  GThreadPool *thread_pool;           /* the thread pool if multi-threaded */
};

#endif
