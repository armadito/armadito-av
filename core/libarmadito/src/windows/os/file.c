#include "libuhuru-config.h"

#include "os/file.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

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
