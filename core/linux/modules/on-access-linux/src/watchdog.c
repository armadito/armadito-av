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
#include "libarmadito-config.h"

#include "queue.h"
#include "stamp.h"
#include "response.h"
#include "onaccessmod.h"

#include <glib.h>
#include <linux/fanotify.h>
#include <stdlib.h>

#undef ENABLE_WATCHDOG

struct watchdog {
	GThread *thread;
	struct queue *queue;
	int fanotify_fd;
};

#define N_FD 100
#define TIMEOUT  100000  /* micro seconds */
#define SLEEP    TIMEOUT  /* micro seconds */

static gpointer watchdog_thread_fun(gpointer data)
{
	struct watchdog *w = (struct watchdog *)data;
	struct timespec timeout = { 0, TIMEOUT * ONE_MICROSECOND};
	struct timespec sleep_duration = { 0, SLEEP * ONE_MICROSECOND};
	struct timespec before;
	int i, n_fd;
	struct queue_entry entries[N_FD];

	while (1) {
		nanosleep(&sleep_duration, NULL);

		stamp_now(&before);
		stamp_sub(&before, &timeout);

		n_fd = queue_pop_timeout(w->queue, &before, entries, N_FD);

		if (n_fd) {
			a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "%d file descriptors in timeout", n_fd);

			for(i = 0; i < n_fd; i++)
				response_write(w->fanotify_fd, entries[i].fd, FAN_ALLOW, NULL, "timeout");
		}
	}

	return NULL;
}

struct watchdog *watchdog_new(int fanotify_fd)
{
	struct watchdog *w = malloc(sizeof(struct watchdog));

#ifdef ENABLE_WATCHDOG
	w->queue = queue_new();
	w->thread = g_thread_new("timeout thread", watchdog_thread_fun, w);
	w->fanotify_fd = fanotify_fd;
#endif

	return w;
}

void watchdog_add(struct watchdog *w, int fd)
{
	struct timespec now;

#ifndef ENABLE_WATCHDOG
	return;
#endif

	stamp_now(&now);

	queue_push(w->queue, fd, &now);
}

int watchdog_remove(struct watchdog *w, int fd, struct timespec *after)
{
	int r;
	struct queue_entry entry;

#ifndef ENABLE_WATCHDOG
	return 1;
#endif

	r = queue_pop_fd(w->queue, fd, &entry);

	if (r) {
		if (after != NULL) {
			stamp_now(after);
			stamp_sub(after, &entry.timestamp);
		}
	}

	return r;
}

