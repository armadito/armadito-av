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

#ifndef _LIBARMADITO_CONFPARSER_H_
#define _LIBARMADITO_CONFPARSER_H_

#include <stddef.h>

struct a6o_conf_parser;

enum conf_parser_value_type {
	CP_VALUE_NONE    = 0,
	CP_VALUE_INT     = 1,
	CP_VALUE_STRING  = 2,
	CP_VALUE_LIST    = 3,
};

typedef int (*conf_parser_callback_t)(const char *section, const char *key, struct a6o_conf_value *value, void *user_data);

struct a6o_conf_parser *a6o_conf_parser_new(const char *filename, conf_parser_callback_t callback, void *user_data);

int a6o_conf_parser_parse(struct a6o_conf_parser *cp);

void a6o_conf_parser_free(struct a6o_conf_parser *cp);

#endif

