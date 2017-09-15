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

/* glib is "half-removed" because of GMainLoop & co */
#define RM_GLIB

#include <libarmadito/armadito.h>
#include "armadito-config.h"

#include "core/ondemand.h"

#include "monitor.h"
#include "famonitor.h"
#include "imonitor.h"
#include "mount.h"
#include "modname.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
/* must still be included for GMainLoop & co */
#include <glib.h>
#ifdef RM_GLIB
#include "ptrarray.h"
#include "pollset.h"
#include <pthread.h>
#endif
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

enum entry_flag {
	ENTRY_MOUNT = 1,
	ENTRY_DIR,
};

struct monitor_entry {
	const char *path;
	enum entry_flag flag;
};

struct access_monitor {
	int enable;
	int enable_removable_media;
	int autoscan_removable_media;

#ifdef RM_GLIB
	struct ptr_array *entries;
#else
	GPtrArray *entries;
#endif

	int start_pipe[2];
	int command_pipe[2];

#ifdef RM_GLIB
	pthread_t monitor_thread;
	struct poll_set *poll_fds;
#else
	GThread *monitor_thread;
	GMainContext *monitor_thread_context;
#endif

	struct fanotify_monitor *fanotify_monitor;
	struct inotify_monitor *inotify_monitor;
	struct mount_monitor *mount_monitor;

	struct armadito *ar;
};

static gboolean delayed_start_cb(GIOChannel *source, GIOCondition condition, gpointer data);

static int command_cb(void *data);

#ifdef RM_GLIB
static void *monitor_pthread_fun(void *arg);
#else
static gpointer monitor_gthread_fun(gpointer data);
#endif

#ifdef RM_GLIB
static void entry_destroy_cb(void *data)
{
	struct monitor_entry *e = (struct monitor_entry *)data;

	free((void *)e->path);
	free(e);
}
#else
static void entry_destroy_cb(gpointer data)
{
	struct monitor_entry *e = (struct monitor_entry *)data;

	free((void *)e->path);
	free(e);
}
#endif

struct access_monitor *access_monitor_new(struct armadito *armadito)
{
	struct access_monitor *m = malloc(sizeof(struct access_monitor));
	GIOChannel *start_channel;

	m->enable = 0;
	m->enable_removable_media = 0;
	m->autoscan_removable_media = 0;

	m->ar = armadito;

#ifdef RM_GLIB
	m->entries = ptr_array_new(entry_destroy_cb);
#else
	m->entries = g_ptr_array_new_full(10, entry_destroy_cb);
#endif

	/* this pipe will be used to trigger creation of the monitor thread when entering main thread loop, */
	/* so that the monitor thread does not start before all modules are initialized  */
	/* and the daemon main loop is entered */
	if (pipe(m->start_pipe) < 0) {
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": pipe failed (%s)", strerror(errno));
		free(m);
		return NULL;
	}

	/* this pipe will be used to send commands to the monitor thread */
	if (pipe(m->command_pipe) < 0) {
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": pipe failed (%s)", strerror(errno));
		free(m);
		return NULL;
	}

	start_channel = g_io_channel_unix_new(m->start_pipe[0]);
	g_io_add_watch(start_channel, G_IO_IN, delayed_start_cb, m);

	m->fanotify_monitor = fanotify_monitor_new(m, armadito);
	m->inotify_monitor = inotify_monitor_new(m);
#if 0
	m->mount_monitor = mount_monitor_new(mount_cb, m);
#endif

#ifdef RM_GLIB
	m->poll_fds = poll_set_new();
	if (pthread_create(&m->monitor_thread, NULL, monitor_pthread_fun, m))
		;
#else
	m->monitor_thread_context = g_main_context_new();
	m->monitor_thread = g_thread_new("access monitor thread", monitor_gthread_fun, m);
#endif

	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": created monitor thread %p", m->monitor_thread);

	return m;
}

int access_monitor_enable(struct access_monitor *m, int enable)
{
	m->enable = enable;

	return m->enable;
}

int access_monitor_is_enable(struct access_monitor *m)
{
	return m->enable;
}

