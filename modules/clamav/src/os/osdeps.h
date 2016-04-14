#ifndef _OS_DEPS_H_
#define _OS_DEPS_H_

#include <stdio.h>

#ifdef _WIN32
FILE * os_fopen(const char * filename, const char * mode);
#define os_strncpy strncpy_s
#define os_strncat strncat_s
#define os_strdup _strdup
#define os_sscanf sscanf_s
#define os_sprintf sprintf_s
char * GetDBDirectory( );
#define MODULE_CLAMAV_DBDIR "modules\\DB\\clamav"
#else
#include <clamav.h>
#define os_fopen fopen
#define os_strdup strdup
#define os_sscanf sscanf
char * os_strncpy(char * dest, size_t sizeDest, char * src, size_t count);
char * os_strncat(char * dest, size_t sizeDest, char * src, size_t count);
int os_sprintf(char * dest, size_t len, const char * fmt...); // To implement.
#endif

#endif
