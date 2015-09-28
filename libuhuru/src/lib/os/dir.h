#ifndef _LIBUHURU_OS_DIR_H_
#define _LIBUHURU_OS_DIR_H_

enum dir_entry_flag {
  DIR_ENTRY_IS_ERROR       = 1 << 0,  /* entry is an error, error is given in entry_errno */
  DIR_ENTRY_IS_DIRECTORY   = 1 << 1,  /* entry is a directory */
  DIR_ENTRY_IS_PLAIN_FILE  = 1 << 2,  /* entry is a regular file */
  DIR_ENTRY_IS_LINK        = 1 << 3,  /* entry is a link or a shortcut */
  DIR_ENTRY_IS_DEVICE      = 1 << 4,  /* entry is a device */
  DIR_ENTRY_IS_IPC         = 1 << 5,  /* entry is an IPC (socket, named pipe...) */
  DIR_ENTRY_IS_UNKNOWN     = 1 << 6,  /* entry is not a known type */
};

typedef void (*dirent_cb_t)(const char *full_path, enum dir_entry_flag flags, int entry_errno, void *data);

int os_dir_map(const char *path, int recurse, dirent_cb_t dirent_cb, void *data);

int os_mkdir_p(const char *path);

#endif
