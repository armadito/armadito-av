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

#ifndef _MONITOR_H_
#define _MONITOR_H_

struct access_monitor;

enum access_monitor_flags {
	MONITOR_MOUNT        = 1 << 0,
	MONITOR_RECURSIVE    = 1 << 1,
	MONITOR_TYPE_CHECK   = 1 << 2,
	MONITOR_LOG_EVENT    = 1 << 3,
	MONITOR_ENABLE_PERM  = 1 << 4,
};

struct access_monitor *access_monitor_new(enum access_monitor_flags flags);

int access_monitor_get_poll_fd(struct access_monitor *m);

void access_monitor_cb(void *user_data);

int access_monitor_add(struct access_monitor *m, const char *path, unsigned int flags);

#if 0
int access_monitor_remove(struct access_monitor *m, const char *path);
#endif

void access_monitor_free(struct access_monitor *m);

#if 0
int access_monitor_activate(struct access_monitor *m);
#endif

#endif
