#include <libuhuru/core.h>

#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct fanotify_data {
};

static enum uhuru_mod_status fanotify_init(struct uhuru_module *module)
{
  struct fanotify_data *fa_data = NULL;

  module->data = fa_data;

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status fanotify_conf_set_watch_dir(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;

  /* fa_data->db_dir = os_strdup(argv[0]); */

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status fanotify_post_init(struct uhuru_module *module)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;

  fprintf(stderr, "fanotify is initialized\n");

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status fanotify_close(struct uhuru_module *module)
{
  struct fanotify_data *fa_data = (struct fanotify_data *)module->data;

  return UHURU_MOD_OK;
}

struct uhuru_conf_entry fanotify_conf_table[] = {
  { "watch-dir", fanotify_conf_set_watch_dir},
  { NULL, NULL},
};

struct uhuru_module module = {
  .init_fun = fanotify_init,
  .conf_table = fanotify_conf_table,
  .post_init_fun = fanotify_post_init,
  .scan_fun = NULL,
  .close_fun = fanotify_close,
  .info_fun = NULL,
  .name = "Linux on-access scan using fanotify",
  .size = sizeof(struct fanotify_data),
};
