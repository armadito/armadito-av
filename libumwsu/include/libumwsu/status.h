#ifndef _LIBUMWSU_STATUS_H_
#define _LIBUMWSU_STATUS_H_

enum umwsu_status {
  UMWSU_CLEAN,
  UMWSU_MALWARE,
  UMWSU_EINVAL,
  UMWSU_IERROR,
  UMWSU_UNKNOWN_FILE_TYPE,
};

const char *umwsu_status_str(enum umwsu_status status);

#endif
