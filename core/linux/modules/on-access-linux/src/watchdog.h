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

#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

struct watchdog;

/* Standard value of timeout (in micro seconds) */
#define TIMEOUT  1000

struct watchdog *watchdog_new(int fanotify_fd);

void watchdog_add(struct watchdog *w, int fd);

int watchdog_remove(struct watchdog *w, int fd, struct timespec *after);

#endif
