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

#ifndef _LIBARMADITO_SCANCONF_H_
#define _LIBARMADITO_SCANCONF_H_

struct a6o_scan_conf;

struct a6o_scan_conf *a6o_scan_conf_on_demand(void);

struct a6o_scan_conf *a6o_scan_conf_on_access(void);

void a6o_scan_conf_white_list_directory(struct a6o_scan_conf *c, const char *path);

int a6o_scan_conf_is_white_listed(struct a6o_scan_conf *c, const char *path);

void a6o_scan_conf_add_mime_type(struct a6o_scan_conf *c, const char *mime_type);

void a6o_scan_conf_add_module(struct a6o_scan_conf *c, const char *module_name, struct armadito *u);

struct a6o_module **a6o_scan_conf_get_applicable_modules(struct a6o_scan_conf *c, const char *mime_type);

void a6o_scan_conf_max_file_size(struct a6o_scan_conf *c, int max_file_size);

void a6o_scan_conf_free(struct a6o_scan_conf *scan_conf);

#endif
