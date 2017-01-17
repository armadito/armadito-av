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

#ifndef _IMONITOR_H_
#define _IMONITOR_H_

#include "monitor.h"

struct inotify_monitor;

struct inotify_monitor *inotify_monitor_new(struct access_monitor *m);

int inotify_monitor_start(struct inotify_monitor *im);

int inotify_monitor_mark_directory(struct inotify_monitor *im, const char *path);

int inotify_monitor_unmark_directory(struct inotify_monitor *im, const char *path);

#endif
