/***

Copyright (C) 2015, 2016, 2017 Teclib'

This file is part of Armadito.

Armadito is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito.  If not, see <http://www.gnu.org/licenses/>.

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

#ifndef _LIBARMADITO_CONF_H_
#define _LIBARMADITO_CONF_H_

#include <libarmadito/error.h>

enum a6o_conf_value_type {
	CONF_TYPE_VOID     = 0,
	CONF_TYPE_INT      = 1 << 0,
	CONF_TYPE_STRING   = 1 << 1,
	CONF_TYPE_LIST     = 1 << 2,
};

struct a6o_conf_value {
	enum a6o_conf_value_type type;
	union {
		int int_v;
		const char *str_v;
		struct {
			size_t len;
			const char **values;
		} list_v;
	} v;
};

void a6o_conf_value_init(struct a6o_conf_value *cv);

void a6o_conf_value_destroy(struct a6o_conf_value *cv);

#define a6o_conf_value_get_type(cv) ((cv)->type)

#define a6o_conf_value_is_int(cv) (a6o_conf_value_get_type(cv) == CONF_TYPE_INT)
#define a6o_conf_value_is_string(cv) (a6o_conf_value_get_type(cv) == CONF_TYPE_STRING)
#define a6o_conf_value_is_list(cv) (a6o_conf_value_get_type(cv) == CONF_TYPE_LIST)

#define a6o_conf_value_get_int(cv) ((cv)->v.int_v)
#define a6o_conf_value_get_string(cv) ((cv)->v.str_v)
#define a6o_conf_value_get_list(cv) ((cv)->v.list_v.values)
#define a6o_conf_value_get_list_len(cv) ((cv)->v.list_v.len)

void a6o_conf_value_set(struct a6o_conf_value *cv, const struct a6o_conf_value *src);

void a6o_conf_value_set_void(struct a6o_conf_value *cv);

void a6o_conf_value_set_int(struct a6o_conf_value *cv, unsigned int val);

void a6o_conf_value_set_string(struct a6o_conf_value *cv, const char *val);

void a6o_conf_value_set_list(struct a6o_conf_value *cv, const char **val, size_t len);

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
