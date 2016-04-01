#include "libuhuru-config.h"
#include "libuhuru\libcore\log.h"
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

// cpp
#include <string>

static enum os_file_flag dirent_flags(DWORD fileAttributes)
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
}

BOOL DirectoryExists(LPCTSTR szPath)
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


void os_dir_map(const char *path, int recurse, dirent_cb_t dirent_cb, void *data ) {

	char * sPath = NULL, *entryPath= NULL;
	char * escapedPath = NULL;

	HANDLE fh = NULL;
	int size = 0;
	WIN32_FIND_DATAA fdata ;
	WIN32_FIND_DATAA tmp ;
	enum os_file_flag flags;

	// Check parameters
	if (path == NULL || dirent_cb == NULL) {
		uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "Error :: NULL parameter in function os_dir_map()");
		return;
	}
	
	// We escape ? and * characters
	// escapedPath = cpp_escape_str(path);

	// Check if it is a directory
	if (!(GetFileAttributesA(path) & FILE_ATTRIBUTE_DIRECTORY)) {
		uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "Warning :: os_dir_map() :: (%s) is not a directory. ", path);
		return;
	}

	size = strlen(path) + 3;
	sPath = (char*)calloc(size + 1, sizeof(char));
	sPath[size] = '\0';
	sprintf_s(sPath, size, "%s\\*", path);

	//g_log(NULL, UHURU_LOG_LEVEL_WARNING, "os_dir_map() :: (%s)", sPath);

	/*
	FindFirstFile note
	Be aware that some other thread or process could create or delete a file with this name between the time you query for the result and the time you act on the information. If this is a potential concern for your application, one possible solution is to use the CreateFile function with CREATE_NEW (which fails if the file exists) or OPEN_EXISTING (which fails if the file does not exist).
	*/


	fh = FindFirstFile(sPath, &fdata);
	if (fh == INVALID_HANDLE_VALUE) {
		uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "Warning :: os_dir_map() :: FindFirstFileA() failed ::  (%s) :: [%s]", os_strerror(errno),sPath);
		return;
	}

	while (FindNextFile(fh, &tmp) != FALSE) {

		//printf("[+] Debug :: os_dir_map :: entry name ===> [%s] <===\n",tmp.cFileName);

		// exclude paths "." and ".."
		if (strncmp(tmp.cFileName,".",strlen(tmp.cFileName)) == 0 || strncmp(tmp.cFileName,"..",strlen(tmp.cFileName)) == 0)
		{
			continue;
		}

		// build the entry complete path.
		size = strlen(path)+strlen(tmp.cFileName)+2;

		entryPath = (char*)calloc(size + 1, sizeof(char));
		entryPath[size] = '\0';
		sprintf_s(entryPath, size,"%s\\%s", path, tmp.cFileName);


		// If it is a directory and we do recursive scan
		if ((GetFileAttributesA(entryPath) & FILE_ATTRIBUTE_DIRECTORY) && recurse >= 1) {
			os_dir_map(entryPath, recurse, dirent_cb, data);
		}
		else {
			
			flags = dirent_flags(tmp.dwFileAttributes);
			(*dirent_cb)(entryPath, flags, 0, data);
		}

		free(entryPath);
	}

	free(sPath);
	FindClose(fh);

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


char * GetBinaryDirectory( ) {

	char * dirpath = NULL;
	char filepath[MAX_PATH];
	char * ptr = NULL;
	int len = 0;

	if (!GetModuleFileNameA(NULL, (LPSTR)&filepath, MAX_PATH)) {		
		uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_ERROR, "[-] Error :: GetBinaryDirectory!GetModuleFileName() failed :: %d\n",GetLastError());
		return NULL;
	}	

	// get the file name from the complete file path
	ptr = strrchr(filepath,'\\');
	if (ptr == NULL) {		
		uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "[-] Error :: GetBinaryDirectory!strrchr() failed :: backslash not found in the path :: %s.\n",filepath);
		return NULL;
	}
	
	// calc the dir buffer length.
	len = (int)(ptr - filepath);
	//printf("[+] Debug :: GetBinaryDirectory :: ptr=%d :: filepath =%d :: len = %d :: strlen = %d\n",ptr,filepath,len,strlen(filepath));

	dirpath = (char*)(calloc(len+1,sizeof(char)));
	dirpath[len] = '\0';

	memcpy_s(dirpath, len, filepath, len);

	//uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_DEBUG, "[+] Debug :: GetBinaryDirectory :: dirpath = %s\n",dirpath);
	

	return dirpath;
}

