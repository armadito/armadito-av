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

#ifndef _LIBARMADITO_ONDEMANDP_H_
#define _LIBARMADITO_ONDEMANDP_H_

#include <glib.h>

#include <core/scan.h>

struct a6o_on_demand {
	struct a6o_scan_conf *scan_conf;
	struct a6o_scan *scan;
	GThread *count_thread;              /* thread used to count the files to compute progress */
	const char *root_path;              /* root path of the scan */
	enum a6o_scan_flags flags;        /* scan flags (recursive, threaded, etc) */
	GThreadPool *thread_pool;           /* the thread pool if multi-threaded */
};

#endif
