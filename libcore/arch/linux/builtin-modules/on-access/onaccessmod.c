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

#include "core/scanconf.h"

#include "monitor.h"
#include "onaccessmod.h"
#include "modname.h"

#include <glib.h>
#include <stdlib.h>
#include <string.h>

struct mod_oal_data {
	struct access_monitor *monitor;
};

static enum a6o_mod_status mod_oal_init(struct a6o_module *module)
{
	struct mod_oal_data *data = malloc(sizeof(struct mod_oal_data));

	module->data = data;

	data->monitor = access_monitor_new(module->armadito);

	/* if access monitor is NULL, for instance because this process does not */
	/* have priviledge (i.e. not running as root), we don't return an error because this will make */
	/* the scan daemon terminates */
	/* if (fa_data->monitor == NULL) */
	/*   return A6O_MOD_INIT_ERROR; */

	return A6O_MOD_OK;
}

static enum a6o_mod_status mod_oal_conf_enable(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct mod_oal_data *data = (struct mod_oal_data *)module->data;

	access_monitor_enable(data->monitor, a6o_conf_value_get_int(value));

	return A6O_MOD_OK;
}

static enum a6o_mod_status mod_oal_conf_enable_permission(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct mod_oal_data *data = (struct mod_oal_data *)module->data;

	access_monitor_enable_permission(data->monitor, a6o_conf_value_get_int(value));

	return A6O_MOD_OK;
}

static enum a6o_mod_status mod_oal_conf_enable_removable_media(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct mod_oal_data *data = (struct mod_oal_data *)module->data;

	access_monitor_enable_removable_media(data->monitor, a6o_conf_value_get_int(value));

	return A6O_MOD_OK;
}

static enum a6o_mod_status mod_oal_conf_autoscan_removable_media(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct mod_oal_data *data = (struct mod_oal_data *)module->data;

	access_monitor_autoscan_removable_media(data->monitor, a6o_conf_value_get_int(value));

	return A6O_MOD_OK;
}

static enum a6o_mod_status mod_oal_conf_mount(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct mod_oal_data *data = (struct mod_oal_data *)module->data;

	if (a6o_conf_value_is_string(value))
		access_monitor_add_mount(data->monitor, a6o_conf_value_get_string(value));
	else {
		const char **p;

		for (p = a6o_conf_value_get_list(value); *p != NULL; p++)
			access_monitor_add_mount(data->monitor, *p);
	}

	return A6O_MOD_OK;
}

static enum a6o_mod_status mod_oal_conf_directory(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct mod_oal_data *data = (struct mod_oal_data *)module->data;

	if (a6o_conf_value_is_string(value))
		access_monitor_add_directory(data->monitor, a6o_conf_value_get_string(value));
	else {
		const char **p;

		for (p = a6o_conf_value_get_list(value); *p != NULL; p++)
			access_monitor_add_directory(data->monitor, *p);
	}

	return A6O_MOD_OK;
}

static enum a6o_mod_status mod_oal_conf_white_list_dir(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct a6o_scan_conf *on_access_conf = a6o_scan_conf_on_access();

	if (a6o_conf_value_is_string(value))
		a6o_scan_conf_white_list_directory(on_access_conf, a6o_conf_value_get_string(value));
	else {
		const char **p;

		for (p = a6o_conf_value_get_list(value); *p != NULL; p++)
			a6o_scan_conf_white_list_directory(on_access_conf, *p);
	}

	return A6O_MOD_OK;
}

static enum a6o_mod_status mod_oal_conf_modules(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct a6o_scan_conf *on_access_conf = a6o_scan_conf_on_access();

	if (a6o_conf_value_is_string(value))
		a6o_scan_conf_add_module(on_access_conf, a6o_conf_value_get_string(value), module->armadito);
	else {
		const char **p;

		for (p = a6o_conf_value_get_list(value); *p != NULL; p++)
			a6o_scan_conf_add_module(on_access_conf, *p, module->armadito);
	}

	return A6O_MOD_OK;
}

static enum a6o_mod_status mod_oal_conf_mime_types(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct a6o_scan_conf *on_access_conf = a6o_scan_conf_on_access();

	if (a6o_conf_value_is_string(value))
		a6o_scan_conf_add_mime_type(on_access_conf, a6o_conf_value_get_string(value));
	else {
		const char **p;

		for (p = a6o_conf_value_get_list(value); *p != NULL; p++)
			a6o_scan_conf_add_mime_type(on_access_conf, *p);
	}

	return A6O_MOD_OK;
}

static enum a6o_mod_status mod_oal_conf_max_size(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct a6o_scan_conf *on_access_conf = a6o_scan_conf_on_access();

	a6o_scan_conf_max_file_size(on_access_conf, a6o_conf_value_get_int(value));

	return A6O_MOD_OK;
}

static enum a6o_mod_status mod_oal_post_init(struct a6o_module *module)
{
	struct mod_oal_data *data = (struct mod_oal_data *)module->data;

#define YES_NO(v) ((v) ? "yes" : "no")

	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_INFO, MODULE_LOG_NAME ": " "protection configuration:");
	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_INFO, MODULE_LOG_NAME ": " "-> enabled: %s", YES_NO(access_monitor_is_enable(data->monitor)));
	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_INFO, MODULE_LOG_NAME ": " "-> permission enabled: %s", YES_NO(access_monitor_is_enable_permission(data->monitor)));
	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_INFO, MODULE_LOG_NAME ": " "-> removable media monitoring: %s", YES_NO(access_monitor_is_enable_removable_media(data->monitor)));
	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_INFO, MODULE_LOG_NAME ": " "-> removable media autoscan: %s", YES_NO(access_monitor_is_autoscan_removable_media(data->monitor)));

	access_monitor_delayed_start(data->monitor);

	return A6O_MOD_OK;
}

static enum a6o_mod_status mod_oal_close(struct a6o_module *module)
{
	struct mod_oal_data *data = (struct mod_oal_data *)module->data;

	return A6O_MOD_OK;
}

struct a6o_conf_entry mod_oal_conf_table[] = {
	{ "enable", CONF_TYPE_INT, mod_oal_conf_enable},
	{ "enable-permission", CONF_TYPE_INT, mod_oal_conf_enable_permission},
	{ "enable-removable-media", CONF_TYPE_INT, mod_oal_conf_enable_removable_media},
	{ "autoscan-removable-media", CONF_TYPE_INT, mod_oal_conf_autoscan_removable_media},
	{ "mount", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_oal_conf_mount},
	{ "directory", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_oal_conf_directory},
	{ "white-list-dir", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_oal_conf_white_list_dir},
	{ "mime-types", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_oal_conf_mime_types},
	{ "modules", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_oal_conf_modules},
	{ "max-size", CONF_TYPE_INT, mod_oal_conf_max_size},
	{ NULL, 0, NULL},
};

struct a6o_module on_access_linux_module = {
	.init_fun = mod_oal_init,
	.conf_table = mod_oal_conf_table,
	.post_init_fun = mod_oal_post_init,
	.scan_fun = NULL,
	.close_fun = mod_oal_close,
	.info_fun = NULL,
	.name = MODULE_NAME,
	.size = sizeof(struct mod_oal_data),
};
