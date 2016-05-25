#include "osdeps.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

char * os_strncpy(char * dest, size_t sizeDest, char * src, size_t count) {

	char * ret = NULL;

	ret = strncpy( dest, src, count );
	
	return ret;
}

char * os_strncat(char * dest, size_t sizeDest, char * src, size_t count) {

	char * ret = NULL;

	ret = strncat( dest, src, count );
	
	return ret;
}

int os_file_size(int fd, int *pfile_errno)
{
	struct stat sb;

	if (fstat(fd, &sb) == -1) {
		*pfile_errno = errno;
		return -1;
	}

	return sb.st_size;
}
