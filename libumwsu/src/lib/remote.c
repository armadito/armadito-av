#include <libumwsu/scan.h>
#include <libumwsu/module.h>
#include "modulep.h"
#include "remote.h"

#include <stdlib.h>
#include <string.h>

static char *remote_sock_path;

static enum umwsu_mod_status mod_remote_conf(void *mod_data, const char *key, const char *value)
{
  if (!strcmp(key, "socket-path")) {
    fprintf(stderr, "remote: got config %s -> %s\n", key, value);
    remote_sock_path = strdup(value);
  } 

  return UMWSU_MOD_OK;
}

struct umwsu_module umwsu_mod_remote = {
  .init = NULL,
  .conf = &mod_remote_conf,
  .scan = NULL,
  .close = NULL,
  .name = "remote",
  .mime_types = NULL,
  .data = NULL,
};
