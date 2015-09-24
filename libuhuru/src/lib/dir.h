#ifndef _LIBUHURU_DIR_H_
#define _LIBUHURU_DIR_H_

enum dir_entry_flag {
  DIR_ENTRY_IS_ERROR     = 1 << 0,
  DIR_ENTRY_IS_DIR       = 1 << 1,
  DIR_ENTRY_IS_REG       = 1 << 2,
  DIR_ENTRY_IS_LINK      = 1 << 3,
  DIR_ENTRY_IS_DEV       = 1 << 4,
  DIR_ENTRY_IS_IPC       = 1 << 5,
  DIR_ENTRY_IS_UNKNOWN   = 1 << 6,
};

typedef void (*dirent_fun_t)(const char *full_path, enum dir_entry_flag flags, int entry_errno, void *data);

int dir_map(const char *path, int recurse, dirent_fun_t dirent_fun, void *data);

int mkdir_p(const char *path);

#endif
