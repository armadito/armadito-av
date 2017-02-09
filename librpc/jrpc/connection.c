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

#define DEFAULT_INPUT_BUFFER_SIZE 4096

#ifdef HAVE_PTHREAD
static void connection_lock(struct jrpc_connection *conn)
{
	if (pthread_mutex_lock(&conn->connection_mutex))
		perror("pthread_mutex_lock");
}

static void connection_unlock(struct jrpc_connection *conn)
{
	if (pthread_mutex_unlock(&conn->connection_mutex))
		perror("pthread_mutex_unlock");
}

static void connection_lock_init(struct jrpc_connection *conn)
{
	pthread_mutex_init(&conn->connection_mutex, NULL);
}

static void connection_lock_destroy(struct jrpc_connection *conn)
{
	pthread_mutex_destroy(&conn->connection_mutex);
}
#else
static void connection_lock(struct jrpc_connection *conn)
{
}

static void connection_unlock(struct jrpc_connection *conn)
{
}

static void connection_lock_init(struct jrpc_connection *conn)
{
}

static void connection_lock_destroy(struct jrpc_connection *conn)
{
}
#endif

struct jrpc_connection *jrpc_connection_new(struct jrpc_mapper *mapper, void *connection_data)
{
	struct jrpc_connection *conn = malloc(sizeof(struct jrpc_connection));

	conn->mapper = mapper;

	conn->current_id = 1L;
	conn->response_table = hash_table_new(HASH_KEY_INT, NULL, (free_cb_t)free);

	conn->read_cb = NULL;
	conn->read_cb_data = NULL;
	conn->write_cb = NULL;
	conn->write_cb_data = NULL;

	conn->connection_data = connection_data;
	conn->error_handler = NULL;

	connection_lock_init(conn);

	return conn;
}

void *jrpc_connection_get_data(struct jrpc_connection *conn)
{
	return conn->connection_data;
}

struct jrpc_mapper *connection_get_mapper(struct jrpc_connection *conn)
{
	return conn->mapper;
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

jrpc_error_handler_t jrpc_connection_get_error_handler(struct jrpc_connection *conn)
{
	return conn->error_handler;
}

void jrpc_connection_free(struct jrpc_connection *conn)
{
	hash_table_free(conn->response_table);
	connection_lock_destroy(conn);
	free(conn);
}

size_t connection_register_callback(struct jrpc_connection *conn, jrpc_cb_t cb, void *user_data)
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

jrpc_cb_t connection_find_callback(struct jrpc_connection *conn, size_t id, void **p_user_data)
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

static int json_buffer_dump_cb(const char *buffer, size_t size, void *data)
{
	struct buffer *b = (struct buffer *)data;

	buffer_append(b, buffer, size);

	return 0;
}

int connection_send(struct jrpc_connection *conn, json_t *obj)
{
	struct buffer b;
	int ret = JRPC_OK;

	assert(conn->write_cb != NULL);

	buffer_init(&b, 0);

	if (json_dump_callback(obj, json_buffer_dump_cb, &b, JSON_COMPACT) < 0) {
		ret = JRPC_ERR_INTERNAL_ERROR;
		goto end;
	}

	/* \0 for DEBUG fprintf */
	buffer_append(&b, "\r\n\r\n\0", 5);

#ifdef JRPC_DEBUG
	fprintf(stderr, "sending buffer: %s\n", buffer_data(&b));
#endif
	connection_lock(conn);
	/* - 1 so that we don't write the trailing '\0' */
	if ((*conn->write_cb)(buffer_data(&b), buffer_size(&b) - 1, conn->write_cb_data) < 0)
		ret = JRPC_ERR_INTERNAL_ERROR;
	connection_unlock(conn);

end:
	buffer_destroy(&b);

	return ret;
}

int connection_receive(struct jrpc_connection *conn, json_t **p_obj)
{
	json_error_t error;
	ssize_t n_read;
	char buffer[DEFAULT_INPUT_BUFFER_SIZE];

	assert(conn->read_cb != NULL);

	memset(buffer, 0, sizeof(buffer));

	n_read = (*conn->read_cb)(buffer, sizeof(buffer), conn->read_cb_data);
	if (n_read < 0)
		return JRPC_ERR_INTERNAL_ERROR;

	if (n_read == 0)
		return JRPC_EOF;

#ifdef JRPC_DEBUG
	fprintf(stderr, "received buffer: %s\n", buffer);
#endif

	/* TODO: must check that the buffer terminates with "\r\n\r\n" */

	*p_obj = json_loadb(buffer, n_read, JSON_DISABLE_EOF_CHECK, &error);

	if (*p_obj == NULL)
		return JRPC_ERR_PARSE_ERROR;

	return JRPC_OK;
}

