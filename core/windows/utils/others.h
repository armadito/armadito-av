#ifndef __UTILS_OTHERS__
#define __UTILS_OTHERS__

#include <Windows.h>

char * GetFileContent(char * filename, int * retsize);
BYTE * GetFileContent_b(char * filename, int * retsize);
char * GetLocationCompletepath(char * specialDir);


#endif