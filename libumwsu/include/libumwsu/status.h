#ifndef _LIBUMWSU_STATUS_H_
#define _LIBUMWSU_STATUS_H_

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

struct umwsu_report {
  enum umwsu_status status;
  char *path;
  char *module_name;
  char *module_report;
};

struct umwsu_report umwsu_report_new(enum umwsu_status status, char *path, char *module_name, char *module_report);

void umwsu_report_free(struct umwsu_report *report);

#ifdef __cplusplus
}
#endif

#endif
