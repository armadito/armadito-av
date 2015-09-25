#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "libuhuru-config.h"


#ifdef WIN32
// Windows specific strerror
char * os_strerror(int errnum) {

	char * msg = NULL;
	int size = MAXPATHLEN;

	msg = (char*)calloc(size + 1, sizeof(char));
	msg[size] = '\0';

	strerror_s(msg,size,errno);

	return msg;
}


// Dir.c windows specific func

#endif