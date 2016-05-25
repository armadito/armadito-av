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

#ifndef _LIBARMADITO_REPORTP_H_
#define _LIBARMADITO_REPORTP_H_

#include <libarmadito.h>

void a6o_report_init(struct a6o_report *report, int scan_id, const char *path, int progress);

void a6o_report_destroy(struct a6o_report *report);

void a6o_report_change(struct a6o_report *report, enum a6o_file_status status, const char *mod_name, const char *mod_report);

#endif
