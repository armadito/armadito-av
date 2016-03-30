#include <libuhuru/core.h>
#include "onaccess_windows.h"
#include "uhurup.h"

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

static enum uhuru_mod_status mod_onaccess_init(struct uhuru_module *module)
{
	struct onaccess_data *fa_data = g_new(struct onaccess_data, 1);

	module->data = fa_data;

	fa_data->enable_permission = 0;

	return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_onaccess_conf_set_enable_on_access(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
	struct onaccess_data *fa_data = (struct onaccess_data *)module->data;

	fa_data->enable_permission = uhuru_conf_value_get_int(value);

	return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_onaccess_conf_modules(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
	struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

	if (uhuru_conf_value_is_string(value))
		uhuru_scan_conf_add_module(on_access_conf, uhuru_conf_value_get_string(value), module->uhuru);
	else {
		const char **p;

		for (p = uhuru_conf_value_get_list(value); *p != NULL; p++)
			uhuru_scan_conf_add_module(on_access_conf, *p, module->uhuru);
	}

	return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_onaccess_conf_max_size(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
	struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

	uhuru_scan_conf_max_file_size(on_access_conf, uhuru_conf_value_get_int(value));

	return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_onaccess_close(struct uhuru_module *module)
{
	struct onaccess_data *fa_data = (struct onaccess_data *)module->data;

#ifdef HAVE_LIBNOTIFY
	notify_uninit();
#endif

	return UHURU_MOD_OK;
}

struct uhuru_conf_entry mod_onaccess_conf_table[] = {
	{ "enable-on-access", CONF_TYPE_INT, mod_onaccess_conf_set_enable_on_access},
	{ "modules", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_onaccess_conf_modules},
	{ "max-size", CONF_TYPE_INT, mod_onaccess_conf_max_size},
	{ NULL, 0, NULL},
};

struct uhuru_module on_access_win_module = {
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
