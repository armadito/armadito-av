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

#ifndef TOOLS_APICLIENT_H_
#define TOOLS_APICLIENT_H_

#include <json.h>

struct api_client;

struct api_client *api_client_new(unsigned short port);

int api_client_call(struct api_client *client, const char *path, struct json_object *in, struct json_object **out);

int api_client_register(struct api_client *client);

int api_client_unregister(struct api_client *client);

void api_client_free(struct api_client *client);

#endif
