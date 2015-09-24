#ifndef _LIBUHURU_CONFIG_WIN32_H
#define _LIBUHURU_CONFIG_WIN32_H

/* features */
#undef HAVE_UNIX_SOCKET

/* headers */
#undef HAVE_UNISTD_H 

/* mapping posix names to win32 names */
#define strdup _strdup

/* file handling macros */
#define MAXPATHLEN _MAX_PATH

/* misc */
#define LIBUHURU_CONF_DIR "Path/to/conf/dir"
#define LIBUHURU_MODULES_PATH "Path/to/module"

#endif
