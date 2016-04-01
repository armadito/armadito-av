#ifndef _LIBARMADITO_OS_STRING_H_
#define _LIBARMADITO_OS_STRING_H_

#include "libarmadito-config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(HAVE_STRDUP)
#include <string.h>
#define os_strdup strdup
#elif defined(HAVE__STRDUP)
#include <string.h>
#define os_strdup _strdup
#endif

#ifdef HAVE_STRERROR
#include <string.h>
#define os_strerror strerror
#else
char *os_strerror(int errnum);
#endif

#ifdef __cplusplus
}
#endif

#endif
