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

#include "mapper.h"
#include "hash.h"

struct a6o_rpc_mapper {
	struct hash_table *method_table;
};

struct a6o_rpc_mapper *a6o_rpc_mapper_new(void)
{
	return NULL;
}

int a6o_rpc_mapper_add(struct a6o_rpc_mapper *m, const char *method, const char *param_type, const char *return_type, a6o_rpc_method_cb_t method_cb)
{
	return 0;
}
