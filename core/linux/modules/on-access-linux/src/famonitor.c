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

#define _GNU_SOURCE

#include <libarmadito.h>
#include <libarmadito-config.h>

#include "monitor.h"
#include "response.h"
#include "watchdog.h"
#include "onaccessmod.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <limits.h>
#include <sys/fanotify.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* #undef ENABLE_THREAD_POOL */
#define ENABLE_THREAD_POOL

struct fanotify_monitor {
	struct access_monitor *monitor;
	struct armadito *armadito;

	struct a6o_scan_conf *scan_conf;

	pid_t my_pid;

	int fanotify_fd;

	GThreadPool *thread_pool;

	struct watchdog *watchdog;
};

static gboolean fanotify_cb(GIOChannel *source, GIOCondition condition, gpointer data);
static void scan_file_thread_fun(gpointer data, gpointer user_data);

struct fanotify_monitor *fanotify_monitor_new(struct access_monitor *m, struct armadito *u)
{
	struct fanotify_monitor *f = malloc(sizeof(struct fanotify_monitor));

	f->monitor = m;
	f->armadito = u;

	f->scan_conf = a6o_scan_conf_on_access();
	f->my_pid = getpid();

	return f;
}

int fanotify_monitor_start(struct fanotify_monitor *f)
{
	GIOChannel *fanotify_channel;
	GSource *source;

	f->fanotify_fd = fanotify_init(FAN_CLASS_CONTENT | FAN_UNLIMITED_QUEUE | FAN_UNLIMITED_MARKS, O_LARGEFILE | O_RDONLY);

	if (f->fanotify_fd < 0) {
		if (errno == EPERM)
			a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "you must be root or have CAP_SYS_ADMIN capability to enable on-access protection");
		else if (errno == ENOSYS)
			a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "this kernel does not implement fanotify_init(). The fanotify API is available only if the kernel was configured with CONFIG_FANOTIFY");
		else
			a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "fanotify_init failed (%s)", strerror(errno));

		return -1;
	}

	f->watchdog = watchdog_new(f->fanotify_fd);

#ifdef ENABLE_THREAD_POOL
	f->thread_pool = g_thread_pool_new(scan_file_thread_fun, f, -1, FALSE, NULL);
#endif

	/* add the fanotify file desc to the thread loop */
	fanotify_channel = g_io_channel_unix_new(f->fanotify_fd);

	/* g_io_add_watch(fanotify_channel, G_IO_IN, fanotify_cb, f); */
	source = g_io_create_watch(fanotify_channel, G_IO_IN);
	g_source_set_callback(source, (GSourceFunc)fanotify_cb, f, NULL);
	g_source_attach(source, access_monitor_get_main_context(f->monitor));
	g_source_unref(source);

	return 0;
}

static char *get_file_path_from_fd(int fd, char *buffer, size_t buffer_size)
{
	ssize_t len;

	if (fd <= 0)
		return NULL;

	sprintf(buffer, "/proc/self/fd/%d", fd);
	if ((len = readlink(buffer, buffer, buffer_size - 1)) < 0)
		return NULL;

	buffer[len] = '\0';

	return buffer;
}

static void scan_file_thread_fun(gpointer data, gpointer user_data)
{
	struct fanotify_monitor *f = (struct fanotify_monitor *)user_data;
	struct a6o_file_context *file_context = (struct a6o_file_context *)data;
	struct a6o_scan *scan;
	enum a6o_file_status status;
	__u32 fan_r;

	scan = a6o_scan_new(f->armadito, -1);
	status = a6o_scan_context(scan, file_context);
	fan_r = status == ARMADITO_MALWARE ? FAN_DENY : FAN_ALLOW;

	if (watchdog_remove(f->watchdog, file_context->fd, NULL))
		response_write(f->fanotify_fd, file_context->fd, fan_r, file_context->path, "scanned");

	file_context->fd = -1; /* this will prevent a6o_file_context_free from closing the file descriptor twice :( */
	a6o_file_context_free(file_context);
	a6o_scan_free(scan);
}

static void fanotify_perm_event_process(struct fanotify_monitor *f, struct fanotify_event_metadata *event, const char *path)
{
	struct a6o_file_context file_context;
	enum a6o_file_context_status context_status;
	struct stat buf;

	/* the 2 following tests could be removed: */
	/* if file descriptor does not refer to a file, read() will fail inside os_mime_type_guess_fd() */
	/* in this case, mime_type will be null, context_status will be error and response will be ALLOW */
	/* BUT: this gives a lot of warning in os_mime_type_guess() */
	/* and the read() in os_mime_type_guess() could be successfull for a device for instance */
	/* so for now I keep the fstat() */
	if (fstat(event->fd, &buf) < 0) {
		if (watchdog_remove(f->watchdog, event->fd, NULL))
			response_write(f->fanotify_fd, event->fd, FAN_ALLOW, path, "stat failed");
		return;
	}

	if (!S_ISREG(buf.st_mode)) {
		if (watchdog_remove(f->watchdog, event->fd, NULL))
			response_write(f->fanotify_fd, event->fd, FAN_ALLOW, path, "not a file");
		return;
	}

	context_status = a6o_file_context_get(&file_context, event->fd, path, f->scan_conf);

	if (context_status) {   /* means file must not be scanned */
		if (watchdog_remove(f->watchdog, event->fd, NULL))
			response_write(f->fanotify_fd, event->fd, FAN_ALLOW, path, "not scanned");

#if 0
		a6o_file_context_close(&file_context);
#endif

		return;
	}

#ifdef ENABLE_THREAD_POOL
	/* scan in thread pool */
	g_thread_pool_push(f->thread_pool, a6o_file_context_clone(&file_context), NULL);
#else
	if (watchdog_remove(f->watchdog, event->fd, NULL))
		response_write(f->fanotify_fd, event->fd, FAN_ALLOW, path, "thread pool disabled");
#endif
}

