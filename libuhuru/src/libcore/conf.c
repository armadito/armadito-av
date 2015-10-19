#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "conf.h"
#include "uhurup.h"
#include "confparser.h"
#include "os/dir.h"

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

static void conf_load_dirent_cb(const char *full_path, enum os_file_flag flags, int entry_errno, void *data)
{
  if (flags & FILE_FLAG_IS_PLAIN_FILE) {
    size_t len = strlen(full_path);

    if (len > 5 && !strcmp(full_path + len - 5, ".conf"))
      conf_load_file((struct uhuru *)data, full_path);
  }
}

void conf_load_path(struct uhuru *uhuru, const char *path)
{
  os_dir_map(path, 0, conf_load_dirent_cb, uhuru);
}


