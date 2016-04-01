#ifndef _LIBARMADITO_ONDEMANDP_H_
#define _LIBARMADITO_ONDEMANDP_H_

#include <glib.h>

struct a6o_on_demand {
	struct a6o_scan_conf *scan_conf;
	struct a6o_scan *scan;
	GThread *count_thread;              /* thread used to count the files to compute progress */
	const char *root_path;              /* root path of the scan */
	enum a6o_scan_flags flags;        /* scan flags (recursive, threaded, etc) */
	GThreadPool *thread_pool;           /* the thread pool if multi-threaded */
};

#endif
