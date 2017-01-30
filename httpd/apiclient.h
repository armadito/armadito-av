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

#ifndef HTTPD_APICLIENT_H
#define HTTPD_APICLIENT_H

#include <jansson.h>
#include <libjrpc/jrpc.h>

struct api_client;

struct api_client *api_client_new(void);

void api_client_free(struct api_client *client);

int api_client_connect(struct api_client *client);

struct jrpc_connection *api_client_get_connection(struct api_client *client);

void api_client_start_thread(struct api_client *client);

void api_client_push_event(struct api_client *client, json_t *event);

void api_client_pop_event(struct api_client *client, json_t **p_event);

#endif
