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

#include "libarmadito-config.h"

#include "os/file.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

FILE * os_fopen(char * filename, char * mode) {

	FILE * f = NULL;
	errno_t err = 0;

	err = fopen_s(&f,filename,mode);
	if (err != 0) {
		return NULL;
	}

	return f;

}

int os_file_stat(const char *path, struct os_file_stat *buf, int *pfile_errno)
{
	struct stat sb;

	if (_stat(path, &sb) == -1) {
		*pfile_errno = errno;
		buf->flags = FILE_FLAG_IS_ERROR;
		return -1;
	}

	if (sb.st_mode & S_IFDIR)
		buf->flags = FILE_FLAG_IS_DIRECTORY;
	else if (sb.st_mode & S_IFREG)
		buf->flags = FILE_FLAG_IS_PLAIN_FILE;
	else
		buf->flags = FILE_FLAG_IS_UNKNOWN;

	return 0;
}

int os_file_do_not_scan(const char *path)
{
  return 0;
}
