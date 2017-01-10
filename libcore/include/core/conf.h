/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

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

#ifndef ARMADITO_CORE_CONF_H
#define ARMADITO_CORE_CONF_H

#include <core/error.h>

struct a6o_conf;

struct a6o_conf *a6o_conf_new(void);

void a6o_conf_free(struct a6o_conf *conf);

typedef void (*a6o_conf_fun_t)(const char *section, const char *key, struct a6o_conf_value *value, void *user_data);

void a6o_conf_apply(struct a6o_conf *conf, a6o_conf_fun_t fun, void *user_data);

int a6o_conf_load_file(struct a6o_conf *conf, const char *path, a6o_error **error);

int a6o_conf_save_file(struct a6o_conf *conf, const char *path, a6o_error **error);

const char **a6o_conf_get_sections(struct a6o_conf *conf, size_t *length);

const char **a6o_conf_get_keys(struct a6o_conf *conf, const char *section, size_t *length);

int a6o_conf_has_key(struct a6o_conf *conf, const char *section, const char *key);

enum a6o_conf_value_type a6o_conf_get_type(struct a6o_conf *conf, const char *section, const char *key);

int a6o_conf_is_int(struct a6o_conf *conf, const char *section, const char *key);

int a6o_conf_is_string(struct a6o_conf *conf, const char *section, const char *key);

int a6o_conf_is_list(struct a6o_conf *conf, const char *section, const char *key);

int a6o_conf_get_value(struct a6o_conf *conf, const char *section, const char *key, struct a6o_conf_value *value);

int a6o_conf_get_uint(struct a6o_conf *conf, const char *section, const char *key);

const char *a6o_conf_get_string(struct a6o_conf *conf, const char *section, const char *key);

const char **a6o_conf_get_list(struct a6o_conf *conf, const char *section, const char *key, size_t *length);

int a6o_conf_set_value(struct a6o_conf *conf, const char *section, const char *key, struct a6o_conf_value *value);

int a6o_conf_set_uint(struct a6o_conf *conf, const char *section, const char *key, unsigned int value);

int a6o_conf_set_string(struct a6o_conf *conf, const char *section, const char *key, const char *value);

int a6o_conf_set_list(struct a6o_conf *conf, const char *section, const char *key, const char **list, size_t length);

int a6o_conf_add_value(struct a6o_conf *conf, const char *section, const char *key, struct a6o_conf_value *value);

int a6o_conf_add_uint(struct a6o_conf *conf, const char *section, const char *key, unsigned int value);

int a6o_conf_add_string(struct a6o_conf *conf, const char *section, const char *key, const char *value);

int a6o_conf_add_list(struct a6o_conf *conf, const char *section, const char *key, const char **list, size_t length);

#endif
