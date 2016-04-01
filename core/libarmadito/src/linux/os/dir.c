#define _GNU_SOURCE
#include <libarmadito.h>

#include "libarmadito-config.h"

#include "os/dir.h"

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

void os_dir_map(const char *path, int recurse, dirent_cb_t dirent_cb, void *data)
{
	DIR *d;

	d = opendir(path);
	if (d == NULL) {
		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "error opening directory %s (%s)", path, strerror(errno));

		// Call to scan_entry()
		(*dirent_cb)(path, FILE_FLAG_IS_ERROR, errno, data);

#if 0
		if (errno != EACCES && errno != ENOENT && errno != ENOTDIR)
			ret = 1;
#endif

		goto cleanup;
	}

	while(1) {
		char *entry_path, *real_entry_path;
		struct dirent *entry;
		int saved_errno;

		errno = 0;
		entry = readdir(d);

		if (entry == NULL) {
			/* from man readdir: If the end of the directory stream is reached, NULL is returned and errno is not changed */
			if (errno == 0)
				break;

			saved_errno = errno;
			a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "error reading directory entry in directory %s (error %s)", path, strerror(saved_errno));

			// Call to scan_entry()
			(*dirent_cb)(path, FILE_FLAG_IS_ERROR, saved_errno, data);

			goto cleanup;
		}

		if (entry->d_type == DT_DIR && (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")))
			continue;

		if (asprintf(&entry_path, "%s/%s", path, entry->d_name) == -1)
			goto cleanup;

		if (entry->d_type == DT_DIR && recurse)
			os_dir_map(entry_path, recurse, dirent_cb, data);

		// Call to scan_entry()
		real_entry_path = realpath(entry_path, NULL);
		(*dirent_cb)(real_entry_path, dirent_flags(entry), 0, data);

		free(entry_path);
		free(real_entry_path);
	}

cleanup:
	closedir(d);
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
