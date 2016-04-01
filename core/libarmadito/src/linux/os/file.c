#include <libarmadito.h>
#include "libarmadito-config.h"

#include "os/file.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static void fill_stat(struct os_file_stat *buf, struct stat *sb)
{
	switch (sb->st_mode & S_IFMT) {
	case S_IFBLK:
	case S_IFCHR:
		buf->flags = FILE_FLAG_IS_DEVICE;
		break;
	case S_IFDIR:
		buf->flags = FILE_FLAG_IS_DIRECTORY;
		break;
	case S_IFIFO:
	case S_IFSOCK:
		buf->flags = FILE_FLAG_IS_IPC;
		break;
	case S_IFLNK:
		buf->flags = FILE_FLAG_IS_LINK;
		break;
	case S_IFREG:
		buf->flags = FILE_FLAG_IS_PLAIN_FILE;
		break;
	default:
		buf->flags = FILE_FLAG_IS_UNKNOWN;
		break;
	}

	buf->file_size = sb->st_size;
}

int os_file_stat(const char *path, struct os_file_stat *buf, int *pfile_errno)
{
	struct stat sb;

	if (stat(path, &sb) == -1) {
		*pfile_errno = errno;
		buf->flags = FILE_FLAG_IS_ERROR;

		return -1;
	}

	fill_stat(buf, &sb);

	return 0;
}

int os_file_stat_fd(int fd, struct os_file_stat *buf, int *pfile_errno)
{
	struct stat sb;

	if (fstat(fd, &sb) == -1) {
		*pfile_errno = errno;
		buf->flags = FILE_FLAG_IS_ERROR;

		return -1;
	}

	fill_stat(buf, &sb);

	return 0;
}

static const char *do_not_scan_paths[] = {
	"/proc",
	"/run",
	"/sys",
	NULL,
};

static int strprefix(char *s, const char *prefix)
{
	while (*prefix && *s && *prefix++ == *s++)
		;

	if (*prefix == '\0')
		return *s == '\0' || *s == '/';

	return 0;
}

int os_file_do_not_scan(const char *path)
{
	const char **p;
	char * real_path;

	real_path =  realpath(path, NULL);
	if( real_path == NULL){
		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_ERROR, "realpath of %s failed : %s.", path, strerror(errno));
		return 1;
	}

	for(p = do_not_scan_paths; *p != NULL; p++) {
		if (strprefix(real_path, *p)){
			free(real_path);
			return 1;
		}
	}

	free(real_path);
	return 0;
}
