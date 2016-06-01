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

#include <libarmadito.h>
#include "onaccess_windows.h"
#include <armaditop.h>
#include <stdlib.h>
#include <string.h>
#include "os/string.h"


static enum a6o_mod_status mod_onaccess_init(struct a6o_module *module)
{
	struct onaccess_data *fa_data = (struct onaccess_data *)calloc(1,sizeof(struct onaccess_data));

	module->data = fa_data;

	fa_data->enable_permission = 0;
	fa_data->state_flag = 0;

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status mod_onaccess_conf_set_enable_on_access(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct onaccess_data *fa_data = (struct onaccess_data *)module->data;

	fa_data->enable_permission = a6o_conf_value_get_int(value);

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status mod_onaccess_conf_modules(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct a6o_scan_conf *on_access_conf = a6o_scan_conf_on_access();

	if (a6o_conf_value_is_string(value))
		a6o_scan_conf_add_module(on_access_conf, a6o_conf_value_get_string(value), module->armadito);
	else {
		const char **p;

		for (p = a6o_conf_value_get_list(value); *p != NULL; p++)
			a6o_scan_conf_add_module(on_access_conf, *p, module->armadito);
	}

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status mod_onaccess_conf_max_size(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct a6o_scan_conf *on_access_conf = a6o_scan_conf_on_access();

	a6o_scan_conf_max_file_size(on_access_conf, a6o_conf_value_get_int(value));

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status mod_onaccess_conf_mime_types(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct a6o_scan_conf *on_access_conf = a6o_scan_conf_on_access();

	if (a6o_conf_value_is_string(value))
		a6o_scan_conf_add_mime_type(on_access_conf, a6o_conf_value_get_string(value));
	else {
		const char **p;

		for (p = a6o_conf_value_get_list(value); *p != NULL; p++)
			a6o_scan_conf_add_mime_type(on_access_conf, *p);
	}

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status mod_onaccess_close(struct a6o_module *module)
{
	struct onaccess_data *fa_data = (struct onaccess_data *)module->data;

	if (fa_data != NULL) {
		free(fa_data);
		fa_data = NULL;
	}

	return ARMADITO_MOD_OK;
}

static enum a6o_update_status mod_onaccess_info(struct a6o_module *module, struct a6o_module_info *info){
  
	time_t ts = 0;
	struct tm timeptr = {0, 30, 8, 1, 2, 116}; // 01/03/2016 9:30
	struct onaccess_data * data = NULL;

	info->update_date = os_strdup("2016-01-26T09:30:00Z");	
	ts=mktime(&timeptr);
	info->timestamp = ts;

	data = module->data;
	if (data->state_flag == 0) {
		return ARMADITO_UPDATE_NON_AVAILABLE;
	}

	return ARMADITO_UPDATE_OK;
}

struct a6o_conf_entry mod_onaccess_conf_table[] = {
	{ "enable", CONF_TYPE_INT, mod_onaccess_conf_set_enable_on_access},
	{ "modules", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_onaccess_conf_modules},
	{ "mime-types", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_onaccess_conf_mime_types},
	{ "max-size", CONF_TYPE_INT, mod_onaccess_conf_max_size},
	{ NULL, 0, NULL},
};

struct a6o_module on_access_win_module = {
	.init_fun = mod_onaccess_init,
	.conf_table = mod_onaccess_conf_table,
	.post_init_fun = NULL,
	.scan_fun = NULL,
	.close_fun = mod_onaccess_close,
	.info_fun = mod_onaccess_info,
	.supported_mime_types = NULL,
	.name = "on-access",
	.size = sizeof(struct onaccess_data),
};