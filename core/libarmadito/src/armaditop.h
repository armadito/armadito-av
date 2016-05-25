/***

Copyright (C) 2015, 2016, 2017 Teclib'

This file is part of Armadito.

Armadito is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef _LIBARMADITO_ARMADITOP_H_
#define _LIBARMADITO_ARMADITOP_H_

#include <libarmadito.h>

struct a6o_module **a6o_get_modules(struct armadito *u);

struct a6o_module *a6o_get_module_by_name(struct armadito *u, const char *name);

#ifdef DEBUG
const char *a6o_debug(struct armadito *u);
#endif

#endif
