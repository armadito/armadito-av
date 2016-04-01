#include <libarmadito.h>
#include <libarmadito-config.h>

#include "response.h"
#include "onaccessmod.h"

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
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "writing to fd %d failed (%d, %s)", fanotify_fd, ret, strerror(errno));

	if (close(fd) != 0)
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "fd %3d path %s closed failed (%s)", fd, path != NULL ? path : "null", fd, strerror(errno));

#ifdef DEBUG
	{
		enum a6o_log_level log_level = ARMADITO_LOG_LEVEL_INFO;
		const char *auth = "ALLOW";

		if (r == FAN_DENY) {
			log_level = ARMADITO_LOG_LEVEL_WARNING;
			auth = "DENY";
		}

		a6o_log(ARMADITO_LOG_MODULE, log_level, MODULE_LOG_NAME ": " "fd %3d path %s %s (%s)", fd, path != NULL ? path : "null", auth, reason != NULL ? reason : "unknown");
	}
#endif
}

