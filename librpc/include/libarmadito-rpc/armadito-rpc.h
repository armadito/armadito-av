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

#ifndef LIBARMADITO_RPC_ARMADITO_RPC_H
#define LIBARMADITO_RPC_ARMADITO_RPC_H

#include <stddef.h>
#include <jansson.h>

/*
 * Declaration of marshalling functions for struct types
 */
#define A6O_RPC_DEFINE_STRUCT(S) int a6o_rpc_marshall_struct_##S(void *p, json_t **p_obj);

#include <libarmadito-rpc/defs.h>

/*
 * marshalling helper macro
 */
#define A6O_RPC_STRUCT2JSON(S, P, O) a6o_rpc_marshall_struct_##S(P, O)

/*
 * Declaration of unmarshalling functions for struct types
 */
#define A6O_RPC_DEFINE_STRUCT(S) int a6o_rpc_unmarshall_struct_##S(json_t *obj, void **p);

#include <libarmadito-rpc/defs.h>

/*
 * umarshalling helper macro
 */
#define A6O_RPC_JSON2STRUCT(S, O, P) a6o_rpc_unmarshall_struct_##S(O, P)

/*
 * JSON-RPC functions
 */

struct a6o_rpc_connection;

struct a6o_rpc_connection *a6o_rpc_connection_new(int socket_fd);

int a6o_rpc_notify(struct a6o_rpc_connection *conn, const char *method, json_t *params);

/* typedef void (*a6o_rpc_cb_t)(void *result, void *user_data); */

/* int a6o_rpc_call(struct a6o_rpc_handler *m, const char *method, void *params, a6o_rpc_cb_t cb, void *user_data); */



#endif
