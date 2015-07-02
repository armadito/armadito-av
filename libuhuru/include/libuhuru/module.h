#ifndef _LIBUHURU_MODULE_H_
#define _LIBUHURU_MODULE_H_

#include <libuhuru/status.h>

#ifdef __cplusplus
extern "C" {
#endif

enum uhuru_mod_status {
  UHURU_MOD_OK,
  UHURU_MOD_INIT_ERROR,
  UHURU_MOD_CONF_ERROR,
};

/* typedef enum uhuru_mod_status (*uhuru_mod_conf_init_t)(void **pmod_conf_data); */
typedef enum uhuru_mod_status (*uhuru_mod_init_t)(void **pmod_data);
typedef enum uhuru_mod_status (*uhuru_mod_conf_t)(void *mod_data, const char *key, const char *value);
typedef enum uhuru_file_status (*uhuru_mod_scan_t)(const char *path, const char *mime_type, void *mod_data, char **pmod_report);
typedef enum uhuru_mod_status (*uhuru_mod_close)(void *mod_data);

#ifdef __cplusplus
}
#endif

#endif
