/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#include <libarmadito/armadito.h>
#include "armadito-config.h"

#include "core/report.h"
#include "core/ondemand.h"
#include "core/file.h"
#include "core/handle.h"
#include "core/scanctx.h"
#include "core/io.h"
#include "core/dir.h"
#include "core/event.h"

#include "string_p.h"

#include <errno.h>
#include <glib.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef _WIN32
#include <Windows.h>
#endif

struct a6o_on_demand {
	struct armadito *armadito;
	struct a6o_scan_conf *scan_conf;

	const char *root_path;              /* root path of the scan */
	unsigned int scan_id;               /* scan id for client */
	enum a6o_scan_flags flags;          /* scan flags (recursive, threaded, etc) */

	GThread *count_thread;              /* thread used to count the files to compute progress */
	GThreadPool *thread_pool;           /* the thread pool if multi-threaded */

	time_t start_time;                  /* start time in milliseconds */
	time_t duration;                    /* duration in milliseconds */

	int to_scan_count;                  /* files to scan counter, to compute progress */
	int scanned_count;                  /* already scanned counter, to compute progress */
	int malware_count;                  /* detected as malicious counter */
	int suspicious_count;               /* detected as suspicious counter */

	int was_cancelled;

	time_t progress_period;
	time_t last_progress_time;
	int last_progress_value;
};

#ifdef DEBUG
const char *a6o_scan_conf_debug(struct a6o_scan_conf *c);
#endif

#define DEFAULT_PROGRESS_PERIOD 200  /* milliseconds */

struct a6o_on_demand *a6o_on_demand_new(struct armadito *armadito, const char *root_path, unsigned int scan_id, enum a6o_scan_flags flags, int send_progress)
{
	struct a6o_on_demand *on_demand = (struct a6o_on_demand *)malloc(sizeof(struct a6o_on_demand));

	on_demand->armadito = armadito;
	on_demand->scan_conf = a6o_scan_conf_on_demand();

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
	on_demand->scan_id = scan_id;
	on_demand->flags = flags;

	on_demand->count_thread = NULL;
	on_demand->thread_pool = NULL;

	on_demand->to_scan_count = 0;
	on_demand->scanned_count = 0;
	on_demand->malware_count = 0;
	on_demand->suspicious_count = 0;

	on_demand->was_cancelled = 0;

	if (send_progress)
		on_demand->progress_period = DEFAULT_PROGRESS_PERIOD;
	else
		on_demand->progress_period = 0;
	on_demand->last_progress_time = 0L;
	on_demand->last_progress_value = A6O_ON_DEMAND_PROGRESS_UNKNOWN;

	return on_demand;
}

unsigned int a6o_on_demand_get_id(struct a6o_on_demand *on_demand)
{
	return on_demand->scan_id;
}

void a6o_on_demand_cancel(struct a6o_on_demand *on_demand)
{
  a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "-- on_demand_cancel call !");

  /* to be reimplemented cleanly */
}

static int a6o_on_demand_is_cancelled(struct a6o_on_demand *on_demand)
{
 // a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "cancel = %d", cancel);

  /* to be reimplemented cleanly */

  return 0;
}

static void update_counters(struct a6o_on_demand *on_demand, struct a6o_report *report)
{
	g_atomic_int_inc(&on_demand->scanned_count);

	switch(report->status) {
	case A6O_FILE_SUSPICIOUS:
		g_atomic_int_inc(&on_demand->suspicious_count);
		break;
	case A6O_FILE_MALWARE:
		g_atomic_int_inc(&on_demand->malware_count);
		break;
	default:
		break;
	}
}

#ifdef linux
static time_t get_milliseconds(void)
{
	struct timeval now;

	if (gettimeofday(&now, NULL) < 0) {
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_ERROR, "error getting time IHM (%s)", strerror(errno));
		return 0;
	}

	return now.tv_sec * 1000 + now.tv_usec / 1000;
}
#endif

