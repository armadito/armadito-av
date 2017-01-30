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

#include <libarmadito/armadito.h>
#include <armadito-config.h>

#include "response.h"
#include "modname.h"

#include <errno.h>
#include <linux/fanotify.h>
#include <string.h>
#include <unistd.h>

static const char *rr_str[] = {
	[RR_TIMEOUT]                     = "timeout",
	[RR_SCANNED_ALLOWED]             = "file scanned and allowed",
	[RR_SCANNED_DENIED]              = "file scanned and denied",
	[RR_FILE_TYPE_NOT_SCANNED]       = "file type is not scanned",
	[RR_NOT_REGULAR_FILE]            = "file is not a regular file",
	[RR_FILE_STAT_FAILED]            = "file stat() failed",
	[RR_EVENT_PID_IS_MYSELF]         = "event PID is myself",
};

void response_write(int fanotify_fd, int fd, __u32 r, const char *path, const char *reason)
{
	struct fanotify_response response;
	ssize_t ret;

	response.fd = fd;
	response.response = r;

	if ((ret = write(fanotify_fd, &response, sizeof(struct fanotify_response))) != sizeof(struct fanotify_response))
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "writing to fd %d failed (%d, %s)", fanotify_fd, ret, strerror(errno));

	if (close(fd) != 0)
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "fd %3d path %s closed failed (%s)", fd, path != NULL ? path : "null", fd, strerror(errno));

#ifdef DEBUG
	{
		enum a6o_log_level log_level = A6O_LOG_LEVEL_INFO;
		const char *auth = "ALLOW";

		if (r == FAN_DENY) {
			log_level = A6O_LOG_LEVEL_WARNING;
			auth = "DENY";
		}

		// we don't log this special case
		if(reason != NULL && strncmp(reason,"PID is myself",13) == 0)
                  return;

		a6o_log(A6O_LOG_MODULE, log_level, MODULE_LOG_NAME ": " "fd %3d path %s %s (%s)", fd, path != NULL ? path : "null", auth, reason != NULL ? reason : "unknown");
	}
#endif
}

