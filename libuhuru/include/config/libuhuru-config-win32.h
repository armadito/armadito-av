#ifndef _LIBUHURU_CONFIG_WIN32_H
#define _LIBUHURU_CONFIG_WIN32_H

/* features */
#undef HAVE_UNIX_SOCKET

/* headers */
#undef HAVE_UNISTD_H
#define HAVE_IO_H

/* mapping posix names to win32 names */
#undef HAVE_STRDUP
#define HAVE__STRDUP
#undef HAVE_OPEN
#define HAVE__OPEN
#undef HAVE_READ
#define HAVE__READ
#undef HAVE_WRITE
#define HAVE__WRITE
#undef HAVE_CLOSE
#define HAVE__CLOSE
#undef HAVE_LSEEK
#define HAVE__LSEEK
/* windows does not have realpath() */
#undef HAVE_REALPATH

/* specific strerror */
#undef HAVE_STRERROR

/* file handling macros */
#define MAXPATHLEN _MAX_PATH

/* misc */
#define LIBUHURU_CONF_DIR "Path/to/conf/dir"
#define LIBUHURU_MODULES_PATH "Path/to/module"

/* modules */
#undef HAVE_ALERT_MODULE
#undef HAVE_QUARANTINE_MODULE

#endif
