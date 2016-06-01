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

#ifndef _LIBARMADITO_OS_IO_H_
#define _LIBARMADITO_OS_IO_H_

#include "libarmadito-config.h"

#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if defined(HAVE_OPEN)
#define os_open open
#elif defined(HAVE__OPEN)
#include <share.h> // for sharing flags
#include <sys\stat.h> // for mode flags
#define os_open _open
#endif

#if defined(HAVE_READ)
#define os_read read
#elif defined(HAVE__READ)
#define os_read _read
#endif

#if defined(HAVE_WRITE)
#define os_write write
#elif defined(HAVE__WRITE)
#define os_write _write
#endif

#if defined(HAVE_CLOSE)
#define os_close close
#elif defined(HAVE__CLOSE)
#define os_close _close
#endif

#if defined(HAVE_LSEEK)
#define os_lseek lseek
#elif defined(HAVE__LSEEK)
#define os_lseek _lseek
#endif

#endif
