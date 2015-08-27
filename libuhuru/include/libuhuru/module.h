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

struct uhuru;
struct uhuru_module;

struct uhuru_conf_entry {
  const char *directive;
  enum uhuru_mod_status (*conf_fun)(struct uhuru_module *module, const char *directive, const char **argv);
};

struct uhuru_module {
  enum uhuru_mod_status (*init_fun)(struct uhuru_module *module);

  struct uhuru_conf_entry *conf_table;

  enum uhuru_mod_status (*post_init_fun)(struct uhuru_module *module);

  enum uhuru_file_status (*scan_fun)(struct uhuru_module *module, const char *path, const char *mime_type, char **pmod_report);

  enum uhuru_mod_status (*close_fun)(struct uhuru_module *module);

  const char *name;

  size_t size;

  /* following fields will be initialized by the library */
  enum uhuru_mod_status status;

  void *data;

  struct uhuru *uhuru;
};

#ifdef __cplusplus
}
#endif

#endif
