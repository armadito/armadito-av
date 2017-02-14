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

#ifndef HTTPD_APIHANDLER_H
#define HTTPD_APIHANDLER_H

#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <microhttpd.h>
#include <jansson.h>

#include "httpd.h"
#include "apiclient.h"

struct api_handler;

struct api_handler *api_handler_new(void *user_data);

int api_handler_serve(struct api_handler *a, struct MHD_Connection *connection,
	enum http_method method, const char *path, const char *post_data, size_t post_data_size);

typedef int (*check_cb_t)(struct MHD_Connection *connection, json_t *in);
typedef int (*process_cb_t)(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data);

const char *api_get_user_agent(struct MHD_Connection *connection);
const char *api_get_token(struct MHD_Connection *connection);
const char *api_get_argument(struct MHD_Connection *connection, const char *key);

int api_handler_add_client(struct api_handler *a, const char *token, struct api_client *client);
struct api_client *api_handler_get_client(struct api_handler *a, const char *token);
int api_handler_remove_client(struct api_handler *a, const char *token);

#endif
