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

#ifndef LIBJRPC_CONNECTION_H
#define LIBJRPC_CONNECTION_H

struct jrpc_mapper *connection_get_mapper(struct jrpc_connection *conn);

size_t connection_register_callback(struct jrpc_connection *conn, jrpc_cb_t cb, void *user_data);

jrpc_cb_t connection_find_callback(struct jrpc_connection *conn, size_t id, void **p_user_data);

int connection_send(struct jrpc_connection *conn, json_t *obj);

int connection_receive(struct jrpc_connection *conn, json_t **p_obj);

#endif
