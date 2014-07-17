#ifndef _LIBUMWSU_DIR_H_
#define _LIBUMWSU_DIR_H_

#include <dirent.h>

int dir_map(const char *path, int recurse, void (*dirent_fun)(const char *full_path, void *data), void *data);

#endif
