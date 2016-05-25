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

#ifndef _RESPONSE_H_
#define _RESPONSE_H_

#include <linux/fanotify.h>

enum response_reason {
	RR_TIMEOUT                       = 0,
	RR_SCANNED_ALLOWED               = 1,
	RR_SCANNED_DENIED                = 2,
	RR_FILE_TYPE_NOT_SCANNED         = 3,
	RR_NOT_REGULAR_FILE              = 4,
	RR_FILE_STAT_FAILED              = 5,
	RR_EVENT_PID_IS_MYSELF           = 6,
	RR_LAST                          = 6,
};

void response_write(int fanotify_fd, int fd, __u32 r, const char *path, const char *reason);

#endif
