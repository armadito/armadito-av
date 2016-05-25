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

#ifndef _LIBARMADITO_MODULE_H_
#define _LIBARMADITO_MODULE_H_

#include <libarmadito/status.h>
#include <libarmadito/conf.h>
#include <libarmadito/info.h>

struct armadito;
struct a6o_module;

enum a6o_mod_status {
	ARMADITO_MOD_OK,
	ARMADITO_MOD_INIT_ERROR,
	ARMADITO_MOD_CONF_ERROR,
	ARMADITO_MOD_CLOSE_ERROR,
};

struct a6o_conf_entry {
	const char *key;
	enum a6o_conf_value_type type;
	enum a6o_mod_status (*conf_fun)(struct a6o_module *module, const char *key, struct a6o_conf_value *value);
};

struct a6o_module {
	enum a6o_mod_status (*init_fun)(struct a6o_module *module);

	struct a6o_conf_entry *conf_table;

	enum a6o_mod_status (*post_init_fun)(struct a6o_module *module);

	enum a6o_file_status (*scan_fun)(struct a6o_module *module, int fd, const char *path, const char *mime_type, char **pmod_report);

	enum a6o_mod_status (*close_fun)(struct a6o_module *module);

	enum a6o_update_status (*info_fun)(struct a6o_module *module, struct a6o_module_info *info);

	const char **supported_mime_types;

	const char *name;

	size_t size;

	enum a6o_mod_status status;

	void *data;

	struct armadito *armadito;
};

#endif
