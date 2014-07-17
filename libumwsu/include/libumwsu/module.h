#ifndef _LIBUMWSU_MODULE_H_
#define _LIBUMWSU_MODULE_H_

#if 0
enum umw_scan_status;

enum umw_mod_status {
  UMW_MOD_OK,
  UMW_MOD_INIT_ERROR,
  UMW_MOD_CONF_ERROR,
};

struct umw_module {
  enum umw_mod_status (*conf_init)(void **pmod_conf_data);
  enum umw_mod_status (*conf)(const char *key, const char *value, void *mod_conf_data);
  enum umw_mod_status (*init)(void **pmod_data);
  enum umw_scan_status (*scan)(const char *path, void *mod_data);
  enum umw_mod_status (*close)(void *mod_data);
};
#endif

int umw_module_install(const char *path);

#endif
