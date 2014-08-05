#ifndef _LIBUMWSU_SCAN_H_
#define _LIBUMWSU_SCAN_H_

#include <libumwsu/status.h>

struct umw;

struct umw *umw_open(void);

void umw_close(struct umw *u);

enum umw_status umw_scan_file(struct umw *umw_handle, const char *path);

enum umw_status umw_scan_dir(struct umw *umw_handle, const char *path, int recurse);

#endif
