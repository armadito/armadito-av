/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef ARMADITO_CORE_OS_DIR_H
#define ARMADITO_CORE_OS_DIR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/file.h"   /* for os_file_flag */

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
