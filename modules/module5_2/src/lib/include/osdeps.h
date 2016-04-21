#ifndef _OS_DEPS_H_
#define _OS_DEPS_H_

#include <stdio.h>

#ifdef _WIN32
#include <io.h>
FILE * os_fopen(const char * filename, const char * mode);
#define os_strncpy strncpy_s
#define os_strncat strncat_s
#define os_strdup _strdup
#define os_fdopen _fdopen
#define os_read _read
#define MODULE5_2_DBDIR "modules/DB/module5_2"
#else
#define os_fopen fopen
#define os_fdopen fdopen
#define os_strdup strdup
char * os_strncpy(char * dest, size_t sizeDest, char * src, size_t count);
char * os_strncat(char * dest, size_t sizeDest, char * src, size_t count);
#define os_read read
#endif

#endif
