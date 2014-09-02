#ifndef _LIBUMWSU_STATUS_H_
#define _LIBUMWSU_STATUS_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

enum umwsu_status {
  UMWSU_CLEAN,
  UMWSU_UNKNOWN_FILE_TYPE,
  UMWSU_EINVAL,
  UMWSU_IERROR,
  UMWSU_SUSPICIOUS,
  UMWSU_MALWARE,
};

const char *umwsu_status_str(enum umwsu_status status);
const char *umwsu_status_pretty_str(enum umwsu_status status);

#ifdef __cplusplus
}
#endif

#endif
