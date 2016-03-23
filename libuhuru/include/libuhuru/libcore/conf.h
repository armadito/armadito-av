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
 * definition : key '=' value
 * key : STRING
 * value : int_value | string_value opt_string_list
 * int_value: INTEGER
 * string_value: STRING
 * opt_string_list : list_sep string_value opt_string_list | EMPTY
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

  const char **uhuru_conf_get_sections(struct uhuru_conf *conf, size_t *length);

  const char **uhuru_conf_get_keys(struct uhuru_conf *conf, const char *section, size_t *length);

  int uhuru_conf_has_key(struct uhuru_conf *conf, const char *section, const char *key);

  int uhuru_conf_is_int(struct uhuru_conf *conf, const char *section, const char *key);

  int uhuru_conf_is_string(struct uhuru_conf *conf, const char *section, const char *key);

  int uhuru_conf_is_list(struct uhuru_conf *conf, const char *section, const char *key);

  int uhuru_conf_get_uint(struct uhuru_conf *conf, const char *section, const char *key);

  const char *uhuru_conf_get_string(struct uhuru_conf *conf, const char *section, const char *key);

  const char **uhuru_conf_get_list(struct uhuru_conf *conf, const char *section, const char *key, size_t *length);

  int uhuru_conf_set_uint(struct uhuru_conf *conf, const char *section, const char *key, unsigned int value);

  int uhuru_conf_set_string(struct uhuru_conf *conf, const char *section, const char *key, const char *value);

  int uhuru_conf_set_list(struct uhuru_conf *conf, const char *section, const char *key, const char **list, size_t length);

  int uhuru_conf_add_uint(struct uhuru_conf *conf, const char *section, const char *key, unsigned int value);

  int uhuru_conf_add_string(struct uhuru_conf *conf, const char *section, const char *key, const char *value);

  int uhuru_conf_add_list(struct uhuru_conf *conf, const char *section, const char *key, const char **list, size_t length);

#ifdef __cplusplus
}
#endif

#endif
