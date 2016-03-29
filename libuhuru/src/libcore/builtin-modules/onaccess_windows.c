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

static enum uhuru_mod_status mod_onaccess_conf_set_enable_on_access(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct onaccess_data *fa_data = (struct onaccess_data *)module->data;

  fa_data->enable_permission = atoi(argv[0]);

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_onaccess_conf_mime_type(struct uhuru_module *module, const char *directive, const char **argv)
{
  const char *mime_type;
  struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

  if (argv[0] == NULL || argv[1] == NULL) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "on access : invalid configuration directive, not enough arguments");
    return UHURU_MOD_CONF_ERROR;
  }

  mime_type = argv[0];

  for(argv++; *argv != NULL; argv++)
    uhuru_scan_conf_add_mime_type(on_access_conf, mime_type, *argv, module->uhuru);

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_onaccess_conf_max_size(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

  uhuru_scan_conf_max_file_size(on_access_conf, atoi(argv[0]));

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
  /* { "mime-type", mod_onaccess_conf_mime_type}, */
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
  .name = "on-access",
  .size = sizeof(struct onaccess_data),
};
