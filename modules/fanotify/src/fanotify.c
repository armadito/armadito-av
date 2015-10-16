#include <libuhuru/core.h>

#include "fawatch.h"

#include <glib.h>

struct fanotify_data {
  struct fa_watch *w;
};

static enum uhuru_mod_status mod_fanotify_init(struct uhuru_module *module)
{
  struct fanotify_data *fa_data = g_new(struct fanotify_data, 1);

  fa_data->w = fa_watch_new();

  module->data = fa_data;

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mod_fanotify_conf_set_watch_dir(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;

  while (*argv != NULL) {
    fa_watch_add(fa_data->w, *argv);

    argv++;
  }

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

  fa_watch_free(fa_data->w);

  fa_data->w = NULL;

  return UHURU_MOD_OK;
}

struct uhuru_conf_entry mod_fanotify_conf_table[] = {
  { "watch-dir", mod_fanotify_conf_set_watch_dir},
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
