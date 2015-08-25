#include <libuhuru/module.h>
#include "mimetype.h"
#include "uhurup.h"

#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

struct mimetype_data {
};

static enum uhuru_mod_status mimetype_init(void **pmod_data)
{
  struct mimetype_data *re_data = g_new(struct mimetype_data, 1);

  /* ... */

  *pmod_data = re_data;

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status mimetype_conf_set_mime_type(void *mod_data, const char *key, const char **argv)
{
  struct mimetype_data *re_data = (struct mimetype_data *)mod_data;
  const char *mime_type;

  if (argv[0] == NULL || argv[1] == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "mime-type: invalid configuration directive, not enough arguments");
    return UHURU_MOD_CONF_ERROR;
  }

  mime_type = argv[0];

  for(argv++; *argv != NULL; argv++) {
    /* FIXME */
    struct uhuru_module *mod = uhuru_get_module_by_name(NULL, *argv);
    
    if (mod == NULL) {
      g_log(NULL, G_LOG_LEVEL_WARNING, "mime-type: no module '%s' for MIME type '%s'", *argv, mime_type);
      return UHURU_MOD_CONF_ERROR;
    }

    /* FIXME */
    uhuru_add_mime_type(NULL, mime_type, mod);
  }

  return UHURU_MOD_OK;
}

struct uhuru_conf_entry mimetype_conf_table[] = {
  { 
    .key = "mime-type", 
    .conf_set_fun = mimetype_conf_set_mime_type, 
    .conf_get_fun = NULL,
  },
  { 
    .key = NULL, 
    .conf_set_fun = NULL, 
    .conf_get_fun = NULL,
  },
};

struct uhuru_module mimetype_module = {
  .init_fun = mimetype_init,
  .conf_table = mimetype_conf_table,
  .post_init_fun = NULL,
  .scan_fun = NULL,
  .close_fun = NULL,
  .name = "mime-type",
};
