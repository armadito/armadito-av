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

#ifndef DAEMON_API_H_
#define DAEMON_API_H_

#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <microhttpd.h>
#include <json.h>

#include "apihandler.h"

int register_api_cb(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out, void *user_data);
int unregister_api_cb(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out, void *user_data);
int ping_api_cb(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out, void *user_data);
int scan_api_cb(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out, void *user_data);
int poll_api_cb(struct api_handler *a, struct MHD_Connection *connection, struct json_object *in, struct json_object **out, void *user_data);

#endif
