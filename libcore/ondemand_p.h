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

#ifndef LIBCORE_ONDEMANDP_H
#define LIBCORE_ONDEMANDP_H

#include <glib.h>

struct a6o_on_demand {
	struct a6o_scan_conf *scan_conf;

	const char *root_path;              /* root path of the scan */
	enum a6o_scan_flags flags;        /* scan flags (recursive, threaded, etc) */

	GThread *count_thread;              /* thread used to count the files to compute progress */
	GThreadPool *thread_pool;           /* the thread pool if multi-threaded */

	int to_scan_count;                  /* files to scan counter, to compute progress */
	int scanned_count;                  /* already scanned counter, to compute progress */
        int malware_count;                  /* detected as malicious counter */
	int suspicious_count;               /* detected as suspicious counter */
};

#endif
