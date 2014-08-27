#ifndef _LIBUMWSU_MODULEP_H_
#define _LIBUMWSU_MODULEP_H_

#include <stdio.h>

struct umwsu_module {
  enum umwsu_mod_status (*init)(void **pmod_data);
  enum umwsu_status (*scan)(const char *path, void *mod_data);
  enum umwsu_mod_status (*close)(void *mod_data);
  const char *name;
  const char **mime_types;
  void *data;
};

struct umwsu_module *module_new(const char *path);

void module_print(struct umwsu_module *mod, FILE *out);

#endif
