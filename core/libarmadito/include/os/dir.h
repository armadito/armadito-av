#ifndef _LIBARMADITO_OS_DIR_H_
#define _LIBARMADITO_OS_DIR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os/file.h"   /* for os_file_flag */

/* the callback function, called for each entry (file or directory) */
/* if it returns a nonzero value, the tree walk is stopped and this returned value become the return value of os_dir_map() */
/* if it returns 0, os_dir_map() will continue until it has traversed the entire tree, in which case it will return zero, or */
/* until it encounters an error, in which case it will return -1 */
typedef int (*dirent_cb_t)(const char *full_path, enum os_file_flag flags, int entry_errno, void *data);

int os_dir_map(const char *path, int recurse, dirent_cb_t dirent_cb, void *data);

int os_mkdir_p(const char *path);

#ifdef __cplusplus
}
#endif

#endif
