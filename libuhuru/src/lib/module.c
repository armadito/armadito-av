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

  if (!module) {
    fprintf(stderr, "unable to open %s\n", path);
    return NULL;
  }

  mod->name = module_name_from_path(path);
  mod->data = NULL;

  mod->conf_set = get_symbol(module, mod->name, "conf_set");
  mod->conf_get = get_symbol(module, mod->name, "conf_get");
  mod->init = get_symbol(module, mod->name, "init");
  mod->scan = get_symbol(module, mod->name, "scan");
  mod->close = get_symbol(module, mod->name, "close");
  mod->mime_types = get_symbol(module, mod->name, "mime_types");

#if 0
  if(!g_module_close(module))
    g_warning("%s: %s", path, g_module_error());
#endif

  return mod;
}

void module_print(struct uhuru_module *mod, FILE *out)
{
  fprintf(out, "name: %s\n", mod->name);
  fprintf(out, "init: %p\n", mod->init);
  fprintf(out, "scan: %p\n", mod->scan);
  fprintf(out, "close: %p\n", mod->close);

  if (mod->mime_types != NULL) {
    const char **p;

    for (p = mod->mime_types; *p != NULL; p++)
      fprintf(out, "mime type: %s\n", *p);
  }
}


