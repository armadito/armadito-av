#ifndef _LIBUHURU_OS_STRING_H_
#define _LIBUHURU_OS_STRING_H_

#include "libuhuru-config.h"

#if defined(HAVE_STRDUP)
#define os_strdup strdup
#define os_strerror strerror
#elif defined(HAVE__STRDUP)
#define os_strdup _strdup
char * os_strerror(int errnum);
#endif



#endif
