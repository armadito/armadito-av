#include "libuhuru-config.h"

#include "os/file.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

void os_file_stat(const char *path, struct os_file_stat *buf, int *pfile_errno)
{
  struct stat sb;

  if (stat(path, &sb) == -1) {
    *pfile_errno = errno;
    buf->flags = FILE_FLAG_IS_ERROR;

    return;
  }

  switch (sb.st_mode & S_IFMT) {
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

  return;
}
