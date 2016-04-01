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
