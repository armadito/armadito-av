#ifndef _LIBUHURU_LIBCORE_MODULE_H_
#define _LIBUHURU_LIBCORE_MODULE_H_

#include <libuhuru/common/status.h>
#include <libuhuru/libcore/conf.h>
#include <libuhuru/libcore/info.h>

#ifdef __cplusplus
extern "C" {
#endif

struct uhuru;
struct uhuru_module;

enum uhuru_mod_status {
  UHURU_MOD_OK,
  UHURU_MOD_INIT_ERROR,
  UHURU_MOD_CONF_ERROR,
  UHURU_MOD_CLOSE_ERROR,
};

struct uhuru_conf_entry {
  const char *key;
  enum uhuru_conf_value_type type;
  enum uhuru_mod_status (*conf_fun)(struct uhuru_module *module, const char *key, struct uhuru_conf_value *value);
};

struct uhuru_module {
  enum uhuru_mod_status (*init_fun)(struct uhuru_module *module);

  struct uhuru_conf_entry *conf_table;

  enum uhuru_mod_status (*post_init_fun)(struct uhuru_module *module);

  enum uhuru_file_status (*scan_fun)(struct uhuru_module *module, int fd, const char *path, const char *mime_type, char **pmod_report);

  enum uhuru_mod_status (*close_fun)(struct uhuru_module *module);

  enum uhuru_update_status (*info_fun)(struct uhuru_module *module, struct uhuru_module_info *info);

  const char **supported_mime_types;

  const char *name;

  size_t size;

  enum uhuru_mod_status status;

  void *data;

  struct uhuru *uhuru;
};

#ifdef __cplusplus
}
#endif

#endif
