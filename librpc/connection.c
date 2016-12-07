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


#include <libarmadito-rpc/armadito-rpc.h>

#include "connection.h"
#include "buffer.h"
#include "hash.h"

#include <assert.h>
#include <errno.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct a6o_rpc_connection {
	int socket_fd;
	size_t current_id;
	struct hash_table *response_table;
};

struct rpc_callback_entry {
	a6o_rpc_cb_t cb;
	void *user_data;
};

struct a6o_rpc_connection *a6o_rpc_connection_new(int socket_fd)
{
	struct a6o_rpc_connection *conn = malloc(sizeof(struct a6o_rpc_connection));

	conn->socket_fd = socket_fd;
	conn->current_id = 1L;
	conn->response_table = hash_table_new(HASH_KEY_INT, NULL, (free_cb_t)free);

	return conn;
}

#define NOT_YET_IMPLEMENTED(F) fprintf(stderr, "%s: not yet implemented\n", F)

static void connection_lock(struct a6o_rpc_connection *conn)
{
	NOT_YET_IMPLEMENTED(__func__);
}

static void connection_unlock(struct a6o_rpc_connection *conn)
{
	NOT_YET_IMPLEMENTED(__func__);
}

static ssize_t write_n(int fd, char *buffer, size_t size)
{
	size_t to_write = size;

	assert(size > 0);

	while (to_write > 0) {
		int w = write(fd, buffer, to_write);

		if (w < 0)
			return w;

		if (w == 0)
			return 0;

		buffer += w;
		to_write -= w;
	}

	return size;
}

static int json_buffer_dump_cb(const char *buffer, size_t size, void *data)
{
	struct buffer *b = (struct buffer *)data;

	buffer_append(b, buffer, size);

	return 0;
}

static int connection_send(struct a6o_rpc_connection *conn, json_t *obj)
{
	struct buffer b;
	int ret;

	buffer_init(&b, 0);

	ret = json_dump_callback(obj, json_buffer_dump_cb, &b, JSON_COMPACT);

	json_decref(obj);

	if (ret)
		goto end;

	buffer_append(&b, "\r\n\r\n", 4);

	connection_lock(conn);
	ret = write_n(conn->socket_fd, buffer_data(&b), buffer_size(&b));
	connection_unlock(conn);

end:
	buffer_destroy(&b);

	return ret;
}

static size_t connection_register_callback(struct a6o_rpc_connection *conn, a6o_rpc_cb_t cb, void *user_data)
{
	size_t id;
	struct rpc_callback_entry *entry;

	entry = malloc(sizeof(struct rpc_callback_entry));
	entry->cb = cb;
	entry->user_data = user_data;

	connection_lock(conn);

	id = conn->current_id;

	conn->current_id++;

	/* insertion should always work??? */
	if (!hash_table_insert(conn->response_table, H_INT_TO_POINTER(id), entry))
		free(entry);

	connection_unlock(conn);

	return id;
}

static a6o_rpc_cb_t connection_find_callback(struct a6o_rpc_connection *conn, size_t id, void **p_user_data)
{
	struct rpc_callback_entry *entry;
	a6o_rpc_cb_t cb = NULL;

	entry = hash_table_search(conn->response_table, H_INT_TO_POINTER(id));

	if (entry != NULL) {
		cb = entry->cb;
		*p_user_data = entry->user_data;
		hash_table_remove(conn->response_table, H_INT_TO_POINTER(id));
	}

	return cb;
}

static json_t *make_json_rpc_obj(const char *method, json_t *params, size_t id)
{
	json_t *call = json_object();

	json_object_set(call, "jsonrpc", json_string("2.0"));
	json_object_set(call, "method", json_string(method));

	if (params != NULL)
		json_object_set(call, "params", params);

	if (id)
		json_object_set(call, "id", json_integer(id));

	return call;
}

int a6o_rpc_notify(struct a6o_rpc_connection *conn, const char *method, json_t *params)
{
	return a6o_rpc_call(conn, method, params, NULL, NULL);
}

