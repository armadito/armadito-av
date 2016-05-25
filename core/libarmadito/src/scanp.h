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

#ifndef _LIBARMADITO_SCANP_H_
#define _LIBARMADITO_SCANP_H_

#include <glib.h>

struct a6o_scan {
	GArray *callbacks;                  /* array of struct callback_entry */

	int scan_id;                        /* scan id for GUI */

	int to_scan_count;                  /* files to scan counter, to compute progress */
	int scanned_count;                  /* already scanned counter, to compute progress */
        int malware_count;                /* detected as malicious counter */
	int suspicious_count;               /* detected as suspicious counter */
};

void a6o_scan_call_callbacks(struct a6o_scan *scan, struct a6o_report *report);

#endif
