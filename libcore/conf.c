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

#include "armadito-config.h"

#include <libarmadito/armadito.h>

#include "string_p.h"
#include "confparser.h"
#include "core/dir.h"
#include "core/file.h"
#include "core/conf.h"

#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct key_entry {
	const char *key;
	struct a6o_conf_value value;
};

struct section_entry {
	const char *section;
	GPtrArray *keys;
};

struct a6o_conf {
	GPtrArray *sections;
};

typedef int (*cmp_fun_t)(void *element, const char *name);

/* a little utility function for GPtrArrays */
static void *array_search(GPtrArray *array, cmp_fun_t cmp_fun, const char *name)
{
	int i;

	for (i = 0; i < array->len; i++) {
		void *element = g_ptr_array_index(array, i);

		if (!(*cmp_fun)(element, name))
			return element;
	}

	return NULL;
}

static void key_entry_free(struct key_entry *k);

static struct section_entry *section_entry_new(const char *section)
{
	struct section_entry *s = malloc(sizeof(struct section_entry));

	s->section = os_strdup(section);

	s->keys = g_ptr_array_new_with_free_func((GDestroyNotify)key_entry_free);

	return s;
}

static void section_entry_free(struct section_entry *s)
{
	free((void *)s->section);

	g_ptr_array_free(s->keys, TRUE);

	free((void *)s);
}

static int section_entry_cmp(struct section_entry *s, const char *name)
{
	return strcmp(s->section, name);
}

static struct section_entry *section_entry_get(struct a6o_conf *conf, const char *section)
{
	return array_search(conf->sections, (cmp_fun_t)section_entry_cmp, section);
}

static struct section_entry *section_entry_add(struct a6o_conf *conf, const char *section)
{
	struct section_entry *s = section_entry_new(section);

	g_ptr_array_add(conf->sections, s);

	return s;
}

static struct key_entry *key_entry_new(const char *key)
{
	struct key_entry *k = malloc(sizeof(struct key_entry));

	k->key = os_strdup(key);

	return k;
}

static void key_entry_free(struct key_entry *k)
{
	free((void *)k->key);

	a6o_conf_value_destroy(&k->value);

	free((void *)k);
}

static int key_entry_cmp(struct key_entry *k, const char *name)
{
	return strcmp(k->key, name);
}

static struct key_entry *key_entry_get_sect(struct section_entry *s, const char *key)
{
	return array_search(s->keys, (cmp_fun_t)key_entry_cmp, key);
}

static struct key_entry *key_entry_get(struct a6o_conf *conf, const char *section, const char *key)
{
	struct section_entry *s = section_entry_get(conf, section);

	if (s == NULL)
		return NULL;

	return key_entry_get_sect(s, key);
}

static struct key_entry *key_entry_add_sect(struct section_entry *s, const char *key)
{
	struct key_entry *k = key_entry_new(key);

	g_ptr_array_add(s->keys, k);

	return k;
}

static struct key_entry *key_entry_add(struct a6o_conf *conf, const char *section, const char *key)
{
	struct section_entry *s = section_entry_get(conf, section);

	if (s == NULL)
		s = section_entry_add(conf, section);

	if (key_entry_get_sect(s, key) != NULL) {
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "duplicate key in configuration section %s: %s", section, key);
		return NULL;
	}

	return key_entry_add_sect(s, key);
}

struct a6o_conf *a6o_conf_new(void)
{
	struct a6o_conf *conf = malloc(sizeof(struct a6o_conf));

	conf->sections = g_ptr_array_new_with_free_func((GDestroyNotify)section_entry_free);

	return conf;
}

void a6o_conf_free(struct a6o_conf *conf)
{
	g_ptr_array_free(conf->sections, TRUE);
}

void a6o_conf_apply(struct a6o_conf *conf, a6o_conf_fun_t fun, void *user_data)
{
	int i;

	for(i = 0; i < conf->sections->len; i++) {
		struct section_entry *s = g_ptr_array_index(conf->sections, i);
		int j;

		for(j = 0; j < s->keys->len; j++) {
			struct key_entry *k = g_ptr_array_index(s->keys, j);

			(*fun)(s->section, k->key, &k->value, user_data);
		}
	}
}

static int conf_load_parser_cb(const char *section, const char *key, struct a6o_conf_value *value, void *user_data)
{
	struct a6o_conf *conf = (struct a6o_conf *)user_data;

	return a6o_conf_add_value(conf, section, key, value);
}

