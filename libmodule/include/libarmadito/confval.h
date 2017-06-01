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

/**
 * \file confval.h
 *
 * \brief definition of configuration values
 *
 */

#ifndef __LIBARMADITO_CONFVAL_H_
#define __LIBARMADITO_CONFVAL_H_

/* for size_t */
#include <stdlib.h>

enum a6o_conf_value_type {
	CONF_TYPE_VOID     = 0,
	CONF_TYPE_INT      = 1 << 0,
	CONF_TYPE_STRING   = 1 << 1,
	CONF_TYPE_LIST     = 1 << 2,
};

struct a6o_conf_value {
	enum a6o_conf_value_type type;
	union {
		int int_v;
		const char *str_v;
		struct {
			size_t len;
			const char **values;
		} list_v;
	} v;
};

A6O_API void a6o_conf_value_init(struct a6o_conf_value *cv);

A6O_API void a6o_conf_value_destroy(struct a6o_conf_value *cv);

#define a6o_conf_value_get_type(cv) ((cv)->type)

#define a6o_conf_value_is_int(cv) (a6o_conf_value_get_type(cv) == CONF_TYPE_INT)
#define a6o_conf_value_is_string(cv) (a6o_conf_value_get_type(cv) == CONF_TYPE_STRING)
#define a6o_conf_value_is_list(cv) (a6o_conf_value_get_type(cv) == CONF_TYPE_LIST)

#define a6o_conf_value_get_int(cv) ((cv)->v.int_v)
#define a6o_conf_value_get_string(cv) ((cv)->v.str_v)
#define a6o_conf_value_get_list(cv) ((cv)->v.list_v.values)
#define a6o_conf_value_get_list_len(cv) ((cv)->v.list_v.len)

A6O_API void a6o_conf_value_set(struct a6o_conf_value *cv, const struct a6o_conf_value *src);

A6O_API void a6o_conf_value_set_void(struct a6o_conf_value *cv);

A6O_API void a6o_conf_value_set_int(struct a6o_conf_value *cv, unsigned int val);

A6O_API void a6o_conf_value_set_string(struct a6o_conf_value *cv, const char *val);

A6O_API void a6o_conf_value_set_list(struct a6o_conf_value *cv, const char **val, size_t len);

#endif
