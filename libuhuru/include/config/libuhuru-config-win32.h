#ifndef _LIBUHURU_CONFIG_WIN32_H
#define _LIBUHURU_CONFIG_WIN32_H

/* features */
#undef HAVE_UNIX_SOCKET

/* headers */
#undef HAVE_UNISTD_H

/* windows implemented libmagic*/


/* mapping posix names to win32 names */
#define os_strdup _strdup

//#define os_fopen fopen_s

/* file handling macros */
#define MAXPATHLEN _MAX_PATH

/* misc */
#define LIBUHURU_CONF_DIR "Path/to/conf/dir"
#define LIBUHURU_MODULES_PATH "Path/to/module"

char * os_strerror(int errnum);

#endif
