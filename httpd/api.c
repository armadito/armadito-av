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

#define _GNU_SOURCE

#include "armadito-config.h"

#include <glib.h>
#include <jansson.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <rpc/io.h>
#include <rpc/rpctypes.h>
#include <libjrpc/jrpc.h>

#include "api.h"
#include "log.h"

#define HASH_ONE(H, C) (H) ^= ((H) << 5) + ((H) >> 2) + (C)
#define HASH_INIT_VAL 0

/* #define HASH_ONE(H, C) (H) = (((H) << 5) + (H)) + (C) */
/* #define HASH_INIT_VAL 5381 */

static void hash_buff(const char *buff, size_t len, int64_t *hash)
{
	for ( ; len--; buff++)
		HASH_ONE(*hash, *buff);
}

static void hash_str(const char *str, int64_t *hash)
{
	for ( ; *str; str++)
		HASH_ONE(*hash, *str);
}

int register_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data)
{
	int64_t token = HASH_INIT_VAL;
	const char *user_agent;
	time_t now;
	char *s_token;
	struct api_client *client;

	time(&now);
	hash_buff((const char *)&now, sizeof(time_t), &token);
	user_agent = api_get_user_agent(connection);
	hash_str(user_agent, &token);

	if (token < 0)
		token = -token;

	log_d("token %lld", token);

	*out = json_object();
	asprintf(&s_token, "%ld", token);
	json_object_set(*out, "token", json_string(s_token));

	client = api_client_new(CLIENT_THREADED);

	return api_handler_add_client(a, s_token, client);
}

int unregister_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data)
{
	const char *token = api_get_token(connection);

	/* this should not happen because the token presence has already been tested in API handler */
	if (token != NULL)
		return api_handler_remove_client(a, token);

	return 1;
}

int ping_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data)
{
	*out = json_object();
	json_object_set(*out, "status", json_string("ok"));

	return 0;
}

int event_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data)
{
	const char *token;
	struct api_client *client;

	token = api_get_token(connection);
	/* this should not happen because the token presence has already been tested in API handler */
	if (token == NULL)
		return 1;

	client = api_handler_get_client(a, token);

	if (client != NULL) {
		api_client_pop_event(client, out);

		return 0;
	}

	return 1;
}

int scan_check_cb(struct MHD_Connection *connection, json_t *in)
{
	json_t *j_path;

	j_path = json_object_get(in, "path");
	if (j_path == NULL)
		return 1;

	if (!json_is_string(j_path))
		return 1;

	return 0;
}

static int notify_event_method(struct jrpc_connection *conn, json_t *params, json_t **result)
{
	struct api_client *client = (struct api_client *)jrpc_connection_get_data(conn);
	struct a6o_event *ev;
	int ret;

	if ((ret = JRPC_JSON2STRUCT(a6o_event, params, &ev)))
		return ret;

	api_client_push_event(client, params);

	if (ev->type == EVENT_ON_DEMAND_COMPLETED)
		api_client_done(client);

	return JRPC_OK;
}

static struct jrpc_mapper *create_scan_mapper(void)
{
	struct jrpc_mapper *mapper;

	mapper = jrpc_mapper_new();
	jrpc_mapper_add(mapper, "notify_event", notify_event_method);

	return mapper;
}

int scan_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data)
{
	json_t *j_path;
	json_t *j_param;
	const char *path;
	const char *token;
	struct api_client *client;
	struct a6o_rpc_scan_param param;
	int ret;

	token = api_get_token(connection);
	/* this should not happen because the token presence has already been tested in API handler */
	if (token == NULL)
		return 1;

	client = api_handler_get_client(a, token);
	if (client == NULL)
		return 1;

	/* check of parameters has already been done in scan_check_cb */
	j_path = json_object_get(in, "path");
	path = json_string_value(j_path);

	if (api_client_connect(client, create_scan_mapper()) < 0)
		return 1;

	param.root_path = path;
	param.send_progress = 1;
	if ((ret = JRPC_STRUCT2JSON(a6o_rpc_scan_param, &param, &j_param)))
		return ret;

	if ((ret = jrpc_call(api_client_get_connection(client), "scan", j_param, NULL, NULL)))
		return 1;

	*out = json_object();
	json_object_set(*out, "status", json_string("ok"));

	return 0;
}

int status_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data)
{
	struct api_client *client;
	int ret;

	client = api_client_new(CLIENT_NON_THREADED);
	if (client == NULL)
		return 1;

	if (api_client_connect(client, NULL) < 0)
		return 1;

	if ((ret = api_client_sync_call(client, "status", NULL, out)))
		return 1;

	return 0;
}

static const char *dirent_type(struct dirent *entry)
{
	switch(entry->d_type) {
	case DT_DIR:
		return "folder";
	case DT_LNK:
		return "link";
	case DT_REG:
		return "file";
	default:
		return "other";
	}
}

static char *get_root_path(const char *path)
{
	size_t len = strlen(path);
	char *root_path;

	/* len > 0 because this is tested in browse_path before calling this function */
	if (path[len - 1] == '/')
		return strdup(path);

	root_path = malloc(len + 2);
	snprintf(root_path, len + 2, "%s/", path);

	return root_path;
}

static int browse_path(const char *path, json_t *result)
{
	DIR *d;
	json_t *j_entries;
	json_t *j_entry;
	char *root_path;
	char *entry_path;

	if (path == NULL)
		path = "/";

	if (path[0] == '\0') {
		json_object_set(result, "error", json_string("path is empty"));
		return 1;
	}

	json_object_set(result, "path", json_string(path));

	root_path = get_root_path(path);

	if ((d = opendir(path)) == NULL) {
		json_object_set(result, "error", json_string(strerror(errno)));
		free((void *)root_path);
		return 1;
	}

	j_entries = json_array();
	json_object_set(result, "content", j_entries);

	while(1) {
		struct dirent *entry;

		errno = 0;
		entry = readdir(d);

		if (entry == NULL) {
			/* from man readdir: If the end of the directory stream is reached, NULL is returned and errno is not changed */
			if (errno == 0)
				break;
		}

		if (entry->d_type == DT_DIR && (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")))
			continue;

		j_entry = json_object();
		json_object_set(j_entry, "name", json_string(entry->d_name));
		json_object_set(j_entry, "type", json_string(dirent_type(entry)));

		if (asprintf(&entry_path, "%s%s", root_path, entry->d_name) != -1){
			json_object_set(j_entry, "full_path", json_string(entry_path));
			free(entry_path);
		}

		json_array_append(j_entries, j_entry);
	}

	free((void *)root_path);

	if (closedir(d) < 0)
		log_w("error closing directory %s (%s)", path, strerror(errno));

	return 0;
}

int browse_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data)
{
	const char *path = api_get_argument(connection, "path");

	*out = json_object();

	return browse_path(path, *out);
}

int version_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data)
{
	*out = json_object();
	json_object_set(*out, "antivirus-version", json_string(VERSION));

	return 0;
}

