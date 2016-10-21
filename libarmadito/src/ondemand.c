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

#include <libarmadito.h>
#include "armadito-config.h"

#include "reportp.h"
#include "ondemandp.h"
#include "scanp.h"
#include "os/file.h"
#include "os/io.h"
#include "os/string.h"
#include "os/dir.h"

#include <errno.h>
#include <glib.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#endif

static void process_error(struct a6o_scan *scan, const char *full_path, int entry_errno);

#ifdef DEBUG
const char *a6o_scan_conf_debug(struct a6o_scan_conf *c);
#endif

struct a6o_on_demand *a6o_on_demand_new(struct armadito *armadito, int scan_id, const char *root_path, enum a6o_scan_flags flags)
{
	struct a6o_on_demand *on_demand = (struct a6o_on_demand *)malloc(sizeof(struct a6o_on_demand));

	/* in future, can have many scan configurations */
	on_demand->scan_conf = a6o_scan_conf_on_demand();
	on_demand->scan = a6o_scan_new(armadito, scan_id);
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

struct a6o_scan *a6o_on_demand_get_scan(struct a6o_on_demand *on_demand)
{
	return on_demand->scan;
}

/* DIRTY */
static int cancel = 0;

void a6o_on_demand_cancel(struct a6o_on_demand *on_demand)
{
  a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "-- on_demand_cancel call !");
  cancel = 1;

}

static int a6o_on_demand_is_canceled(struct a6o_on_demand *on_demand)
{
 // a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "cancel = %d", cancel);
 
  return cancel;
}

static void scan_file(struct a6o_on_demand *on_demand, const char *path)
{
	struct a6o_file_context file_context;
	enum a6o_file_context_status context_status;

	context_status = a6o_file_context_get(&file_context, -1, path, on_demand->scan_conf);

	if (context_status == ARMADITO_FC_MUST_SCAN)
		a6o_scan_context(on_demand->scan, &file_context);
	else if (context_status == ARMADITO_FC_FILE_OPEN_ERROR)
		process_error(on_demand->scan, path, errno);

	a6o_file_context_destroy(&file_context);
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
	if(!cancel)
	   scan_file(on_demand, path);

	/* path was strdup'ed, so free it */
	free(path);

#ifdef _WIN32
	if (Wow64RevertWow64FsRedirection(OldValue) == FALSE ){
		return;
	}
#endif
}

/* this function is called when an error is found during directory traversal */
static void process_error(struct a6o_scan *scan, const char *full_path, int entry_errno)
{
	struct a6o_report report;

	a6o_report_init(&report, scan->scan_id, full_path, REPORT_PROGRESS_UNKNOWN);

	a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "error processing %s (error %s)", full_path, os_strerror(entry_errno));

	report.status = ARMADITO_IERROR;
	report.mod_report = os_strdup(os_strerror(entry_errno));
	a6o_scan_call_callbacks(scan, &report);

	a6o_report_destroy(&report);
}

/* scan one entry of the directory traversal */
/* entry can be either a directory, a file or anything else */
/* we scan only plain files, but also signal errors */
static int scan_entry(const char *full_path, enum os_file_flag flags, int entry_errno, void *data)
{
	struct a6o_on_demand *on_demand = (struct a6o_on_demand *)data;
	int canceled = a6o_on_demand_is_canceled(on_demand);

	if (canceled){
		//a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "scan canceled on path %s", full_path);
		return 2;
	}

	if (flags & FILE_FLAG_IS_ERROR) {
		process_error(on_demand->scan, full_path, entry_errno);
		return 1;
	}

	if (!(flags & FILE_FLAG_IS_PLAIN_FILE))
		return 1;

	/* if scan is multi thread, just queue the scan to the thread pool, otherwise do it here */
	if (on_demand->flags & ARMADITO_SCAN_THREADED){
		
		if(full_path != NULL)
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
	int recurse = on_demand->flags & ARMADITO_SCAN_RECURSE;
	int count = 0;

#ifdef WIN32
	void * OldValue = NULL;
	if (Wow64DisableWow64FsRedirection(&OldValue) == FALSE) {
		return NULL;
	}
#endif

	os_dir_map(on_demand->root_path, recurse, count_entry, &count);
	/* set the counter inside the a6o_scan struct only at the end, so */
	/* that the scan function does not see the intermediate values, only the last one */
	on_demand->scan->to_scan_count = count;

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

/* this function is called at the end of a scan, to send the 100% progress */
static void final_progress(struct a6o_scan *scan)
{
	struct a6o_report report;

	a6o_report_init(&report, scan->scan_id, NULL, 100);

        report.scanned_count = scan->scanned_count;
        report.suspicious_count = scan->suspicious_count;	
        report.malware_count = scan->malware_count;

	a6o_scan_call_callbacks(scan, &report);
	a6o_report_destroy(&report);
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

	a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_INFO, "starting %sthreaded scan of %s",
		on_demand->flags & ARMADITO_SCAN_THREADED ? "" : "non-",
		on_demand->root_path);

	/* create the thread pool now */
	if (on_demand->flags & ARMADITO_SCAN_THREADED)
		on_demand->thread_pool = g_thread_pool_new(scan_entry_thread_fun, on_demand, get_max_threads(), FALSE, NULL);

	/* what is scan root_path? a file or a directory? */
	os_file_stat(on_demand->root_path, &stat_buf, &stat_errno);

	/* it is a file, scan it, in a thread if scan is threaded */
	/* otherwise, walk through the directory and apply 'scan_entry' function to each entry (either file or directory) */
	if (stat_buf.flags & FILE_FLAG_IS_PLAIN_FILE) {
		on_demand->scan->to_scan_count = 1;
		if (on_demand->flags & ARMADITO_SCAN_THREADED)
			g_thread_pool_push(on_demand->thread_pool, (gpointer)os_strdup(on_demand->root_path), NULL);
		else
			scan_file(on_demand, on_demand->root_path);
	} else if (stat_buf.flags & FILE_FLAG_IS_DIRECTORY) {
		int recurse = on_demand->flags & ARMADITO_SCAN_RECURSE;

		count_to_scan(on_demand);
		os_dir_map(on_demand->root_path, recurse, scan_entry, on_demand);
	}

	/* if threaded, free the thread_pool */
	/* this has a side effect to wait for completion of *all* the scans queue'd in the thread pool */
	if (on_demand->flags & ARMADITO_SCAN_THREADED)
		g_thread_pool_free(on_demand->thread_pool, FALSE, TRUE);

	/* send the final progress (100%) */
	final_progress(on_demand->scan);

	if (on_demand->count_thread != NULL)
		g_thread_join(on_demand->count_thread);
}

void a6o_on_demand_free(struct a6o_on_demand *on_demand)
{
	/* (FD) NO!!!!! yet, scan_conf are allocated once for all and must NOT be freed, */
	/* otherwise next on-demand scan will get a memory corruption */
	// a6o_scan_conf_free(on_demand->scan_conf);

	cancel = 0;

	a6o_scan_free(on_demand->scan);

	free(on_demand);
}

