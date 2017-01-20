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

#include "core/scan.h"

#include "status_p.h"
#include "scan_p.h"
#include "string_p.h"
#include "core/report.h"
#include "core/event.h"
#include "core/io.h"
#include "core/file.h"
#include "core/filectx.h"
#include "core/mimetype.h"

#include <errno.h>
#include <glib.h>
#include <stdlib.h>

struct a6o_scan {
	struct a6o_event_source *event_source;

	int scan_id;                        /* scan id for GUI */
};

struct a6o_scan *a6o_scan_new(struct armadito *armadito, int scan_id)
{
	struct a6o_scan *scan = (struct a6o_scan *)malloc(sizeof(struct a6o_scan));

	scan->event_source = a6o_event_source_new();

	scan->scan_id = scan_id;

	scan->to_scan_count = 0;
	scan->scanned_count = 0;
        scan->malware_count = 0;
        scan->suspicious_count = 0;

	return scan;
}

static void scan_progress(struct a6o_scan *scan, struct a6o_report *report)
{
	int progress;

	/* update the progress */
	/* may be not thread safe, but who cares about precise values? */
	scan->scanned_count++;

	if (scan->to_scan_count == 0) {
		report->progress = REPORT_PROGRESS_UNKNOWN;
		report->scanned_count = scan->scanned_count;
		return;
	}

	progress = (int)((100.0 * scan->scanned_count) / scan->to_scan_count);

        // a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "Progress = %d = ( 100 * %d / %d ) ", progress, scan->scanned_count, scan->to_scan_count );

	if (progress > 100)
		progress = 100;

	report->progress = progress;
	report->scanned_count = scan->scanned_count;
}

static void update_counters(struct a6o_scan *scan, struct a6o_report *report, enum a6o_file_status status )
{
       switch(status) {
	case A6O_FILE_UNDECIDED: break;
	case A6O_FILE_CLEAN: break;
	case A6O_FILE_UNKNOWN_TYPE: break;
	case A6O_FILE_EINVAL: break;
	case A6O_FILE_IERROR: break;
	case A6O_FILE_SUSPICIOUS:
		scan->suspicious_count++;
		break;
	case A6O_FILE_WHITE_LISTED: break;
	case A6O_FILE_MALWARE:
		scan->malware_count++;
		break;
      }

      report->suspicious_count = scan->suspicious_count;
      report->malware_count = scan->malware_count;
}

void a6o_scan_free(struct a6o_scan *scan)
{
	a6o_event_source_free(scan->event_source);

	free(scan);
}


