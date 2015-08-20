#include <libuhuru/module.h>
#include "modulep.h"

#include <assert.h>
#include <gmodule.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *module_name_from_path(const char *path)
{
  char *start, *end, *module_name;
  size_t len;

  start = strrchr(path, '/');
  assert(start != NULL);

  end = strrchr(path, '.');
  assert(end != NULL);

  assert(end > start);

  len = end - start;

  module_name = (char *)malloc(len);
  strncpy(module_name, start + 1, len - 1);
  module_name[len - 1] = '\0';

  return module_name;
}

static gpointer get_symbol(GModule *module, const char *module_name, const char *sym_name)
{
  char *full_name;
  gpointer sym;

  if (asprintf(&full_name, "%s_%s", module_name, sym_name) == -1)
    return NULL;

  if (!g_module_symbol(module, full_name, (gpointer *)&sym))
    return NULL;

  return sym;
}

struct uhuru_module *module_new(const char *path)
{
  struct uhuru_module *mod;
  GModule *module;

  mod = (struct uhuru_module *)malloc(sizeof(struct uhuru_module));

  module = g_module_open(path, G_MODULE_BIND_LAZY);

  if (!module)
    return NULL;

  mod->name = module_name_from_path(path);
  mod->mod_status = UHURU_MOD_OK;
  mod->data = NULL;

  /* FIXME: must check if module contains essential symbols */
  mod->init_fun = get_symbol(module, mod->name, "init");
  mod->conf_set = get_symbol(module, mod->name, "conf_set");
  mod->conf_get = get_symbol(module, mod->name, "conf_get");
  mod->post_init_fun = get_symbol(module, mod->name, "post_init");
  mod->scan_fun = get_symbol(module, mod->name, "scan");
  mod->close_fun = get_symbol(module, mod->name, "close");
  mod->mime_types = get_symbol(module, mod->name, "mime_types");

  return mod;
}

void module_debug(struct uhuru_module *mod)
{
  g_log(NULL, G_LOG_LEVEL_DEBUG, "name: %s", mod->name);
  g_log(NULL, G_LOG_LEVEL_DEBUG, "init: %p", mod->init_fun);
  g_log(NULL, G_LOG_LEVEL_DEBUG, "scan: %p", mod->scan_fun);
  g_log(NULL, G_LOG_LEVEL_DEBUG, "close: %p", mod->close_fun);

  if (mod->mime_types != NULL) {
    const char **p;

    for (p = mod->mime_types; *p != NULL; p++)
      g_log(NULL, G_LOG_LEVEL_DEBUG, "mime type: %s", *p);
  }
}


