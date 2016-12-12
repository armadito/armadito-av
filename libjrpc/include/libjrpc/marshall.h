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

#include <jansson.h>
#include <stddef.h>
#include <stdint.h>

void jrpc_marshall_field_int(json_t *obj, const char *field_name, int val);
void jrpc_marshall_field_uint32_t(json_t *obj, const char *field_name, uint32_t val);
void jrpc_marshall_field_int32_t(json_t *obj, const char *field_name, int32_t val);
void jrpc_marshall_field_time_t(json_t *obj, const char *field_name, time_t val);
void jrpc_marshall_field_size_t(json_t *obj, const char *field_name, size_t val);

#ifdef UINT64_MAX
void jrpc_marshall_field_uint64_t(json_t *obj, const char *field_name, uint64_t val);
#endif

#ifdef INT64_MAX
void jrpc_marshall_field_int64_t(json_t *obj, const char *field_name, int64_t val);
#endif

void jrpc_marshall_field_string(json_t *obj, const char *field_name, const char *val);

typedef int (*jrpc_marshall_cb_t)(void *p, json_t **p_obj);

int jrpc_marshall_field_array(json_t *obj, const char *field_name, void **array, jrpc_marshall_cb_t marshall_elem_cb);

int jrpc_unmarshall_field_int(json_t *obj, const char *name, int *p_val);
int jrpc_unmarshall_field_uint32_t(json_t *obj, const char *name, uint32_t *p_val);
int jrpc_unmarshall_field_int32_t(json_t *obj, const char *name, int32_t *p_val);
int jrpc_unmarshall_field_time_t(json_t *obj, const char *name, time_t *p_val);
int jrpc_unmarshall_field_size_t(json_t *obj, const char *name, size_t *p_val);

#ifdef UINT64_MAX
int jrpc_unmarshall_field_uint64_t(json_t *obj, const char *name, uint64_t *p_val);
#endif

#ifdef INT64_MAX
int jrpc_unmarshall_field_int64_t(json_t *obj, const char *name, int64_t *p_val);
#endif

int jrpc_unmarshall_field_string(json_t *obj, const char *name, const char **p_val);

typedef int (*jrpc_unmarshall_cb_t)(json_t *obj, void **pp);

int unmarshall_field_array(json_t *obj, const char *name, void ***p_array, jrpc_unmarshall_cb_t unmarshall_cb);

#endif
