#include <libumwsu/module.h>
#include "dir.h"
#include "modulep.h"

#include <alloca.h>
#include <assert.h>
#include <gmodule.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <magic.h>

static void module_name_from_path(const char *path, char *module_name, size_t n)
{
  char *start, *end;

  start = strrchr(path, '/');
  assert(start != NULL);

  end = strrchr(path, '.');
  assert(end != NULL);

  assert(end > start);

  assert(end - start < n -1);

  n = end - start;
  strncpy(module_name, start + 1, n);
  module_name[n - 1] = '\0';
}

int module_install(const char *path)
{
  GModule *module;
#define MODULE_NAME_LEN 128
  char module_name[MODULE_NAME_LEN];
  char *install_fun_name;
  void (*install_fun)(void);

  module = g_module_open(path, G_MODULE_BIND_LAZY);

  if (!module) {
    /* g_set_error(error, FOO_ERROR, FOO_ERROR_BLAH,  "%s", g_module_error()); */
    fprintf(stderr, "unable to open %s\n", path);
    return FALSE;
  }

  module_name_from_path(path, module_name, MODULE_NAME_LEN);

  install_fun_name = (char *)alloca(MODULE_NAME_LEN + 8 + 1);
  snprintf(install_fun_name, MODULE_NAME_LEN + 8 + 1, "%s_install", module_name);

  if (!g_module_symbol(module, install_fun_name, (gpointer *)&install_fun)) {
    /* g_set_error(error, SAY_ERROR, SAY_ERROR_OPEN, "%s: %s", path, g_module_error()); */
    fprintf(stderr, "cannot find symbol %s\n", install_fun_name);
    if (!g_module_close(module))
      g_warning("%s: %s", path, g_module_error());
    return FALSE;
  }

  if (install_fun == NULL) {
    /* g_set_error(error, SAY_ERROR, SAY_ERROR_OPEN, "install function is NULL"); */
    fprintf(stderr, "install function is NULL\n");
    if(!g_module_close(module))
      g_warning("%s: %s", path, g_module_error());
    return FALSE;
  }

  (*install_fun)();

  if(!g_module_close(module))
    g_warning("%s: %s", path, g_module_error());

  return TRUE;
}

static void load_entry(const char *full_path, void *data)
{
  magic_t *p = (magic_t *)data;
  const char *t = magic_file(*p, full_path);
  
  if (strncmp("ELF 64-bit LSB shared object", t, 28))
    return;

  printf("loading module: %s\n", full_path);

  module_install(full_path);
}

int module_load_directory(const char *directory)
{
  magic_t *p;
  int r;

  p = (magic_t *)malloc(sizeof(magic_t));

  *p = magic_open(MAGIC_NONE);
  magic_load(*p, NULL);

  r = dir_map(directory, 0, load_entry, p);

  magic_close(*p);
  free(p);

  return r;
}

