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

static enum uhuru_mod_status mod_oal_conf_enable(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct mod_oal_data *data = (struct mod_oal_data *)module->data;

  access_monitor_enable(data->monitor, atoi(argv[0]));

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_oal_conf_enable_permission(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct mod_oal_data *data = (struct mod_oal_data *)module->data;

  access_monitor_enable_permission(data->monitor, atoi(argv[0]));

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_oal_conf_enable_removable_media(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct mod_oal_data *data = (struct mod_oal_data *)module->data;

  access_monitor_enable_removable_media(data->monitor, atoi(argv[0]));

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_oal_conf_mount(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct mod_oal_data *data = (struct mod_oal_data *)module->data;

  for(; *argv != NULL; argv++)
    access_monitor_add_mount(data->monitor, *argv);

  return UHURU_MOD_OK;
}
static enum uhuru_mod_status mod_oal_conf_directory(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct mod_oal_data *data = (struct mod_oal_data *)module->data;

  for(; *argv != NULL; argv++)
    access_monitor_add_directory(data->monitor, *argv);

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_oal_conf_white_list_dir(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

  while (*argv != NULL) {
    uhuru_scan_conf_white_list_directory(on_access_conf, *argv);

    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, MODULE_LOG_NAME ": " "white list %s", *argv);

    argv++;
  }

  return UHURU_MOD_OK;
}

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

static enum uhuru_mod_status mod_oal_conf_max_size(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

  uhuru_scan_conf_max_file_size(on_access_conf, atoi(argv[0]));

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_oal_post_init(struct uhuru_module *module)
{
  struct mod_oal_data *data = (struct mod_oal_data *)module->data;

  access_monitor_delayed_start(data->monitor);

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_oal_close(struct uhuru_module *module)
{
  struct mod_oal_data *data = (struct mod_oal_data *)module->data;

  return UHURU_MOD_OK;
}

struct uhuru_conf_entry mod_oal_conf_table[] = {
  { "enable", mod_oal_conf_enable},
  { "enable-permission", mod_oal_conf_enable_permission},
  { "enable-removable-media", mod_oal_conf_enable_removable_media},
  { "mount", mod_oal_conf_mount},
  { "directory", mod_oal_conf_directory},
  { "white-list-dir", mod_oal_conf_white_list_dir},
  { "mime-type", mod_oal_conf_mime_type},
  { "max-size", mod_oal_conf_max_size},
  { NULL, NULL},
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