#ifdef _WIN32
static time_t get_milliseconds( ) {

	time_t ms = 0;
	struct _timeb tb;

	_ftime64_s(&tb);
	ms = tb.time * 1000 + tb.millitm;

	return ms;
}
#endif

static int must_send_progress_event(struct a6o_on_demand *on_demand, struct a6o_report *report, int progress, time_t now)
{
	if (report->path == NULL)
		return 0;

	if (on_demand->last_progress_value == A6O_ON_DEMAND_PROGRESS_UNKNOWN)
		return 1;

	if (on_demand->last_progress_value != progress)
		return 1;

	if (on_demand->last_progress_time == 0)
		return 1;

	if((now - on_demand->last_progress_time) >= on_demand->progress_period)
		return 1;

	return 0;
}

static void fire_progress_event(struct a6o_on_demand *on_demand, struct a6o_report *report, int progress)
{
	struct a6o_on_demand_progress_event progress_ev;
	struct a6o_event *ev;

	/* should strdup? */
	progress_ev.path = report->path;
	progress_ev.scan_id = on_demand->scan_id;
	progress_ev.progress = progress;
	progress_ev.malware_count = on_demand->malware_count;
	progress_ev.suspicious_count = on_demand->suspicious_count;
	progress_ev.scanned_count = on_demand->scanned_count;

	ev = a6o_event_new(EVENT_ON_DEMAND_PROGRESS, &progress_ev);

	a6o_event_source_fire_event(a6o_get_event_source(on_demand->armadito), ev);
	a6o_event_free(ev);
}

static void update_progress(struct a6o_on_demand *on_demand, struct a6o_report *report)
{
	int progress;
	time_t now;

	if (on_demand->progress_period == 0)
		return;

	if (on_demand->to_scan_count == 0)
		return;

	progress = (int)((100.0 * on_demand->scanned_count) / on_demand->to_scan_count);

	if (progress > 100)
		progress = 100;

	now = get_milliseconds();

	if (must_send_progress_event(on_demand, report, progress, now)) {
		fire_progress_event(on_demand, report, progress);
		on_demand->last_progress_time = now;
		on_demand->last_progress_value = progress;
	}
}

static void fire_detection_event(struct a6o_on_demand *on_demand, struct a6o_report *report)
{
	struct a6o_detection_event detection_ev;
	struct a6o_event *ev;

	detection_ev.context = CONTEXT_ON_DEMAND;
	detection_ev.scan_id = on_demand->scan_id;
	/* should strdup? */
	detection_ev.path = report->path;
	detection_ev.scan_status = report->status;
	detection_ev.scan_action = report->action;
	detection_ev.module_name = report->module_name;
	detection_ev.module_report = report->module_report;

	ev = a6o_event_new(EVENT_DETECTION, &detection_ev);

	a6o_event_source_fire_event(a6o_get_event_source(on_demand->armadito), ev);
	a6o_event_free(ev);
}

static void fire_on_demand_start_event(struct a6o_on_demand *on_demand)
{
	struct a6o_on_demand_start_event start_ev;
	struct a6o_event *ev;

	start_ev.root_path = strdup(on_demand->root_path);
	start_ev.scan_id = on_demand->scan_id;

	ev = a6o_event_new(EVENT_ON_DEMAND_START, &start_ev);

	a6o_event_source_fire_event(a6o_get_event_source(on_demand->armadito), ev);
	a6o_event_free(ev);
}

static void fire_on_demand_completed_event(struct a6o_on_demand *on_demand)
{
	struct a6o_on_demand_completed_event completed_ev;
	struct a6o_event *ev;

	completed_ev.scan_id = on_demand->scan_id;
	completed_ev.cancelled = on_demand->was_cancelled;
	completed_ev.total_malware_count = on_demand->malware_count;
	completed_ev.total_suspicious_count = on_demand->suspicious_count;
	completed_ev.total_scanned_count = on_demand->scanned_count;
	completed_ev.duration = on_demand->duration;

	ev = a6o_event_new(EVENT_ON_DEMAND_COMPLETED, &completed_ev);

	a6o_event_source_fire_event(a6o_get_event_source(on_demand->armadito), ev);
	a6o_event_free(ev);
}

