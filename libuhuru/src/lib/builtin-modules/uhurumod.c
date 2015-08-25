#include <libuhuru/module.h>
#include "uhurumod.h"
#include "uhurup.h"

#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

static enum uhuru_mod_status uhurumod_conf_set_mime_type(void *mod_data, const char *key, const char **argv)
{
  struct uhuru *u = (struct uhuru *)mod_data;
  const char *mime_type;

  if (argv[0] == NULL || argv[1] == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "mime-type: invalid configuration directive, not enough arguments");
    return UHURU_MOD_CONF_ERROR;
  }

  mime_type = argv[0];

  for(argv++; *argv != NULL; argv++) {
    struct uhuru_module *mod = uhuru_get_module_by_name(u, *argv);
    
    if (mod == NULL) {
      g_log(NULL, G_LOG_LEVEL_WARNING, "mime-type: no module '%s' for MIME type '%s'", *argv, mime_type);
      return UHURU_MOD_CONF_ERROR;
    }

    uhuru_add_mime_type(u, mime_type, mod);
  }

  return UHURU_MOD_OK;
}

#if 0
static enum uhuru_mod_status uhurumod_conf_set_conf_dir(void *mod_data, const char *key, const char **argv)
{
  struct uhuru *u = (struct uhuru *)mod_data;

  return UHURU_MOD_OK;
}
#endif

struct uhuru_conf_entry uhurumod_conf_table[] = {
  { 
    .key = "mime-type", 
    .conf_set_fun = uhurumod_conf_set_mime_type, 
    .conf_get_fun = NULL,
  },
#if 0
  { 
    .key = "conf-dir", 
    .conf_set_fun = uhurumod_conf_set_conf_dir, 
    .conf_get_fun = NULL,
  },
#endif
  { 
    .key = NULL, 
    .conf_set_fun = NULL, 
    .conf_get_fun = NULL,
  },
};

struct uhuru_module uhurumod_module = {
  .init_fun = NULL,
  .conf_table = uhurumod_conf_table,
  .post_init_fun = NULL,
  .scan_fun = NULL,
  .close_fun = NULL,
  .name = "uhuru",
};
