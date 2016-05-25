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

#ifndef __ARMADITO_CONF_H__
#define __ARMADITO_CONF_H__

#include <libarmadito.h>
#include <Windows.h>

struct conf_reg_data{
	HKEY hRootKey;
	HKEY hSectionKey;
	char * path;
	char * prev_section;
};

void display_entry(const char *section, const char *key, struct a6o_conf_value *value, void *user_data);
int save_conf_in_registry(struct a6o_conf * conf);
int restore_conf_from_registry(struct a6o_conf * conf);

int conf_poc_windows( );
int disable_onaccess( );

#endif
