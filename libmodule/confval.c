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

#include "armadito-config.h"

#include <libarmadito/armadito.h>
#include <stdlib.h>
#include <string.h>

A6O_API void a6o_conf_value_init(struct a6o_conf_value *cv)
{
	a6o_conf_value_set_void(cv);
}

A6O_API void a6o_conf_value_destroy(struct a6o_conf_value *cv)
{
	const char **p;

	switch (cv->type) {
	case CONF_TYPE_VOID:
		break;
	case CONF_TYPE_INT:
		break;
	case CONF_TYPE_STRING:
		if (cv->v.str_v != NULL)
			free((void *)cv->v.str_v);
		break;
	case CONF_TYPE_LIST:
		for(p = cv->v.list_v.values; *p != NULL; p++)
			free((void *)*p);
		free((void *)cv->v.list_v.values);
		break;
	}

	cv->type = CONF_TYPE_VOID;
}

A6O_API void a6o_conf_value_set(struct a6o_conf_value *cv, const struct a6o_conf_value *src)
{
	switch (src->type) {
	case CONF_TYPE_VOID:
		a6o_conf_value_set_void(cv);
		break;
	case CONF_TYPE_INT:
		a6o_conf_value_set_int(cv, a6o_conf_value_get_int(src));
		break;
	case CONF_TYPE_STRING:
		a6o_conf_value_set_string(cv, a6o_conf_value_get_string(src));
		break;
	case CONF_TYPE_LIST:
		a6o_conf_value_set_list(cv, a6o_conf_value_get_list(src), a6o_conf_value_get_list_len(src));
		break;
	}
}

A6O_API void a6o_conf_value_set_void(struct a6o_conf_value *cv)
{
	cv->type = CONF_TYPE_VOID;
}

A6O_API void a6o_conf_value_set_int(struct a6o_conf_value *cv, unsigned int val)
{
	cv->type = CONF_TYPE_INT;
	cv->v.int_v = val;
}

A6O_API void a6o_conf_value_set_string(struct a6o_conf_value *cv, const char *val)
{
	cv->type = CONF_TYPE_STRING;
	cv->v.str_v = a6o_strdup(val);
}

A6O_API void a6o_conf_value_set_list(struct a6o_conf_value *cv, const char **val, size_t len)
{
	size_t i;

	cv->type = CONF_TYPE_LIST;
	cv->v.list_v.len = len;
	cv->v.list_v.values = malloc((len + 1)*sizeof(char *));

	for(i = 0; i < len; i++)
		cv->v.list_v.values[i] = a6o_strdup(val[i]);

	cv->v.list_v.values[len] = NULL;
}