int access_monitor_enable_permission(struct access_monitor *m, int enable_permission)
{
	return fanotify_monitor_enable_permission(m->fanotify_monitor, enable_permission);
}

int access_monitor_is_enable_permission(struct access_monitor *m)
{
	return fanotify_monitor_is_enable_permission(m->fanotify_monitor);
}

int access_monitor_enable_removable_media(struct access_monitor *m, int enable_removable_media)
{
	m->enable_removable_media = enable_removable_media;

	return m->enable_removable_media;
}

int access_monitor_is_enable_removable_media(struct access_monitor *m)
{
	return m->enable_removable_media;
}

int access_monitor_autoscan_removable_media(struct access_monitor *m, int autoscan_removable_media)
{
	m->autoscan_removable_media = autoscan_removable_media;

	return m->autoscan_removable_media;
}

int access_monitor_is_autoscan_removable_media(struct access_monitor *m)
{
	return m->autoscan_removable_media;
}

static void add_entry(struct access_monitor *m, const char *path, enum entry_flag flag)
{
	struct monitor_entry *e = malloc(sizeof(struct monitor_entry));

	e->path = strdup(path);
	e->flag = flag;

#ifdef RM_GLIB
	ptr_array_add(m->entries, e);
#else
	g_ptr_array_add(m->entries, e);
#endif
}

static dev_t get_dev_id(const char *path)
{
	struct stat buf;

	if (stat(path, &buf) < 0)
		return -1;

	return buf.st_dev;
}

static int check_mount_point_is_not_slash(const char *mount_point)
{
	dev_t slash_dev_id = get_dev_id("/");
	dev_t mount_dev_id = get_dev_id(mount_point);

	if (slash_dev_id < 0 || mount_dev_id < 0)
		return -1;

	if (mount_dev_id == slash_dev_id)
		return 0;

	return 1;
}

void access_monitor_add_mount(struct access_monitor *m, const char *mount_point)
{
	int ret = check_mount_point_is_not_slash(mount_point);

	if (ret < 0)
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_ERROR,
			MODULE_LOG_NAME ": cannot get device id for / or for %s (%s)",
			mount_point,
			strerror(errno));
	else if (ret == 0)
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_WARNING,
			MODULE_LOG_NAME ": \"%s\" same partition as \"/\"; adding \"/\" as monitored mount point not supported",
			mount_point);

	add_entry(m, mount_point, ENTRY_MOUNT);
}

void access_monitor_add_directory(struct access_monitor *m, const char *path)
{
	add_entry(m, path, ENTRY_DIR);
}

int access_monitor_delayed_start(struct access_monitor *m)
{
	char c = 'A';

	if (m == NULL)
		return 0;

	if (write(m->start_pipe[1], &c, 1) < 0)
		return -1;

	return 0;
}

static gboolean delayed_start_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
	struct access_monitor *m = (struct access_monitor *)data;
	char c;

	if (read(m->start_pipe[0], &c, 1) < 0) {
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": read() in activation callback failed (%s)", strerror(errno));

		return FALSE;
	}

	if (c != 'A') {
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": unexpected character ('%c' (0x%x) != 'A')", c, c);
		return FALSE;
	}

	/* commented out: closing the pipe leaded to an obscure race condition with other threads, resulting in a reuse */
	/* of the pipe input file descriptor (namely, for one associated with a client connection) and in IPC errors */
	/* g_io_channel_shutdown(source, FALSE, NULL); */

	if (access_monitor_is_enable(m))
		access_monitor_send_command(m, ACCESS_MONITOR_START);

	return TRUE;
}

int access_monitor_unmark_directory(struct access_monitor *m, const char *path)
{
	if (fanotify_monitor_unmark_directory(m->fanotify_monitor, path) < 0)
		return -1;

	if (inotify_monitor_unmark_directory(m->inotify_monitor, path) < 0)
		return -1;

	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": removed mark for directory %s", path);

	return 0;
}

