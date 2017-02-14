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

#include <jansson.h>
#include <glib.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <rpc/io.h>
#include <rpc/rpctypes.h>
#include <libjrpc/jrpc.h>
#include <net/unixsockclient.h>
#include <net/netdefaults.h>

#include "apiclient.h"

struct api_client {
	enum api_client_mode mode;
	GAsyncQueue *event_queue;
	struct jrpc_connection *conn;
	json_t *sync_call_result;
	int client_sock;
	int done;
};

static int api_client_poll(struct api_client *client);

struct api_client *api_client_new(enum api_client_mode mode)
{
	struct api_client *client = malloc(sizeof(struct api_client));

	client->mode = mode;
	if (client->mode == CLIENT_THREADED)
		client->event_queue = g_async_queue_new();
	else
		client->event_queue = NULL;

	client->conn = NULL;
	client->client_sock = -1;
	client->done = 0;

	return client;
}

void api_client_free(struct api_client *client)
{
	g_async_queue_unref(client->event_queue);

	free((void *)client);
}

static gpointer api_client_thread(gpointer data)
{
	struct api_client *client = (struct api_client *)data;

	api_client_poll(client);

	return NULL;
}

int api_client_connect(struct api_client *client, struct jrpc_mapper *mapper)
{
	int client_sock;

	client_sock = unix_client_connect(DEFAULT_SOCKET_PATH, 10);

	if (client_sock < 0) {
		perror("cannot connect");
		return -1;
	}

	client->client_sock = client_sock;

	client->conn = jrpc_connection_new(mapper, client);

	jrpc_connection_set_read_cb(client->conn, unix_fd_read_cb, &client->client_sock);
	jrpc_connection_set_write_cb(client->conn, unix_fd_write_cb, &client->client_sock);

	if (client->mode == CLIENT_THREADED)
		g_thread_new("api client thread", api_client_thread, client);

	return 0;
}

static int api_client_poll(struct api_client *client)
{
	int ret = 0;

	fprintf(stderr, "before poll\n");
	while (1) {
		ret = jrpc_process(client->conn);

		if (ret == JRPC_EOF)
			break;

		if (client->done)
			break;
	}

	fprintf(stderr, "after poll\n");
	if (close(client->client_sock) < 0)
		perror("closing connection");

	return ret;
}

static void sync_call_cb(json_t *result, void *user_data)
{
	struct api_client *client = (struct api_client *)user_data;

	fprintf(stderr, "in sync_call_cb\n");

	client->sync_call_result = result;

	api_client_done(client);
}

int api_client_sync_call(struct api_client *client, const char *method, json_t *params, json_t **result)
{
	int ret;

	*result = NULL;

	if ((ret = jrpc_call(api_client_get_connection(client), method, params, sync_call_cb, client)))
		return ret;

	if ((ret = api_client_poll(client)))
		return ret;

	*result = client->sync_call_result;

	return 0;
}

struct jrpc_connection *api_client_get_connection(struct api_client *client)
{
	return client->conn;
}

void api_client_done(struct api_client *client)
{
	client->done = 1;
}

void api_client_push_event(struct api_client *client, json_t *event)
{
	g_async_queue_push(client->event_queue, event);
}

void api_client_pop_event(struct api_client *client, json_t **p_event)
{
	*p_event = g_async_queue_pop(client->event_queue);
}

