#ifndef _LIBUMWSU_MODULE_H_
#define _LIBUMWSU_MODULE_H_

enum uw_mod_status {
  UMW_MOD_OK,
  UMW_MOD_INIT_ERROR,
  UMW_MOD_CONF_ERROR,
};

typedef enum uw_mod_status (umw_fun_init_t)(void **pdata);
typedef int (umw_fun_conf_t)(const char *key, const char *value, void *data);

void umw_module_install(void);

#endif
