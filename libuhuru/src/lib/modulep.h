#ifndef _LIBUHURU_MODULEP_H_
#define _LIBUHURU_MODULEP_H_

#include <stdio.h>

#include <libuhuru/module.h>

#if 0
struct uhuru_module {
  /* in public api */
  enum uhuru_mod_status (*init)(void **pmod_data);
  enum uhuru_mod_status (*conf_set)(void *mod_data, const char *key, const char *value);
  char *(*conf_get)(void *mod_data, const char *key);
  enum uhuru_file_status (*scan)(const char *path, const char *mime_type, void *mod_data, char **pmod_report);
  enum uhuru_mod_status (*close)(void *mod_data);

  /* internal */
  enum uhuru_mod_status mod_status;
  const char *name;
  const char **mime_types;
  void *data;
};

struct uhuru_module_internal {
  struct uhuru_module *module;
  enum uhuru_mod_status mod_status;
  const char *name;
  void *data;
  /* FIXME: must be handled by configuration */
  const char **mime_types;
};
#endif

struct uhuru_module *module_new(const char *path);

void module_debug(struct uhuru_module *mod);

#endif