int a6o_conf_load_file(struct a6o_conf *conf, const char *path)
{
	struct a6o_conf_parser *cp = a6o_conf_parser_new(path, conf_load_parser_cb, conf);

	a6o_conf_parser_parse(cp);

	a6o_conf_parser_free(cp);

	return 0;
}

struct conf_save_data {
	FILE *f;
	const char *p;
};

static void conf_save_fun(const char *section, const char *key, struct a6o_conf_value *value, void *user_data)
{
	struct conf_save_data *data = (struct conf_save_data *)user_data;
	int n;
	const char **p;

	if (data->p == NULL || strcmp(data->p, section)) {
		fprintf(data->f, "\n[%s]\n\n", section);
		data->p = section;
	}

	fprintf(data->f, "%s =", key);

	switch (value->type) {
	case CONF_TYPE_INT:
		fprintf(data->f, " %d", a6o_conf_value_get_int(value));
		break;
	case CONF_TYPE_STRING:
		fprintf(data->f, " \"%s\"", a6o_conf_value_get_string(value));
		break;
	case CONF_TYPE_LIST:
		for(n = 0, p = a6o_conf_value_get_list(value); n < a6o_conf_value_get_list_len(value); n++, p++)
			fprintf(data->f, "%s\"%s\"", (n == 0) ? " " : "; ", *p);
		break;
	}

	fprintf(data->f, "\n");
}

int a6o_conf_save_file(struct a6o_conf *conf, const char *path)
{
	FILE *f = NULL;
	struct conf_save_data data;

	if (!strcmp(path, "-"))
		f = stdout;
	else
		f = os_fopen(path, "w");

	data.f = f;
	data.p = NULL;
	a6o_conf_apply(conf, conf_save_fun, &data);

	if (f != stdout)
		fclose(f);

	return 0;
}

const char **a6o_conf_get_sections(struct a6o_conf *conf, size_t *p_len)
{
	const char **ret;
	size_t len;
	size_t i;

	len = conf->sections->len;
	ret = malloc((len + 1)*sizeof(const char *));

	for(i = 0; i < len; i++) {
		struct section_entry *s = g_ptr_array_index(conf->sections, i);

		ret[i] = os_strdup(s->section);
	}

	ret[len] = NULL;

	if (p_len != NULL)
		*p_len = len;

	return ret;
}

const char **a6o_conf_get_keys(struct a6o_conf *conf, const char *section, size_t *p_len)
{
	struct section_entry *s;
	const char **ret;
	size_t len;
	size_t i;

	s = section_entry_get(conf, section);
	if (s == NULL)
		return NULL;

	len = s->keys->len;
	ret = malloc((len + 1)*sizeof(const char *));

	for(i = 0; i < len; i++) {
		struct key_entry *k = g_ptr_array_index(s->keys, i);

		ret[i] = os_strdup(k->key);
	}

	ret[len] = NULL;

	if (p_len != NULL)
		*p_len = len;

	return ret;
}

int a6o_conf_has_key(struct a6o_conf *conf, const char *section, const char *key)
{
	return key_entry_get(conf, section, key) != NULL;
}

enum a6o_conf_value_type a6o_conf_get_type(struct a6o_conf *conf, const char *section, const char *key)
{
	struct key_entry *k = key_entry_get(conf, section, key);

	if (k == NULL)
		return CONF_TYPE_VOID;

	return a6o_conf_value_get_type(&k->value);
}

int a6o_conf_is_int(struct a6o_conf *conf, const char *section, const char *key)
{
	struct key_entry *k = key_entry_get(conf, section, key);

	if (k == NULL)
		return 0;

	return a6o_conf_value_get_type(&k->value) == CONF_TYPE_INT;
}

int a6o_conf_is_string(struct a6o_conf *conf, const char *section, const char *key)
{
	struct key_entry *k = key_entry_get(conf, section, key);

	if (k == NULL)
		return 0;

	return a6o_conf_value_get_type(&k->value) == CONF_TYPE_STRING;
}

int a6o_conf_is_list(struct a6o_conf *conf, const char *section, const char *key)
{
	struct key_entry *k = key_entry_get(conf, section, key);

	if (k == NULL)
		return 0;

	return a6o_conf_value_get_type(&k->value) == CONF_TYPE_LIST;
}

int a6o_conf_get_value(struct a6o_conf *conf, const char *section, const char *key, struct a6o_conf_value *value)
{
	struct key_entry *k = key_entry_get(conf, section, key);

	if (k == NULL)
		return 0;

	a6o_conf_value_set(value, &k->value);

	return 1;
}