static void fanotify_notify_event_process(struct fanotify_monitor *m, struct fanotify_event_metadata *event)
{
	/* FIXME: TODO */
}

static void fanotify_pass_1(struct fanotify_monitor *f, struct fanotify_event_metadata *buf, ssize_t len)
{
	struct fanotify_event_metadata *event;

	/* first pass: allow all PERM events from myself, enqueue other PERM events */
	for(event = buf; FAN_EVENT_OK(event, len); event = FAN_EVENT_NEXT(event, len)) {
		if ((event->mask & FAN_OPEN_PERM))
			if (event->pid == f->my_pid)
				response_write(f->fanotify_fd, event->fd, FAN_ALLOW, NULL, "PID is myself");
			else
				watchdog_add(f->watchdog, event->fd);
	}
}

static void fanotify_pass_2(struct fanotify_monitor *f, struct fanotify_event_metadata *buf, ssize_t len)
{
	struct fanotify_event_metadata *event;

	/* second pass: process all PERM events that were not from myself and all other events */
	for(event = buf; FAN_EVENT_OK(event, len); event = FAN_EVENT_NEXT(event, len)) {
		if ((event->mask & FAN_OPEN_PERM)) {
			char file_path[PATH_MAX + 1];
			char *p;

			if (event->pid == f->my_pid)
				continue;

			p = get_file_path_from_fd(event->fd, file_path, PATH_MAX);

			fanotify_perm_event_process(f, event, p);
		} else
			fanotify_notify_event_process(f, event);
	}
}

/* Size of buffer to use when reading fanotify events */
/* 8192 is recommended by fanotify man page */
#define FANOTIFY_BUFFER_SIZE 8192

static gboolean fanotify_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
	struct fanotify_monitor *f = (struct fanotify_monitor *)data;
	char buf[FANOTIFY_BUFFER_SIZE];
	ssize_t len;

	assert(g_main_context_is_owner(access_monitor_get_main_context(f->monitor)));

	if ((len = read(f->fanotify_fd, buf, FANOTIFY_BUFFER_SIZE)) < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "error reading fanotify event descriptor (%s)", strerror(errno));
		return TRUE;
	}

	if (len) {
		fanotify_pass_1(f, (struct fanotify_event_metadata *)buf, len);
		fanotify_pass_2(f, (struct fanotify_event_metadata *)buf, len);
	}

	return TRUE;
}

int fanotify_monitor_mark_directory(struct fanotify_monitor *f, const char *path, int enable_permission)
{
	uint64_t fan_mask;
	int r;

	fan_mask = (enable_permission ? FAN_OPEN_PERM : FAN_CLOSE_WRITE) | FAN_EVENT_ON_CHILD;

	r = fanotify_mark(f->fanotify_fd, FAN_MARK_ADD, fan_mask, AT_FDCWD, path);

	if (r < 0)
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "adding fanotify mark for %s failed (%s)", path, strerror(errno));

	return r;
}

int fanotify_monitor_unmark_directory(struct fanotify_monitor *f, const char *path, int enable_permission)
{
	uint64_t fan_mask;
	int r;

	fan_mask = (enable_permission ? FAN_OPEN_PERM : FAN_CLOSE_WRITE) | FAN_EVENT_ON_CHILD;

	r = fanotify_mark(f->fanotify_fd, FAN_MARK_REMOVE, fan_mask, AT_FDCWD, path);

	if (r < 0)
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "removing fanotify mark for %s failed (%s)", path, strerror(errno));

	return r;
}

int fanotify_monitor_mark_mount(struct fanotify_monitor *f, const char *path, int enable_permission)
{
	uint64_t fan_mask = enable_permission ? FAN_OPEN_PERM : FAN_CLOSE_WRITE;
	int r;

	r = fanotify_mark(f->fanotify_fd, FAN_MARK_ADD | FAN_MARK_MOUNT, fan_mask, AT_FDCWD, path);
	if (r < 0)
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "adding fanotify mark on mount point %s failed (%s)", path, strerror(errno));

	return r;
}

int fanotify_monitor_unmark_mount(struct fanotify_monitor *f, const char *path, int enable_permission)
{
	uint64_t fan_mask = enable_permission ? FAN_OPEN_PERM : FAN_CLOSE_WRITE;
	int r;

	r = fanotify_mark(f->fanotify_fd, FAN_MARK_REMOVE | FAN_MARK_MOUNT, fan_mask, AT_FDCWD, path);

	if (r < 0)
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "removing fanotify mark for mount point %s failed (%s)", path, strerror(errno));

	return r;
}
