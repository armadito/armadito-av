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

/*
 * JSON schema generation macros for marshalled structures, enums and unions
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
 * This generates the root of the JSON schema
 */
#if defined(JSON_SCHEMA_ROOT)
#undef JSON_SCHEMA_ROOT

/* note that this will generate JSON schema for all structures, not only */
/* the ones that are used as arguments to JSON-RPC calls */
#define JRPC_STRUCT(S) { "$ref": "#/definitions/" #S },

/*
 * This generates the definitions of the JSON schema
 */
#elif defined(JSON_SCHEMA_DEFINITIONS)
#undef JSON_SCHEMA_DEFINITIONS

#define JRPC_ENUM(E) #E : { \
	"type" : "string", \
		"enum" : [

#define JRPC_ENUM_VALUE(NAME) #NAME,

#define JRPC_ENUM_END \
      ] \
    },

#define JRPC_STRUCT(S) #S : { \
	"type" : "object", \
		"properties" : { \

#define JRPC_STRUCT_FIELD_INT(INT_TYPE, NAME) #NAME : { "type" : "integer" },

#define JRPC_STRUCT_FIELD_STRING(NAME) #NAME : { "type" : "string" },

#define JRPC_STRUCT_FIELD_ENUM(ENUM_TYPE, NAME) #NAME : { "$ref": "#/definitions/" #ENUM_TYPE },

#define JRPC_STRUCT_FIELD_PTR_ARRAY(ELEMENT_TYPE, NAME) #NAME : { "type" : "array", "items" : { "$ref": "#/definitions/" #ELEMENT_TYPE } },

#define JRPC_STRUCT_FIELD_STRUCT(STRUCT_TYPE, NAME)

#define JRPC_STRUCT_FIELD_STRUCT_PTR(STRUCT_TYPE, NAME)

#define JRPC_STRUCT_FIELD_UNION(UNION_TYPE, NAME, TAG) #NAME : { "oneOf" : [ { "$ref": "#/definitions/" #UNION_TYPE } ] },

#define JRPC_STRUCT_END \
		},					\
		"additionalProperties": false, \
},

#define JRPC_UNION(U) #U : { "properties": { \

#define JRPC_UNION_FIELD_INT(INT_TYPE, NAME, TAG_VALUE) #NAME : { "type" : "integer" },

#define JRPC_UNION_FIELD_STRING(NAME, TAG_VALUE) #NAME : { "type" : "string" },

#define JRPC_UNION_FIELD_STRUCT(STRUCT_TYPE, NAME, TAG_VALUE)  #NAME : { "$ref": "#/definitions/" #STRUCT_TYPE },

#define JRPC_UNION_END \
	}, \
    },

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
