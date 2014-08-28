#include "dir.h"

#include <dirent.h>
#include <sys/types.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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

