#include "libuhuru-config.h"

#include "os/string.h"
#include "os/dir.h"
#include "Windows.h"
#include <glib.h>
#include <sys/types.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static enum dir_entry_flag dirent_flags(DWORD fileAttributes)
{
	switch(fileAttributes) {

		case FILE_ATTRIBUTE_DIRECTORY:
			return FILE_FLAG_IS_DIRECTORY;	  
		case FILE_ATTRIBUTE_DEVICE:
			return FILE_FLAG_IS_DEVICE;	  	  
		//case DT_LNK:
		//return FILE_FLAG_IS_LINK;
		case FILE_ATTRIBUTE_NORMAL:
			return FILE_FLAG_IS_PLAIN_FILE;
		case FILE_ATTRIBUTE_ARCHIVE:
			return FILE_FLAG_IS_PLAIN_FILE;
		default:
			return FILE_FLAG_IS_UNKNOWN;

	}

  return FILE_FLAG_IS_UNKNOWN;
}

BOOL DirectoryExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}


void os_dir_map(const char *path, int recurse, dirent_cb_t dirent_cb, void *data ) {

	char * sPath = NULL, *entryPath= NULL;
	HANDLE fh = NULL;
	int size = 0;
	WIN32_FIND_DATAA fdata ;
	WIN32_FIND_DATAA tmp ;
	enum os_file_flag flags;

	// Check parameters
	if (path == NULL || dirent_cb == NULL) {
		g_log(NULL, G_LOG_LEVEL_WARNING, "error :: NULL parameter in function os_dir_map()");
		return;
	}
	
	// Check if it is a directory
	if (!(GetFileAttributesA(path) & FILE_ATTRIBUTE_DIRECTORY)) {
		g_log(NULL, G_LOG_LEVEL_WARNING, "Warning :: os_dir_map() :: (%s) is not a directory. ", path);
		return;
	}

	size = strlen(path) + 3;
	sPath = (char*)calloc(size + 1, sizeof(char));
	sPath[size] = '\0';
	sprintf_s(sPath, size, "%s\\*", path);

	// g_log(NULL, G_LOG_LEVEL_WARNING, "os_dir_map() :: (%s)", sPath);

	fh = FindFirstFileA(sPath, &fdata);
	if (fh == INVALID_HANDLE_VALUE) {
		g_log(NULL, G_LOG_LEVEL_WARNING, "warning :: os_dir_map() :: FindFirstFileA() failed ::  (%s) ",os_strerror(errno));
		return;
	}

	while (FindNextFileA(fh, &tmp) != FALSE) {

		// exclude paths "." and ".."
		if (strncmp(tmp.cFileName,".",strlen(tmp.cFileName)) == 0 || strncmp(tmp.cFileName,"..",strlen(tmp.cFileName)) == 0)
		{
			continue;
		}

		// build the entry complete path.
		size = strlen(path)+strlen(tmp.cFileName)+2;
		free(entryPath);
		entryPath = (char*)calloc(size + 1, sizeof(char));
		entryPath[size] = '\0';
		sprintf_s(entryPath, size,"%s\\%s", path, tmp.cFileName);


		// If it is a directory and we do recursive scan
		if ((GetFileAttributesA(entryPath) & FILE_ATTRIBUTE_DIRECTORY) && recurse >= 1) {
			os_dir_map(entryPath, recurse, dirent_cb, NULL);
		}
		else {
			// Add file to scan
			// g_log(NULL, G_LOG_LEVEL_WARNING, "Scanning file !: %s", entryPath);

			flags = dirent_flags(tmp.dwFileAttributes);
			(*dirent_cb)(entryPath, flags, 0, data);
		}
	}

	free(entryPath);
	free(sPath);
	CloseHandle(fh);

	return;
}

/*
 * Returns:
 * 1 if path exists
 * 0 it path does not exist and must be created
 * -1 if error (path exists and is not a directory, or other error)
 */
#if 0
static int stat_dir(const char *path)
{
  struct stat st;

  if (!stat(path, &st) && S_ISDIR(st.st_mode))
    return 1;

  return (errno == ENOENT) ? 0 : -1;
}
#endif

int os_mkdir_p(const char *path)
{
	fprintf(stderr, "os_mkdir_p not implemented\n");
	return -1;
}
