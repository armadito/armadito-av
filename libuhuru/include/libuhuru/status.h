#ifndef _LIBUHURU_STATUS_H_
#define _LIBUHURU_STATUS_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

enum uhuru_file_status {
  UHURU_UNDECIDED = 1,
  UHURU_CLEAN,
  UHURU_UNKNOWN_FILE_TYPE,
  UHURU_EINVAL,
  UHURU_IERROR,
  UHURU_SUSPICIOUS,
  UHURU_WHITE_LISTED,
  UHURU_MALWARE,
};

#ifdef __cplusplus
}
#endif

#endif
