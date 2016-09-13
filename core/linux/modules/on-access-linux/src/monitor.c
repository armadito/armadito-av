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

/*
  TODO:
  - implement fanotify non-perm event
*/

#define _GNU_SOURCE

#include <libarmadito.h>
#include "libarmadito-config.h"

#include "monitor.h"
#include "famonitor.h"
#include "imonitor.h"
#include "mount.h"
#include "onaccessmod.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>
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
	int enable_permission;
	int enable_removable_media;
	GPtrArray *entries;

	int start_pipe[2];
	int command_pipe[2];

	GThread *monitor_thread;
	GMainContext *monitor_thread_context;

	struct fanotify_monitor *fanotify_monitor;
	struct inotify_monitor *inotify_monitor;
	struct mount_monitor *mount_monitor;
};

static gboolean delayed_start_cb(GIOChannel *source, GIOCondition condition, gpointer data);
static gboolean command_cb(GIOChannel *source, GIOCondition condition, gpointer data);

static void mount_cb(enum mount_event_type ev_type, const char *path, void *user_data);

static gpointer monitor_thread_fun(gpointer data);
static void scan_file_thread_fun(gpointer data, gpointer user_data);

static void entry_destroy_notify(gpointer data)
{
	struct monitor_entry *e = (struct monitor_entry *)data;

	free((void *)e->path);
	free(e);
}

struct access_monitor *access_monitor_new(struct armadito *u)
{
	struct access_monitor *m = malloc(sizeof(struct access_monitor));
	GIOChannel *start_channel;

	m->enable = 0;
	m->enable_permission = 0;
	m->enable_removable_media = 0;

	m->entries = g_ptr_array_new_full(10, entry_destroy_notify);

	/* this pipe will be used to trigger creation of the monitor thread when entering main thread loop, */
	/* so that the monitor thread does not start before all modules are initialized  */
	/* and the daemon main loop is entered */
	if (pipe(m->start_pipe) < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "pipe failed (%s)", strerror(errno));
		g_free(m);
		return NULL;
	}

	/* this pipe will be used to send commands to the monitor thread */
	if (pipe(m->command_pipe) < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "pipe failed (%s)", strerror(errno));
		g_free(m);
		return NULL;
	}

	start_channel = g_io_channel_unix_new(m->start_pipe[0]);
	g_io_add_watch(start_channel, G_IO_IN, delayed_start_cb, m);

	m->fanotify_monitor = fanotify_monitor_new(m, u);
	m->inotify_monitor = inotify_monitor_new(m);
	m->mount_monitor = mount_monitor_new(mount_cb, m);

	m->monitor_thread_context = g_main_context_new();

	m->monitor_thread = g_thread_new("access monitor thread", monitor_thread_fun, m);

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "created monitor thread %p", m->monitor_thread);

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
	m->enable_permission = enable_permission;

	return m->enable_permission;
}

int access_monitor_is_enable_permission(struct access_monitor *m)
{
	return m->enable_permission;
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

GMainContext *access_monitor_get_main_context(struct access_monitor *m)
{
	return m->monitor_thread_context;
}

static void add_entry(struct access_monitor *m, const char *path, enum entry_flag flag)
{
	struct monitor_entry *e = malloc(sizeof(struct monitor_entry));

	e->path = strdup(path);
	e->flag = flag;

	g_ptr_array_add(m->entries, e);
}

static dev_t get_dev_id(const char *path)
{
	struct stat buf;

	if (stat(path, &buf) < 0)
		return -1;

	return buf.st_dev;
}

void access_monitor_add_mount(struct access_monitor *m, const char *mount_point)
{
	dev_t mount_dev_id, slash_dev_id;

	/* check that mount_point is not in the same partition as / */
	slash_dev_id = get_dev_id("/");
	if (slash_dev_id < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "cannot get device id for / (%s)", strerror(errno));
		return;
	}

	mount_dev_id = get_dev_id(mount_point);
	if (mount_dev_id < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "cannot get device id for %s (%s)", mount_point, strerror(errno));
		return;
	}

	if (mount_dev_id == slash_dev_id) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "\"%s\" is in same partition as \"/\"; adding \"/\" as monitored mount point is not supported", mount_point);
		return;
	}

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

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "delayed_start_cb: thread %p", g_thread_self());

	if (read(m->start_pipe[0], &c, 1) < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "read() in activation callback failed (%s)", strerror(errno));

		return FALSE;
	}

	if (c != 'A') {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "unexpected character ('%c' (0x%x) != 'A')", c, c);
		return FALSE;
	}

	/* commented out: closing the pipe leaded to an obscure race condition with other threads, resulting in a reuse */
	/* of the pipe input file descriptor (namely, for one associated with a client connection) and in IPC errors */
	/* g_io_channel_shutdown(source, FALSE, NULL); */

	if (access_monitor_is_enable(m))
		access_monitor_send_command(m, ACCESS_MONITOR_START);

	return TRUE;
}

static void mark_directory(struct access_monitor *m, const char *path)
{
	if (fanotify_monitor_mark_directory(m->fanotify_monitor, path, m->enable_permission) < 0)
		return;

	if (inotify_monitor_mark_directory(m->inotify_monitor, path) < 0)
		return;

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "added mark for directory %s", path);
}

