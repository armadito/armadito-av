#ifndef _LIBUMWSU_UMWSU_H_
#define _LIBUMWSU_UMWSU_H_

enum umw_status {
  UMW_CLEAN,
  UMW_MALWARE,
  UMW_EINVAL,
  UMW_IERROR,
};

struct umw;

struct umw *umw_open(void);

enum umw_status umw_scan_file(struct umw *umw_handle, const char *path);

enum umw_status umw_scan_dir(struct umw *umw_handle, const char *path, int recurse);

#endif
