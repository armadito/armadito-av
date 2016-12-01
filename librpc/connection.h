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

#ifndef LIBRPC_CONNECTION_H
#define LIBRPC_CONNECTION_H

#include <libarmadito-rpc/armadito-rpc.h>

#include <jansson.h>

int a6o_rpc_connection_send(struct a6o_rpc_connection *conn, json_t *obj);

size_t a6o_rpc_connection_register_callback(struct a6o_rpc_connection *conn, a6o_rpc_cb_t cb, void *user_data);

#endif
