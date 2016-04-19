#include <libarmadito.h>
#include "onaccess_windows.h"
#include <armaditop.h>

#include <glib.h>
#include <stdlib.h>
#include <string.h>

struct onaccess_data {
	int enable_permission;
};

void path_destroy_notify(gpointer data)
{
	free(data);
}

static enum a6o_mod_status mod_onaccess_init(struct a6o_module *module)
{
	struct onaccess_data *fa_data = g_new(struct onaccess_data, 1);

	module->data = fa_data;

	fa_data->enable_permission = 0;

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

static enum a6o_mod_status mod_onaccess_close(struct a6o_module *module)
{
	struct onaccess_data *fa_data = (struct onaccess_data *)module->data;

#ifdef HAVE_LIBNOTIFY
	notify_uninit();
#endif

	return ARMADITO_MOD_OK;
}

struct a6o_conf_entry mod_onaccess_conf_table[] = {
	{ "enable-on-access", CONF_TYPE_INT, mod_onaccess_conf_set_enable_on_access},
	{ "modules", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_onaccess_conf_modules},
	{ "max-size", CONF_TYPE_INT, mod_onaccess_conf_max_size},
	{ NULL, 0, NULL},
};

struct a6o_module on_access_win_module = {
	.init_fun = mod_onaccess_init,
	.conf_table = mod_onaccess_conf_table,
	.post_init_fun = NULL,
	.scan_fun = NULL,
	.close_fun = mod_onaccess_close,
	.info_fun = NULL,
	.supported_mime_types = NULL,
	.name = "on-access",
	.size = sizeof(struct onaccess_data),
};
