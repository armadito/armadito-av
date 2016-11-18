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

#ifndef IPC_DEFINE_STRUCT
#define IPC_DEFINE_STRUCT(S)
#endif
#ifndef IPC_DEFINE_FIELD_INT
#define IPC_DEFINE_FIELD_INT(INT_TYPE, NAME)
#endif
#ifndef IPC_DEFINE_FIELD_STRING
#define IPC_DEFINE_FIELD_STRING(NAME)
#endif
#ifndef IPC_DEFINE_FIELD_ENUM
#define IPC_DEFINE_FIELD_ENUM(ENUM_TYPE, NAME)
#endif
#ifndef IPC_DEFINE_FIELD_ARRAY
#define IPC_DEFINE_FIELD_ARRAY(ELEM_TYPE, NAME)
#endif
#ifndef IPC_END_STRUCT
#define IPC_END_STRUCT
#endif
#ifndef IPC_DEFINE_ENUM
#define IPC_DEFINE_ENUM(E)
#endif
#ifndef IPC_DEFINE_ENUM_VALUE
#define IPC_DEFINE_ENUM_VALUE(NAME)
#endif
#ifndef IPC_END_ENUM
#define IPC_END_ENUM
#endif

#include <libarmadito-ipc/ipctypes.h>

#undef IPC_DEFINE_STRUCT
#undef IPC_DEFINE_FIELD_INT
#undef IPC_DEFINE_FIELD_STRING
#undef IPC_DEFINE_FIELD_ENUM
#undef IPC_DEFINE_FIELD_ARRAY
#undef IPC_END_STRUCT
#undef IPC_DEFINE_ENUM
#undef IPC_DEFINE_ENUM_VALUE
#undef IPC_END_ENUM
