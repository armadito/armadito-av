#ifndef _LIBUMWSU_STATUS_H_
#define _LIBUMWSU_STATUS_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

enum umwsu_file_status {
  UMWSU_UNDECIDED = 1,
  UMWSU_CLEAN,
  UMWSU_UNKNOWN_FILE_TYPE,
  UMWSU_EINVAL,
  UMWSU_IERROR,
  UMWSU_SUSPICIOUS,
  UMWSU_WHITE_LISTED,
  UMWSU_MALWARE,
};

#ifdef __cplusplus
}
#endif

#endif
