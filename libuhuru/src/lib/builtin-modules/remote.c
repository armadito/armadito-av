#include <libuhuru/module.h>
#include "remote.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct remote_data {
  char *remote_sock_dir;
};

static enum uhuru_mod_status remote_init(void **pmod_data)
{
  struct remote_data *re_data;

  re_data = (struct remote_data *)malloc(sizeof(struct remote_data));
  assert(re_data != NULL);

  re_data->remote_sock_dir = NULL;

  *pmod_data = re_data;

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status remote_conf_set_socket_dir(void *mod_data, const char *key, const char **argv)
{
  struct remote_data *re_data = (struct remote_data *)mod_data;

  re_data->remote_sock_dir = strdup(argv[0]);

  return UHURU_MOD_OK;
}

static const char **remote_conf_get_socket_dir(void *mod_data, const char *key, enum uhuru_mod_status *pstatus)
{
  struct remote_data *re_data = (struct remote_data *)mod_data;
  const char **argv = (const char **)malloc(2 * sizeof(const char *));

  argv[0] = strdup(re_data->remote_sock_dir);
  argv[1] = NULL;

  *pstatus = UHURU_MOD_OK;

  return argv;
}

struct uhuru_conf_entry remote_conf_table[] = {
  { 
    .key = "socket-dir", 
    .conf_set_fun = remote_conf_set_socket_dir, 
    .conf_get_fun = remote_conf_get_socket_dir,
  },
  { 
    .key = NULL, 
    .conf_set_fun = NULL, 
    .conf_get_fun = NULL,
  },
};

struct uhuru_module remote_module = {
  .init_fun = remote_init,
  .conf_table = remote_conf_table,
  .post_init_fun = NULL,
  .scan_fun = NULL,
  .close_fun = NULL,
  .name = "remote",
};
