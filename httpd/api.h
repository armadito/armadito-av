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

#ifndef HTTPD_API_H
#define HTTPD_API_H

#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <microhttpd.h>
#include <jansson.h>

#include "apihandler.h"

int register_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data);

int unregister_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data);

int ping_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data);

int event_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data);

int scan_check_cb(struct MHD_Connection *connection, json_t *in);
int scan_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data);

int status_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data);

int browse_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data);

int version_process_cb(struct api_handler *a, struct MHD_Connection *connection, json_t *in, json_t **out, void *user_data);

#endif
