#include <libuhuru/module.h>
#include "remote.h"

#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

struct remote_data {
  char *remote_sock_dir;
};

static enum uhuru_mod_status remote_init(struct uhuru_module *module)
{
  struct remote_data *re_data = g_new(struct remote_data, 1);

  re_data->remote_sock_dir = NULL;

  module->data = re_data;

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status remote_conf_socket_dir(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct remote_data *re_data = (struct remote_data *)module->data;

  re_data->remote_sock_dir = strdup(argv[0]);

  return UHURU_MOD_OK;
}

struct uhuru_conf_entry remote_conf_table[] = {
  { 
    .directive = "socket-dir", 
    .conf_fun = remote_conf_socket_dir, 
  },
  { 
    .directive = NULL, 
    .conf_fun = NULL, 
  },
};

struct uhuru_module remote_module = {
  .init_fun = remote_init,
  .conf_table = remote_conf_table,
  .post_init_fun = NULL,
  .scan_fun = NULL,
  .close_fun = NULL,
  .name = "remote",
  .size = sizeof(struct remote_data),
};

const char *remote_module_get_sock_dir(struct uhuru_module *remote_module)
{
  struct remote_data *re_data = (struct remote_data *)(remote_module->data);

  assert(!strcmp(remote_module->name, "remote"));

  return re_data->remote_sock_dir;
}