int a6o_conf_get_uint(struct a6o_conf *conf, const char *section, const char *key)
{
	struct key_entry *k = key_entry_get(conf, section, key);

	if (k == NULL || a6o_conf_value_get_type(&k->value) != CONF_TYPE_INT)
		return -1;

	return a6o_conf_value_get_int(&k->value);
}

const char *a6o_conf_get_string(struct a6o_conf *conf, const char *section, const char *key)
{
	struct key_entry *k = key_entry_get(conf, section, key);

	if (k == NULL || a6o_conf_value_get_type(&k->value) != CONF_TYPE_STRING)
		return NULL;

	return a6o_conf_value_get_string(&k->value);
}

const char **a6o_conf_get_list(struct a6o_conf *conf, const char *section, const char *key, size_t *p_len)
{
	struct key_entry *k = key_entry_get(conf, section, key);

	if (k == NULL || a6o_conf_value_get_type(&k->value) != CONF_TYPE_LIST)
		return NULL;

	if (p_len != NULL)
		*p_len = a6o_conf_value_get_list_len(&k->value);

	return a6o_conf_value_get_list(&k->value);
}

int a6o_conf_set_value(struct a6o_conf *conf, const char *section, const char *key, struct a6o_conf_value *value)
{
	struct key_entry *k = key_entry_get(conf, section, key);

	if (k == NULL)
		return 1;

	if (a6o_conf_value_get_type(&k->value) != a6o_conf_value_get_type(value)) {
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "cannot set key to a different type in configuration section %s: %s", section, key);
		return 1;
	}

	a6o_conf_value_set(&k->value, value);

	return 0;
}

int a6o_conf_set_uint(struct a6o_conf *conf, const char *section, const char *key, unsigned int val)
{
	struct key_entry *k = key_entry_get(conf, section, key);

	if (k == NULL)
		return 1;

	if (a6o_conf_value_get_type(&k->value) != CONF_TYPE_INT) {
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "cannot set key to a different type in configuration section %s: %s", section, key);
		return 1;
	}

	a6o_conf_value_set_int(&k->value, val);

	return 0;
}

int a6o_conf_set_string(struct a6o_conf *conf, const char *section, const char *key, const char *val)
{
	struct key_entry *k = key_entry_get(conf, section, key);

	if (k == NULL)
		return 1;

	if (a6o_conf_value_get_type(&k->value) != CONF_TYPE_STRING) {
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "cannot set key to a different type in configuration section %s: %s", section, key);
		return 1;
	}

	a6o_conf_value_destroy(&k->value);
	a6o_conf_value_set_string(&k->value, val);

	return 0;
}

int a6o_conf_set_list(struct a6o_conf *conf, const char *section, const char *key, const char **val, size_t len)
{
	struct key_entry *k = key_entry_get(conf, section, key);

	if (k == NULL)
		return 1;

	if (a6o_conf_value_get_type(&k->value) != CONF_TYPE_LIST) {
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "cannot set key to a different type in configuration section %s: %s", section, key);
		return 1;
	}

	a6o_conf_value_destroy(&k->value);
	a6o_conf_value_set_list(&k->value, val, len);

	return 0;
}

int a6o_conf_add_value(struct a6o_conf *conf, const char *section, const char *key, struct a6o_conf_value *value)
{
	struct key_entry *k = key_entry_add(conf, section, key);

	if (k == NULL)
		return 1;

	a6o_conf_value_set(&k->value, value);

	return 0;
}

int a6o_conf_add_uint(struct a6o_conf *conf, const char *section, const char *key, unsigned int val)
{
	struct key_entry *k = key_entry_add(conf, section, key);

	if (k == NULL)
		return 1;

	a6o_conf_value_set_int(&k->value, val);

	return 0;
}

int a6o_conf_add_string(struct a6o_conf *conf, const char *section, const char *key, const char *val)
{
	struct key_entry *k = key_entry_add(conf, section, key);

	if (k == NULL)
		return 1;

	a6o_conf_value_set_string(&k->value, val);

	return 0;
}

int a6o_conf_add_list(struct a6o_conf *conf, const char *section, const char *key, const char **val, size_t len)
{
	struct key_entry *k = key_entry_add(conf, section, key);

	if (k == NULL)
		return 1;

	a6o_conf_value_set_list(&k->value, val, len);

	return 0;
}
