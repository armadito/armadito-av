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

#ifndef LIBJRPC_ERROR_H
#define LIBJRPC_ERROR_H

/*
 * JSON-RPC error codes
 *
 * Standard JSON-RPC code defined in http://www.jsonrpc.org/specification#error_object
 */

enum jrpc_status {
	JRPC_OK = 0,
	JRPC_EOF = 1,

	JRPC_ERR_PARSE_ERROR = -32700,               /* Parse error Invalid JSON was received by the server. An error
							occurred on the server while parsing the JSON text. */
	JRPC_ERR_INVALID_REQUEST = -32600,           /* The JSON sent is not a valid Request object. */
	JRPC_ERR_METHOD_NOT_FOUND = -32601,          /* The method does not exist / is not available. */
	JRPC_ERR_INVALID_PARAMS = -32602, 	     /* Invalid params, Invalid method parameter(s). */
	JRPC_ERR_INTERNAL_ERROR = -32603,            /* Internal error Internal JSON-RPC error. */

	/* -32000 to -32099 	Server error 	Reserved for implementation-defined server-errors. */
	/* Error codes breakdown:  */
	/* -32099 to -32090  reserved for marshalling errors */
	JRPC_ERR_MARSHALL_FIELD_NOT_FOUND = -32099,      /* when unmarshalling a structure, a field was not found in the JSON object */
	JRPC_ERR_MARSHALL_TYPE_MISMATCH = -32098,        /* when unmarshalling a structure, the JSON object property was
							    not of the right type */
	JRPC_ERR_MARSHALL_INVALID_ENUM_STRING = -32097,  /* when unmarshalling an enum, the JSON string was not matching any enum value */
	JRPC_ERR_MARSHALL_INVALID_ENUM_VALUE = -32096,   /* when marshalling an enum, the value was not matching any defined value */

	/* -32089 to -32080  reserved for runtime errors */
	JRPC_ERR_INVALID_RESPONSE_ID = -32089,       /* Response id is not associated with a callback */

	/* -30000 to (-30000 + 256) reserved for called method errors */
	JRPC_ERR_METHOD_ERROR = -30000,              /* Base value for method specific errors */
};

#define JRPC_ERR_IS_METHOD_ERROR(C) (JRPC_ERR_METHOD_ERROR < (C) && (C) < JRPC_ERR_METHOD_ERROR + 256)

#define JRPC_ERR_METHOD_TO_CODE(E) (JRPC_ERR_METHOD_ERROR + ((E) & 0xff))
#define JRPC_ERR_CODE_TO_METHOD(C) (((C) - JRPC_ERR_METHOD_ERROR) & 0xff)

#endif
