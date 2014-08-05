#ifndef _LIBUMWSU_STATUS_H_
#define _LIBUMWSU_STATUS_H_

enum umw_status {
  UMW_CLEAN,
  UMW_MALWARE,
  UMW_EINVAL,
  UMW_IERROR,
  UMW_UNKNOWN_FILE_TYPE,
};

const char *umw_status_str(enum umw_status status);

#endif
