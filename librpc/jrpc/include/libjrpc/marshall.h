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

#include <libjrpc/error.h>

#include <jansson.h>

/*
 * Helper functions used by the generated code
 */
typedef int (*jrpc_marshall_cb_t)(void *p, json_t **p_obj);

int jrpc_marshall_array(void **array, json_t **p_obj, jrpc_marshall_cb_t marshall_elem_cb);

int jrpc_unmarshall_field(json_t *obj, const char *name, json_type expected_json_type, int allow_null, json_t **p_field);

typedef int (*jrpc_unmarshall_cb_t)(json_t *obj, void *p);

int jrpc_unmarshall_array(json_t *obj, void ***p_array, jrpc_unmarshall_cb_t unmarshall_cb, size_t elem_size);

int jrpc_unmarshall_struct_ptr(json_t *obj, void **pp, jrpc_unmarshall_cb_t unmarshall_cb, size_t struct_size);

/*
 * Marshalling helper macro
 */
#define JRPC_STRUCT2JSON(S, P, O) jrpc_marshall_struct_##S((void *)P, O)

/*
 * Umarshalling helper macro
 */
#define JRPC_JSON2STRUCT(S, O, P) jrpc_unmarshall_struct_ptr(O, (void **)P, jrpc_unmarshall_struct_##S, sizeof(struct S))

#endif

#include <string.h>

/*
 * Code generation macros for marshalling/unmarshalling functions
 *
 * This file is included before including the header containing the structures and enums definitions
 */

/* declaration of a structure */
#undef JRPC_STRUCT
/* a struct field of integer type, giving the int type */
#undef JRPC_STRUCT_FIELD_INT
/* a struct field of type const char * */
#undef JRPC_STRUCT_FIELD_STRING
/* a struct field of type enum, giving the enum type */
#undef JRPC_STRUCT_FIELD_ENUM
/* a struct field of type null-terminated array of pointers to structures, giving the pointed structure type */
#undef JRPC_STRUCT_FIELD_PTR_ARRAY
/* a struct field of type struct, giving the structure type */
#undef JRPC_STRUCT_FIELD_STRUCT
/* a struct field of type struct pointer, giving the structure type */
#undef JRPC_STRUCT_FIELD_STRUCT_PTR
/* a struct field of type union, giving the union type and the tag field in the structure */
#undef JRPC_STRUCT_FIELD_UNION
/* end of the structure declaration */
#undef JRPC_STRUCT_END

/* declaration of an enum */
#undef JRPC_ENUM
/* declaration of one enum value */
#undef JRPC_ENUM_VALUE
/* end of the enum declaration */
#undef JRPC_ENUM_END

/* declaration of an union */
#undef JRPC_UNION
/* a union field of integer type, giving the int type */
#undef JRPC_UNION_FIELD_INT
/* a union field of type const char * */
#undef JRPC_UNION_FIELD_STRING
/* a union field of type struct, giving the structure type */
#undef JRPC_UNION_FIELD_STRUCT
/* a union field of type null-terminated array of pointers to structures, giving the pointed structure type */
#undef JRPC_STRUCT_FIELD_PTR_ARRAY
/* end of the union declaration */
#undef JRPC_UNION_END

/*
 * This generates the declarations of marshalling functions
 */
#if defined(MARSHALL_DECLARATIONS)
#undef MARSHALL_DECLARATIONS

#define JRPC_ENUM(E) int jrpc_marshall_enum_##E(int value, json_t **p_obj);

#define JRPC_STRUCT(S) int jrpc_marshall_struct_##S(void *p, json_t **p_obj);

#define JRPC_UNION(U) int jrpc_marshall_union_##U(void *p, json_t **p_obj, int tag);

/*
 * This generates the definitions of marshalling functions
 */
#elif defined(MARSHALL_FUNCTIONS)
#undef MARSHALL_FUNCTIONS

/*
 * Marshalling functions for enum types
 */
#define JRPC_ENUM(E)					\
int jrpc_marshall_enum_##E(int value, json_t **p_obj)	\
{							\
	switch(value) {

#define JRPC_ENUM_VALUE(NAME) case NAME: *p_obj = json_string(#NAME); return JRPC_OK;

#define JRPC_ENUM_END					\
	}						\
	*p_obj = NULL;					\
	return JRPC_ERR_MARSHALL_INVALID_ENUM_VALUE;	\
}

