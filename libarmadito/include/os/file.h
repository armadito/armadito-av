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

#ifndef _LIBARMADITO_OS_FILE_H_
#define _LIBARMADITO_OS_FILE_H_

#include <stdio.h>
#include <stddef.h>

#ifdef _WIN32
FILE * os_fopen(const char * filename,const char * mode);
#else
#define os_fopen fopen
#endif

enum os_file_flag {
	FILE_FLAG_IS_ERROR       = 1 << 0,  /* entry is an error, error is given in file_errno */
	FILE_FLAG_IS_DIRECTORY   = 1 << 1,  /* entry is a directory */
	FILE_FLAG_IS_PLAIN_FILE  = 1 << 2,  /* entry is a regular file */
	FILE_FLAG_IS_LINK        = 1 << 3,  /* entry is a link or a shortcut */
	FILE_FLAG_IS_DEVICE      = 1 << 4,  /* entry is a device */
	FILE_FLAG_IS_IPC         = 1 << 5,  /* entry is an IPC (socket, named pipe...) */
	FILE_FLAG_IS_UNKNOWN     = 1 << 6,  /* entry is not a known type */
};

struct os_file_stat {
	enum os_file_flag flags;
	size_t file_size;
};

int os_file_stat(const char *path, struct os_file_stat *buf, int *pfile_errno);

int os_file_stat_fd(int fd, struct os_file_stat *buf, int *pfile_errno);

/**
 *      \fn int os_file_do_not_scan(const char *path);
 *      \brief Returns true if path must never be scanned (like /proc on linux)
 *
 *      \param[in] path the path of the file
 *
 *      \return 1 if path must not be scanned, 0 if not
 */
int os_file_do_not_scan(const char *path);

#endif
