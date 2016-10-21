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

#include "fanotify.h"

#define _GNU_SOURCE

#include <glib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/signalfd.h>
#include <fcntl.h>
#include <linux/fanotify.h>

struct watchd {
	int fanotify_fd;
	GPtrArray *paths;
};

static gboolean watchd_cb(GIOChannel *source, GIOCondition condition, gpointer data);

/* Size of buffer to use when reading fanotify events */
/* 8192 is recommended by fanotify man page */
#define FANOTIFY_BUFFER_SIZE 8192

struct watchd *watchd_new(void)
{
	struct watchd *w = g_new(struct watchd, 1);
	GIOChannel *channel;

	w->fanotify_fd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_CONTENT, O_RDONLY | O_CLOEXEC | O_LARGEFILE | O_NOATIME);

	if (w->fanotify_fd < 0)
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, "fanotify init failed (%s)", strerror(errno));

	w->paths = g_ptr_array_new();

	channel = g_io_channel_unix_new(w->fanotify_fd);

	g_io_add_watch(channel, G_IO_IN, watchd_cb, w);

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, "fanotify init ok");

	return w;
}

void watchd_add(struct watchd *w, const char *path)
{
	const char *tmp = strdup(path);

	if (fanotify_mark(w->fanotify_fd, FAN_MARK_ADD | FAN_MARK_MOUNT, FAN_OPEN_PERM, AT_FDCWD, tmp) < 0)
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, "fanotify mark on %s failed (%s)", tmp, strerror(errno));

	g_ptr_array_add(w->paths, (gpointer)tmp);

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, "fanotify marked %s", tmp);
}

static char *get_file_path_from_fd(int fd, char *buffer, size_t buffer_size)
{
	ssize_t len;

	if (fd <= 0)
		return NULL;

	sprintf (buffer, "/proc/self/fd/%d", fd);
	if ((len = readlink (buffer, buffer, buffer_size - 1)) < 0)
		return NULL;

	buffer[len] = '\0';
	return buffer;
}

static void event_process(struct watchd *w, struct fanotify_event_metadata *event)
{
	char file_path[PATH_MAX];
	char program_path[PATH_MAX];
	char *p;

	p = get_file_path_from_fd(event->fd, file_path, PATH_MAX);
	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, "received event fd %d path '%s'", event->fd, p ? p : "unknown");

	if (event->mask & FAN_OPEN_PERM) {
		struct fanotify_response access;

		access.fd = event->fd;
		access.response = FAN_ALLOW;
		/* access.response = FAN_DENY; */

		write(w->fanotify_fd, &access, sizeof(access));
	}

	close(event->fd);
}

static gboolean watchd_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
	struct watchd *w = (struct watchd *)data;
	char buf[FANOTIFY_BUFFER_SIZE];
	ssize_t len;

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, "fanotify callback");

	if ((len = read (w->fanotify_fd, buf, FANOTIFY_BUFFER_SIZE)) > 0)  {
		struct fanotify_event_metadata *ev;

		for(ev = (struct fanotify_event_metadata *)buf; FAN_EVENT_OK(ev, len); ev = FAN_EVENT_NEXT(ev, len))
			event_process(w, ev);
	}

	return TRUE;
}

void watchd_free(struct watchd *w)
{
	int i;

	for (i = 0; i < w->paths->len; ++i) {
		const char *path = (const char *)g_ptr_array_index(w->paths, i);

		fanotify_mark(w->fanotify_fd, FAN_MARK_REMOVE, FAN_OPEN_PERM, AT_FDCWD, path);
		free((void *)path);
	}

	g_ptr_array_free(w->paths, TRUE);

	close(w->fanotify_fd);

	free(w);

	a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, "fanotify free ok");
}

