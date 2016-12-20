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

#include <libjrpc/jrpc.h>

#include "connection.h"
#include "mapper.h"

#include <string.h>

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
	int ret;

	if (cb != NULL)
		id = connection_register_callback(conn, cb, user_data);

#ifdef DEBUG
	fprintf(stderr, "call: method %s id %ld\n", method, id);
#endif
	call = make_json_call_obj(method, params, id);

	ret = connection_send(conn, call);

	json_decref(call);

	return ret;
}

enum rpc_obj_type {
	REQUEST,
	ERROR_RESPONSE,
	RESULT_RESPONSE,
};

struct rpc_obj {
	enum rpc_obj_type type;
	union {
		struct {
			const char *method;
			size_t id;
			json_t *params;
		} request;
		struct {
			size_t id;
			json_t *result;
		} result_response;
		struct {
			size_t id;
			int code;
			const char *message;
			json_t *data;
		} error_response;
	} u;
};

static int json_rpc_unpack(json_t *j_obj, struct rpc_obj *r_obj)
{
	const char *version, *method, *message = NULL;
	size_t id = 0;
	json_t *params = NULL, *result = NULL, *error = NULL, *data = NULL;
	int code;

	if (json_unpack(j_obj, "{s:s, s:s, s?o, s?i}", "jsonrpc", &version, "method", &method, "params", &params,  "id", &id) == 0
		&& !strcmp(version, "2.0")) {
		r_obj->type = REQUEST;
		r_obj->u.request.method = strdup(method);
		r_obj->u.request.id = id;
		r_obj->u.request.params = params;

		return JRPC_OK;
	}

	if (json_unpack(j_obj, "{s:s, s?o, s?o, s:i}", "jsonrpc", &version, "result", &result, "error", &error,  "id", &id) != 0
		|| strcmp(version, "2.0") != 0)
		return JRPC_ERR_INVALID_REQUEST;

	if (error && result || !error && !result)
		return JRPC_ERR_INVALID_REQUEST;

	if (error != NULL) {
		if (json_unpack(error, "{s:i, s:s, s?o}", "code", &code, "message", &message, "data", &data) != 0)
			return JRPC_ERR_INVALID_REQUEST;

		r_obj->type = ERROR_RESPONSE;
		r_obj->u.error_response.id = id;
		r_obj->u.error_response.code = code;
		r_obj->u.error_response.message = strdup(message);
		r_obj->u.error_response.data = data;
	} else {
		r_obj->type = RESULT_RESPONSE;
		r_obj->u.result_response.result = result;
		r_obj->u.result_response.id = id;
	}

	return JRPC_OK;
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

static int connection_process_request(struct jrpc_connection *conn, struct rpc_obj *r_obj)
{
	jrpc_method_t method_cb;
	const char *method = r_obj->u.request.method;
	size_t id = r_obj->u.request.id;
	json_t *params = r_obj->u.request.params;
	json_t *result = NULL;
	int ret;

#ifdef DEBUG
	fprintf(stderr, "processing request: method %s id %ld\n", method, id);
#endif

	method_cb = jrpc_mapper_find(connection_get_mapper(conn), method);
	if (method_cb == NULL) {
		ret = JRPC_ERR_METHOD_NOT_FOUND;
		connection_send(conn, make_json_error_obj(ret, "method was not found", NULL, id));
		return ret;
	}

	ret = (*method_cb)(params, &result, jrpc_connection_get_data(conn));
	if (ret) {
#ifdef DEBUG
		fprintf(stderr, "processing request: method %s returned error\n", method);
#endif
		connection_send(conn, make_json_error_obj(ret, "method returned an error", NULL, id));
		return ret;
	}

	/* was it a notification, i.e. id == 0? if yes, no result to send back */
	if (id != 0)
		connection_send(conn, make_json_result_obj(result, id));

	return JRPC_OK;
}

static int connection_process_result(struct jrpc_connection *conn, struct rpc_obj *r_obj)
{
	jrpc_cb_t cb;
	void *user_data;
	size_t id = r_obj->u.result_response.id;
	json_t *result = r_obj->u.result_response.result;

#ifdef DEBUG
	fprintf(stderr, "processing result: id %ld\n", id);
#endif
	cb = connection_find_callback(conn, id, &user_data);

	if (cb == NULL)
		return JRPC_ERR_INVALID_RESPONSE_ID;

	(*cb)(result, user_data);

	return JRPC_OK;
}

static int connection_process_error(struct jrpc_connection *conn, struct rpc_obj *r_obj)
{
	void *user_data;
	size_t id = r_obj->u.error_response.id;
	int code = r_obj->u.error_response.code;
	const char *message = r_obj->u.error_response.message;
	json_t *data = r_obj->u.error_response.data;
	jrpc_error_handler_t error_handler = jrpc_connection_get_error_handler(conn);

#ifdef DEBUG
	fprintf(stderr, "processing error: id %ld\n", id);
#endif

	if (error_handler != NULL)
		(*error_handler)(conn, id, code, message, data);

	return JRPC_OK;
}

int jrpc_process(struct jrpc_connection *conn)
{
	int ret;
	json_t *j_obj;
	struct rpc_obj r_obj;

	if ((ret = connection_receive(conn, &j_obj)))
		return ret;

	if (json_rpc_unpack(j_obj, &r_obj))
		return JRPC_ERR_INVALID_REQUEST;

	switch(r_obj.type) {
	case REQUEST:
		return connection_process_request(conn, &r_obj);
	case RESULT_RESPONSE:
		return connection_process_result(conn, &r_obj);
	case ERROR_RESPONSE:
		return connection_process_error(conn, &r_obj);
	}

	return JRPC_OK;
}
