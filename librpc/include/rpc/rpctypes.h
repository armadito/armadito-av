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

#include "core/action.h"
#include "core/event.h"
#include "core/info.h"

struct a6o_rpc_scan_param {
	const char *root_path;
	int send_progress;
};

#define MARSHALL_DECLARATIONS
#include "rpc/rpcdefs.h"

#define UNMARSHALL_DECLARATIONS
#include "rpc/rpcdefs.h"
