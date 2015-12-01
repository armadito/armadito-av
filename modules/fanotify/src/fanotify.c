#include <libuhuru/core.h>

#include "monitor.h"

#include <glib.h>
#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>
#endif

struct fanotify_data {
  struct access_monitor *monitor;
};

static enum uhuru_mod_status mod_fanotify_init(struct uhuru_module *module)
{
  struct fanotify_data *fa_data = g_new(struct fanotify_data, 1);

  module->data = fa_data;

  fa_data->monitor = access_monitor_new(module->uhuru);

  /* if access monitor is NULL, for instance because this process does not */
  /* have priviledge (i.e. not running as root), we don't return an error because this will make */
  /* the scan daemon terminates */
  /* if (fa_data->monitor == NULL) */
  /*   return UHURU_MOD_INIT_ERROR; */

#ifdef HAVE_LIBNOTIFY
  notify_init ("TatouAV");
#endif

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_fanotify_conf_set_watch_dir(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;

  if (fa_data->monitor != NULL) {
    while (*argv != NULL) {
      access_monitor_add(fa_data->monitor, *argv);

      argv++;
    }
  }

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_fanotify_conf_set_enable_on_access(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;

  if (fa_data->monitor != NULL)
    access_monitor_enable_permission(fa_data->monitor, atoi(argv[0]));

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_fanotify_post_init(struct uhuru_module *module)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;

  access_monitor_activate(fa_data->monitor);

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_fanotify_close(struct uhuru_module *module)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;

  if (fa_data->monitor != NULL) {
    access_monitor_free(fa_data->monitor);
  }

  fa_data->monitor = NULL;

#ifdef HAVE_LIBNOTIFY
  notify_uninit();
#endif

  return UHURU_MOD_OK;
}

struct uhuru_conf_entry mod_fanotify_conf_table[] = {
  { "watch-dir", mod_fanotify_conf_set_watch_dir},
  { "enable-on-access", mod_fanotify_conf_set_enable_on_access},
  { NULL, NULL},
};

struct uhuru_module module = {
  .init_fun = mod_fanotify_init,
  .conf_table = mod_fanotify_conf_table,
  .post_init_fun = mod_fanotify_post_init,
  .scan_fun = NULL,
  .close_fun = mod_fanotify_close,
  .info_fun = NULL,
  .name = "fanotify",
  .size = sizeof(struct fanotify_data),
};
