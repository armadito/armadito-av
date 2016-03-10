/**
 * \file conf.h
 *
 * \brief definition of configuration management
 *
 * ...
 */

#ifndef _LIBUHURU_LIBCORE_CONF_H_
#define _LIBUHURU_LIBCORE_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libuhuru/libcore/error.h>

  struct uhuru_conf;

  struct uhuru_conf *uhuru_conf_new(void);

  void uhuru_conf_free(struct uhuru_conf *conf);

  int uhuru_conf_load_file(struct uhuru_conf *conf, const char *path, uhuru_error **error);

  int uhuru_conf_save_file(struct uhuru_conf *conf, const char *path, uhuru_error **error);

  const char **uhuru_conf_get_sections(struct uhuru_conf *conf, size_t *length);

  const char **uhuru_conf_get_keys(struct uhuru_conf *conf, const char *section, size_t *length);

  const char *uhuru_conf_get_value(struct uhuru_conf *conf, const char *section, const char *key);

  const char **uhuru_conf_get_list(struct uhuru_conf *conf, const char *section, const char *key, size_t *length);

  void uhuru_conf_set_value(struct uhuru_conf *conf, const char *section, const char *key, const char *value);

  void uhuru_conf_set_list(struct uhuru_conf *conf, const char *section, const char *key, const char **list, size_t length);

#ifdef __cplusplus
}
#endif

#endif
