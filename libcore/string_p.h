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

#ifndef LIBCORE_STRING_P_H
#define LIBCORE_STRING_P_H

#include "armadito-config.h"

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
