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

#ifndef ARMADITO_CORE_ONDEMAND_H
#define ARMADITO_CORE_ONDEMAND_H

enum a6o_scan_flags {
	A6O_SCAN_THREADED   = 1 << 0,
	A6O_SCAN_RECURSE    = 1 << 1,
	A6O_SCAN_STANDARD   = A6O_SCAN_THREADED | A6O_SCAN_RECURSE,
};

#define A6O_ON_DEMAND_PROGRESS_UNKNOWN (-1)

struct a6o_on_demand;

struct a6o_on_demand *a6o_on_demand_new(struct armadito *armadito, const char *root_path, enum a6o_scan_flags flags, int progress_period);

struct a6o_event_source *a6o_on_demand_get_event_source(struct a6o_on_demand *on_demand);

void a6o_on_demand_cancel(struct a6o_on_demand *on_demand);

void a6o_on_demand_run(struct a6o_on_demand *on_demand);

void a6o_on_demand_free(struct a6o_on_demand *on_demand);

#endif
