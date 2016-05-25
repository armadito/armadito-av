/***

Copyright (C) 2015, 2016, 2017 Teclib'

This file is part of Armadito.

Armadito is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <glib.h>

struct access_monitor;

struct access_monitor *access_monitor_new(struct armadito *armadito);

int access_monitor_enable(struct access_monitor *m, int enable);

int access_monitor_is_enable(struct access_monitor *m);

int access_monitor_enable_permission(struct access_monitor *m, int enable_permission);

int access_monitor_is_enable_permission(struct access_monitor *m);

int access_monitor_enable_removable_media(struct access_monitor *m, int enable_removable_media);

int access_monitor_is_enable_removable_media(struct access_monitor *m);

GMainContext *access_monitor_get_main_context(struct access_monitor *m);

void access_monitor_add_mount(struct access_monitor *m, const char *mount_point);

void access_monitor_add_directory(struct access_monitor *m, const char *path);

int access_monitor_recursive_mark_directory(struct access_monitor *m, const char *path);

int access_monitor_unmark_directory(struct access_monitor *m, const char *path);

int access_monitor_delayed_start(struct access_monitor *m);

#define ACCESS_MONITOR_START    ((char)0x1)
#define ACCESS_MONITOR_STOP     ((char)0x2)
#define ACCESS_MONITOR_STATUS   ((char)0x3)

int access_monitor_send_command(struct access_monitor *m, char command);

#endif
