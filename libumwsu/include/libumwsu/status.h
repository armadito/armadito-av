#ifndef _LIBUMWSU_STATUS_H_
#define _LIBUMWSU_STATUS_H_

#ifdef __cplusplus
extern "C" {
#endif

enum umwsu_status {
  UMWSU_CLEAN,
  UMWSU_SUSPICIOUS,
  UMWSU_MALWARE,
  UMWSU_EINVAL,
  UMWSU_IERROR,
  UMWSU_UNKNOWN_FILE_TYPE,
};

const char *umwsu_status_str(enum umwsu_status status);

#ifdef __cplusplus
}
#endif

#endif
