#include "conf.h"
#include "uhurup.h"

#include <glib.h>
#include <stdlib.h>
#include <assert.h>
#define _GNU_SOURCE
#include <stdio.h>

void conf_load(struct uhuru_module *mod)
{
  GKeyFile *key_file;
  char *filename;
  /* GError *error; */
  static char *dirs[] = { LIBUHURU_CONF_DIR, LIBUHURU_CONF_DIR "/conf.d", NULL};
  char **keys, **pkey;

  if (mod->conf_set == NULL)
    return;

  key_file = g_key_file_new();

  asprintf(&filename, "%s.conf", mod->name);

  if (!g_key_file_load_from_dirs(key_file, filename, (const gchar **)dirs, NULL, G_KEY_FILE_NONE, NULL)) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "cannot load conf file %s", mod->name);
    return;
  }

  keys = g_key_file_get_keys(key_file, mod->name, NULL, NULL);
  if (keys == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "cannot find group %s", mod->name);
    return;
  }

  for(pkey = keys; *pkey != NULL; pkey++) {
    char *value = g_key_file_get_value(key_file, mod->name, *pkey, NULL);

    assert(value != NULL);
    (*mod->conf_set)(mod->data, *pkey, value);
  }

  g_strfreev(keys);

  g_key_file_free(key_file);
  free(filename);
}

void conf_set(struct uhuru *uhuru, const char *mod_name, const char *key, const char *value)
{
  struct uhuru_module *mod = uhuru_get_module_by_name(uhuru, mod_name);

  if (mod == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "No such module: %s", mod_name);
    return;
  }

  (*mod->conf_set)(mod->data, key, value);
}

char *conf_get(struct uhuru *uhuru, const char *mod_name, const char *key)
{
  struct uhuru_module *mod = uhuru_get_module_by_name(uhuru, mod_name);

  if (mod == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "No such module: %s", mod_name);
    return;
  }

  return (*mod->conf_get)(mod->data, key);
}
