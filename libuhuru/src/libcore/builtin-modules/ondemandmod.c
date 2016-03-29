#include <libuhuru/core.h>
#include "ondemandmod.h"
#include "uhurup.h"

#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

static enum uhuru_mod_status mod_on_demand_conf_white_list_dir(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
  struct uhuru_scan_conf *on_demand_conf = uhuru_scan_conf_on_demand();

  if (uhuru_conf_value_is_string(value))
    uhuru_scan_conf_white_list_directory(on_demand_conf, uhuru_conf_value_get_string(value));
  else {
    const char **p;

    for (p = uhuru_conf_value_get_list(value); *p != NULL; p++)
      uhuru_scan_conf_white_list_directory(on_demand_conf, *p);
  }

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_on_demand_conf_modules(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
  struct uhuru_scan_conf *on_demand_conf = uhuru_scan_conf_on_demand();

  if (uhuru_conf_value_is_string(value))
    uhuru_scan_conf_add_module(on_demand_conf, uhuru_conf_value_get_string(value), module->uhuru);
  else {
    const char **p;

    for (p = uhuru_conf_value_get_list(value); *p != NULL; p++)
      uhuru_scan_conf_add_module(on_demand_conf, *p, module->uhuru);
  }

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_on_demand_conf_max_size(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
  struct uhuru_scan_conf *on_demand_conf = uhuru_scan_conf_on_demand();

  uhuru_scan_conf_max_file_size(on_demand_conf, uhuru_conf_value_get_int(value));
  
  return UHURU_MOD_OK;
}

struct uhuru_conf_entry on_demand_conf_table[] = {
  { "white-list-dir", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_on_demand_conf_white_list_dir},
  { "modules", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_on_demand_conf_modules},
  { "max-size", CONF_TYPE_INT, mod_on_demand_conf_max_size},
  { NULL, 0, NULL},
};

struct uhuru_module on_demand_module = {
  .init_fun = NULL,
  .conf_table = on_demand_conf_table,
  .post_init_fun = NULL,
  .scan_fun = NULL,
  .close_fun = NULL,
  .supported_mime_types = NULL,
  .name = "on-demand",
};


