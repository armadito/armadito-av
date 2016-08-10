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

#ifndef _LIBARMADITO_INFO_H_
#define _LIBARMADITO_INFO_H_

#include <libarmadito/status.h>
#include <libarmadito/handle.h>
#include <time.h>

enum a6o_update_status {
	ARMADITO_UPDATE_OK,
	ARMADITO_UPDATE_LATE,
	ARMADITO_UPDATE_CRITICAL,
	ARMADITO_UPDATE_NON_AVAILABLE,
};

const char *a6o_update_status_str(enum a6o_update_status status);

struct a6o_base_info {
	const char *name;
	time_t base_update_ts;
	const char *version;
	unsigned int signature_count;
	const char *full_path;
};

struct a6o_module_info {
	const char *name;
	enum a6o_update_status mod_status;
	time_t mod_update_ts;
	/* NULL terminated array of pointers to struct base_info */
	struct a6o_base_info **base_infos;
};

struct a6o_info {
	enum a6o_update_status global_status;
	time_t global_update_ts;
	/* NULL terminated array of pointers to struct a6o_module_info */
	struct a6o_module_info **module_infos;
};

struct a6o_info *a6o_info_new(struct armadito *armadito);

void a6o_info_to_stdout(struct a6o_info *info);

void a6o_info_free(struct a6o_info *info);

#endif
