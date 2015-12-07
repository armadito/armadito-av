#include <libuhuru/core.h>

#include <glib.h>
#include <stdlib.h>
#include <string.h>

/* #ifdef HAVE_LIBNOTIFY */
/* #include <libnotify/notify.h> */
/* #endif */

struct fanotify_data {
  GPtrArray *paths;
  int enable_permission;
};

void path_destroy_notify(gpointer data)
{
  free(data);
}

static enum uhuru_mod_status mod_fanotify_init(struct uhuru_module *module)
{
  struct fanotify_data *fa_data = g_new(struct fanotify_data, 1);

  module->data = fa_data;

  fa_data->paths = g_ptr_array_new_full(10, path_destroy_notify);
  fa_data->enable_permission = 0;

  /* if access monitor is NULL, for instance because this process does not */
  /* have priviledge (i.e. not running as root), we don't return an error because this will make */
  /* the scan daemon terminates */
  /* if (fa_data->monitor == NULL) */
  /*   return UHURU_MOD_INIT_ERROR; */

#ifdef HAVE_LIBNOTIFY
  notify_init("TatouAV");
#endif

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_fanotify_conf_set_watch_dir(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;

  while (*argv != NULL) {
    g_ptr_array_add(fa_data->paths, strdup(*argv));

    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "fanotify_mod: added %s", *argv);

    argv++;
  }

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_fanotify_conf_set_enable_on_access(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;

  fa_data->enable_permission = atoi(argv[0]);

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_fanotify_conf_white_list_dir(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

  while (*argv != NULL) {
    uhuru_scan_conf_white_list_directory(on_access_conf, *argv);

    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "fanotify_mod: white list %s", *argv);

    argv++;
  }

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_fanotify_conf_mime_type(struct uhuru_module *module, const char *directive, const char **argv)
{
  const char *mime_type;
  struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

  if (argv[0] == NULL || argv[1] == NULL) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "fanotify: invalid configuration directive, not enough arguments");
    return UHURU_MOD_CONF_ERROR;
  }

  mime_type = argv[0];

  for(argv++; *argv != NULL; argv++)
    uhuru_scan_conf_add_mime_type(on_access_conf, mime_type, *argv, module->uhuru);

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_fanotify_conf_max_size(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct uhuru_scan_conf *on_access_conf = uhuru_scan_conf_on_access();

  uhuru_scan_conf_max_file_size(on_access_conf, atoi(argv[0]));

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_fanotify_post_init(struct uhuru_module *module)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_fanotify_close(struct uhuru_module *module)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;

#ifdef HAVE_LIBNOTIFY
  notify_uninit();
#endif

  return UHURU_MOD_OK;
}

struct uhuru_conf_entry mod_fanotify_conf_table[] = {
  { "watch-dir", mod_fanotify_conf_set_watch_dir},
  { "enable-on-access", mod_fanotify_conf_set_enable_on_access},
  { "white-list-dir", mod_fanotify_conf_white_list_dir},
  { "mime-type", mod_fanotify_conf_mime_type},
  { "max-size", mod_fanotify_conf_max_size},
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
