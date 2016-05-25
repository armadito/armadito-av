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

#ifndef _JSONHANDLER_H_
#define _JSONHANDLER_H_

#include <libarmadito.h>

#include <json.h>

enum a6o_json_status {
	JSON_OK = 0,
	JSON_PARSE_ERROR = 100,
	JSON_MALFORMED_REQUEST = 101,
	JSON_INVALID_REQUEST = 102,
	JSON_REQUEST_FAILED = 200,
	JSON_UNEXPECTED_ERR = 300,
};

struct a6o_json_handler;

struct a6o_json_handler *a6o_json_handler_new(struct armadito *armadito);

void a6o_json_handler_free(struct a6o_json_handler *jh);

enum a6o_json_status a6o_json_handler_get_response(struct a6o_json_handler *jh, const char *req, int req_len, char **p_resp, int *p_resp_len);

void a6o_json_handler_process(struct a6o_json_handler *j);

static const char *status_2_error(enum a6o_json_status status);

#endif

