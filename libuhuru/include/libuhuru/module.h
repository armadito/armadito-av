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
  UHURU_MOD_CLOSE_ERROR,
};

struct uhuru_conf_entry {
  const char *key;
  enum uhuru_mod_status (*conf_set_fun)(void *mod_data, const char *key, int argc, const char **argv);
  /* the 'get' function can be set to NULL if module is too lazy to return its configuration */
  enum uhuru_mod_status (*conf_get_fun)(void *mod_data, const char *key, int *pargc, const char ***pargv);
};

struct uhuru_module {
  enum uhuru_mod_status (*init_fun)(void **pmod_data);

  struct uhuru_conf_entry *conf_table;

  enum uhuru_mod_status (*post_init_fun)(void *mod_data);

  enum uhuru_file_status (*scan_fun)(const char *path, const char *mime_type, void *mod_data, char **pmod_report);

  enum uhuru_mod_status (*close_fun)(void *mod_data);

  const char *name;

  enum uhuru_mod_status mod_status;

  void *mod_data;
};

#ifdef __cplusplus
}
#endif

#endif
