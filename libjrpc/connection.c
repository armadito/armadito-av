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

#include "armadito-config.h"

#include <libjrpc/jrpc.h>

#include "buffer.h"
#include "hash.h"
#include "mapper.h"

#include <assert.h>
#include <errno.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

struct jrpc_connection {
	struct jrpc_mapper *mapper;
	size_t current_id;
	struct hash_table *response_table;
	struct buffer input_buffer;
	jrpc_read_cb_t read_cb;
	void *read_cb_data;
	jrpc_write_cb_t write_cb;
	void *write_cb_data;
	void *connection_data;
	jrpc_error_handler_t error_handler;
#ifdef HAVE_PTHREAD
	pthread_mutex_t connection_mutex;
#endif
};

struct rpc_callback_entry {
	jrpc_cb_t cb;
	void *user_data;
};

#define DEFAULT_INPUT_BUFFER_SIZE 1024

#ifdef HAVE_PTHREAD
static void connection_lock(struct jrpc_connection *conn)
{
	pthread_mutex_lock(&conn->connection_mutex);
}

static void connection_unlock(struct jrpc_connection *conn)
{
	pthread_mutex_unlock(&conn->connection_mutex);
}

static void connection_lock_init(struct jrpc_connection *conn)
{
	pthread_mutex_init(&conn->connection_mutex, NULL);
}
#endif

struct jrpc_connection *jrpc_connection_new(struct jrpc_mapper *mapper, void *connection_data)
{
	struct jrpc_connection *conn = malloc(sizeof(struct jrpc_connection));

	conn->mapper = mapper;
	conn->current_id = 1L;
	conn->response_table = hash_table_new(HASH_KEY_INT, NULL, (free_cb_t)free);
	buffer_init(&conn->input_buffer, DEFAULT_INPUT_BUFFER_SIZE);
	conn->read_cb = NULL;
	conn->read_cb_data = NULL;
	conn->write_cb = NULL;
	conn->write_cb_data = NULL;
	conn->connection_data = connection_data;

	return conn;
}

void *jrpc_get_connection_data(struct jrpc_connection *conn)
{
	return conn->connection_data;
}

void jrpc_connection_set_read_cb(struct jrpc_connection *conn, jrpc_read_cb_t read_cb, void *data)
{
	conn->read_cb = read_cb;
	conn->read_cb_data = data;
}

void jrpc_connection_set_write_cb(struct jrpc_connection *conn, jrpc_write_cb_t write_cb, void *data)
{
	conn->write_cb = write_cb;
	conn->write_cb_data = data;
}

void jrpc_connection_set_error_handler(struct jrpc_connection *conn, jrpc_error_handler_t error_handler)
{
	conn->error_handler = error_handler;
}

static int json_buffer_dump_cb(const char *buffer, size_t size, void *data)
{
	struct buffer *b = (struct buffer *)data;

	buffer_append(b, buffer, size);

	return 0;
}

static int connection_send(struct jrpc_connection *conn, json_t *obj)
{
	struct buffer b;
	int ret;

	assert(conn->write_cb != NULL);

	buffer_init(&b, 0);

	ret = json_dump_callback(obj, json_buffer_dump_cb, &b, JSON_COMPACT);

	json_decref(obj);

	if (ret)
		goto end;

	/* \0 for DEBUG fprintf */
	buffer_append(&b, "\r\n\r\n\0", 5);

#ifdef DEBUG
	fprintf(stderr, "sending buffer: %s\n", buffer_data(&b));
#endif
	connection_lock(conn);
	/* - 1 so that we don't write the trailing '\0' */
	ret = (*conn->write_cb)(buffer_data(&b), buffer_size(&b) - 1, conn->write_cb_data);
	connection_unlock(conn);

end:
	buffer_destroy(&b);

	return ret;
}

static size_t connection_register_callback(struct jrpc_connection *conn, jrpc_cb_t cb, void *user_data)
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

static jrpc_cb_t connection_find_callback(struct jrpc_connection *conn, size_t id, void **p_user_data)
{
	struct rpc_callback_entry *entry;
	jrpc_cb_t cb = NULL;

	entry = hash_table_search(conn->response_table, H_INT_TO_POINTER(id));

	if (entry != NULL) {
		cb = entry->cb;
		*p_user_data = entry->user_data;
		hash_table_remove(conn->response_table, H_INT_TO_POINTER(id));
	}

	return cb;
}

static json_t *make_json_call_obj(const char *method, json_t *params, size_t id)
{
	json_t *obj;

	obj = json_pack("{s:s, s:s}", "jsonrpc", "2.0", "method", method);

	if (params != NULL)
		json_object_set(obj, "params", params);

	if (id)
		json_object_set(obj, "id", json_integer(id));

	return obj;
}

int jrpc_notify(struct jrpc_connection *conn, const char *method, json_t *params)
{
	return jrpc_call(conn, method, params, NULL, NULL);
}

