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
 * JSON-RPC functions
 */

/*
 * RPC mapper
 * handles
 * - mapping method name to method definition (callback, params un/marshalling, result un/marshalling)
 */
struct a6o_rpc_mapper;

struct a6o_rpc_mapper *a6o_rpc_mapper_new(void);

typedef int (*a6o_rpc_method_t)(json_t *params, json_t **result, void *connection_data);

/* may be a macro with #P and #R ???
   + no need to have a hash table to map type names to marshall functions
   + check is done at compile time
   - but with an obscure link error message related to code coming from a macro expansion
   - need to declare all marshall functions in this header
*/
int a6o_rpc_mapper_add(struct a6o_rpc_mapper *mapper, const char *method, a6o_rpc_method_t method_cb);

/*
 * RPC connection
 * handles
 * - JSON objects unpacking
 * - id generation and management
 *
 */

struct a6o_rpc_connection;

typedef ssize_t (*a6o_rpc_write_cb_t)(const char *buffer, size_t size, void *data);

typedef ssize_t (*a6o_rpc_read_cb_t)(char *buffer, size_t size, void *data);

struct a6o_rpc_connection *a6o_rpc_connection_new(struct a6o_rpc_mapper *mapper, void *connection_data);

void a6o_rpc_connection_set_read_cb(struct a6o_rpc_connection *conn, a6o_rpc_read_cb_t read_cb, void *data);

void a6o_rpc_connection_set_write_cb(struct a6o_rpc_connection *conn, a6o_rpc_write_cb_t write_cb, void *data);

int a6o_rpc_notify(struct a6o_rpc_connection *conn, const char *method, json_t *params);

typedef void (*a6o_rpc_cb_t)(json_t *result, void *user_data);

int a6o_rpc_call(struct a6o_rpc_connection *conn, const char *method, json_t *params, a6o_rpc_cb_t cb, void *user_data);

/* temporary */
int a6o_rpc_process(struct a6o_rpc_connection *conn, const char *buffer, size_t size);

#endif