int access_monitor_recursive_mark_directory(struct access_monitor *m, const char *path)
{
	DIR *dir;
	struct dirent *entry;
	GString *entry_path;

	if (fanotify_monitor_mark_directory(m->fanotify_monitor, path) < 0)
		return -1;

	if (inotify_monitor_mark_directory(m->inotify_monitor, path) < 0)
		return -1;

	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": added mark for directory %s", path);

	if ((dir = opendir(path)) == NULL) {
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": error opening directory %s (%s)", path, strerror(errno));
		return -1;
	}

	entry_path = g_string_new("");

	while((entry = readdir(dir)) != NULL) {
		if (entry->d_type != DT_DIR || !strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;

		g_string_printf(entry_path, "%s/%s", path, entry->d_name);

		/* FIXME: should return if failed? */
		access_monitor_recursive_mark_directory(m, entry_path->str);
	}

	g_string_free(entry_path, TRUE);

	if (closedir(dir) < 0)
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": error closing directory %s (%s)", path, strerror(errno));

	return 0;
}

void access_monitor_mark_mount_point(struct access_monitor *m, const char *path)
{
	if (fanotify_monitor_mark_mount(m->fanotify_monitor, path) < 0)
		return;

	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": added mark for mount point %s", path);
}

static void access_monitor_unmark_mount_point(struct access_monitor *m, const char *path)
{
	if (path == NULL)
		return;

	if (fanotify_monitor_unmark_mount(m->fanotify_monitor, path) < 0)
		return;

	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": removed mark for mount point %s", path);
}

static void mark_entries(struct access_monitor *m)
{
	int i;

#ifdef RM_GLIB
	for(i = 0; i < ptr_array_size(m->entries); i++) {
		struct monitor_entry *e = (struct monitor_entry *)ptr_array_index(m->entries, i);
#else
	for(i = 0; i < m->entries->len; i++) {
		struct monitor_entry *e = (struct monitor_entry *)g_ptr_array_index(m->entries, i);
#endif

		switch (e->flag) {
		case ENTRY_DIR:
			access_monitor_recursive_mark_directory(m, e->path);
			break;
		case ENTRY_MOUNT:
			access_monitor_mark_mount_point(m, e->path);
			break;
		}
	}
}

#ifdef RM_GLIB
void access_monitor_add_fd(struct access_monitor *m, int fd, int (*cb)(void *data), void *data)
{
	poll_set_add_fd(m->poll_fds, fd, (poll_cb_t)cb, m);
}
#else
void access_monitor_add_fd(struct access_monitor *m, int fd, int (*cb)(void *data), void *data)
{
	GIOChannel *channel;
	GSource *source;

	channel = g_io_channel_unix_new(fd);

	source = g_io_create_watch(channel, G_IO_IN);
	g_source_set_callback(source, (GSourceFunc)cb, data, NULL);
	g_source_attach(source, m->monitor_thread_context);
	g_source_unref(source);
}
#endif

#ifdef RM_GLIB
static void *monitor_pthread_fun(void *arg)
{
	struct access_monitor *m = (struct access_monitor *)arg;

	access_monitor_add_fd(m, m->command_pipe[0], command_cb, m);

	poll_set_loop(m->poll_fds);
}
#else
static gpointer monitor_gthread_fun(gpointer data)
{
	struct access_monitor *m = (struct access_monitor *)data;
	GMainLoop *loop;
	GIOChannel *command_channel;
	GSource *source;

	g_main_context_push_thread_default(m->monitor_thread_context);

	loop = g_main_loop_new(m->monitor_thread_context, FALSE);

	access_monitor_add_fd(m, m->command_pipe[0], command_cb, m);

	g_main_loop_run(loop);
}
#endif

static void access_monitor_start_command(struct access_monitor *m)
{
	if (fanotify_monitor_start(m->fanotify_monitor))
		return;

	if (inotify_monitor_start(m->inotify_monitor))
		return;

	if (mount_monitor_start(m->mount_monitor) < 0)
		return;

	/* init all fanotify and inotify marks */
	mark_entries(m);

	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_INFO, MODULE_LOG_NAME ": on-access protection started");
}

