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

#define RPC_NO_ID (-1)

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
		id = a6o_rpc_connection_register_callback(conn, cb, user_data);

	call = make_json_rpc_obj(method, params, id);

	return a6o_rpc_connection_send(conn, call);
}

