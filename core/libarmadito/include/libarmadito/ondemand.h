/***

Copyright (C) 2015, 2016, 2017 Teclib'

This file is part of Armadito.

Armadito is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef _LIBARMADITO_ONDEMAND_H_
#define _LIBARMADITO_ONDEMAND_H_

#include <libarmadito/scan.h>

struct a6o_on_demand;

struct a6o_on_demand *a6o_on_demand_new(struct armadito *armadito, int scan_id, const char *root_path, enum a6o_scan_flags flags);

struct a6o_scan *a6o_on_demand_get_scan(struct a6o_on_demand *on_demand);

void a6o_on_demand_cancel(struct a6o_on_demand *on_demand);

void a6o_on_demand_run(struct a6o_on_demand *on_demand);

void a6o_on_demand_free(struct a6o_on_demand *on_demand);

#endif
