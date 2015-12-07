#include "osdeps.h"
#include <stdio.h>
#include <string.h>

char * os_strncpy(char * dest, size_t sizeDest, char * src, size_t count) {

	char * ret = NULL;

	ret = strncpy( dest, src, count );
	
	return ret;
}

char * os_strncat(char * dest, size_t sizeDest, char * src, size_t count) {

	char * ret = NULL;

	ret = strncat( dest, src, count );
	
	return ret;
}
