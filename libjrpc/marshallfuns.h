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

#ifndef LIBJRPC_MARSHALLFUNS_H
#define LIBJRPC_MARSHALLFUNS_H

typedef int (*rpc_marshall_cb_t)(void *p, json_t **p_obj);

/*
 * Declaration of marshalling functions for struct types
 */
#define JRPC_DEFINE_STRUCT(S) int jrpc_marshall_struct_##S(void *p, json_t **p_obj);

#include <libarmadito-rpc/defs.h>

#endif
