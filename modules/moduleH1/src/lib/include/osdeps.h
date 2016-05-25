/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito module H1.

Armadito module H1 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito module H1 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Armadito module H1.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef _OS_DEPS_H_
#define _OS_DEPS_H_

#include <stdio.h>

#ifdef _WIN32
#include <io.h>
FILE * os_fopen(const char * filename, const char * mode);
#define os_strncpy strncpy_s
#define os_strncat strncat_s
#define os_strdup _strdup
#define os_fdopen _fdopen
#define os_read _read
#define MODULEH1_DBDIR "modules/DB/moduleH1"
#else
#define os_fopen fopen
#define os_fdopen fdopen
#define os_strdup strdup
char * os_strncpy(char * dest, size_t sizeDest, char * src, size_t count);
char * os_strncat(char * dest, size_t sizeDest, char * src, size_t count);
#define os_read read
#endif

#endif