/* this function is called when an error is found during directory traversal */
/* or when an error occur during file scan */
static void process_error(struct a6o_on_demand *on_demand, const char *path, int entry_errno)
{
	/* FIXME */
	/* must send a detection event? */
#if 0
	struct a6o_report report;

	a6o_report_init(&report, 42 /* scan->scan_id */, full_path, A6O_ON_DEMAND_PROGRESS_UNKNOWN);
	a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "error processing %s (error %s)", full_path, os_strerror(entry_errno));

	report.status = A6O_FILE_IERROR;
	report.module_report = os_strdup(os_strerror(entry_errno));

	a6o_report_destroy(&report);
#endif
}

static void scan_file(struct a6o_on_demand *on_demand, const char *path)
{
	struct a6o_scan_context file_context;
	enum a6o_scan_context_status context_status;
	struct a6o_report report;

	context_status = a6o_scan_context_get(&file_context, -1, path, on_demand->scan_conf, &report);

	if (context_status == A6O_SC_MUST_SCAN)
		a6o_scan_context_scan(&file_context, &report);
	else if (context_status == A6O_SC_FILE_OPEN_ERROR)
		process_error(on_demand, report.path, errno);

	if ((report.status == A6O_FILE_MALWARE || report.status == A6O_FILE_SUSPICIOUS)
		&& report.path != NULL)
		fire_detection_event(on_demand, &report);

	/* update counters */
	update_counters(on_demand, &report);

	/* update progress */
	update_progress(on_demand, &report);

	a6o_scan_context_destroy(&file_context);
	a6o_report_destroy(&report);
}

/* the thread function called by the thread pool, in case of threaded scan */
static void scan_entry_thread_fun(gpointer data, gpointer user_data)
{
	struct a6o_on_demand *on_demand = (struct a6o_on_demand *)user_data;
	char *path = (char *)data;

#ifdef _WIN32
	void * OldValue = NULL;
	if (Wow64DisableWow64FsRedirection(&OldValue) == FALSE) {
		return;
	}
#endif

	/* must check cancellation */
	scan_file(on_demand, path);

	/* path was strdup'ed, so free it */
	free(path);

#ifdef _WIN32
	if (Wow64RevertWow64FsRedirection(OldValue) == FALSE ){
		return;
	}
#endif
}

/* scan one entry of the directory traversal */
/* entry can be either a directory, a file or anything else */
/* we scan only plain files, but also signal errors */
static int scan_entry(const char *full_path, enum os_file_flag flags, int entry_errno, void *data)
{
	struct a6o_on_demand *on_demand = (struct a6o_on_demand *)data;
	int canceled = a6o_on_demand_is_cancelled(on_demand);

	if (canceled){
		//a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "scan canceled on path %s", full_path);
		return 2;
	}

	if (flags & FILE_FLAG_IS_ERROR) {
		process_error(on_demand, full_path, entry_errno);
		return 1;
	}

	if (!(flags & FILE_FLAG_IS_PLAIN_FILE))
		return 1;

	/* if scan is multi thread, just queue the scan to the thread pool, otherwise do it here */
	if (on_demand->flags & A6O_SCAN_THREADED) {
		if( full_path != NULL)
		   g_thread_pool_push(on_demand->thread_pool, (gpointer)os_strdup(full_path), NULL);
		/*
		   full_path can be NULL if AV is launched as normal user and file rights are 700 for example.
		   In this case, we just skip these files to avoid segfault. To be improved.
		*/
	}
	else
		scan_file(on_demand, full_path);

	return 0;
}

/* dummy function, should use platform (# of cores) or configuration data */
static int get_max_threads(void)
{
	return 4;
}

static int count_entry(const char *full_path, enum os_file_flag flags, int entry_errno, void *data)
{
	int *pcount = (int *)data;

	if (!(flags & FILE_FLAG_IS_PLAIN_FILE))
		return 0;

	(*pcount)++;
	return 0;
}

