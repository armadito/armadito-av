#include <libuhuru/core.h>
#include "mimetypemod.h"
#include "uhurup.h"

#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

static enum uhuru_mod_status mimetype_conf_mime_type(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct mimetype_data *mi_data = (struct mimetype_data *)module->data;
  const char *mime_type;
  enum uhuru_mod_status ret = UHURU_MOD_OK;

  if (argv[0] == NULL || argv[1] == NULL) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "mime-type: invalid configuration directive, not enough arguments");
    return UHURU_MOD_CONF_ERROR;
  }

  mime_type = argv[0];

  for(argv++; *argv != NULL; argv++) {
    struct uhuru_module *mod = uhuru_get_module_by_name(module->uhuru, *argv);
    
    if (mod == NULL) {
      g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "mime-type: no module '%s' for MIME type '%s'", *argv, mime_type);
      ret = UHURU_MOD_CONF_ERROR;
      continue;
    }

    uhuru_add_mime_type(module->uhuru, mime_type, mod);
  }

  return UHURU_MOD_OK;
}

struct uhuru_conf_entry mimetype_conf_table[] = {
  { 
    .directive = "mime-type", 
    .conf_fun = mimetype_conf_mime_type, 
  },
  { 
    .directive = NULL, 
    .conf_fun = NULL, 
  },
};

struct uhuru_module mimetype_module = {
  .init_fun = NULL,
  .conf_table = mimetype_conf_table,
  .post_init_fun = NULL,
  .scan_fun = NULL,
  .close_fun = NULL,
  .name = "mime-type",
};


