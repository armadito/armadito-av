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

#define RM_GLIB

#include <libarmadito/armadito.h>
#include <armadito-config.h>

#include "imonitor.h"
#include "monitor.h"
#include "modname.h"

#include <errno.h>
#ifdef RM_GLIB
#include "hash.h"
#else
#include <glib.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <stdio.h>

struct inotify_monitor {
	struct access_monitor *monitor;

	int inotify_fd;

#ifdef RM_GLIB
	struct hash_table *wd2path_table;
	struct hash_table *path2wd_table;
#else
	GHashTable *wd2path_table;
	GHashTable *path2wd_table;
#endif
};

static int inotify_cb(void *data);

#ifndef RM_GLIB
static void path_destroy_notify(gpointer data)
{
	free(data);
}
#endif

struct inotify_monitor *inotify_monitor_new(struct access_monitor *m)
{
	struct inotify_monitor *im = malloc(sizeof(struct inotify_monitor));

	im->monitor = m;

#ifdef RM_GLIB
	im->wd2path_table = hash_table_new(hash_int, equal_int, NULL, (destroy_cb_t)free);
	im->path2wd_table = hash_table_new(hash_str, equal_str, (destroy_cb_t)free, NULL);
#else
	im->wd2path_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, path_destroy_notify);
	im->path2wd_table = g_hash_table_new_full(g_str_hash, g_str_equal, path_destroy_notify, NULL);
#endif

	return im;
}

int inotify_monitor_start(struct inotify_monitor *im)
{
	im->inotify_fd = inotify_init();
	if (im->inotify_fd == -1) {
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_ERROR, MODULE_LOG_NAME ": inotify_init failed (%s)", strerror(errno));
		return -1;
	}

	access_monitor_add_fd(im->monitor, im->inotify_fd, inotify_cb, im);

	return 0;
}

int inotify_monitor_mark_directory(struct inotify_monitor *im, const char *path)
{
	int wd;

	wd = inotify_add_watch(im->inotify_fd, path, (uint64_t)IN_ONLYDIR | (uint64_t)IN_MOVE | (uint64_t)IN_DELETE | (uint64_t)IN_CREATE);
	if (wd == -1) {
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": adding inotify watch for %s failed (%s)", path, strerror(errno));
		return -1;
	}

#ifdef RM_GLIB
	hash_table_insert(im->wd2path_table, H_INT_TO_POINTER(wd), strdup(path));
	hash_table_insert(im->path2wd_table, strdup(path), H_INT_TO_POINTER(wd));
#else
	g_hash_table_insert(im->wd2path_table, GINT_TO_POINTER(wd), (gpointer)strdup(path));
	g_hash_table_insert(im->path2wd_table, (gpointer)strdup(path), GINT_TO_POINTER(wd));
#endif

	return 0;
}

int inotify_monitor_unmark_directory(struct inotify_monitor *im, const char *path)
{
	void *p;
	int wd;

	/* retrieve the watch descriptor associated to path */
#ifdef RM_GLIB
	p = hash_table_lookup(im->path2wd_table, path);
#else
	p = g_hash_table_lookup(im->path2wd_table, path);
#endif

	if (p == NULL) {
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": retrieving inotify watch id for %s failed", path);
		return 1;
	}

#ifdef RM_GLIB
	wd = H_POINTER_TO_INT(p);
#else
	wd = GPOINTER_TO_INT(p);
#endif

	/* errors are ignored: if the watch descriptor is invalid, it means it is no longer being watched because of deletion */
	if (inotify_rm_watch(im->inotify_fd, wd) == -1) {
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": removing inotify watch %d for %s failed (%s)", wd, path, strerror(errno));
	}

#ifdef RM_GLIB
	hash_table_remove(im->wd2path_table, p);
	hash_table_remove(im->path2wd_table, path);
#else
	g_hash_table_remove(im->wd2path_table, p);
	g_hash_table_remove(im->path2wd_table, path);
#endif

	return 0;
}

#ifndef RM_GLIB
#ifdef DEBUG
static void inotify_event_log(const struct inotify_event *e, const char *full_path)
{
	GString *s = g_string_new("");

	g_string_append_printf(s, "inotify event: wd=%2d ", e->wd);

	if (e->cookie > 0)
		g_string_append_printf(s, "cookie=%4d ", e->cookie);

	g_string_append_printf(s, "mask=");

#define M(_mask, _mask_bit) do { if ((_mask) & (_mask_bit)) g_string_append_printf(s, #_mask_bit " "); } while(0)

	M(e->mask, IN_ACCESS);
	M(e->mask, IN_ATTRIB);
	M(e->mask, IN_CLOSE_NOWRITE);
	M(e->mask, IN_CLOSE_WRITE);
	M(e->mask, IN_CREATE);
	M(e->mask, IN_DELETE);
	M(e->mask, IN_DELETE_SELF);
	M(e->mask, IN_IGNORED);
	M(e->mask, IN_ISDIR);
	M(e->mask, IN_MODIFY);
	M(e->mask, IN_MOVE_SELF);
	M(e->mask, IN_MOVED_FROM);
	M(e->mask, IN_MOVED_TO);
	M(e->mask, IN_OPEN);
	M(e->mask, IN_Q_OVERFLOW);
	M(e->mask, IN_UNMOUNT);

	if (e->len > 0)
		g_string_append_printf(s, "name=%s", e->name);

	g_string_append_printf(s, " full_path=%s", full_path != NULL ? full_path : "null");

	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": %s", s->str);

	g_string_free(s, TRUE);
}
#endif
#endif

static char *inotify_event_full_path(struct inotify_monitor *im, struct inotify_event *event)
{
	char *dir;
	char *full_path;

#ifdef RM_GLIB
	dir = (char *)hash_table_lookup(im->wd2path_table, H_INT_TO_POINTER(event->wd));
#else
	dir = (char *)g_hash_table_lookup(im->wd2path_table, GINT_TO_POINTER(event->wd));
#endif

	if (dir == NULL)
		return NULL;

	if (event->len) {
		asprintf(&full_path, "%s/%s", dir, event->name);
	} else {
		full_path = strdup(dir);
	}

	return full_path;
}

static void inotify_event_process(struct inotify_monitor *im, struct inotify_event *event)
{
	char *full_path;

	if (!(event->mask & IN_ISDIR))
		return;

	full_path = inotify_event_full_path(im, event);

#if 0
	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "inotify full path %s", full_path != NULL ? full_path : "(null)");
#endif

	if (full_path == NULL)
		return;

	if (event->mask & IN_CREATE || event->mask & IN_MOVED_TO)
		access_monitor_recursive_mark_directory(im->monitor, full_path);
	else if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM)
		access_monitor_unmark_directory(im->monitor, full_path);

	free(full_path);
}

/* Size of buffer to use when reading inotify events */
#define INOTIFY_BUFFER_SIZE 8192

static int inotify_cb(void *data)
{
	struct inotify_monitor *im = (struct inotify_monitor *)data;
	char event_buffer[INOTIFY_BUFFER_SIZE];
	ssize_t len;

	len = read (im->inotify_fd, event_buffer, INOTIFY_BUFFER_SIZE);

	if (len > 0)  {
		char *p;

		p = event_buffer;
		while (p < event_buffer + len) {
			struct inotify_event *event = (struct inotify_event *) p;

			inotify_event_process(im, event);

			p += sizeof(struct inotify_event) + event->len;
		}
	}

	return 1;
}
