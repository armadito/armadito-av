#include <libjrpc/marshall.h>

#include <jansson.h>

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
	int ret;					\
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
	if ((ret = jrpc_marshall_enum_##ENUM_TYPE(s->NAME, &field))	\
		goto error_end;						\
	json_object_set_new(obj, #NAME, field);

#define JRPC_DEFINE_FIELD_ARRAY(ELEMENT_TYPE, NAME)			\
	if ((ret = jrpc_marshall_array((void **)s->NAME, &field, jrpc_marshall_struct_##ELEMENT_TYPE)) \
		goto error_end;						\
	json_object_set_new(obj, #NAME, field);

#define JRPC_END_STRUCT				\
	*p_obj = obj;				\
	return 0;				\
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

#define JRPC_DEFINE_ENUM_VALUE(NAME) case NAME: *p_obj = json_string(#NAME); return 0;

#define JRPC_END_ENUM				\
	}					\
	*p_obj = NULL;				\
	return 1;				\
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

#define JRPC_DEFINE_ENUM_VALUE(NAME) if (!strcmp(enum_string, #NAME)) { *p_val = NAME; return 0; }

#define JRPC_END_ENUM				\
	return JRPC_ERR_MARSHALL_INVALID_ENUM;	\
}

/*
 * Unmarshalling functions for struct types
 */
#define JRPC_DEFINE_STRUCT(S)				\
int jrpc_unmarshall_struct_##S(json_t *obj, void **pp)	\
{							\
	int ret;					\
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
	return 0;				\
error_end:					\
	free(s);				\
	*pp = NULL;				\
	return ret;				\
}

#else

#error either one of MARSHALL_DECLARATIONS, MARSHALL_FUNCTIONS, UNMARSHALL_DECLARATIONS, UNMARSHALL_FUNCTIONS must be defined

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