/*
 * Marshalling functions for struct types
 */
#define JRPC_STRUCT(S)					\
int jrpc_marshall_struct_##S(void *p, json_t **p_obj)	\
{							\
	int ret = JRPC_OK;				\
	struct S *s = (struct S *)p;			\
	json_t *obj, *field;				\
							\
	if (s == NULL) {				\
		*p_obj = json_null();			\
		return JRPC_OK;				\
	}						\
	obj = json_object();

#define JRPC_STRUCT_FIELD_INT(INT_TYPE, NAME)	\
	field = json_integer(s->NAME);		\
	json_object_set_new(obj, #NAME, field);

#define JRPC_STRUCT_FIELD_STRING(NAME)		\
	field = json_string(s->NAME);		\
	json_object_set_new(obj, #NAME, field);

#define JRPC_STRUCT_FIELD_ENUM(ENUM_TYPE, NAME)				\
	if ((ret = jrpc_marshall_enum_##ENUM_TYPE(s->NAME, &field)))	\
		goto error_end;						\
	json_object_set_new(obj, #NAME, field);

#define JRPC_STRUCT_FIELD_PTR_ARRAY(ELEMENT_TYPE, NAME)				\
	if ((ret = jrpc_marshall_array((void **)s->NAME, &field, jrpc_marshall_struct_##ELEMENT_TYPE))) \
		goto error_end;						\
	json_object_set_new(obj, #NAME, field);

#define JRPC_STRUCT_FIELD_STRUCT(STRUCT_TYPE, NAME)			\
	if ((ret = jrpc_marshall_struct_##STRUCT_TYPE((void *)&s->NAME, &field))) \
		goto error_end;						\
	json_object_set_new(obj, #NAME, field);

#define JRPC_STRUCT_FIELD_STRUCT_PTR(STRUCT_TYPE, NAME)			\
	if ((ret = jrpc_marshall_struct_##STRUCT_TYPE((void *)s->NAME, &field))) \
		goto error_end;						\
	json_object_set_new(obj, #NAME, field);

#define JRPC_STRUCT_FIELD_UNION(UNION_TYPE, NAME, TAG)			\
	if ((ret = jrpc_marshall_union_##UNION_TYPE((void *)&s->NAME, &field, s->TAG))) \
		goto error_end;						\
	json_object_set_new(obj, #NAME, field);

#define JRPC_STRUCT_END				\
	*p_obj = obj;				\
	return ret;				\
error_end:					\
	json_decref(obj);			\
	*p_obj = NULL;				\
	return ret;				\
}

/*
 * Marshalling functions for union types
 */
#define JRPC_UNION(U)					\
int jrpc_marshall_union_##U(void *p, json_t **p_obj, int tag)	\
{								\
	int ret = JRPC_OK;					\
	union U *u = (union U *)p;				\
	json_t *obj, *field;					\
								\
	if (u == NULL) {					\
		*p_obj = json_null();				\
		return JRPC_OK;					\
	}							\
	obj = json_object();					\
	switch(tag) {

#define JRPC_UNION_FIELD_INT(INT_TYPE, NAME, TAG_VALUE) \
	case TAG_VALUE:					\
		field = json_integer(u->NAME);		\
		json_object_set_new(obj, #NAME, field);	\
		break;

#define JRPC_UNION_FIELD_STRING(NAME, TAG_VALUE)	\
	case TAG_VALUE:					\
		field = json_string(u->NAME);		\
		json_object_set_new(obj, #NAME, field);	\
		break;

#define JRPC_UNION_FIELD_STRUCT(STRUCT_TYPE, NAME, TAG_VALUE)		\
	case TAG_VALUE:							\
		if ((ret = jrpc_marshall_struct_##STRUCT_TYPE((void *)&u->NAME, &field))) \
			goto error_end;					\
		json_object_set_new(obj, #NAME, field);			\
		break;

#define JRPC_UNION_END				\
	} /* end switch */			\
	*p_obj = obj;				\
	return ret;				\
error_end:					\
	json_decref(obj);			\
	*p_obj = NULL;				\
	return ret;				\
}

/*
 * This generates the declarations of unmarshalling functions
 */
#elif defined(UNMARSHALL_DECLARATIONS)
#undef UNMARSHALL_DECLARATIONS

#define JRPC_ENUM(E) int jrpc_unmarshall_enum_##E(json_t *obj, enum E *p_val);

#define JRPC_STRUCT(S) int jrpc_unmarshall_struct_##S(json_t *obj, void *p);

#define JRPC_UNION(U) int jrpc_unmarshall_union_##U(json_t *obj, void *p, int tag);

/*
 * This generates the definitions of unmarshalling functions
 */
#elif defined(UNMARSHALL_FUNCTIONS)
#undef UNMARSHALL_FUNCTIONS

/*
 * Unmarshalling functions for enum types
 */
#define JRPC_ENUM(E)						\
int jrpc_unmarshall_enum_##E(json_t *obj, enum E *p_val)	\
{								\
	const char *enum_string = json_string_value(obj);

#define JRPC_ENUM_VALUE(NAME) if (!strcmp(enum_string, #NAME)) { *p_val = NAME; return JRPC_OK; }

#define JRPC_ENUM_END					\
	return JRPC_ERR_MARSHALL_INVALID_ENUM_STRING;	\
}

/*
 * Unmarshalling functions for struct types
 */
#define JRPC_STRUCT(S)					\
int jrpc_unmarshall_struct_##S(json_t *obj, void *p)	\
{							\
	int ret = JRPC_OK;				\
	struct S *s = (struct S *)p;			\
	json_t *field;

#define JRPC_STRUCT_FIELD_INT(INT_TYPE, NAME)				\
	if ((ret = jrpc_unmarshall_field(obj, #NAME, JSON_INTEGER, 0, &field))) \
		goto error_end;						\
	s->NAME = (INT_TYPE)json_integer_value(field);

#define JRPC_STRUCT_FIELD_STRING(NAME)					\
	if ((ret = jrpc_unmarshall_field(obj, #NAME, JSON_STRING, 0, &field))) \
		goto error_end;						\
	s->NAME = strdup(json_string_value(field));

#define JRPC_STRUCT_FIELD_ENUM(ENUM_TYPE, NAME)				\
	if ((ret = jrpc_unmarshall_field(obj, #NAME, JSON_STRING, 0, &field))) \
		goto error_end;						\
	if ((ret = jrpc_unmarshall_enum_##ENUM_TYPE(field, &(s->NAME))) != 0) \
		goto error_end;

#define JRPC_STRUCT_FIELD_PTR_ARRAY(ELEM_TYPE, NAME)			\
	if ((ret = jrpc_unmarshall_field(obj, #NAME, JSON_ARRAY, 1, &field))) \
		goto error_end;						\
	if ((ret = jrpc_unmarshall_array(field, (void ***)&(s->NAME), jrpc_unmarshall_struct_##ELEM_TYPE, sizeof(struct ELEM_TYPE)))) \
		goto error_end;

#define JRPC_STRUCT_FIELD_STRUCT(STRUCT_TYPE, NAME)			\
	if ((ret = jrpc_unmarshall_field(obj, #NAME, JSON_OBJECT, 1, &field))) \
		goto error_end;						\
	if ((ret = jrpc_unmarshall_struct_##STRUCT_TYPE(field, (void *)&(s->NAME)))) \
		goto error_end;

#define JRPC_STRUCT_FIELD_STRUCT_PTR(STRUCT_TYPE, NAME)			\
	if ((ret = jrpc_unmarshall_field(obj, #NAME, JSON_OBJECT, 1, &field))) \
		goto error_end;						\
	if ((ret = jrpc_unmarshall_struct_ptr(field, (void **)&(s->NAME), jrpc_unmarshall_struct_##S, sizeof(struct S))))	\
		goto error_end;

#define JRPC_STRUCT_FIELD_UNION(UNION_TYPE, NAME, TAG)			\
	if ((ret = jrpc_unmarshall_field(obj, #NAME, JSON_OBJECT, 0, &field))) \
		goto error_end;						\
	if ((ret = jrpc_unmarshall_union_##UNION_TYPE(field, (void *)&(s->NAME), s->TAG))) \
		goto error_end;

#define JRPC_STRUCT_END				\
error_end:					\
	return ret;				\
}

/*
 * Unmarshalling functions for union types
 */
#define JRPC_UNION(U)						\
int jrpc_unmarshall_union_##U(json_t *obj, void *p, int tag)	\
{								\
	int ret = JRPC_OK;					\
	union U *u = (union U *)p;				\
	json_t *field;						\
								\
	switch(tag) {

#define JRPC_UNION_FIELD_INT(INT_TYPE, NAME, TAG_VALUE)			\
	case TAG_VALUE:							\
		if ((ret = jrpc_unmarshall_field(obj, #NAME, JSON_INTEGER, 0, &field))) \
			goto error_end;					\
		u->NAME = (INT_TYPE)json_integer_value(field);		\
		break;

#define JRPC_UNION_FIELD_STRING(NAME, TAG_VALUE)			\
	case TAG_VALUE:							\
		if ((ret = jrpc_unmarshall_field(obj, #NAME, JSON_INTEGER, 0, &field))) \
			goto error_end;					\
		u->NAME = strdup(json_string_value(field));		\
		break;

#define JRPC_UNION_FIELD_STRUCT(STRUCT_TYPE, NAME, TAG_VALUE)		\
	case TAG_VALUE:							\
		if ((ret = jrpc_unmarshall_field(obj, #NAME, JSON_OBJECT, 0, &field))) \
			goto error_end;					\
		if ((ret = jrpc_unmarshall_struct_##STRUCT_TYPE(field, (void *)&(u->NAME)))) \
			goto error_end;					\
		break;

#define JRPC_UNION_END				\
	} /* end switch */			\
error_end:					\
	return ret;				\
}

#endif

#ifndef JRPC_STRUCT
#define JRPC_STRUCT(S)
#endif
#ifndef JRPC_STRUCT_FIELD_INT
#define JRPC_STRUCT_FIELD_INT(INT_TYPE, NAME)
#endif
#ifndef JRPC_STRUCT_FIELD_STRING
#define JRPC_STRUCT_FIELD_STRING(NAME)
#endif
#ifndef JRPC_STRUCT_FIELD_ENUM
#define JRPC_STRUCT_FIELD_ENUM(ENUM_TYPE, NAME)
#endif
#ifndef JRPC_STRUCT_FIELD_PTR_ARRAY
#define JRPC_STRUCT_FIELD_PTR_ARRAY(ELEMENT_TYPE, NAME)
#endif
#ifndef JRPC_STRUCT_FIELD_STRUCT
#define JRPC_STRUCT_FIELD_STRUCT(STRUCT_TYPE, NAME)
#endif
#ifndef JRPC_STRUCT_FIELD_STRUCT_PTR
#define JRPC_STRUCT_FIELD_STRUCT_PTR(STRUCT_TYPE, NAME)
#endif
#ifndef JRPC_STRUCT_FIELD_UNION
#define JRPC_STRUCT_FIELD_UNION(UNION_TYPE, NAME, TAG)
#endif
#ifndef JRPC_STRUCT_END
#define JRPC_STRUCT_END
#endif

#ifndef JRPC_ENUM
#define JRPC_ENUM(E)
#endif
#ifndef JRPC_ENUM_VALUE
#define JRPC_ENUM_VALUE(NAME)
#endif
#ifndef JRPC_ENUM_END
#define JRPC_ENUM_END
#endif

#ifndef JRPC_UNION
#define JRPC_UNION(U)
#endif
#ifndef JRPC_UNION_FIELD_INT
#define JRPC_UNION_FIELD_INT(INT_TYPE, NAME, TAG_VALUE)
#endif
#ifndef JRPC_UNION_FIELD_STRING
#define JRPC_UNION_FIELD_STRING(NAME, TAG_VALUE)
#endif
#ifndef JRPC_UNION_FIELD_STRUCT
#define JRPC_UNION_FIELD_STRUCT(STRUCT_TYPE, NAME, TAG_VALUE)
#endif
#ifndef JRPC_UNION_FIELD_PTR_ARRAY
#define JRPC_UNION_FIELD_PTR_ARRAY(ELEMENT_TYPE, NAME, TAG_VALUE)
#endif
#ifndef JRPC_UNION_END
#define JRPC_UNION_END
#endif
