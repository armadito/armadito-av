#include "libuhuru-config.h"

#include "os/mimetype.h"
#include "os/string.h"
#include <glib.h>
#include <Windows.h>
#include <stdio.h>

#define MIME_SIZE 100


const char *os_mime_type_guess(const char *path)
{
	char *mime_type;
	HANDLE fh;
	int high=0, size=0, read = 0, i=0;
	void * buf = NULL;
	LPWSTR * mt = NULL;

	fh = CreateFileA(path, GENERIC_READ, 0, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL );
	if (fh == INVALID_HANDLE_VALUE) {
		g_log(NULL, G_LOG_LEVEL_WARNING, "error :: os_mime_type_guess() :: CreateFileA() failed ::  (%s) ",os_strerror(errno));
		return NULL;
	}

	// get file content size in bytes.
	size = GetFileSize(fh,&high);

	buf = (char*)calloc(size + 1, sizeof(char));
	((char*)buf)[size] = '\0';

	if (ReadFile(fh, buf, size, &read, NULL) == FALSE) {
		g_log(NULL, G_LOG_LEVEL_WARNING, "error :: os_mime_type_guess() :: ReadFile() failed ::  (%s) ",os_strerror(errno));
		free(buf);
		CloseHandle(fh);
		return NULL; 
	}

	
	if (FindMimeFromData(NULL, NULL, buf, size, NULL, FMFD_DEFAULT, &mt, 0) != S_OK) {
		printf("Error :: FindMimeFromData failed :: %d :: %s\n", GetLastError(), os_strerror(GetLastError()) );
		free(buf);
		CloseHandle(fh);
		return(NULL);
	}

	// convert wchar * to char *
	mime_type = (char*)calloc(MIME_SIZE+1,sizeof(char));
	mime_type[MIME_SIZE] = '\0';
	wcstombs_s(&i, mime_type, MIME_SIZE,(wchar_t*)mt, MIME_SIZE);

	
	free(buf);
	CloseHandle(fh);


	return mime_type;
}

const char *os_mime_type_guess_fd(int fd)
{
	return "application/octet-stream";
}

void os_mime_type_init(void) {

	// This function is empty in windows version.
	return;
}