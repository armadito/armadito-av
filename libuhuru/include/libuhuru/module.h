#ifndef _LIBUHURU_MODULE_H_
#define _LIBUHURU_MODULE_H_

#include <libuhuru/status.h>

#include <time.h>

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

enum uhuru_update_status {
  UHURU_UPDATE_OK,
  UHURU_UPDATE_LATE,
  UHURU_UPDATE_CRITICAL,
  UHURU_UPDATE_NON_AVAILABLE,
};

struct uhuru_conf_entry {
  const char *directive;
  enum uhuru_mod_status (*conf_fun)(struct uhuru_module *module, const char *directive, const char **argv);
};

struct uhuru_base_info {
  const char *name;
  struct tm date;
  const char *version;
  unsigned int signature_count;
  const char *full_path;
};

struct uhuru_module_info {
  enum uhuru_update_status update_status;

  struct tm update_date;

  /* NULL terminated array of pointers to struct base_info */
  struct uhuru_base_info **base_infos;
};

struct uhuru_module {
  enum uhuru_mod_status (*init_fun)(struct uhuru_module *module);

  struct uhuru_conf_entry *conf_table;

  enum uhuru_mod_status (*post_init_fun)(struct uhuru_module *module);

  enum uhuru_file_status (*scan_fun)(struct uhuru_module *module, const char *path, const char *mime_type, char **pmod_report);

  enum uhuru_mod_status (*close_fun)(struct uhuru_module *module);

  enum uhuru_update_status (*update_check_fun)(struct uhuru_module *module, struct uhuru_module_info *info);

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
