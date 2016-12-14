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

/*
 * JSON-RPC marshalling error codes
 *
 * Uses standard JSON-RPC code defined in http://www.jsonrpc.org/specification#error_object
 */

enum jrpc_marshall_status {
	JRPC_MARSHALL_OK = 0,
	JRPC_MARSHALL_ERR_FIELD_NOT_FOUND = -32099,      /* when unmarshalling a structure, a field was not found in the JSON object */
	JRPC_MARSHALL_ERR_TYPE_MISMATCH = -32098,        /* when unmarshalling a structure, the JSON object property was
							    not of the right type */
	JRPC_MARSHALL_ERR_INVALID_ENUM_STRING = -32097,  /* when unmarshalling an enum, the JSON string was not matching any enum value */
	JRPC_MARSHALL_ERR_INVALID_ENUM_VALUE = -32096,   /* when marshalling an enum, the value was not matching any defined value */
};

#include <jansson.h>

typedef int (*jrpc_marshall_cb_t)(void *p, json_t **p_obj);

int jrpc_marshall_array(void **array, json_t **p_obj, jrpc_marshall_cb_t marshall_elem_cb);

int jrpc_unmarshall_field(json_t *obj, const char *name, json_type expected_json_type, json_t **p_field);

typedef int (*jrpc_unmarshall_cb_t)(json_t *obj, void **pp);

int jrpc_unmarshall_array(json_t *obj, void ***p_array, jrpc_unmarshall_cb_t unmarshall_cb);

#endif

#include <string.h>

/*
 * Code generation macros for marshalling/unmarshalling functions
 *
 * This file is included before including the header containing the structures and enums definitions
 */

#undef JRPC_DEFINE_STRUCT
#undef JRPC_DEFINE_FIELD_INT
#undef JRPC_DEFINE_FIELD_STRING
#undef JRPC_DEFINE_FIELD_ENUM
#undef JRPC_DEFINE_FIELD_ARRAY
#undef JRPC_END_STRUCT
#undef JRPC_DEFINE_ENUM
#undef JRPC_DEFINE_ENUM_VALUE
#undef JRPC_END_ENUM

/*
 * This generates the declarations of marshalling functions
 */
#if defined(MARSHALL_DECLARATIONS)
#undef MARSHALL_DECLARATIONS

#define JRPC_DEFINE_STRUCT(S)  int jrpc_marshall_struct_##S(void *p, json_t **p_obj);

#define JRPC_DEFINE_ENUM(S)  int jrpc_marshall_enum_##S(int value, json_t **p_obj);

/*
 * This generates the definitions of marshalling functions
 */
#elif defined(MARSHALL_FUNCTIONS)
#undef MARSHALL_FUNCTIONS

/*
 * Marshalling functions for struct types
 */
