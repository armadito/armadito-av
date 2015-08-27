#include "conf.h"
#include "uhurup.h"
#include "confparser.h"

#include <glib.h>
#include <stdlib.h>
#include <assert.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

static struct uhuru_conf_entry *conf_entry_get(struct uhuru_module *module, const char *directive)
{
  struct uhuru_conf_entry *p;

  if (module->conf_table == NULL)
    return NULL;

  for(p = module->conf_table; p->directive != NULL; p++)
    if (!strcmp(directive, p->directive))
      return p;

  return NULL;
}

static int conf_set(struct uhuru *uhuru, const char *mod_name, const char *directive, const char **argv)
{
  struct uhuru_module *mod;
  struct uhuru_conf_entry *conf_entry;

  mod = uhuru_get_module_by_name(uhuru, mod_name);
  if (mod == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "conf_set: no module '%s'", mod_name);
    return -1;
  }

  conf_entry = conf_entry_get(mod, directive);
  if (conf_entry == NULL || conf_entry->conf_fun == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "conf_set: no directive '%s' for module '%s'", directive, mod_name);
    return -1;
  }

  if ((*conf_entry->conf_fun)(mod, directive, argv) != UHURU_MOD_OK) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "conf_set: cannot assign value to directive '%s' for module '%s'", directive, mod_name);
    return -1;
  }

  return 0;
}

/* const char **conf_get_value(struct uhuru *uhuru, const char *mod_name, const char *key) */
/* { */
/*   struct uhuru_module *mod; */
/*   struct uhuru_conf_entry *conf_entry; */
/*   enum uhuru_mod_status mod_status; */
/*   const char **argv; */
 
/*   mod = uhuru_get_module_by_name(uhuru, mod_name); */
/*   if (mod == NULL) { */
/*     g_log(NULL, G_LOG_LEVEL_WARNING, "conf_get: no module '%s'", mod_name); */
/*     return NULL; */
/*   } */

/*   conf_entry = conf_entry_get(mod, key); */
/*   if (conf_entry == NULL || conf_entry->conf_get_fun == NULL) { */
/*     g_log(NULL, G_LOG_LEVEL_WARNING, "conf_get: no key '%s' for module '%s'", key, mod_name); */
/*     return NULL; */
/*   } */

/*   argv = (*conf_entry->conf_get_fun)(mod->data, key, &mod_status); */

/*   if (mod_status != UHURU_MOD_OK) { */
/*     g_log(NULL, G_LOG_LEVEL_WARNING, "conf_get: cannot get value of key '%s' for module '%s'", key, mod_name); */
/*     return NULL; */
/*   } */

/*   return argv; */
/* } */

static void conf_parser_set_cb(const char *group, const char *directive, const char **argv, void *user_data)
{
  conf_set((struct uhuru *)user_data, group, directive, argv);
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
  GString *full_path = g_string_new("");
  GError *err = NULL;

  dir = g_dir_open(path, 0, &err);

  while((filename = g_dir_read_name(dir)) != NULL) {
    if (g_str_has_suffix(filename, ".conf"))
      g_string_printf(full_path, "%s%s%s", path, G_DIR_SEPARATOR_S, filename);

      conf_load_file(uhuru, full_path->str);
  }

  g_dir_close(dir);

  g_string_free(full_path, TRUE);
}


