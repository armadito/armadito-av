#include "dir.h"

#include <dirent.h>
#include <sys/types.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int dir_map(const char *path, int recurse, void (*dirent_fun)(const char *fpath, const struct dirent *dir_entry, void *data), void *data)
{
  DIR *d;
  struct dirent entry, *result;
  int ret = 0;

  d = opendir(path);
  if (d == NULL) {
    fprintf(stderr, "error opening directory %s (%s)\n", path, strerror(errno));
    ret = 1;
    goto cleanup;
  }

  while(1) {
    char *entry_path;
    int r = readdir_r(d, &entry, &result);

    if (r != 0) {
      fprintf(stderr, "error reading directory entry (%s)\n", strerror(errno));
      ret = 1;
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

    (*dirent_fun)(entry_path, &entry, data);

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
     
  token = full = strdup(path);
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
