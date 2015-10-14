#ifndef _LIBUHURU_OS_IO_H_
#define _LIBUHURU_OS_IO_H_

#include "libuhuru-config.h"

#ifdef HAVE_IO_H
#include <io.h>
#endif

#if defined(HAVE_OPEN)
#define os_open open
#elif defined(HAVE__OPEN)
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