int a6o_rpc_call(struct a6o_rpc_connection *conn, const char *method, json_t *params, a6o_rpc_cb_t cb, void *user_data)
{
	json_t *call;
	size_t id = 0L;

	if (cb != NULL)
		id = connection_register_callback(conn, cb, user_data);

	call = make_json_rpc_obj(method, params, id);

	return connection_send(conn, call);
}

enum json_rpc_obj_type {
	MALFORMED_JSON,
	REQUEST,
	ERROR_RESPONSE,
	RESULT_RESPONSE,
};

struct json_rpc_obj {
	enum json_rpc_obj_type type;
	union {
		struct {
			const char *method;
			size_t id;
			json_t *params;
		} request;
		struct {
			json_t *result;
			size_t id;
		} result_response;
		struct {
			json_t *error;
			size_t id;
		} error_response;
	} u;
};

static int json_rpc_unpack_request(json_t *o, struct json_rpc_obj *s)
{
	const char *version;
	const char *method;
	size_t id = 0;
	json_t *params = NULL;

	if (json_unpack(o, "{s:s, s:s, s?o, s?i}", "jsonrpc", &version, "method", &method, "params", &params,  "id", &id) < 0
		|| strcmp(version, "2.0") != 0) {
		s->type = MALFORMED_JSON;
		return 0;
	}

	s->type = REQUEST;
	s->u.request.method = strdup(method);
	s->u.request.id = id;
	s->u.request.params = params;

	return 1;
}

static int json_rpc_unpack_response(json_t *o, struct json_rpc_obj *s)
{
	const char *version;
	size_t id = 0;
	json_t *result = NULL;
	json_t *error = NULL;

	if (json_unpack(o, "{s:s, s?o, s?o, s:i}", "jsonrpc", &version, "result", &result, "error", &error,  "id", &id) < 0
		|| strcmp(version, "2.0") != 0) {
		s->type = MALFORMED_JSON;
		return 0;
	}

	if ((error != NULL && result != NULL) || (error == NULL && result == NULL))
		return 0;

	if (error != NULL) {
		s->type = ERROR_RESPONSE;
		s->u.error_response.error = error;
		s->u.error_response.id = id;
	} else {
		s->type = RESULT_RESPONSE;
		s->u.result_response.result = result;
		s->u.result_response.id = id;
	}

	return 1;
}

#if 0
static enum response_type rpc_obj_get_response_type(json_t *obj, size_t *p_id, json_t **p_content)
{
	json_t *error, *result, *id;

	if (!rpc_obj_basic_check(obj))
		return MALFORMED_RESPONSE;

	error = json_object_get(obj, "error");
	result = json_object_get(obj, "result");

	if (error != NULL && result != NULL)
		return MALFORMED_RESPONSE;

	if (error == NULL && result == NULL)
		return MALFORMED_RESPONSE;

	if (error != NULL && !json_is_object(error))
		return MALFORMED_RESPONSE;

	if (result != NULL && !json_is_object(result))
		return MALFORMED_RESPONSE;

	id = json_object_get(obj, "id");
	if (!json_is_integer(id))
		return MALFORMED_RESPONSE;
	*p_id = json_integer_value(id);

	if (result != NULL) {
		*p_content = result;
		return RESULT_RESPONSE;
	}

	*p_content = error;
	return ERROR_RESPONSE;
}
#endif

static int process_result(struct a6o_rpc_connection *conn, size_t id, json_t *result)
{
	a6o_rpc_cb_t cb;
	void *user_data;

	cb = connection_find_callback(conn, id, &user_data);

	if (cb != NULL) {
		(*cb)(result, user_data);
		return 0;
	}

	return 1;
}

int a6o_rpc_process(struct a6o_rpc_connection *conn, const char *buffer, size_t size)
{
	json_t *obj, *content;
	size_t id;
	json_error_t error;

	obj = json_loadb(buffer, size, JSON_DISABLE_EOF_CHECK, &error);

	if (obj == NULL)
		return 1; /* TODO: error code */

#if 0
	switch(rpc_obj_get_response_type(obj, &id, &content)) {
	case MALFORMED_RESPONSE:
		return 1; /* TODO: error code */
	case ERROR_RESPONSE:
		return 1; /* TODO : error code && process error */
	case RESULT_RESPONSE:
		return process_result(conn, id, content);
	}
#endif

	return 1;
}
