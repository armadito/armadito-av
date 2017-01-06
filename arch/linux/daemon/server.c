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

#include "server.h"

#include <assert.h>
#include <errno.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

struct server {
	int listen_sock;
	struct armadito *armadito;
	GThreadPool *thread_pool;
	GIOChannel *channel;
};

static void client_thread(gpointer data, gpointer user_data)
{
	struct server *server = (struct server *)user_data;

#if 0
	switch(server->ipc_type) {
	case OLD_IPC:
	{
		struct ipc_client *client = (struct ipc_client *)data;

		while (ipc_client_process(client) > 0)
			;

		ipc_client_free(client);
	}
	break;
	case JSON_IPC:
	{
		struct json_client *client = (struct json_client *)data;

		json_client_process(client);

		json_client_free(client);
	}
	break;
	}
#endif
}

static gboolean server_listen_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
	struct server *server = (struct server *)data;
	void *client;
	int client_sock;

	client_sock = accept(server->listen_sock, NULL, NULL);

	if (client_sock < 0) {
		a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_ERROR, "accept() failed (%s)", strerror(errno));
		return FALSE;
	}

	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_DEBUG, "accepted client connection: fd = %d", client_sock);

#if 0
	switch(server->ipc_type) {
	case OLD_IPC:
		client = ipc_client_new(client_sock, server->armadito);
		break;
	case JSON_IPC:
		client = json_client_new(client_sock, server->armadito);
		break;
	}
#endif

	g_thread_pool_push(server->thread_pool, (gpointer)client, NULL);

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
