#ifndef _LIBUHURU_CONFIG_H_
#define _LIBUHURU_CONFIG_H_

#ifdef WIN32
#include "libuhuru-config-win32.h"
#else
#include <libuhuru-config-autoconf.h>
#define os_strdup strdup
#endif

#endif
