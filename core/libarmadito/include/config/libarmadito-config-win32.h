/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef _LIBARMADITO_CONFIG_WIN32_H
#define _LIBARMADITO_CONFIG_WIN32_H

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

/* modules */
#define HAVE_ONDEMAND_MODULE
#undef HAVE_ALERT_MODULE
#define HAVE_QUARANTINE_MODULE
#define HAVE_ON_ACCESS_WINDOWS_MODULE
/* glib (current version of glib on windows is 2.28.8, which had g_thread_create) */
#define HAVE_GTHREAD_NEW
#undef HAVE_GTHREAD_CREATE
#undef HAVE_GTHREAD_INIT



#endif
