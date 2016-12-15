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

#include <libjrpc/marshall.h>

#include <jansson.h>
#include <stdint.h>
#include <string.h>

/*
 *
 * Marshalling functions for array types
 *
 */
int jrpc_marshall_array(void **array, json_t **p_obj, jrpc_marshall_cb_t marshall_elem_cb)
{
	json_t *obj;
	void **p;
	int ret = JRPC_OK;

	obj = json_array();

	for(p = array; *p != NULL; p++) {
		json_t *elem;

		if ((ret = (*marshall_elem_cb)(*p, &elem)))
			goto error_end;
		json_array_append_new(obj, elem);
	}

	*p_obj = obj;
	return ret;

error_end:
	json_decref(obj);
	*p_obj = NULL;
	return ret;
}

/*
 *
 * Unmarshalling helper for fields
 *
 */
int jrpc_unmarshall_field(json_t *obj, const char *name, json_type expected_json_type, json_t **p_field)
{
	*p_field = json_object_get(obj, name);

	if (*p_field == NULL)
		return JRPC_ERR_MARSHALL_FIELD_NOT_FOUND;

	if (json_typeof(*p_field) != expected_json_type) {
		*p_field = NULL;
		return JRPC_ERR_MARSHALL_TYPE_MISMATCH;
	}

	return JRPC_OK;
}

/*
 *
 * Unmarshalling functions for array types
 *
 */
int jrpc_unmarshall_array(json_t *obj, void ***p_array, jrpc_unmarshall_cb_t unmarshall_cb)
{
	size_t index, size;
	json_t *elem;
	void **array, **p;
	int ret = JRPC_OK;

	size = json_array_size(obj);

	array = calloc(size + 1, sizeof(void *));

	p = array;
	json_array_foreach(obj, index, elem) {
		if ((ret = (*unmarshall_cb)(elem, p)))
			goto error_end;
		p++;
	}

	*p_array = array;
	return ret;

error_end:
	free(array);
	*p_array = NULL;
	return ret;
}

