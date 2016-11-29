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

#include <jansson.h>
#include <stdint.h>
#include <string.h>

#include "common.h"

/*
 *
 * Marshalling functions for struct fields of integer types
 * - int
 * - int and unsigned int 32 and 64 bits
 * - time_t and size_t
 *
 */
#define JSON_MARSHALL_FIELD_INT(TYPE)					\
static void json_marshall_field_##TYPE(json_t *obj, const char *name, TYPE val) \
{									\
	json_t *field;							\
									\
	field = json_integer(val);					\
	json_object_set(obj, name, field);				\
}

JSON_MARSHALL_FIELD_INT(int)
JSON_MARSHALL_FIELD_INT(uint32_t)
JSON_MARSHALL_FIELD_INT(int32_t)
JSON_MARSHALL_FIELD_INT(time_t)
JSON_MARSHALL_FIELD_INT(size_t)

#ifdef UINT64_MAX
JSON_MARSHALL_FIELD_INT(uint64_t)
#endif

#ifdef INT64_MAX
JSON_MARSHALL_FIELD_INT(int64_t)
#endif

/*
 *
 * Marshalling functions for struct fields of string type
 *
 */
static void json_marshall_field_string(json_t *obj, const char *name, const char *val)
{
	json_t *field;

	field = json_string(val);
	json_object_set(obj, name, field);
}

/*
 *
 * Marshalling functions for fields of enum types
 *
 */
#define IPC_DEFINE_ENUM(S)				\
static void json_marshall_field_enum_##S(json_t *obj, const char *name, int value) \
{							\
	json_t *field;					\
							\
	switch(value) {
#define IPC_DEFINE_ENUM_VALUE(NAME) case NAME: field = json_string(#NAME); break;
#define IPC_END_ENUM				\
	}					\
	json_object_set(obj, name, field);	\
}

#include <libarmadito-rpc/defs.h>

/*
 *
 * Marshalling functions for fields of array types
 *
 */
static void json_marshall_field_array(json_t *obj, const char *name, void **array, int (marshall_struct_cb)(void *p, json_t **))
{
	json_t *field;
	void **p;

	field = json_array();

	for(p = array; *p != NULL; p++) {
		json_t *elem;

		(*marshall_struct_cb)(*p, &elem);
		json_array_append_new(field, elem);
	}

	json_object_set(obj, name, field);
}

/*
 *
 * Marshalling functions for struct types
 *
 */
#define IPC_DEFINE_STRUCT(S)					\
int json_marshall_struct_##S(struct S *p, json_t **p_obj)	\
{								\
	json_t *obj = json_object();				\
	*p_obj = obj;

#define IPC_DEFINE_FIELD_INT(INT_TYPE, NAME) json_marshall_field_##INT_TYPE(obj, #NAME, p->NAME);

#define IPC_DEFINE_FIELD_STRING(NAME) json_marshall_field_string(obj, #NAME, p->NAME);

#define IPC_DEFINE_FIELD_ENUM(ENUM_TYPE, NAME) json_marshall_field_enum_##ENUM_TYPE(obj, #NAME, p->NAME);

#define IPC_DEFINE_FIELD_ARRAY(ELEM_TYPE, NAME) json_marshall_field_array(obj, #NAME, (void **)p->NAME, json_marshall_struct_##ELEM_TYPE);

#define IPC_END_STRUCT				\
	return 0;				\
};

#include <libarmadito-rpc/defs.h>
