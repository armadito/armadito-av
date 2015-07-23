#ifndef _LIBUHURU_MODULEP_H_
#define _LIBUHURU_MODULEP_H_

#include <stdio.h>

#include <libuhuru/module.h>

struct uhuru_module {
  enum uhuru_mod_status (*init)(void **pmod_data);
  enum uhuru_mod_status (*conf_set)(void *mod_data, const char *key, const char *value);
  char *(*conf_get)(void *mod_data, const char *key);
  enum uhuru_file_status (*scan)(const char *path, const char *mime_type, void *mod_data, char **pmod_report);
  enum uhuru_mod_status (*close)(void *mod_data);
  enum uhuru_mod_status mod_status;
  const char *name;
  const char **mime_types;
  void *data;
};

struct uhuru_module *module_new(const char *path);

void module_print(struct uhuru_module *mod, FILE *out);

#endif
