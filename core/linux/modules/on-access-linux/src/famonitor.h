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

#ifndef _FAMONITOR_H_
#define _FAMONITOR_H_

#include <libarmadito.h>

struct fanotify_monitor;

struct fanotify_monitor *fanotify_monitor_new(struct access_monitor *m, struct armadito *u);

int fanotify_monitor_start(struct fanotify_monitor *f);

int fanotify_monitor_mark_directory(struct fanotify_monitor *f, const char *path, int enable_permission);

int fanotify_monitor_unmark_directory(struct fanotify_monitor *f, const char *path, int enable_permission);

int fanotify_monitor_mark_mount(struct fanotify_monitor *f, const char *path, int enable_permission);

int fanotify_monitor_unmark_mount(struct fanotify_monitor *f, const char *path, int enable_permission);

#endif
