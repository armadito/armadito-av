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

#include "mapper.h"
#include "hash.h"

struct a6o_rpc_mapper {
	struct hash_table *method_table;
	struct hash_table *marshall_table;
};

struct a6o_rpc_mapper *a6o_rpc_mapper_new(void)
{
	struct a6o_rpc_mapper *m = malloc(sizeof(struct a6o_rpc_mapper));

	m->method_table = hash_table_new(HASH_KEY_STR, (free_cb_t)free, (free_cb_t)free);
	m->marshall_table = hash_table_new(HASH_KEY_STR, (free_cb_t)free, (free_cb_t)free);

	/* insert all struct mappings */
#define A6O_RPC_DEFINE_STRUCT(S) \
	a6o_rpc_mapper_add_struct(m, \
				#S, \
				a6o_rpc_marshall_struct_##S, \
				a6o_rpc_unmarshall_struct_##S);

#include <libarmadito-rpc/defs.h>

	return m;
}

int a6o_rpc_mapper_add_struct(struct a6o_rpc_mapper *m, const char *struct_name, rpc_marshall_cb_t marshall_cb, rpc_unmarshall_cb_t unmarshall_cb)
{
	struct marshall_def *md = malloc(sizeof(struct marshall_def));

	md->marshall_cb = marshall_cb;
	md->unmarshall_cb = unmarshall_cb;

	return hash_table_insert(m->marshall_table, (void *)struct_name, md);
}

static int find_marshall(struct a6o_rpc_mapper *m, const char *type_name, rpc_marshall_cb_t *p_marshall, rpc_unmarshall_cb_t *p_unmarshall)
{
	struct marshall_def *md;

	md = hash_table_search(m->marshall_table, (void *)type_name);
	if (md == NULL)
		return -1; /* TODO : define proper error code */

	*p_marshall = md->marshall_cb;
	*p_unmarshall = md->unmarshall_cb;

	return 0;
}

int a6o_rpc_mapper_add_method(struct a6o_rpc_mapper *m, const char *method, const char *param_type, const char *return_type, a6o_rpc_method_t method_fun)
{
	struct method_def *mth_def;
	int ret;

	mth_def = calloc(1, sizeof(struct method_def));
	mth_def->method_fun = method_fun;

	if (param_type != NULL) {
		ret = find_marshall( m, param_type, &mth_def->params.marshall_cb, &mth_def->params.unmarshall_cb);

		if (!ret)
			goto err_exit;
	}

	if (return_type != NULL) {
		ret = find_marshall( m, return_type, &mth_def->result.marshall_cb, &mth_def->result.unmarshall_cb);

		if (!ret)
			goto err_exit;
	}

	return 0;

err_exit:
	free(mth_def);
	return ret;
}
