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

#include <libarmadito/armadito.h>

#include "test.h"

A6O_RPC_DEFINE_ENUM(a6o_file_status)
	A6O_RPC_DEFINE_ENUM_VALUE(A6O_FILE_UNDECIDED)
	A6O_RPC_DEFINE_ENUM_VALUE(A6O_FILE_CLEAN)
	A6O_RPC_DEFINE_ENUM_VALUE(A6O_FILE_UNKNOWN_TYPE)
	A6O_RPC_DEFINE_ENUM_VALUE(A6O_FILE_EINVAL)
	A6O_RPC_DEFINE_ENUM_VALUE(A6O_FILE_IERROR)
	A6O_RPC_DEFINE_ENUM_VALUE(A6O_FILE_SUSPICIOUS)
	A6O_RPC_DEFINE_ENUM_VALUE(A6O_FILE_WHITE_LISTED)
	A6O_RPC_DEFINE_ENUM_VALUE(A6O_FILE_MALWARE)
A6O_RPC_END_ENUM

A6O_RPC_DEFINE_ENUM(a6o_update_status)
	A6O_RPC_DEFINE_ENUM_VALUE(A6O_UPDATE_OK)
	A6O_RPC_DEFINE_ENUM_VALUE(A6O_UPDATE_LATE)
	A6O_RPC_DEFINE_ENUM_VALUE(A6O_UPDATE_CRITICAL)
	A6O_RPC_DEFINE_ENUM_VALUE(A6O_UPDATE_NON_AVAILABLE)
A6O_RPC_END_ENUM

A6O_RPC_DEFINE_STRUCT(a6o_base_info)
	A6O_RPC_DEFINE_FIELD_STRING(name)
	A6O_RPC_DEFINE_FIELD_INT(time_t, base_update_ts)
	A6O_RPC_DEFINE_FIELD_STRING(version)
	A6O_RPC_DEFINE_FIELD_INT(size_t, signature_count)
	A6O_RPC_DEFINE_FIELD_STRING(full_path)
A6O_RPC_END_STRUCT

A6O_RPC_DEFINE_STRUCT(a6o_module_info)
	A6O_RPC_DEFINE_FIELD_STRING(name)
	A6O_RPC_DEFINE_FIELD_ENUM(a6o_update_status, mod_status)
	A6O_RPC_DEFINE_FIELD_INT(time_t, mod_update_ts)
	A6O_RPC_DEFINE_FIELD_ARRAY(a6o_base_info, base_infos)
A6O_RPC_END_STRUCT

/* for testing */
A6O_RPC_DEFINE_STRUCT(operands)
	A6O_RPC_DEFINE_FIELD_INT(int, op1)
	A6O_RPC_DEFINE_FIELD_INT(int, op2)
A6O_RPC_END_STRUCT

A6O_RPC_DEFINE_STRUCT(result)
	A6O_RPC_DEFINE_FIELD_INT(int, res)
A6O_RPC_END_STRUCT

