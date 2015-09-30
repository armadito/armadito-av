#ifndef _LIBUHURU_OS_STRING_H_
#define _LIBUHURU_OS_STRING_H_

#include "libuhuru-config.h"

#if defined(HAVE_STRDUP)
#define os_strdup strdup
#elif defined(HAVE__STRDUP)
#define os_strdup _strdup
#endif

#define os_strerror strerror

#endif
