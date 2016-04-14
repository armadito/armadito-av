#ifndef _LIBARMADITO_OS_STRING_H_
#define _LIBARMADITO_OS_STRING_H_

#include "config/libarmadito-config.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#ifdef _WIN32
#define os_strdup _strdup
char *os_strerror(int errnum);
#else
#define os_strdup strdup
#define os_strerror strerror
#endif

#if 0
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
#endif


#ifdef __cplusplus
}
#endif

#endif
