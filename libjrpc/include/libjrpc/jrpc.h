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

#ifndef LIBJRPC_JRPC_H
#define LIBJRPC_JRPC_H

#include <libjrpc/error.h>

#include <stddef.h>
#include <jansson.h>

/*
 * JSON-RPC functions
 */

/*
 * RPC mapper
 * handles mapping method name to method definition
 */
struct jrpc_mapper;

struct jrpc_mapper *jrpc_mapper_new(void);

typedef int (*jrpc_method_t)(json_t *params, json_t **result, void *connection_data);

int jrpc_mapper_add(struct jrpc_mapper *mapper, const char *method, jrpc_method_t method_cb);

int jrpc_mapper_add_error_message(struct jrpc_mapper *mapper, unsigned char error_code, const char *error_message);

/*
 * RPC connection
 * handles
 * - JSON objects unpacking
 * - id generation and management
 *
 */

struct jrpc_connection;

struct jrpc_connection *jrpc_connection_new(struct jrpc_mapper *mapper, void *connection_data);

void *jrpc_connection_get_data(struct jrpc_connection *conn);

typedef ssize_t (*jrpc_read_cb_t)(char *buffer, size_t size, void *data);

void jrpc_connection_set_read_cb(struct jrpc_connection *conn, jrpc_read_cb_t read_cb, void *data);

typedef ssize_t (*jrpc_write_cb_t)(const char *buffer, size_t size, void *data);

void jrpc_connection_set_write_cb(struct jrpc_connection *conn, jrpc_write_cb_t write_cb, void *data);

typedef void (*jrpc_error_handler_t)(struct jrpc_connection *conn, size_t id, int code, const char *message, json_t *data);

void jrpc_connection_set_error_handler(struct jrpc_connection *conn, jrpc_error_handler_t error_handler);

int jrpc_notify(struct jrpc_connection *conn, const char *method, json_t *params);

typedef void (*jrpc_cb_t)(json_t *result, void *user_data);

int jrpc_call(struct jrpc_connection *conn, const char *method, json_t *params, jrpc_cb_t cb, void *user_data);

int jrpc_process(struct jrpc_connection *conn);

#endif
