#ifndef _LIBUHURU_OS_FILE_H_
#define _LIBUHURU_OS_FILE_H_

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
  /* will may be contain more info, like size */
};

void os_file_stat(const char *path, struct os_file_stat *buf, int *pfile_errno);

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
