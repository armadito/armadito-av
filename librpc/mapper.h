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

#ifndef LIBRPC_MAPPER_H
#define LIBRPC_MAPPER_H

#include <libarmadito-rpc/armadito-rpc.h>

#include "marshallfuns.h"
#include "unmarshallfuns.h"

struct marshall_def {
	rpc_marshall_cb_t marshall_cb;
	rpc_unmarshall_cb_t unmarshall_cb;
};

struct method_def {
	a6o_rpc_method_t method_fun;
	struct marshall_def params;
	struct marshall_def result;
};

int a6o_rpc_mapper_add_struct(struct a6o_rpc_mapper *m, const char *struct_name, rpc_marshall_cb_t marshall_cb, rpc_unmarshall_cb_t unmarshall_cb);

#endif
