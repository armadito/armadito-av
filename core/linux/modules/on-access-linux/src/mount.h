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

#ifndef _ONACCESS_MOUNT_H_
#define _ONACCESS_MOUNT_H_

struct mount_monitor;

enum mount_event_type {
	EVENT_MOUNT,
	EVENT_UMOUNT,
	EVENT_UNKNOWN,
};

typedef void (*mount_cb_t)(enum mount_event_type ev_type, const char *path, void *user_data);

struct mount_monitor *mount_monitor_new(mount_cb_t cb, void *user_data);

int mount_monitor_start(struct mount_monitor *m);

int mount_monitor_stop(struct mount_monitor *m);

void mount_monitor_free(struct mount_monitor *m);

#endif
