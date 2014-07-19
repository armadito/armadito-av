#ifndef _LIBUMWSU_MODULE_H_
#define _LIBUMWSU_MODULE_H_

#include <libumwsu/status.h>

enum umw_mod_status {
  UMW_MOD_OK,
  UMW_MOD_INIT_ERROR,
  UMW_MOD_CONF_ERROR,
};

typedef enum umw_mod_status (*umw_mod_conf_init_t)(void **pmod_conf_data);
typedef enum umw_mod_status (*umw_mod_conf_t)(const char *key, const char *value, void *mod_conf_data);
typedef enum umw_mod_status (*umw_mod_init_t)(void **pmod_data);
typedef enum umw_scan_status (*umw_mod_scan)(const char *path, void *mod_data);
typedef enum umw_mod_status (*umw_mod_close)(void *mod_data);

#endif
