#include "others.h"
#include <Windows.h>
#include <stdio.h>

char * GetFileContent(char * filename, int * retsize) {

	char * content = NULL;
	HANDLE hFile = NULL;
	LARGE_INTEGER fileSize = {0};
	int size = 0, read = 0;
	int ret = 0;

	if (filename == NULL || retsize == NULL) {
		printf("[-] Error :: GetFileContent :: Invalid parameter\n");
		return NULL;
	}

	__try {

		hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			printf("[-] Error :: GetFileContent :: Opening the file [%s] failed! :: error = %d\n",filename,GetLastError());
			ret = -1;
			__leave;
		}

		if (GetFileSizeEx(hFile, &fileSize) == FALSE) {
			printf("[-] Error :: GetFileContent :: Get file size failed! :: error = %d\n",GetLastError());
			ret = -2;
			__leave;
		}

		size = fileSize.QuadPart;
		*retsize = size;

		//printf("[+] Debug :: GetFileContent :: file size = %d\n",size);

		content = (char*)calloc(size+1,sizeof(char));
		content[size]='\0';

		if (ReadFile(hFile, content, size, &read, NULL) == FALSE) {
			printf("[-] Error :: GetFileContent :: Read file content failed! :: error = %d\n",GetLastError());
			ret = -4;
			__leave;
		}

	}
	__finally {

		
		if (hFile != INVALID_HANDLE_VALUE && hFile != NULL) {
			CloseHandle(hFile);
			hFile = NULL;
		}

		if (content != NULL && ret < 0) {
			free(content);
			content = NULL;
		}

	}

	return content;
}

/*
	This function returns the complete path of a location given in parameter
	according to the installation path.
*/
char * GetLocationCompletepath(char * specialDir) {

	char * dirpath = NULL;
	char * completePath = NULL;
	char filepath[MAX_PATH];	
	char * ptr = NULL;
	int dir_len = 0, len= 0;
	int ret = 0;
	
	__try {

		if (!GetModuleFileNameA(NULL, (LPSTR)&filepath, MAX_PATH)) {	
			printf("[-] Error :: GetLocationCompletepath :: GetModuleFilename failed :: GLE = %d\n",GetLastError());
			return NULL;
		}

		// Remove the module filename from the complete file path
		ptr = strrchr(filepath,'\\');
		if (ptr == NULL) {
			printf("[-] Error :: GetLocationCompletepath :: No backslash found in the path\n");
			return NULL;
		}

		// calc the dir buffer length.
		dir_len = (int)(ptr - filepath);
		dirpath = (char*)(calloc(dir_len+1,sizeof(char)));
		dirpath[dir_len] = '\0';

		memcpy_s(dirpath, dir_len, filepath, dir_len);
		//printf("[+] Debug :: GetLocationCompletepath :: dirpath = %s\n",dirpath);

		len = dir_len + strnlen(specialDir, MAX_PATH) + 2;
		
		completePath = (char*)calloc(len+1,sizeof(char));
		completePath[len] = '\0';

		strncat_s(completePath, len, dirpath, dir_len);
		strncat_s(completePath, len, "\\", 1);
		strncat_s(completePath, len, specialDir, strnlen(specialDir, MAX_PATH));		

		//printf("[+] Debug :: GetLocationCompletepath :: completePath = %s\n",completePath);

		
	}
	__finally {

		if (dirpath != NULL) {
			free(dirpath);
			dirpath = NULL;
		}

	}	

	return completePath;
}
