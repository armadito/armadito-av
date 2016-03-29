#include <libuhuru/core.h>

#include "monitor.h"
#include "onaccessmod.h"

#include <glib.h>
#include <stdlib.h>
#include <string.h>

struct mod_oal_data {
  struct access_monitor *monitor;
};

static enum uhuru_mod_status mod_oal_init(struct uhuru_module *module)
{
  struct mod_oal_data *data = malloc(sizeof(struct mod_oal_data));

  module->data = data;

  data->monitor = access_monitor_new(module->uhuru);

  /* if access monitor is NULL, for instance because this process does not */
  /* have priviledge (i.e. not running as root), we don't return an error because this will make */
  /* the scan daemon terminates */
  /* if (fa_data->monitor == NULL) */
  /*   return UHURU_MOD_INIT_ERROR; */

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_oal_conf_enable(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
  struct mod_oal_data *data = (struct mod_oal_data *)module->data;

  access_monitor_enable(data->monitor, uhuru_conf_value_get_int(value));

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_oal_conf_enable_permission(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
  struct mod_oal_data *data = (struct mod_oal_data *)module->data;

  access_monitor_enable_permission(data->monitor, uhuru_conf_value_get_int(value));

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_oal_conf_enable_removable_media(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
  struct mod_oal_data *data = (struct mod_oal_data *)module->data;

  access_monitor_enable_removable_media(data->monitor, uhuru_conf_value_get_int(value));

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_oal_conf_mount(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
  struct mod_oal_data *data = (struct mod_oal_data *)module->data;

  if (uhuru_conf_value_is_string(value))
    access_monitor_add_mount(data->monitor, uhuru_conf_value_get_string(value));
  else {
    const char **p;

    for (p = uhuru_conf_value_get_list(value); *p != NULL; p++)
      access_monitor_add_mount(data->monitor, *p);
  }

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_oal_conf_directory(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
  struct mod_oal_data *data = (struct mod_oal_data *)module->data;

  if (uhuru_conf_value_is_string(value))
    access_monitor_add_directory(data->monitor, uhuru_conf_value_get_string(value));
  else {
    const char **p;

    for (p = uhuru_conf_value_get_list(value); *p != NULL; p++)
      access_monitor_add_directory(data->monitor, *p);
  }

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_oal_conf_white_list_dir(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
  struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

  if (uhuru_conf_value_is_string(value))
    uhuru_scan_conf_white_list_directory(on_access_conf, uhuru_conf_value_get_string(value));
  else {
    const char **p;

    for (p = uhuru_conf_value_get_list(value); *p != NULL; p++)
      uhuru_scan_conf_white_list_directory(on_access_conf, *p);
  }

  return UHURU_MOD_OK;
}

#if 0
static enum uhuru_mod_status mod_oal_conf_mime_type(struct uhuru_module *module, const char *directive, const char **argv)
{
  const char *mime_type;
  struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

  if (argv[0] == NULL || argv[1] == NULL) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "invalid configuration directive, not enough arguments");
    return UHURU_MOD_CONF_ERROR;
  }

  mime_type = argv[0];

  for(argv++; *argv != NULL; argv++)
    uhuru_scan_conf_add_mime_type(on_access_conf, mime_type, *argv, module->uhuru);

  return UHURU_MOD_OK;
}
#endif

static enum uhuru_mod_status mod_oal_conf_max_size(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value)
{
  struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

  uhuru_scan_conf_max_file_size(on_access_conf, uhuru_conf_value_get_int(value));

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_oal_post_init(struct uhuru_module *module)
{
  struct mod_oal_data *data = (struct mod_oal_data *)module->data;

#define YES_NO(v) ((v) ? "yes" : "no")

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_INFO, MODULE_LOG_NAME ": " "protection configuration:");
  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_INFO, MODULE_LOG_NAME ": " "-> enabled: %s", YES_NO(access_monitor_is_enable(data->monitor)));
  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_INFO, MODULE_LOG_NAME ": " "-> permission enabled: %s", YES_NO(access_monitor_is_enable_permission(data->monitor)));
  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_INFO, MODULE_LOG_NAME ": " "-> removable media monitoring: %s", YES_NO(access_monitor_is_enable_removable_media(data->monitor)));

  access_monitor_delayed_start(data->monitor);

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_oal_close(struct uhuru_module *module)
{
  struct mod_oal_data *data = (struct mod_oal_data *)module->data;

  return UHURU_MOD_OK;
}

struct uhuru_conf_entry mod_oal_conf_table[] = {
  { "enable", CONF_TYPE_INT, mod_oal_conf_enable},
  { "enable-permission", CONF_TYPE_INT, mod_oal_conf_enable_permission},
  { "enable-removable-media", CONF_TYPE_INT, mod_oal_conf_enable_removable_media},
  { "mount", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_oal_conf_mount},
  { "directory", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_oal_conf_directory},
  { "white-list-dir", CONF_TYPE_STRING | CONF_TYPE_LIST, mod_oal_conf_white_list_dir},
  /* { "mime-type", mod_oal_conf_mime_type}, */
  { "max-size", CONF_TYPE_INT, mod_oal_conf_max_size},
  { NULL, 0, NULL},
};

struct uhuru_module module = {
  .init_fun = mod_oal_init,
  .conf_table = mod_oal_conf_table,
  .post_init_fun = mod_oal_post_init,
  .scan_fun = NULL,
  .close_fun = mod_oal_close,
  .info_fun = NULL,
  .name = MODULE_NAME,
  .size = sizeof(struct mod_oal_data),
};
