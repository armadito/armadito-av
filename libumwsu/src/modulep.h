#ifndef _LIBUMWSU_MODULEP_H_
#define _LIBUMWSU_MODULEP_H_

struct umw_module {
  enum umw_mod_status (*init)(void **pmod_data);
  enum umw_scan_status (*scan)(const char *path, void *mod_data);
  enum umw_mod_status (*close)(void *mod_data);
  void *mod_data;
  const char **mod_files;
};

int module_install(const char *path);

int module_load_directory(const char *directory);

#endif
