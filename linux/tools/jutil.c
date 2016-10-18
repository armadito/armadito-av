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

#include <json.h>

#include "jutil.h"

const char *j_get_string(struct json_object *j_obj, const char *key)
{
	struct json_object *j_str = NULL;

	if (!json_object_object_get_ex(j_obj, key, &j_str)
		|| !json_object_is_type(j_str, json_type_string))
		return NULL;

	return json_object_get_string(j_str);
}

int j_get_int(struct json_object *j_obj, const char *key)
{
	struct json_object *j_int = NULL;

	if (!json_object_object_get_ex(j_obj, key, &j_int)
		|| !json_object_is_type(j_int, json_type_int))
		return -1;

	return json_object_get_int(j_int);
}

int64_t j_get_int64(struct json_object *j_obj, const char *key)
{
	struct json_object *j_int = NULL;

	if (!json_object_object_get_ex(j_obj, key, &j_int)
		|| !json_object_is_type(j_int, json_type_int))
		return -1;

	return json_object_get_int64(j_int);
}
