#include <libumwsu/module.h>
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

struct umw_module *module_new(const char *path)
{
  struct umw_module *mod;
  GModule *module;

  mod = (struct umw_module *)malloc(sizeof(struct umw_module));

  module = g_module_open(path, G_MODULE_BIND_LAZY);

  if (!module) {
    fprintf(stderr, "unable to open %s\n", path);
    return NULL;
  }

  mod->name = module_name_from_path(path);
  mod->data = NULL;

  mod->init = get_symbol(module, mod->name, "init");
  mod->scan = get_symbol(module, mod->name, "scan");
  mod->close = get_symbol(module, mod->name, "close");
  mod->files = get_symbol(module, mod->name, "files");

  if (mod->init != NULL)
    (*mod->init)(&mod->data);

#if 0
  if(!g_module_close(module))
    g_warning("%s: %s", path, g_module_error());
#endif

  return mod;
}

void module_print(struct umw_module *mod)
{
  const char **p;

  printf("name: %s\n", mod->name);
  printf("init: %p\n", mod->init);
  printf("scan: %p\n", mod->scan);
  printf("close: %p\n", mod->close);
  if (mod->files == NULL)
    return;
  for (p = mod->files; *p != NULL; p++)
    printf("file: %s\n", *p);
}


