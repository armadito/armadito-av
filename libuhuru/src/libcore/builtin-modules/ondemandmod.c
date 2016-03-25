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

  if (!uhuru_conf_value_is_string(value) && !uhuru_conf_value_is_list(value))
    return UHURU_MOD_CONF_ERROR;
  
  if (uhuru_conf_value_is_string(value))
    uhuru_scan_conf_white_list_directory(on_demand_conf, uhuru_conf_value_get_string(value));
  else {
    const char **p;

    for (p = uhuru_conf_value_get_list(value); *p != NULL; p++)
      uhuru_scan_conf_white_list_directory(on_demand_conf, *p);
  }

  return UHURU_MOD_OK;
}

#if 0
static enum uhuru_mod_status mod_on_demand_conf_mime_type(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
  struct uhuru_scan_conf *on_demand_conf = uhuru_scan_conf_on_demand();
  const char *mime_type;

  if (argv[0] == NULL || argv[1] == NULL) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "on_demand: invalid configuration directive, not enough arguments");
    return UHURU_MOD_CONF_ERROR;
  }

  mime_type = argv[0];

  for(argv++; *argv != NULL; argv++)
    uhuru_scan_conf_add_mime_type(on_demand_conf, mime_type, *argv, module->uhuru);

  return UHURU_MOD_OK;
}
#endif

static enum uhuru_mod_status mod_on_demand_conf_max_size(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
  struct uhuru_scan_conf *on_demand_conf = uhuru_scan_conf_on_demand();

  if (!uhuru_conf_value_is_int(value))
    return UHURU_MOD_CONF_ERROR;

  uhuru_scan_conf_max_file_size(on_demand_conf, uhuru_conf_value_get_int(value));
  
  return UHURU_MOD_OK;
}

struct uhuru_conf_entry on_demand_conf_table[] = {
  { "white-list-dir", mod_on_demand_conf_white_list_dir},
  /* { "mime-type", mod_on_demand_conf_mime_type}, */
  { "max-size", mod_on_demand_conf_max_size},
  { NULL, NULL},
};

struct uhuru_module on_demand_module = {
  .init_fun = NULL,
  .conf_table = on_demand_conf_table,
  .post_init_fun = NULL,
  .scan_fun = NULL,
  .close_fun = NULL,
  .name = "on-demand",
};


