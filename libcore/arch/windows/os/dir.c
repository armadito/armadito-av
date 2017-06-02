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

#include <libarmadito/armadito.h>
#include "armadito-config.h"

#include "string_p.h"
#include <core/dir.h>

#include <Windows.h>

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static enum os_file_flag dirent_flags(DWORD fileAttributes)
{
	/* (FD) fixed test which was not returning PLAIN_FILE for dlls */
	if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return FILE_FLAG_IS_DIRECTORY;
	if (fileAttributes & FILE_ATTRIBUTE_DEVICE)
		return FILE_FLAG_IS_DEVICE;
	if (fileAttributes & FILE_ATTRIBUTE_NORMAL
		|| fileAttributes & FILE_ATTRIBUTE_ARCHIVE
		|| fileAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)
		return FILE_FLAG_IS_PLAIN_FILE;

	return FILE_FLAG_IS_UNKNOWN;
}

static BOOL DirectoryExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

/*
  -- Note : Escaping not used anymore. But we need to be able to scan Chinese encoded name files

  void replaceAll(std::string& str, const std::string& from, const std::string& to) {
  if (from.empty())
  return;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
  str.replace(start_pos, from.length(), to);
  start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
  }

  // We want escape ? and * characters
  char *cpp_escape_str(const char * str_in){

  char * str_out = NULL;
  std::string str(str_in);

  std::string c1("?");
  std::string c2("*");
  std::string c3("\\");
  std::string c1_("\\?");
  std::string c2_("\\*");
  std::string c3_("\\\\");

  replaceAll(str, c3, c3_);
  replaceAll(str, c1, c1_);
  replaceAll(str, c2, c2_);

  return _strdup(str.c_str());
  }
  */


int os_dir_map(const char *path, int recurse, dirent_cb_t dirent_cb, void *data) {

	char * sPath = NULL, *entryPath = NULL;
	char * escapedPath = NULL;
	int ret = 0;

	HANDLE fh = NULL;
	int size = 0;
	WIN32_FIND_DATAA fdata;
	WIN32_FIND_DATAA tmp;
	enum os_file_flag flags;

	// Check parameters
	if (path == NULL || dirent_cb == NULL) {
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "Error :: NULL parameter in function os_dir_map()");
		return 1;
	}

	// We escape ? and * characters
	// escapedPath = cpp_escape_str(path);

	// Check if it is a directory
	if (!(GetFileAttributesA(path) & FILE_ATTRIBUTE_DIRECTORY)) {
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "Warning :: os_dir_map() :: (%s) is not a directory. ", path);
		return 1;
	}

	size = strlen(path) + 3;
	sPath = (char*)calloc(size + 1, sizeof(char));
	sPath[size] = '\0';
	sprintf_s(sPath, size, "%s\\*", path);

	/*
	FindFirstFile note
	Be aware that some other thread or process could create or delete a file with this name between the time you query for the result and the time you act on the information. If this is a potential concern for your application, one possible solution is to use the CreateFile function with CREATE_NEW (which fails if the file exists) or OPEN_EXISTING (which fails if the file does not exist).
	*/


	fh = FindFirstFile(sPath, &fdata);
	if (fh == INVALID_HANDLE_VALUE) {
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "Warning :: os_dir_map() :: FindFirstFileA() failed ::  (%s) :: [%s]", os_strerror(errno), sPath);
		//return 1;
	}

	while (fh != INVALID_HANDLE_VALUE && FindNextFile(fh, &tmp) != FALSE) {
		// exclude paths "." and ".."
		if (!strcmp(tmp.cFileName, ".") || !strcmp(tmp.cFileName, ".."))
			continue;

		// build the entry complete path.
		size = strlen(path) + strlen(tmp.cFileName) + 2;

		entryPath = (char*)calloc(size + 1, sizeof(char));
		entryPath[size] = '\0';
		sprintf_s(entryPath, size, "%s\\%s", path, tmp.cFileName);		

		// If it is a directory and we do recursive scan
		if ((GetFileAttributesA(entryPath) & FILE_ATTRIBUTE_DIRECTORY) && recurse >= 1) {
			ret = os_dir_map(entryPath, recurse, dirent_cb, data);
			if (ret != 0){
				free(entryPath);
				break;
			}
		}
		else {
			flags = dirent_flags(tmp.dwFileAttributes);
			ret = (*dirent_cb)(entryPath, flags, 0, data);
		}

		free(entryPath);
	}

	if (sPath != NULL) {
		free(sPath);
		sPath = NULL;
	}

	FindClose(fh);

	return 0;
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
