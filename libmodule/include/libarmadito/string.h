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

/**
 * \file string.h
 *
 * \brief definition of usefull string functions, to replace unsafe strcpy and sprintf
 *
 */

#ifndef __LIBARMADITO_STRING_H_
#define __LIBARMADITO_STRING_H_

#include <stdlib.h>
#include <string.h>

char *a6o_vstrcat(const char *src1, ...);

/* #define a6o_strdup(S) a6o_vstrcat((S), NULL) */
#define a6o_strdup(S) strdup(S)

#endif
