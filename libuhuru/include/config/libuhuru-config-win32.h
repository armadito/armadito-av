#ifndef _LIBUHURU_CONFIG_WIN32_H
#define _LIBUHURU_CONFIG_WIN32_H

/* features */
#undef HAVE_UNIX_SOCKET

/* headers */
#undef HAVE_UNISTD_H
#define HAVE_IO_H
#define HAVE_FCNTL_H

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
#undef HAVE_GETPID
#define HAVE__GETPID
/* windows does not have realpath() */
#undef HAVE_REALPATH

/* specific strerror */
#undef HAVE_STRERROR

/* file handling macros */
#define MAXPATHLEN _MAX_PATH

/* misc */
#define LIBUHURU_CONF_DIR "conf"
#define LIBUHURU_MODULES_PATH "modules"

/* modules */
#undef HAVE_ALERT_MODULE
#undef HAVE_QUARANTINE_MODULE
#define HAVE_ON_ACCESS_WINDOWS_MODULE
//#define HAVE_ON_ACCESS_WINDOWS_MODULE
/* glib (current version of glib on windows is 2.28.8, which had g_thread_create) */
#undef HAVE_GTHREAD_NEW
#define HAVE_GTHREAD_CREATE
#define HAVE_GTHREAD_INIT



#endif
