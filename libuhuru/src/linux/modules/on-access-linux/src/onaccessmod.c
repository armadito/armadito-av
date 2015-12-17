#include <libuhuru/core.h>

#include "monitor.h"

#include <glib.h>
#include <stdlib.h>
#include <string.h>

struct on_access_linux_data {
  struct access_monitor *monitor;
};

static enum uhuru_mod_status mod_on_access_linux_init(struct uhuru_module *module)
{
  struct on_access_linux_data *data = malloc(sizeof(struct on_access_linux_data));

  module->data = data;

  data->monitor = access_monitor_new(module->uhuru);

  /* if access monitor is NULL, for instance because this process does not */
  /* have priviledge (i.e. not running as root), we don't return an error because this will make */
  /* the scan daemon terminates */
  /* if (fa_data->monitor == NULL) */
  /*   return UHURU_MOD_INIT_ERROR; */

  return UHURU_MOD_OK;
}

#if 0
static enum uhuru_mod_status mod_on_access_linux_conf_set_watch_dir(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct on_access_linux_data *fa_data = (struct on_access_linux_data *)module->data;

  while (*argv != NULL) {
    g_ptr_array_add(fa_data->paths, strdup(*argv));

    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "on_access_linux_mod: added %s", *argv);

    argv++;
  }

  return UHURU_MOD_OK;
}
#endif

#if 0
static enum uhuru_mod_status mod_on_access_linux_conf_set_enable_on_access(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct on_access_linux_data *fa_data = (struct on_access_linux_data *)module->data;

  fa_data->enable_permission = atoi(argv[0]);

  return UHURU_MOD_OK;
}
#endif

static enum uhuru_mod_status mod_on_access_linux_conf_white_list_dir(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

  while (*argv != NULL) {
    uhuru_scan_conf_white_list_directory(on_access_conf, *argv);

    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "on_access_linux_mod: white list %s", *argv);

    argv++;
  }

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_on_access_linux_conf_mime_type(struct uhuru_module *module, const char *directive, const char **argv)
{
  const char *mime_type;
  struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

  if (argv[0] == NULL || argv[1] == NULL) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "on_access_linux: invalid configuration directive, not enough arguments");
    return UHURU_MOD_CONF_ERROR;
  }

  mime_type = argv[0];

  for(argv++; *argv != NULL; argv++)
    uhuru_scan_conf_add_mime_type(on_access_conf, mime_type, *argv, module->uhuru);

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_on_access_linux_conf_max_size(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

  uhuru_scan_conf_max_file_size(on_access_conf, atoi(argv[0]));

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_on_access_linux_post_init(struct uhuru_module *module)
{
  struct on_access_linux_data *data = (struct on_access_linux_data *)module->data;

  access_monitor_start(data->monitor);

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_on_access_linux_close(struct uhuru_module *module)
{
  struct on_access_linux_data *fa_data = (struct on_access_linux_data *)module->data;

  return UHURU_MOD_OK;
}

struct uhuru_conf_entry mod_on_access_linux_conf_table[] = {
#if 0
  { "watch-dir", mod_on_access_linux_conf_set_watch_dir},
  { "enable-on-access", mod_on_access_linux_conf_set_enable_on_access},
#endif
  { "white-list-dir", mod_on_access_linux_conf_white_list_dir},
  { "mime-type", mod_on_access_linux_conf_mime_type},
  { "max-size", mod_on_access_linux_conf_max_size},
  { NULL, NULL},
};

struct uhuru_module module = {
  .init_fun = mod_on_access_linux_init,
  .conf_table = mod_on_access_linux_conf_table,
  .post_init_fun = mod_on_access_linux_post_init,
  .scan_fun = NULL,
  .close_fun = mod_on_access_linux_close,
  .info_fun = NULL,
  .name = "on-access-linux",
  .size = sizeof(struct on_access_linux_data),
};
