#ifndef _REASON_H_
#define _REASON_H_

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

const char *response_reason_str(enum response_reason rr);

#endif
