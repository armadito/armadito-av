/***

Copyright (C) 2015, 2016, 2017 Teclib'

This file is part of Armadito.

Armadito is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef _JSONHANDLER_P_H_
#define _JSONHANDLER_P_H_

#include "jsonhandler.h"

#include <json.h>
#include <stdlib.h>

struct json_request {
	const char *request;
	int id;
	struct json_object *params;
};

struct json_response {
	const char *response;
	int id;
	enum a6o_json_status status;
	struct json_object *info;
	const char *error_message;
};

typedef enum a6o_json_status (*response_cb_t)(struct armadito *armadito, struct json_request *req, struct json_response *resp, void **request_data);

typedef void (*process_cb_t)(struct armadito *armadito, void *request_data);

#endif
