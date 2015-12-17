#include "libuhuru-config.h"
#include <libuhuru/core.h>

#include "os/string.h"
#include "uhurup.h"

#include <glib.h>
#include <stdlib.h>

struct uhuru_scan_conf {
  const char *name;
  size_t max_file_size;
  GHashTable *mime_type_table;
  /* a GArray and not a GPtrArray because GArray can be automatically NULL terminated */
  GArray *directories_white_list;
};

static struct uhuru_scan_conf *uhuru_scan_conf_new(const char *name)
{
  struct uhuru_scan_conf *c = malloc(sizeof(struct uhuru_scan_conf));

  c->name = os_strdup(name);
  
  c->max_file_size = 0;

  c->mime_type_table = g_hash_table_new(g_str_hash, g_str_equal);

  c->directories_white_list = g_array_new(TRUE, TRUE, sizeof(const char *));

  return c;
}

static struct uhuru_scan_conf *on_demand_conf = NULL;
static struct uhuru_scan_conf *on_access_conf = NULL;

static struct uhuru_scan_conf *get_conf(struct uhuru_scan_conf **pc, const char *name)
{
  if (*pc == NULL)
    *pc = uhuru_scan_conf_new(name);

  return *pc;
}

struct uhuru_scan_conf *uhuru_scan_conf_on_demand(void)
{
  return get_conf(&on_demand_conf, "on-demand scan configuration");
}

struct uhuru_scan_conf *uhuru_scan_conf_on_access(void)
{
  return get_conf(&on_access_conf, "on-access scan configuration");
}

void uhuru_scan_conf_white_list_directory(struct uhuru_scan_conf *c, const char *path)
{
  const char *cp = os_strdup(path);

  g_array_append_val(c->directories_white_list, cp);
}

static int strprefix(const char *s, const char *prefix)
{
  while (*prefix && *s && *prefix++ == *s++)
    ;

  if (*prefix == '\0')
    return *s == '\0' || *s == '/';

  return 0;
}

int uhuru_scan_conf_is_white_listed(struct uhuru_scan_conf *c, const char *path)
{
  const char **p = (const char **)c->directories_white_list->data;

  while(*p != NULL) {
    if (strprefix(path, *p))
      return 1;

    p++;
  }
    
  return 0;
}

void uhuru_scan_conf_add_mime_type(struct uhuru_scan_conf *c, const char *mime_type, const char *module_name, struct uhuru *u)
{
  /* a GArray and not a GPtrArray because GArray can be automatically NULL terminated */
  GArray *modules;
  struct uhuru_module *mod = uhuru_get_module_by_name(u, module_name);
    
  if (mod == NULL) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "conf :: no module '%s' for MIME type '%s'", module_name, mime_type);
    return;
  }
    
  modules = (GArray *)g_hash_table_lookup(c->mime_type_table, mime_type);

  if (modules == NULL) {
    modules = g_array_new(TRUE, TRUE, sizeof(struct uhuru_module *));

    g_hash_table_insert(c->mime_type_table, (gpointer)(os_strdup(mime_type)), modules);
  }

  g_array_append_val(modules, mod);
}

struct uhuru_module **uhuru_scan_conf_get_applicable_modules(struct uhuru_scan_conf *c, const char *mime_type)
{
  GArray *modules;

  modules = (GArray *)g_hash_table_lookup(c->mime_type_table, mime_type);

  if (modules != NULL)
    return (struct uhuru_module **)modules->data;

  modules = (GArray *)g_hash_table_lookup(c->mime_type_table, "*");

  if (modules != NULL)
    return (struct uhuru_module **)modules->data;

  return NULL;
}

void uhuru_scan_conf_max_file_size(struct uhuru_scan_conf *c, int max_file_size)
{
  c->max_file_size = max_file_size;
}

