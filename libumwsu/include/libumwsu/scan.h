#ifndef _LIBUMWSU_SCAN_H_
#define _LIBUMWSU_SCAN_H_

enum umw_status {
  UMW_CLEAN,
  UMW_INFECTED,
  UMW_EINVAL,
  UMW_IERROR,
};

enum umw_status umw_scan_file(const char *path);

enum umw_status umw_scan_dir(const char *path, int recurse);

#endif
