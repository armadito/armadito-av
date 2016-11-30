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

#include <assert.h>
#include <errno.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct a6o_rpc_connection {
	int socket_fd;
};

struct a6o_rpc_connection *a6o_rpc_connection_new(int socket_fd)
{
	struct a6o_rpc_connection *conn = malloc(sizeof(struct a6o_rpc_connection));

	conn->socket_fd = socket_fd;

	return conn;
}

static ssize_t write_n(int fd, char *buffer, size_t len)
{
	size_t to_write = len;

	assert(len > 0);

	while (to_write > 0) {
		int w = write(fd, buffer, to_write);

		if (w < 0)
			return w;

		if (w == 0)
			return 0;

		buffer += w;
		to_write -= w;
	}

	return len;
}

static int json_buffer_dump_cb(const char *buffer, size_t size, void *data)
{
	struct buffer *b = (struct buffer *)data;

	buffer_append(b, buffer, size);

	return 0;
}

int a6o_rpc_connection_send(struct a6o_rpc_connection *conn, json_t *obj)
{
	struct buffer b;
	int ret;

	buffer_init(&b, 0);

	ret = json_dump_callback(obj, json_buffer_dump_cb, &b, JSON_COMPACT);

	json_decref(obj);

	if (ret)
		goto end;

	buffer_append(&b, "\r\n\r\n", 4);

	/* TODO: lock the conn */
	ret = write_n(conn->socket_fd, buffer_data(&b), buffer_size(&b));

end:
	buffer_destroy(&b);

	return ret;
}