static gpointer count_thread_fun(gpointer data)
{
	struct a6o_on_demand *on_demand = (struct a6o_on_demand *)data;
	int recurse = on_demand->flags & A6O_SCAN_RECURSE;
	int count = 0;

#ifdef WIN32
	void * OldValue = NULL;
	if (Wow64DisableWow64FsRedirection(&OldValue) == FALSE) {
		return NULL;
	}
#endif

	os_dir_map(on_demand->root_path, recurse, count_entry, &count);
	/* set the counter inside the struct only at the end, so */
	/* that the scan function does not see the intermediate values, only the last one */
	on_demand->to_scan_count = count;

#ifdef WIN32
	if (Wow64RevertWow64FsRedirection(OldValue) == FALSE ) {
		return NULL;
	}
#endif

	return NULL;
}

static void count_to_scan(struct a6o_on_demand *on_demand)
{
#if defined(HAVE_GTHREAD_NEW)
	on_demand->count_thread = g_thread_new("to_scan_count_thread", count_thread_fun, on_demand);
#elif defined(HAVE_GTHREAD_CREATE)
	on_demand->count_thread = g_thread_create(count_thread_fun, on_demand, TRUE, NULL);
#endif
}

/* NOTE: this function has several shortcomings: */
/* - it should return also a file status for directories (but how to compute it?) */
/* - it should be made simpler by separating the file case and the directory case */
/* - it should use a thread pool also for traversing directories */
/* run a scan by traversing the directory (if scan root_path is a directory) */
/* or scanning the file (if not) */
/* blocks until scan is finished, even if scan is multi-threaded */
void a6o_on_demand_run(struct a6o_on_demand *on_demand)
{
	struct os_file_stat stat_buf;
	int stat_errno;

	a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_INFO, "starting %sthreaded scan %d of %s",
		on_demand->flags & A6O_SCAN_THREADED ? "" : "non-",
		on_demand->scan_id,
		on_demand->root_path);

	on_demand->start_time = get_milliseconds();

	/* create the thread pool now */
	if (on_demand->flags & A6O_SCAN_THREADED)
		on_demand->thread_pool = g_thread_pool_new(scan_entry_thread_fun, on_demand, get_max_threads(), FALSE, NULL);

	/* signal start */
	fire_on_demand_start_event(on_demand);

	/* what is scan root_path? a file or a directory? */
	os_file_stat(on_demand->root_path, &stat_buf, &stat_errno);

	/* it is a file, scan it, in a thread if scan is threaded */
	/* otherwise, walk through the directory and apply 'scan_entry' function to each entry (either file or directory) */
	if (stat_buf.flags & FILE_FLAG_IS_PLAIN_FILE) {
		on_demand->to_scan_count = 1;

		if (on_demand->flags & A6O_SCAN_THREADED)
			g_thread_pool_push(on_demand->thread_pool, (gpointer)os_strdup(on_demand->root_path), NULL);
		else
			scan_file(on_demand, on_demand->root_path);
	} else if (stat_buf.flags & FILE_FLAG_IS_DIRECTORY) {
		int recurse = on_demand->flags & A6O_SCAN_RECURSE;

		if (on_demand->progress_period != 0)
			count_to_scan(on_demand);

		os_dir_map(on_demand->root_path, recurse, scan_entry, on_demand);
	}

	/* if threaded, free the thread_pool */
	/* this has a side effect to wait for completion of *all* the scans queue'd in the thread pool */
	if (on_demand->flags & A6O_SCAN_THREADED)
		g_thread_pool_free(on_demand->thread_pool, FALSE, TRUE);

	on_demand->duration = get_milliseconds() - on_demand->start_time;
	/* signal completion */
	fire_on_demand_completed_event(on_demand);

	if (on_demand->count_thread != NULL)
		g_thread_join(on_demand->count_thread);
}

void a6o_on_demand_free(struct a6o_on_demand *on_demand)
{
	free(on_demand);
}

