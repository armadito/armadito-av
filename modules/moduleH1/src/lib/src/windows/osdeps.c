/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito module H1.

Armadito module H1 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito module H1 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Armadito module H1.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "osdeps.h"
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

FILE * os_fopen(const char * filename, const char * mode) {

	FILE * f = NULL;

	fopen_s(&f, filename,mode);

	return f;
}

int os_file_size(int fd, int *pfile_errno)
{
	struct stat sb;

	if (_fstat(fd, &sb) == -1) {
		*pfile_errno = errno;
		return -1;
	}

	return sb.st_size;
}

