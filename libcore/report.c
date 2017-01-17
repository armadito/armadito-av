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

void a6o_report_init(struct a6o_report *report, int scan_id, const char *path, int progress)
{
	report->scan_id = scan_id;
	if (path != NULL)
		report->path = os_strdup(path);
	else
		report->path = NULL;
	report->progress = progress;
	report->status = A6O_FILE_UNDECIDED;
	report->action = A6O_ACTION_NONE;
	report->mod_name = NULL;
	report->mod_report = NULL;
	report->suspicious_count = 0;
        report->malware_count = 0;
	report->scanned_count = 0;
}

void a6o_report_destroy(struct a6o_report *report)
{
	if (report->path != NULL)
		free(report->path);
	if (report->mod_report != NULL)
		free(report->mod_report);
}

void a6o_report_change(struct a6o_report *report, enum a6o_file_status status, const char *mod_name, const char *mod_report)
{
	report->status = status;

	if (mod_name != NULL)
		report->mod_name = (char *)mod_name;

	if (mod_report != NULL) {
		if (report->mod_report != NULL)
			free(report->mod_report);

		report->mod_report = (char *)mod_report;
	}
}