int jrpc_call(struct jrpc_connection *conn, const char *method, json_t *params, jrpc_cb_t cb, void *user_data)
{
	json_t *call;
	size_t id = 0L;

	if (cb != NULL)
		id = connection_register_callback(conn, cb, user_data);

#ifdef DEBUG
	fprintf(stderr, "call: method %s id %ld\n", method, id);
#endif
	call = make_json_call_obj(method, params, id);

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

static int json_rpc_unpack(json_t *o, struct json_rpc_obj *s)
{
	const char *version, *method;
	size_t id = 0;
	json_t *params = NULL, *result = NULL, *error = NULL;

	if (json_unpack(o, "{s:s, s:s, s?o, s?i}", "jsonrpc", &version, "method", &method, "params", &params,  "id", &id) == 0
		&& !strcmp(version, "2.0")) {
		s->type = REQUEST;
		s->u.request.method = strdup(method);
		s->u.request.id = id;
		s->u.request.params = params;

		return JRPC_OK;
	}

	if (json_unpack(o, "{s:s, s?o, s?o, s:i}", "jsonrpc", &version, "result", &result, "error", &error,  "id", &id) == 0
		&& !strcmp(version, "2.0")) {
		if (error && result || !error && !result) {
			s->type = MALFORMED_JSON;
			return JRPC_OK;
		}

		if (error != NULL) {
			s->type = ERROR_RESPONSE;
			s->u.error_response.error = error;
			s->u.error_response.id = id;
		} else {
			s->type = RESULT_RESPONSE;
			s->u.result_response.result = result;
			s->u.result_response.id = id;
		}

		return JRPC_OK;
	}

	s->type = MALFORMED_JSON;
	return JRPC_ERR_INVALID_REQUEST;
}

static json_t *make_json_result_obj(json_t *result, size_t id)
{
	return json_pack("{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", result, "id", (json_int_t)id);
}

static json_t *make_json_error_obj(int code, const char *message, json_t *data, size_t id)
{
	json_t *j_err, *j_id;

	j_err = json_pack("{s:i, s:s}", "code", code, "message", message);

	if (data != NULL)
		json_object_set(j_err, "data", data);

	if (id)
		j_id = json_integer(id);
	else
		j_id = json_null();

	return json_pack("{s:s, s:o, s:o}", "jsonrpc", "2.0", "error", j_err, "id", j_id);
}

static int connection_process_request(struct jrpc_connection *conn, const char *method, size_t id, json_t *params)
{
	jrpc_method_t method_cb;
	json_t *result = NULL;
	int ret;

#ifdef DEBUG
	fprintf(stderr, "processing request: method %s id %ld\n", method, id);
#endif

	method_cb = jrpc_mapper_find(conn->mapper, method);

	if (method_cb == NULL)
		return JRPC_ERR_METHOD_NOT_FOUND;

	ret = (*method_cb)(params, &result, conn->connection_data);

	if (ret) {
#ifdef DEBUG
		fprintf(stderr, "processing request: method %s returned error %d\n", method, ret);
#endif
		return ret;
	}

	if (result != NULL) {
		json_t * res = make_json_result_obj(result, id);

		return connection_send(conn, res);
	}

	return JRPC_OK;
}

static int connection_process_result(struct jrpc_connection *conn, size_t id, json_t *result)
{
	jrpc_cb_t cb;
	void *user_data;

#ifdef DEBUG
	fprintf(stderr, "processing result: id %ld\n", id);
#endif
	cb = connection_find_callback(conn, id, &user_data);

	if (cb == NULL)
		return JRPC_ERR_INVALID_RESPONSE_ID;

	(*cb)(result, user_data);

	return JRPC_OK;
}

static int connection_process_buffer(struct jrpc_connection *conn, const char *buffer, size_t size)
{
	json_error_t error;
	json_t *j_obj;
	struct json_rpc_obj rpc_obj;

#ifdef DEBUG
	fprintf(stderr, "received buffer: %s\n", buffer);
#endif

	j_obj = json_loadb(buffer, size, JSON_DISABLE_EOF_CHECK, &error);

	if (j_obj == NULL)
		return JRPC_ERR_PARSE_ERROR;

	if (json_rpc_unpack(j_obj, &rpc_obj))
		return 1; /* TODO: error code */

	switch(rpc_obj.type) {
	case REQUEST:
		return connection_process_request(conn, rpc_obj.u.request.method, rpc_obj.u.request.id, rpc_obj.u.request.params);
	case RESULT_RESPONSE:
		return connection_process_result(conn, rpc_obj.u.result_response.id, rpc_obj.u.result_response.result);
	case ERROR_RESPONSE:
		return 1; /* TODO : error code && process error */
	case MALFORMED_JSON:
		return 1; /* TODO: error code */
	}

	return 1;
}

int jrpc_process(struct jrpc_connection *conn)
{
	ssize_t n_read, i;
	char buffer[DEFAULT_INPUT_BUFFER_SIZE];
	int state;

	n_read = (*conn->read_cb)(buffer, sizeof(buffer), conn->read_cb_data);
	if (n_read <= 0)
		return 1;

	state = 0;
	for (i = 0; i < n_read; i++) {
		switch(state) {
		case 0:
			if (buffer[i] == '\r')
				state = 1;
			else
				buffer_append(&conn->input_buffer, &buffer[i], 1);
			break;
		case 1:
			if (buffer[i] == '\n')
				state = 2;
			else {
				buffer_append(&conn->input_buffer, &buffer[i], 1);
				state = 0;
			}
			break;
		case 2:
			if (buffer[i] == '\r')
				state = 3;
			else {
				buffer_append(&conn->input_buffer, &buffer[i], 1);
				state = 0;
			}
			break;
		case 3:
			if (buffer[i] == '\n') {
				/* for debug */
				char tmp[1] = { '\0' };

				buffer_append(&conn->input_buffer, tmp, 1);
				/* - 1 because of trailing '\0' */
				connection_process_buffer(conn, buffer_data(&conn->input_buffer), buffer_size(&conn->input_buffer) - 1);
				buffer_clear(&conn->input_buffer);
				state = 0;
			} else {
				buffer_append(&conn->input_buffer, &buffer[i], 1);
				state = 0;
			}
			break;
		}
	}

	return 0;
}
