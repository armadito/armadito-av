#include "libuhuru-config.h"

#include "dir.h"

#include <glib.h>
#include <dirent.h>
#include <sys/types.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static enum dir_entry_flag dirent_flags(struct dirent *entry)
{
  switch(entry->d_type) {
  case DT_DIR:
    return DIR_ENTRY_IS_DIR;
  case DT_BLK:
  case DT_CHR:
    return DIR_ENTRY_IS_DEV;
#ifndef WIN32
  case DT_SOCK:
#endif
  case DT_FIFO:
    return DIR_ENTRY_IS_IPC;
#ifndef WIN32
  case DT_LNK:
    return DIR_ENTRY_IS_LINK;
#endif
  case DT_REG:
    return DIR_ENTRY_IS_REG;
  default:
    return DIR_ENTRY_IS_UNKNOWN;
  }

  return DIR_ENTRY_IS_UNKNOWN;
}

int dir_map(const char *path, int recurse, dirent_fun_t dirent_fun, void *data)
{
  DIR *d;
  struct dirent entry, *result;
  int ret = 0;
  enum dir_entry_flag flags = 0;
    
  d = opendir(path);
  if (d == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "error opening directory %s (%s)", path, os_strerror(errno));

    flags |= DIR_ENTRY_IS_ERROR;
    (*dirent_fun)(path, flags, errno, data);
    
    if (errno != EACCES && errno != ENOENT && errno != ENOTDIR)
      ret = 1;

    goto cleanup;
  }

  while(1) {
    char *entry_path;
    int r = readdir_r(d, &entry, &result);

    if (r != 0) {
      g_log(NULL, G_LOG_LEVEL_WARNING, "error reading directory entry (%s)", os_strerror(errno));
     
      flags |= DIR_ENTRY_IS_ERROR;
      (*dirent_fun)(path, flags, errno, data);

      //  ret = 1;

      goto cleanup;
    }

    if (result == NULL)
      break;

    if (entry.d_type == DT_DIR && (!strcmp(entry.d_name, ".") || !strcmp(entry.d_name, "..")))
      continue;

    if (asprintf(&entry_path, "%s/%s", path, entry.d_name) == -1) {
      ret = 1;
      goto cleanup;
    }

    if (entry.d_type == DT_DIR && recurse) {
      r = dir_map(entry_path, recurse, dirent_fun, data);

      if (r != 0) {
	ret = 1;
	free(entry_path);
	goto cleanup;
      }
    }

    flags = dirent_flags(&entry);
    (*dirent_fun)(entry_path, flags, 0, data);

    free(entry_path);
  }

 cleanup:
  closedir(d);

  return ret;
}

/*
 * Returns:
 * 1 if path exists
 * 0 it path does not exist and must be created
 * -1 if error (path exists and is not a directory, or other error)
 */
static int stat_dir(const char *path)
{
  struct stat st;

  if (!stat(path, &st) && S_ISDIR(st.st_mode))
    return 1;

  return (errno == ENOENT) ? 0 : -1;
}

int mkdir_p(const char *path)
{
  char *token, *full, *end;
  int ret = 0;
     
  token = full = os_strdup(path);
  do {
    end = strchr(token, '/');

    if (token != end) {
      if (end != NULL)
	*end = '\0';

      if (!(ret = stat_dir(full))) {
	ret = mkdir(full, 0777);
      }

      if (end != NULL)
	*end = '/';
    }
    token = end + 1;
  } while (end != NULL && ret >= 0);

  return ret;
}
