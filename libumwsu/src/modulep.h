#ifndef _LIBUMWSU_MODULEP_H_
#define _LIBUMWSU_MODULEP_H_

struct umw_module {
  enum umw_mod_status (*init)(void **pmod_data);
  enum umw_status (*scan)(const char *path, void *mod_data);
  enum umw_mod_status (*close)(void *mod_data);
  const char *name;
  const char **mime_types;
  void *data;
};

struct umw_module *module_new(const char *path);

void module_print(struct umw_module *mod);

#if 0
int module_install(const char *path);

int module_load_directory(const char *directory);
#endif

#endif