static void access_monitor_stop_command(struct access_monitor *m)
{
	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_INFO, MODULE_LOG_NAME ": on-access protection stopped (Not Yet Implemented)");
}

static void access_monitor_status_command(struct access_monitor *m)
{
	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_INFO, MODULE_LOG_NAME ": on-access protection status (Not Yet Implemented)");
}

static int command_cb(void *data)
{
	struct access_monitor *m = (struct access_monitor *)data;
	char cmd;

	if (read(m->command_pipe[0], &cmd, 1) < 0) {
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": read() in command callback failed (%s)", strerror(errno));

		return FALSE;
	}

	switch(cmd) {
	case ACCESS_MONITOR_START:
		access_monitor_start_command(m);
		break;
	case ACCESS_MONITOR_STOP:
		access_monitor_stop_command(m);
		break;
	case ACCESS_MONITOR_STATUS:
		access_monitor_status_command(m);
		break;
	}

	return TRUE;
}

int access_monitor_send_command(struct access_monitor *m, char command)
{
	if (write(m->command_pipe[1], &command, 1) < 0) {
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": error sending command to access monitor thread (%s)", strerror(errno));
		return -1;
	}

	return 0;
}

#if 0
static void mount_cb(enum mount_event_type ev_type, const char *path, void *user_data);

struct mount_data {
	enum mount_event_type ev_type;
	const char *path;
	struct access_monitor *monitor;
};

static struct mount_data *mount_data_new(enum mount_event_type ev_type, const char *path, struct access_monitor *monitor)
{
	struct mount_data *data;

	data = malloc(sizeof(struct mount_data));

	data->ev_type = ev_type;
	if (path != NULL)
		data->path = strdup(path);
	else
		data->path = NULL;
	data->monitor = monitor;

	return data;
}

static void mount_data_free(struct mount_data *data)
{
	if (data->path != NULL)
		free((void *)data->path);
	free(data);
}

static gpointer scan_media_thread_fun(gpointer data)
{
	struct mount_data *mount_data = data;
	struct a6o_on_demand *on_demand;

	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_INFO, MODULE_LOG_NAME ": starting scan of removable media mounted on %s", mount_data->path);

	on_demand = a6o_on_demand_new(mount_data->monitor->ar, mount_data->path, 0, A6O_SCAN_THREADED | A6O_SCAN_RECURSE, 0);
	a6o_on_demand_run(on_demand);

	a6o_on_demand_free(on_demand);

	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_INFO, MODULE_LOG_NAME ": finished scan of removable media mounted on %s", mount_data->path);

	mount_data_free(mount_data);

	return NULL;
}

static void scan_media(struct mount_data *data)
{
	struct mount_data *clone;

	clone = mount_data_new(data->ev_type, data->path, data->monitor);

	g_thread_new("scan removable media thread", scan_media_thread_fun, clone);
}

static gboolean mount_idle_cb(gpointer user_data)
{
	struct mount_data *data = user_data;

	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_INFO, MODULE_LOG_NAME ": received mount notification for %s (%s)", data->path, data->ev_type == EVENT_MOUNT ? "mount" : "umount");

	if (data->ev_type == EVENT_MOUNT) {
		if (access_monitor_is_autoscan_removable_media(data->monitor))
			scan_media(data);
		if (access_monitor_is_enable_removable_media(data->monitor))
			access_monitor_mark_mount_point(data->monitor, data->path);
	}

	/* if ev_type is EVENT_UMOUNT, nothing to be done, the kernel has already removed the fanotify mark */
	/* and anyway, path is NULL, so... */
	/* unmark_mount_point(m, path); */

	return G_SOURCE_REMOVE;
}

static void mount_cb(enum mount_event_type ev_type, const char *path, void *user_data)
{
	struct mount_data *data;
	struct access_monitor *monitor = (struct access_monitor *)user_data;

	data = mount_data_new(ev_type, path, monitor);

	/* Invoke the function. */
	g_main_context_invoke_full(monitor->monitor_thread_context,
				G_PRIORITY_DEFAULT,
				mount_idle_cb,
				data,
				(GDestroyNotify)mount_data_free);
}
#endif

