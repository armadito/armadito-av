/**
 * \file conf.h
 *
 * \brief definition of configuration management
 *
 * ...
 *
 * The syntax of the configuration file is given by the following BNF: 
 *
 * configuration : section_list
 * section_list : section section_list | EMPTY
 * section : '[' section_name ']' definition_list
 * section_name : STRING
 * definition_list: definition definition_list | EMPTY
 * definition : key '=' value opt_value_list
 * key : STRING
 * opt_value_list : list_sep value opt_value_list | EMPTY
 * value : STRING
 * list_sep : ',' | ';' 
 *
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

  /* not yet implemented */
  const char **uhuru_conf_get_sections(struct uhuru_conf *conf, size_t *len);

  /* not yet implemented */
  const char **uhuru_conf_get_keys(struct uhuru_conf *conf, const char *section, size_t *len);

  void uhuru_conf_append(struct uhuru_conf *conf, const char *section, const char *key, const char **values, size_t len);

  typedef void (*uhuru_conf_fun_t)(const char *section, const char *key, const char **values, size_t len, void *user_data);
  
  void uhuru_conf_apply(struct uhuru_conf *conf, uhuru_conf_fun_t fun, void *user_data);

#ifdef __cplusplus
}
#endif

#endif


