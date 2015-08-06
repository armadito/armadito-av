#include <libuhuru/scan.h>
#include <libuhuru/module.h>
#include "modulep.h"
#include "remote.h"

#include <stdlib.h>
#include <string.h>

static char *remote_sock_dir;

static enum uhuru_mod_status mod_remote_conf_set(void *mod_data, const char *key, const char *value)
{
  if (!strcmp(key, "socket-dir")) {
    remote_sock_dir = strdup(value);
    return UHURU_MOD_OK;
  } 

  return UHURU_MOD_CONF_ERROR;
}

static char *mod_remote_conf_get(void *mod_data, const char *key)
{
  if (!strcmp(key, "socket-dir"))
    return remote_sock_dir;

  return NULL;
}

struct uhuru_module uhuru_mod_remote = {
  .init = NULL,
  .conf_set = &mod_remote_conf_set,
  .conf_get = &mod_remote_conf_get,
  .scan = NULL,
  .close = NULL,
  .name = "remote",
  .mime_types = NULL,
  .data = NULL,
};
