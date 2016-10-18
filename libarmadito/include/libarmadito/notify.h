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

#ifndef LIBARMADITO_NOTIFY_H_
#define LIBARMADITO_NOTIFY_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

enum notif_type {
	NONE = 0,
	NOTIF_INFO,
	NOTIF_WARNING,
	NOTIF_ERROR
};

typedef int (*a6o_notify_handler_t)(enum notif_type type, const char *message);

void a6o_notify_set_handler(a6o_notify_handler_t handler);
int a6o_notify_default_handler(enum notif_type type, const char *message);
int a6o_notify(enum notif_type type, const char *format, ...);

#endif
