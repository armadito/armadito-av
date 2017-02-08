/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#define _GNU_SOURCE
#include <libarmadito/armadito.h>

#include "armadito-config.h"

#include "core/dir.h"

#include <glib.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static enum os_file_flag dirent_flags(struct dirent *entry)
{
	switch(entry->d_type) {
	case DT_DIR:
		return FILE_FLAG_IS_DIRECTORY;
	case DT_BLK:
	case DT_CHR:
		return FILE_FLAG_IS_DEVICE;
	case DT_SOCK:
	case DT_FIFO:
		return FILE_FLAG_IS_IPC;
	case DT_LNK:
		return FILE_FLAG_IS_LINK;
	case DT_REG:
		return FILE_FLAG_IS_PLAIN_FILE;
	default:
		return FILE_FLAG_IS_UNKNOWN;
	}

	return FILE_FLAG_IS_UNKNOWN;
}

int os_dir_map(const char *path, int recurse, dirent_cb_t dirent_cb, void *data)
{
	DIR *d;
	int ret = 0;

	d = opendir(path);
	if (d == NULL) {
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "error opening directory %s (%s)", path, strerror(errno));

		// Call to scan_entry()
		 ret = (*dirent_cb)(path, FILE_FLAG_IS_ERROR, errno, data);

#if 0
		if (errno != EACCES && errno != ENOENT && errno != ENOTDIR)
			ret = 1;
#endif

		// goto cleanup;
		return ret;
	}

	while(1) {
		char *entry_path;
		char *real_entry_path;
		struct dirent *entry;
		int saved_errno;

		errno = 0;
		entry = readdir(d);

		if (entry == NULL) {
			/* from man readdir: If the end of the directory stream is reached, NULL is returned and errno is not changed */
			if (errno == 0)
				break;

			saved_errno = errno;
			a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "error reading directory entry in directory %s (error %s)", path, strerror(saved_errno));

			// Call to scan_entry()
			ret = (*dirent_cb)(path, FILE_FLAG_IS_ERROR, saved_errno, data);
			break;
		}

		if (entry->d_type == DT_DIR && (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")))
			continue;

		if (asprintf(&entry_path, "%s/%s", path, entry->d_name) == -1){
			ret = -1;
			break;
		}

		if (entry->d_type == DT_DIR && recurse) {
		      ret = os_dir_map(entry_path, recurse, dirent_cb, data);
		      if (ret != 0) {
			free(entry_path);
			break;
		      }
		}
		else {
			/* Call to scan_entry() */
			real_entry_path = realpath(entry_path, NULL);
			(*dirent_cb)(real_entry_path, dirent_flags(entry), 0, data);
			free(real_entry_path);
		}

		free(entry_path);
	}

	if (closedir(d) < 0)
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "error closing directory %s (%s)", path, strerror(errno));

	return 0;
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

int os_mkdir_p(const char *path)
{
	char *token;
	char *full;
	char *end;
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

	if (full != NULL) {
		free(full);
	}

	return ret;
}
