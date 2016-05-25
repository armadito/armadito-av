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

