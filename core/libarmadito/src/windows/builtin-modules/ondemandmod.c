#include <libarmadito.h>
#include "ondemandmod.h"
#include "armaditop.h"

#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

static enum a6o_mod_status mod_on_demand_conf_white_list_dir(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct a6o_scan_conf *on_demand_conf = a6o_scan_conf_on_demand();

	if (a6o_conf_value_is_string(value))
		a6o_scan_conf_white_list_directory(on_demand_conf, a6o_conf_value_get_string(value));
	else {
		const char **p;

		for (p = a6o_conf_value_get_list(value); *p != NULL; p++)
			a6o_scan_conf_white_list_directory(on_demand_conf, *p);
	}

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status mod_on_demand_conf_modules(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct a6o_scan_conf *on_demand_conf = a6o_scan_conf_on_demand();

	if (a6o_conf_value_is_string(value))
		a6o_scan_conf_add_module(on_demand_conf, a6o_conf_value_get_string(value), module->armadito);
	else {
		const char **p;

		for (p = a6o_conf_value_get_list(value); *p != NULL; p++)
			a6o_scan_conf_add_module(on_demand_conf, *p, module->armadito);
	}

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status mod_on_demand_conf_mime_types(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct a6o_scan_conf *on_demand_conf = a6o_scan_conf_on_demand();

	if (a6o_conf_value_is_string(value))
		a6o_scan_conf_add_mime_type(on_demand_conf, a6o_conf_value_get_string(value));
	else {
		const char **p;

		for (p = a6o_conf_value_get_list(value); *p != NULL; p++)
			a6o_scan_conf_add_mime_type(on_demand_conf, *p);
	}

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status mod_on_demand_conf_max_size(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct a6o_scan_conf *on_demand_conf = a6o_scan_conf_on_demand();

	a6o_scan_conf_max_file_size(on_demand_conf, a6o_conf_value_get_int(value));

	return ARMADITO_MOD_OK;
}

struct a6o_conf_entry on_demand_conf_table[] = {
	{ "white-list-dir", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_on_demand_conf_white_list_dir},
	{ "modules", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_on_demand_conf_modules},
	{ "mime-types", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_on_demand_conf_mime_types},
	{ "max-size", CONF_TYPE_INT, mod_on_demand_conf_max_size},
	{ NULL, 0, NULL},
};

struct a6o_module on_demand_module = {
	.init_fun = NULL,
	.conf_table = on_demand_conf_table,
	.post_init_fun = NULL,
	.scan_fun = NULL,
	.close_fun = NULL,
	.supported_mime_types = NULL,
	.name = "on-demand",
};


