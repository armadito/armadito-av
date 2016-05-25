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

#ifndef _LIBARMADITO_STATUSP_H_
#define _LIBARMADITO_STATUSP_H_

#include <libarmadito.h>

int a6o_file_status_cmp(enum a6o_file_status s1, enum a6o_file_status s2);

const char *a6o_file_status_str(enum a6o_file_status status);

const char *a6o_file_status_pretty_str(enum a6o_file_status status);

#endif
