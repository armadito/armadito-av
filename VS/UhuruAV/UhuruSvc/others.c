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
