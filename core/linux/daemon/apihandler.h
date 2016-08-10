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

#ifndef DAEMON_APIHANDLER_H_
#define DAEMON_APIHANDLER_H_

#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <microhttpd.h>
#include <json.h>

#include "httpd.h"

struct api_handler;

struct api_handler *api_handler_new(void *user_data);

int api_handler_serve(struct api_handler *a, struct MHD_Connection *connection,
	enum http_method method, const char *path, const char *post_data, size_t post_data_size);

typedef int (*check_cb_t)(struct MHD_Connection *connection, struct json_object *in);
typedef int (*process_cb_t)(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out, void *user_data);

const char *api_get_user_agent(struct MHD_Connection *connection);
const char *api_get_token(struct MHD_Connection *connection);
const char *api_get_argument(struct MHD_Connection *connection, const char *key);

struct api_client;

int api_handler_add_client(struct api_handler *a, const char *token);
struct api_client *api_handler_get_client(struct api_handler *a, const char *token);
int api_handler_remove_client(struct api_handler *a, const char *token);

int api_client_push_event(struct api_client *client, struct json_object *event);
int api_client_pop_event(struct api_client *client, struct json_object **p_event);

#endif
