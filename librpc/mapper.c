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

#include <libarmadito-rpc/armadito-rpc.h>

#include "hash.h"

#include <string.h>

struct a6o_rpc_mapper {
	struct hash_table *method_table;
};

struct a6o_rpc_mapper *a6o_rpc_mapper_new(void)
{
	struct a6o_rpc_mapper *mapper = malloc(sizeof(struct a6o_rpc_mapper));

	mapper->method_table = hash_table_new(HASH_KEY_STR, (free_cb_t)free, NULL);

	return mapper;
}

int a6o_rpc_mapper_add(struct a6o_rpc_mapper *mapper, const char *method, a6o_rpc_method_t method_fun)
{
	char *p = strdup(method);

	if (!hash_table_insert(mapper->method_table, p, method_fun)) {
		free(p);
		return 1; /* must add error code */
	}

	return 0;
}

a6o_rpc_method_t a6o_rpc_mapper_find(struct a6o_rpc_mapper *mapper, const char *method)
{
	return hash_table_search(mapper->method_table, (void *)method);
}