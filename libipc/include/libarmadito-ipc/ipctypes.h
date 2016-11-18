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

IPC_DEFINE_ENUM(a6o_file_status)
	IPC_DEFINE_ENUM_VALUE(A6O_FILE_UNDECIDED)
	IPC_DEFINE_ENUM_VALUE(A6O_FILE_CLEAN)
	IPC_DEFINE_ENUM_VALUE(A6O_FILE_UNKNOWN_TYPE)
	IPC_DEFINE_ENUM_VALUE(A6O_FILE_EINVAL)
	IPC_DEFINE_ENUM_VALUE(A6O_FILE_IERROR)
	IPC_DEFINE_ENUM_VALUE(A6O_FILE_SUSPICIOUS)
	IPC_DEFINE_ENUM_VALUE(A6O_FILE_WHITE_LISTED)
	IPC_DEFINE_ENUM_VALUE(A6O_FILE_MALWARE)
IPC_END_ENUM

IPC_DEFINE_ENUM(a6o_update_status)
	IPC_DEFINE_ENUM_VALUE(A6O_UPDATE_OK)
	IPC_DEFINE_ENUM_VALUE(A6O_UPDATE_LATE)
	IPC_DEFINE_ENUM_VALUE(A6O_UPDATE_CRITICAL)
	IPC_DEFINE_ENUM_VALUE(A6O_UPDATE_NON_AVAILABLE)
IPC_END_ENUM

IPC_DEFINE_STRUCT(a6o_base_info)
	IPC_DEFINE_FIELD_STRING(name)
	IPC_DEFINE_FIELD_INT(time_t, base_update_ts)
	IPC_DEFINE_FIELD_STRING(version)
	IPC_DEFINE_FIELD_INT(size_t, signature_count)
	IPC_DEFINE_FIELD_STRING(full_path)
IPC_END_STRUCT

IPC_DEFINE_STRUCT(a6o_module_info)
	IPC_DEFINE_FIELD_STRING(name)
	IPC_DEFINE_FIELD_ENUM(a6o_update_status, mod_status)
	IPC_DEFINE_FIELD_INT(time_t, mod_update_ts)
	IPC_DEFINE_FIELD_ARRAY(a6o_base_info, base_infos)
IPC_END_STRUCT
