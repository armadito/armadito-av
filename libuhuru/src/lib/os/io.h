#ifndef _LIBUHURU_OS_IO_H_
#define _LIBUHURU_OS_IO_H_

#include "libuhuru-config.h"

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

#endif
