#ifndef _LIBUMWSU_DIR_H_
#define _LIBUMWSU_DIR_H_

#include <dirent.h>

int dir_map(const char *path, int recurse, void (*dirent_fun)(const char *fpath, const struct dirent *dir_entry, void *data), void *data);

int mkdir_p(const char *path);

#endif
