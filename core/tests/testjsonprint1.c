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

#include "debug.h"

#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFSIZE (64*1024)

static struct json_object *json_read(int fd)
{
	struct json_tokener *tokener = json_tokener_new();
	char buff[BUFFSIZE];
	size_t len;
	struct json_object *obj;

	len = read(fd, buff, BUFFSIZE);

	if (!len) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	json_tokener_reset(tokener);

	obj = json_tokener_parse_ex(tokener, buff, len);

	if (obj == NULL) {
		fprintf(stderr, "error parsing JSON object (%s)\n", json_tokener_error_desc(json_tokener_get_error(tokener)));
		exit(EXIT_FAILURE);
	}

	return obj;
}

int main(int argc, char **argv)
{
	struct json_object *obj;

	obj = json_read(STDIN_FILENO);
	jobj_debug(obj, "object");

	return 0;
}