int access_monitor_unmark_directory(struct access_monitor *m, const char *path)
{
	if (fanotify_monitor_unmark_directory(m->fanotify_monitor, path, m->enable_permission) < 0)
		return -1;

	if (inotify_monitor_unmark_directory(m->inotify_monitor, path) < 0)
		return -1;

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "removed mark for directory %s", path);

	return 0;
}

int access_monitor_recursive_mark_directory(struct access_monitor *m, const char *path)
{
	DIR *dir;
	struct dirent *entry;
	GString *entry_path;

	mark_directory(m, path);

	if ((dir = opendir(path)) == NULL) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "error opening directory %s (%s)", path, strerror(errno));
		return -1;
	}

	entry_path = g_string_new("");

	while((entry = readdir(dir)) != NULL) {
		if (entry->d_type != DT_DIR || !strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;

		g_string_printf(entry_path, "%s/%s", path, entry->d_name);

		access_monitor_recursive_mark_directory(m, entry_path->str);
	}

	g_string_free(entry_path, TRUE);

	if (closedir(dir) < 0)
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "error closing directory %s (%s)", path, strerror(errno));

	return 0;
}

static void mark_mount_point(struct access_monitor *m, const char *path)
{
	if (fanotify_monitor_mark_mount(m->fanotify_monitor, path, m->enable_permission) < 0)
		return;

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "added mark for mount point %s", path);
}

static void unmark_mount_point(struct access_monitor *m, const char *path)
{
	if (path == NULL)
		return;

	if (fanotify_monitor_unmark_mount(m->fanotify_monitor, path, m->enable_permission) < 0)
		return;

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "removed mark for mount point %s", path);
}

static void mark_entries(struct access_monitor *m)
{
	int i;

	for(i = 0; i < m->entries->len; i++) {
		struct monitor_entry *e = (struct monitor_entry *)g_ptr_array_index(m->entries, i);

		if (e->flag == ENTRY_DIR)
			access_monitor_recursive_mark_directory(m, e->path);
		else
			mark_mount_point(m, e->path);
	}
}

struct mount_data {
	enum mount_event_type ev_type;
	const char *path;
	struct access_monitor *monitor;
};

static void mount_data_free(struct mount_data *data)
{
	if (data->path != NULL)
		free((void *)data->path);
	free(data);
}

static gboolean mount_idle_cb(gpointer user_data)
{
	struct mount_data *data = user_data;

	assert(g_main_context_is_owner(access_monitor_get_main_context(data->monitor)));

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, MODULE_LOG_NAME ": " "received mount notification for %s (%s)", data->path, data->ev_type == EVENT_MOUNT ? "mount" : "umount");

	if (data->ev_type == EVENT_MOUNT)
		mark_mount_point(data->monitor, data->path);

	/* if ev_type is EVENT_UMOUNT, nothing to be done, the kernel has already removed the fanotify mark */
	/* and anyway, path is NULL, so... */
	/* unmark_mount_point(m, path); */

	return G_SOURCE_REMOVE;
}

static void mount_cb(enum mount_event_type ev_type, const char *path, void *user_data)
{
	struct mount_data *data;
	struct access_monitor *monitor = (struct access_monitor *)user_data;

	data = malloc(sizeof(struct mount_data));
	data->ev_type = ev_type;
	if (path != NULL)
		data->path = strdup(path);
	else
		data->path = NULL;
	data->monitor = monitor;

	/* Invoke the function. */
	g_main_context_invoke_full(monitor->monitor_thread_context,
				G_PRIORITY_DEFAULT,
				mount_idle_cb,
				data,
				(GDestroyNotify)mount_data_free);
}

static gpointer monitor_thread_fun(gpointer data)
{
	struct access_monitor *m = (struct access_monitor *)data;
	GMainLoop *loop;
	GIOChannel *command_channel;
	GSource *source;

	g_main_context_push_thread_default(m->monitor_thread_context);

	loop = g_main_loop_new(m->monitor_thread_context, FALSE);

	command_channel = g_io_channel_unix_new(m->command_pipe[0]);

	source = g_io_create_watch(command_channel, G_IO_IN);
	g_source_set_callback(source, (GSourceFunc)command_cb, m, NULL);
	g_source_attach(source, m->monitor_thread_context);
	g_source_unref(source);

	g_main_loop_run(loop);
}

static void access_monitor_start_command(struct access_monitor *m)
{
	if (fanotify_monitor_start(m->fanotify_monitor))
		return;

	if (inotify_monitor_start(m->inotify_monitor))
		return;

	/* if configured, add the mount monitor */
	if (m->enable_removable_media) {
		if (mount_monitor_start(m->mount_monitor) < 0)
			return;
	}

	/* init all fanotify and inotify marks */
	mark_entries(m);

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, MODULE_LOG_NAME ": " "on-access protection started");
}

static void access_monitor_stop_command(struct access_monitor *m)
{
	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_INFO, MODULE_LOG_NAME ": " "on-access protection stopped (Not Yet Implemented)");
}

static void access_monitor_status_command(struct access_monitor *m)
{
	/* ??? */
}

static gboolean command_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
	struct access_monitor *m = (struct access_monitor *)data;
	char cmd;

	assert(g_main_context_is_owner(m->monitor_thread_context));

	if (read(m->command_pipe[0], &cmd, 1) < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": " "read() in command callback failed (%s)", strerror(errno));

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
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "error sending command to access monitor thread (%s)", strerror(errno));
		return -1;
	}

	return 0;
}