#define JRPC_DEFINE_STRUCT(S)				\
int jrpc_marshall_struct_##S(void *p, json_t **p_obj)	\
{							\
	int ret = JRPC_MARSHALL_OK;			\
	struct S *s = (struct S *)p;			\
	json_t *obj = json_object();			\
	json_t *field;

#define JRPC_DEFINE_FIELD_INT(INT_TYPE, NAME)	\
	field = json_integer(s->NAME);		\
	json_object_set_new(obj, #NAME, field);

#define JRPC_DEFINE_FIELD_STRING(NAME)		\
	field = json_string(s->NAME);		\
	json_object_set_new(obj, #NAME, field);

#define JRPC_DEFINE_FIELD_ENUM(ENUM_TYPE, NAME)				\
	if ((ret = jrpc_marshall_enum_##ENUM_TYPE(s->NAME, &field)))	\
		goto error_end;						\
	json_object_set_new(obj, #NAME, field);

#define JRPC_DEFINE_FIELD_ARRAY(ELEMENT_TYPE, NAME)			\
	if ((ret = jrpc_marshall_array((void **)s->NAME, &field, jrpc_marshall_struct_##ELEMENT_TYPE))) \
		goto error_end;						\
	json_object_set_new(obj, #NAME, field);

#define JRPC_END_STRUCT				\
	*p_obj = obj;				\
	return ret;				\
error_end:					\
	json_decref(obj);			\
	*p_obj = NULL;				\
	return ret;				\
}

/*
 * Marshalling functions for enum types
 */
#define JRPC_DEFINE_ENUM(S)				\
int jrpc_marshall_enum_##S(int value, json_t **p_obj)	\
{							\
	switch(value) {

#define JRPC_DEFINE_ENUM_VALUE(NAME) case NAME: *p_obj = json_string(#NAME); return JRPC_MARSHALL_OK;

#define JRPC_END_ENUM					\
	}						\
	*p_obj = NULL;					\
	return JRPC_MARSHALL_ERR_INVALID_ENUM_VALUE;	\
}

/*
 * This generates the declarations of unmarshalling functions
 */
#elif defined(UNMARSHALL_DECLARATIONS)
#undef UNMARSHALL_DECLARATIONS

#define JRPC_DEFINE_STRUCT(S) int jrpc_unmarshall_struct_##S(json_t *obj, void **pp);

#define JRPC_DEFINE_ENUM(S) int jrpc_unmarshall_enum_##S(json_t *obj, enum S *p_val);

/*
 * This generates the definitions of unmarshalling functions
 */
#elif defined(UNMARSHALL_FUNCTIONS)
#undef UNMARSHALL_FUNCTIONS

/*
 * Unmarshalling functions for enum types
 */
#define JRPC_DEFINE_ENUM(S)					\
int jrpc_unmarshall_enum_##S(json_t *obj, enum S *p_val)	\
{								\
	const char *enum_string = json_string_value(obj);

#define JRPC_DEFINE_ENUM_VALUE(NAME) if (!strcmp(enum_string, #NAME)) { *p_val = NAME; return JRPC_MARSHALL_OK; }

#define JRPC_END_ENUM					\
	return JRPC_MARSHALL_ERR_INVALID_ENUM_STRING;	\
}

/*
 * Unmarshalling functions for struct types
 */
#define JRPC_DEFINE_STRUCT(S)				\
int jrpc_unmarshall_struct_##S(json_t *obj, void **pp)	\
{							\
	int ret = JRPC_MARSHALL_OK;			\
	struct S *s = malloc(sizeof(struct S));		\
	json_t *field;

#define JRPC_DEFINE_FIELD_INT(INT_TYPE, NAME)				\
	if ((ret = jrpc_unmarshall_field(obj, #NAME, JSON_INTEGER, &field))) \
		goto error_end;						\
	s->NAME = (INT_TYPE)json_integer_value(field);

#define JRPC_DEFINE_FIELD_STRING(NAME)					\
	if ((ret = jrpc_unmarshall_field(obj, #NAME, JSON_STRING, &field))) \
		goto error_end;						\
	s->NAME = strdup(json_string_value(field));

#define JRPC_DEFINE_FIELD_ENUM(ENUM_TYPE, NAME)				\
	if ((ret = jrpc_unmarshall_field(obj, #NAME, JSON_STRING, &field))) \
		goto error_end;						\
	if ((ret = jrpc_unmarshall_enum_##ENUM_TYPE(field, &(s->NAME))) != 0) \
		goto error_end;

#define JRPC_DEFINE_FIELD_ARRAY(ELEM_TYPE, NAME)			\
	if ((ret = jrpc_unmarshall_field(obj, #NAME, JSON_ARRAY, &field))) \
		goto error_end;						\
	if ((ret = jrpc_unmarshall_array(field, (void ***)&(s->NAME), jrpc_unmarshall_struct_##ELEM_TYPE)) != 0) \
		goto error_end;						\

#define JRPC_END_STRUCT				\
	*pp = s;				\
	return ret;				\
error_end:					\
	free(s);				\
	*pp = NULL;				\
	return ret;				\
}

/* #else */

/* #error either one of MARSHALL_DECLARATIONS, MARSHALL_FUNCTIONS, UNMARSHALL_DECLARATIONS, UNMARSHALL_FUNCTIONS must be defined */

#endif

#ifndef JRPC_DEFINE_STRUCT
#define JRPC_DEFINE_STRUCT(S)
#endif
#ifndef JRPC_DEFINE_FIELD_INT
#define JRPC_DEFINE_FIELD_INT(INT_TYPE, NAME)
#endif
#ifndef JRPC_DEFINE_FIELD_STRING
#define JRPC_DEFINE_FIELD_STRING(NAME)
#endif
#ifndef JRPC_DEFINE_FIELD_ENUM
#define JRPC_DEFINE_FIELD_ENUM(ENUM_TYPE, NAME)
#endif
#ifndef JRPC_DEFINE_FIELD_ARRAY
#define JRPC_DEFINE_FIELD_ARRAY(ELEMENT_TYPE, NAME)
#endif
#ifndef JRPC_END_STRUCT
#define JRPC_END_STRUCT
#endif
#ifndef JRPC_DEFINE_ENUM
#define JRPC_DEFINE_ENUM(E)
#endif
#ifndef JRPC_DEFINE_ENUM_VALUE
#define JRPC_DEFINE_ENUM_VALUE(NAME)
#endif
#ifndef JRPC_END_ENUM
#define JRPC_END_ENUM
#endif
