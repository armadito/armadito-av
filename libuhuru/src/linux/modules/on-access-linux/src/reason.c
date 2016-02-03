#include "reason.h"

static const char *rr_str[] = {
  [RR_TIMEOUT]                     = "timeout",
  [RR_SCANNED_ALLOWED]             = "file scanned and allowed",
  [RR_SCANNED_DENIED]              = "file scanned and denied",
  [RR_FILE_TYPE_NOT_SCANNED]       = "file type is not scanned",
  [RR_NOT_REGULAR_FILE]            = "file is not a regular file",
  [RR_FILE_STAT_FAILED]            = "file stat() failed",
  [RR_EVENT_PID_IS_MYSELF]         = "event PID is myself",
};

const char *response_reason_str(enum response_reason rr)
{
  return rr_str[rr];
}
