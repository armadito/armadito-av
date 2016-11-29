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
#include <stdlib.h>
#include <string.h>

#include "common.h"

/*
 *
 * Unmarshalling functions for struct fields of integer types
 * - int
 * - int and unsigned int 32 and 64 bits
 * - time_t and size_t
 *
 */
#define JSON_UNMARSHALL_FIELD_INT(TYPE)						\
static int json_unmarshall_field_##TYPE(json_t *obj, const char *name, TYPE *p_val)	\
{									\
	json_t *field = json_object_get(obj, name);			\
									\
	if (field == NULL)						\
		return ERR_NO_SUCH_FIELD;				\
									\
	if (!json_is_number(field))					\
		return ERR_TYPE_MISMATCH;				\
									\
	*p_val = (TYPE)json_integer_value(field);			\
									\
	return 0;							\
}

JSON_UNMARSHALL_FIELD_INT(int)
JSON_UNMARSHALL_FIELD_INT(uint32_t)
JSON_UNMARSHALL_FIELD_INT(int32_t)
JSON_UNMARSHALL_FIELD_INT(time_t)
JSON_UNMARSHALL_FIELD_INT(size_t)

#ifdef UINT64_MAX
JSON_UNMARSHALL_FIELD_INT(uint64_t)
#endif

#ifdef INT64_MAX
JSON_UNMARSHALL_FIELD_INT(int64_t)
#endif

/*
 *
 * Unmarshalling functions for struct fields of string type
 *
 */
static int json_unmarshall_field_string(json_t *obj, const char *name, const char **p_val)
{
	json_t *field = json_object_get(obj, name);

	if (field == NULL)
		return ERR_NO_SUCH_FIELD;

	if (!json_is_string(field))
		return ERR_TYPE_MISMATCH;

	*p_val = strdup(json_string_value(field));

	return 0;
}

/*
 *
 * Unmarshalling functions for fields of enum types
 *
 */
#define IPC_DEFINE_ENUM(S)						\
static int json_unmarshall_field_enum_##S(json_t *obj, const char *name, enum S *p_val) \
{									\
	json_t *field = json_object_get(obj, name);			\
	const char *enum_string;					\
									\
	if (field == NULL)						\
		return ERR_NO_SUCH_FIELD;				\
									\
	if (!json_is_string(field))					\
		return ERR_TYPE_MISMATCH;				\
									\
	enum_string = json_string_value(field);
#define IPC_DEFINE_ENUM_VALUE(NAME) if (!strcmp(enum_string, #NAME)) { *p_val = NAME; return 0; }
#define IPC_END_ENUM				\
	return 1;				\
}

#include <libarmadito-rpc/defs.h>

/*
 *
 * Unmarshalling functions for fields of array types
 *
 */
static int json_unmarshall_field_array(json_t *obj, const char *name, void ***p_array, int (*json_unmarshall_cb)(json_t *obj, void **pp))
{
	json_t *field = json_object_get(obj, name);
	size_t index, size;
	json_t *elem;
	void **array, **p;

	if (field == NULL)
		return ERR_NO_SUCH_FIELD;

	if (!json_is_array(field))
		return ERR_TYPE_MISMATCH;

	size = json_array_size(field);
	if (!size)
		return ERR_TYPE_MISMATCH;

	array = calloc(size + 1, sizeof(void *));

	p = array;
	json_array_foreach(field, index, elem) {
		(*json_unmarshall_cb)(elem, p);
		p++;
	}

	*p_array = array;

	return 0;
}

/*
 *
 * Unmarshalling functions for struct types
 *
 */
#define IPC_DEFINE_STRUCT(S)			\
int json_unmarshall_struct_##S(json_t *obj, void **pp)	\
{						\
	int ret;				\
	struct S *s = malloc(sizeof(struct S));	\
	*pp = s;

#define IPC_DEFINE_FIELD_INT(INT_TYPE, NAME) if ((ret = json_unmarshall_field_##INT_TYPE(obj, #NAME, &(s->NAME))) != 0) return ret;

#define IPC_DEFINE_FIELD_STRING(NAME) if ((ret = json_unmarshall_field_string(obj, #NAME, &(s->NAME))) != 0) return ret;

#define IPC_DEFINE_FIELD_ENUM(ENUM_TYPE, NAME) if ((ret = json_unmarshall_field_enum_##ENUM_TYPE(obj, #NAME, &(s->NAME))) != 0) return ret;

#define IPC_DEFINE_FIELD_ARRAY(ELEM_TYPE, NAME) if ((ret = json_unmarshall_field_array(obj, #NAME, (void ***)&(s->NAME), json_unmarshall_struct_##ELEM_TYPE)) != 0) return ret;

#define IPC_END_STRUCT \
	return 0;      \
}

#include <libarmadito-rpc/defs.h>


