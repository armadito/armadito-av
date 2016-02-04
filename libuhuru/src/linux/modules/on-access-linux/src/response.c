#include <libuhuru/core.h>
#include <config/libuhuru-config.h>

#include "response.h"
#include "onaccessmod.h"

#include <linux/fanotify.h>
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

#if 0
void response_write_watchdog(struct access_monitor *m, int fd, __u32 r, const char *path, const char *reason)
{
  const char *path;

  if (dequeue && !watchdog_remove(m->watchdog, fd, &path, NULL))
      return;
}
#endif

void response_write(int fanotify_fd, int fd, __u32 r, const char *path, const char *reason)
{
  struct fanotify_response response;
  enum uhuru_log_level log_level = UHURU_LOG_LEVEL_INFO;
  const char *auth = "ALLOW";

  response.fd = fd;
  response.response = r;

  write(fanotify_fd, &response, sizeof(struct fanotify_response));
  
  close(fd);

  if (r == FAN_DENY) {
    log_level = UHURU_LOG_LEVEL_WARNING;
    auth = "DENY";
  }

  if (path == NULL)
    uhuru_log(UHURU_LOG_MODULE, log_level, MODULE_NAME ": " "fd %d %s (%s)", fd, auth, reason != NULL ? reason : "unknown");
  else
    uhuru_log(UHURU_LOG_MODULE, log_level, MODULE_NAME ": " "path %s %s (%s)", path, auth, reason != NULL ? reason : "unknown");
}

