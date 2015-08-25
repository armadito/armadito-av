#include "conf.h"
#include "uhurup.h"

#include <glib.h>
#include <stdlib.h>
#include <assert.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

  /* conf_set_value(cp->uhuru, cp->current_group, cp->current_key, (const char **)cp->current_args->pdata); */

static struct uhuru_conf_entry *conf_entry_get(struct uhuru_module *mod, const char *key)
{
  struct uhuru_conf_entry *p;

  if (mod->conf_table == NULL)
    return NULL;

  for(p = mod->conf_table; p->key != NULL; p++)
    if (!strcmp(key, p->key))
      return p;

  return NULL;
}

int conf_set_value(struct uhuru *uhuru, const char *mod_name, const char *key, const char **argv)
{
  struct uhuru_module *mod;
  struct uhuru_conf_entry *conf_entry;

  mod = uhuru_get_module_by_name(uhuru, mod_name);
  if (mod == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "conf_set: no module '%s'", mod_name);
    return -1;
  }

  conf_entry = conf_entry_get(mod, key);
  if (conf_entry == NULL || conf_entry->conf_get_fun == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "conf_set: no key '%s' for module '%s'", key, mod_name);
    return -1;
  }

  if ((*conf_entry->conf_set_fun)(mod->mod_data, key, argv) != UHURU_MOD_OK) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "conf_get: cannot get key '%s' value for module '%s'", key, mod_name);
    return -1;
  }

  return 0;
}

const char **conf_get_value(struct uhuru *uhuru, const char *mod_name, const char *key)
{
  struct uhuru_module *mod;
  struct uhuru_conf_entry *conf_entry;
  enum uhuru_mod_status mod_status;
  const char **argv;
 
  mod = uhuru_get_module_by_name(uhuru, mod_name);
  if (mod == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "conf_get: no module '%s'", mod_name);
    return NULL;
  }

  conf_entry = conf_entry_get(mod, key);
  if (conf_entry == NULL || conf_entry->conf_get_fun == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "conf_get: no key '%s' for module '%s'", key, mod_name);
    return NULL;
  }

  argv = (*conf_entry->conf_get_fun)(mod->mod_data, key, &mod_status);

  if (mod_status != UHURU_MOD_OK) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "conf_get: cannot get key '%s' value for module '%s'", key, mod_name);
    return NULL;
  }

  return argv;
}


/* ==================== old code ==================== */

void conf_load(struct uhuru_module *mod)
{
  GKeyFile *key_file;
  char *filename;
  /* GError *error; */
  static char *dirs[] = { LIBUHURU_CONF_DIR, LIBUHURU_CONF_DIR "/conf.d", NULL};
  char **keys, **pkey;

  /* if (mod->conf_set == NULL) */
  /*   return; */

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
    /* (*mod->conf_set)(mod->data, *pkey, value); */
  }

  g_strfreev(keys);

  g_key_file_free(key_file);
  free(filename);
}

