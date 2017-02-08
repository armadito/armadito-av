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

#include <libarmadito/armadito.h>
#include <libjrpc/jrpc.h>

#include <rpc/rpcbe.h>

#include "server.h"

#include <assert.h>
#include <errno.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

struct server {
	int listen_sock;
	struct armadito *armadito;
	GThreadPool *thread_pool;
	GIOChannel *channel;
};

static ssize_t unix_fd_write_cb(const char *buffer, size_t size, void *data)
{
	int fd = *(int *)data;

	return send(fd, buffer, size, MSG_EOR);
}

static ssize_t unix_fd_read_cb(char *buffer, size_t size, void *data)
{
	int fd = *(int *)data;

	return recv(fd, buffer, size, 0);
}

struct client_data {
	int client_sock;
	struct jrpc_connection *conn;
};

static void client_thread(gpointer data, gpointer user_data)
{
	int ret;
	struct client_data *cd = (struct client_data *)data;

	while ((ret = jrpc_process(cd->conn)) != JRPC_EOF)
		;

	if (close(cd->client_sock) < 0)
		a6o_log(A6O_LOG_SERVICE, A6O_LOG_LEVEL_WARNING, "closing client socket failed (%s)", strerror(errno));

	jrpc_connection_free(cd->conn);
	free(cd);
}

static gboolean server_listen_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
	struct server *server = (struct server *)data;
	void *client;
	int client_sock;
	int *p_client_sock;
	struct jrpc_connection *conn;
	struct client_data *cd;

	client_sock = accept(server->listen_sock, NULL, NULL);

	if (client_sock < 0) {
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_ERROR, "accept() failed (%s)", strerror(errno));
		return FALSE;
	}

	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_DEBUG, "accepted client connection: fd = %d", client_sock);

	p_client_sock = malloc(sizeof(int));
	*p_client_sock = client_sock;

	conn = jrpc_connection_new(a6o_get_rpcbe_mapper(), server->armadito);

	jrpc_connection_set_read_cb(conn, unix_fd_read_cb, p_client_sock);
	jrpc_connection_set_write_cb(conn, unix_fd_write_cb, p_client_sock);

	cd = malloc(sizeof(struct client_data));
	cd->client_sock = client_sock;
	cd->conn = conn;

	g_thread_pool_push(server->thread_pool, (gpointer)cd, NULL);

	return TRUE;
}

struct server *server_new(struct armadito *armadito, int server_sock)
{
	struct server *server = (struct server *)malloc(sizeof(struct server));
	assert(server != NULL);

	server->armadito = armadito;
	server->listen_sock = server_sock;

	server->thread_pool = g_thread_pool_new(client_thread, server, -1, FALSE, NULL);

	server->channel = g_io_channel_unix_new(server->listen_sock);
	g_io_add_watch(server->channel, G_IO_IN, server_listen_cb, server);

	return server;
}
