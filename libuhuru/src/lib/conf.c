#include "conf.h"
#include "uhurup.h"
#include "confparser.h"

#include <glib.h>
#include <stdlib.h>
#include <assert.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

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

static void conf_parser_set_cb(const char *group, const char *key, const char **argv, void *user_data)
{
  conf_set_value((struct uhuru *)user_data, group, key, argv);
}

void conf_load_file(struct uhuru *uhuru, const char *filename)
{
  struct uhuru_conf_parser *cp = uhuru_conf_parser_new(filename, conf_parser_set_cb, uhuru);

  uhuru_conf_parser_parse(cp);

  uhuru_conf_parser_free(cp);
}

void conf_load_path(struct uhuru *uhuru, const char *path)
{
  GDir *dir;
  const char *filename;
  GError *err = NULL;

  dir = g_dir_open(path, 0, &err);

  while((filename = g_dir_read_name(dir)) != NULL) {
    if (g_str_has_suffix(filename, ".conf"))
      conf_load_file(uhuru, filename);
  }

  g_dir_close(dir);
}


