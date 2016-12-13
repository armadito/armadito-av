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

#ifndef LIBJRPC_MARSHALL_H
#define LIBJRPC_MARSHALL_H

#include <libjrpc/jrpc.h>

#include <jansson.h>

typedef int (*jrpc_marshall_cb_t)(void *p, json_t **p_obj);

int jrpc_marshall_array(void **array, json_t **p_obj, jrpc_marshall_cb_t marshall_elem_cb);

int jrpc_unmarshall_field(json_t *obj, const char *name, json_type expected_json_type, json_t **p_field);

typedef int (*jrpc_unmarshall_cb_t)(json_t *obj, void **pp);

int unmarshall_field_array(json_t *obj, const char *name, void ***p_array, jrpc_unmarshall_cb_t unmarshall_cb);

#endif
