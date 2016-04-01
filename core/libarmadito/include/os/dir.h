#ifndef _LIBARMADITO_OS_DIR_H_
#define _LIBARMADITO_OS_DIR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os/file.h"   /* for os_file_flag */

typedef void (*dirent_cb_t)(const char *full_path, enum os_file_flag flags, int entry_errno, void *data);

void os_dir_map(const char *path, int recurse, dirent_cb_t dirent_cb, void *data);

int os_mkdir_p(const char *path);

/* What is this doing here? */
char * GetBinaryDirectory( );

#ifdef __cplusplus
}
#endif

#endif
