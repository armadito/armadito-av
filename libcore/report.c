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
#include "armadito-config.h"

#include "status_p.h"
#include "string_p.h"
#include "core/report.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void a6o_report_init(struct a6o_report *report, const char *path)
{
	if (path != NULL)
		report->path = os_strdup(path);
	else
		report->path = NULL;
	report->status = A6O_FILE_UNDECIDED;
	report->action = A6O_ACTION_NONE;
	report->module_name = NULL;
	report->module_report = NULL;
}

void a6o_report_destroy(struct a6o_report *report)
{
	if (report->path != NULL)
		free(report->path);
	if (report->module_report != NULL)
		free(report->module_report);
}

void a6o_report_change(struct a6o_report *report, enum a6o_file_status status, const char *module_name, const char *module_report)
{
	report->status = status;

	if (module_name != NULL)
		report->module_name = (char *)module_name;

	if (module_report != NULL) {
		if (report->module_report != NULL)
			free(report->module_report);

		report->module_report = (char *)module_report;
	}
}
