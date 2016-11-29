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

#ifndef LIBARMADITO_RPC_ARMADITO_RPC_H
#define LIBARMADITO_RPC_ARMADITO_RPC_H

#include <stddef.h>

#include <libarmadito/armadito.h>

/*
 * structure specific serialization functions
 */
#define RPC_DEFINE_STRUCT(S) int a6o_rpc_serialize_struct_##S(void *p, char **p_buffer, size_t *p_size);

#include <libarmadito-rpc/defs.h>

/*
 * functions
 */

#define a6o_rpc_serialize(STRUCT_TYPE, P, P_BUFFER, P_SIZE) a6o_rpc_serialize_struct_##STRUCT_TYPE(P, P_BUFFER, P_SIZE);

int a6o_rpc_deserialize(char *buffer, size_t size, void **p);

#endif
