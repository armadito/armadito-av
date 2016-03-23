#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "confparser.h"
#include "os/dir.h"
#include "os/string.h"

#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct key_entry {
  const char *key;
  struct uhuru_conf_value value;
};

struct section_entry {
  const char *section;
  GPtrArray *keys;
};

struct uhuru_conf {
  GPtrArray *sections;
};

void uhuru_conf_value_set(struct uhuru_conf_value *cv, const struct uhuru_conf_value *src)
{
  switch (src->type) {
  case CONF_TYPE_VOID:
    uhuru_conf_value_set_void(cv);
  case CONF_TYPE_INT:
    uhuru_conf_value_set_int(cv, uhuru_conf_value_get_int(src));
    break;
  case CONF_TYPE_STRING:
    uhuru_conf_value_set_string(cv, uhuru_conf_value_get_string(src));
    break;
  case CONF_TYPE_LIST:
    uhuru_conf_value_set_list(cv, uhuru_conf_value_get_list(src), uhuru_conf_value_get_list_len(src));
    break;
  }
}

void uhuru_conf_value_set_void(struct uhuru_conf_value *cv)
{
  cv->type = CONF_TYPE_VOID;
}

void uhuru_conf_value_set_int(struct uhuru_conf_value *cv, unsigned int val)
{
  cv->type = CONF_TYPE_INT;
  cv->v.int_v = val;
}

void uhuru_conf_value_set_string(struct uhuru_conf_value *cv, const char *val)
{
  cv->type = CONF_TYPE_STRING;
  cv->v.str_v = os_strdup(val);
}

void uhuru_conf_value_set_list(struct uhuru_conf_value *cv, const char **val, size_t len)
{
  int i;

  cv->type = CONF_TYPE_LIST;
  cv->v.list_v.len = len;
  cv->v.list_v.values = malloc((len + 1)*sizeof(char *));

  for(i = 0; i < len; i++)
    cv->v.list_v.values[i] = os_strdup(val[i]);

  cv->v.list_v.values[len] = NULL;
}

void uhuru_conf_value_destroy(struct uhuru_conf_value *cv)
{
  const char **p;

  switch (cv->type) {
  case CONF_TYPE_VOID:
  case CONF_TYPE_INT:
    break;
  case CONF_TYPE_STRING:
    if (cv->v.str_v != NULL)
      free((void *)cv->v.str_v);
    break;
  case CONF_TYPE_LIST:
    for(p = cv->v.list_v.values; *p != NULL; p++)
      free((void *)*p);
    free((void *)cv->v.list_v.values);
    break;
  }

  cv->type = CONF_TYPE_VOID;
}

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

static struct section_entry *section_entry_get(struct uhuru_conf *conf, const char *section)
{
  return array_search(conf->sections, (cmp_fun_t)section_entry_cmp, section);
}

static struct section_entry *section_entry_add(struct uhuru_conf *conf, const char *section)
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

  uhuru_conf_value_destroy(&k->value);

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

static struct key_entry *key_entry_get(struct uhuru_conf *conf, const char *section, const char *key)
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

static struct key_entry *key_entry_add(struct uhuru_conf *conf, const char *section, const char *key)
{
  struct section_entry *s = section_entry_get(conf, section);

  if (s == NULL)
    s = section_entry_add(conf, section);

  if (key_entry_get_sect(s, key) != NULL) {
    uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "duplicate key in configuration section %s: %s", section, key);
    return NULL;
  }

  return key_entry_add_sect(s, key);
}

struct uhuru_conf *uhuru_conf_new(void)
{
  struct uhuru_conf *conf = malloc(sizeof(struct uhuru_conf));  

  conf->sections = g_ptr_array_new_with_free_func((GDestroyNotify)section_entry_free);

  return conf;
}

void uhuru_conf_free(struct uhuru_conf *conf)
{
  g_ptr_array_free(conf->sections, TRUE);
}

typedef void (*uhuru_conf_fun_t)(const char *section, const char *key, struct uhuru_conf_value *value, void *user_data);

static void uhuru_conf_apply(struct uhuru_conf *conf, uhuru_conf_fun_t fun, void *user_data)
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

static int conf_load_parser_cb(const char *section, const char *key, struct uhuru_conf_value *value, void *user_data)
{
  struct uhuru_conf *conf = (struct uhuru_conf *)user_data;

  return uhuru_conf_add_value(conf, section, key, value);
}

int uhuru_conf_load_file(struct uhuru_conf *conf, const char *path, uhuru_error **error)
{
  struct uhuru_conf_parser *cp = uhuru_conf_parser_new(path, conf_load_parser_cb, conf);

  uhuru_conf_parser_parse(cp);

  uhuru_conf_parser_free(cp);

  return 0;
}

struct conf_save_data {
  FILE *f;
  const char *p;
};

static void conf_save_fun(const char *section, const char *key, struct uhuru_conf_value *value, void *user_data)
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
    fprintf(data->f, " %d", uhuru_conf_value_get_int(value));
    break;
  case CONF_TYPE_STRING:
    fprintf(data->f, " \"%s\"", uhuru_conf_value_get_string(value));
    break;
  case CONF_TYPE_LIST:
    for(n = 0, p = uhuru_conf_value_get_list(value); n < uhuru_conf_value_get_list_len(value); n++, p++)
      fprintf(data->f, "%s\"%s\"", (n == 0) ? " " : "; ", *p);
    break;
  }
  
  fprintf(data->f, "\n");
}

int uhuru_conf_save_file(struct uhuru_conf *conf, const char *path, uhuru_error **error)
{
  FILE *f;
  struct conf_save_data data;

  if (!strcmp(path, "-"))
    f = stdout;
  else
    f = fopen(path, "w");

  data.f = f;
  data.p = NULL;
  uhuru_conf_apply(conf, conf_save_fun, &data);

  if (f != stdout)
    fclose(f);
  
  return 0;
}

const char **uhuru_conf_get_sections(struct uhuru_conf *conf, size_t *p_len)
{
  const char **ret;
  size_t len, i;

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

const char **uhuru_conf_get_keys(struct uhuru_conf *conf, const char *section, size_t *p_len)
{
  struct section_entry *s;
  const char **ret;
  size_t len, i;

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

int uhuru_conf_has_key(struct uhuru_conf *conf, const char *section, const char *key)
{
  return key_entry_get(conf, section, key) != NULL;
}

int uhuru_conf_is_int(struct uhuru_conf *conf, const char *section, const char *key)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL)
    return 0;

  return uhuru_conf_value_get_type(&k->value) == CONF_TYPE_INT;
}

int uhuru_conf_is_string(struct uhuru_conf *conf, const char *section, const char *key)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL)
    return 0;

  return uhuru_conf_value_get_type(&k->value) == CONF_TYPE_STRING;
}

int uhuru_conf_is_list(struct uhuru_conf *conf, const char *section, const char *key)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL)
    return 0;

  return uhuru_conf_value_get_type(&k->value) == CONF_TYPE_LIST;
}

int uhuru_conf_get_uint(struct uhuru_conf *conf, const char *section, const char *key)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL || uhuru_conf_value_get_type(&k->value) != CONF_TYPE_INT)
    return -1;

  return uhuru_conf_value_get_int(&k->value);
}

const char *uhuru_conf_get_string(struct uhuru_conf *conf, const char *section, const char *key)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL || uhuru_conf_value_get_type(&k->value) != CONF_TYPE_STRING)
    return NULL;

  return uhuru_conf_value_get_string(&k->value);
}

const char **uhuru_conf_get_list(struct uhuru_conf *conf, const char *section, const char *key, size_t *p_len)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL || uhuru_conf_value_get_type(&k->value) != CONF_TYPE_LIST)
    return NULL;

  if (p_len != NULL)
    *p_len = uhuru_conf_value_get_list_len(&k->value);

  return uhuru_conf_value_get_list(&k->value);
}

int uhuru_conf_set_value(struct uhuru_conf *conf, const char *section, const char *key, struct uhuru_conf_value *value)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL)
    return 1;

  if (uhuru_conf_value_get_type(&k->value) != uhuru_conf_value_get_type(value)) {
    uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "cannot set key to a different type in configuration section %s: %s", section, key);
    return 1;
  }

  uhuru_conf_value_set(&k->value, value);

  return 0;
}

int uhuru_conf_set_uint(struct uhuru_conf *conf, const char *section, const char *key, unsigned int val)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL)
    return 1;

  if (uhuru_conf_value_get_type(&k->value) != CONF_TYPE_INT) {
    uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "cannot set key to a different type in configuration section %s: %s", section, key);
    return 1;
  }

  uhuru_conf_value_set_int(&k->value, val);

  return 0;
}

int uhuru_conf_set_string(struct uhuru_conf *conf, const char *section, const char *key, const char *val)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL)
    return 1;

  if (uhuru_conf_value_get_type(&k->value) != CONF_TYPE_STRING) {
    uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "cannot set key to a different type in configuration section %s: %s", section, key);
    return 1;
  }

  uhuru_conf_value_destroy(&k->value);
  uhuru_conf_value_set_string(&k->value, val);

  return 0;
}

int uhuru_conf_set_list(struct uhuru_conf *conf, const char *section, const char *key, const char **val, size_t len)
{
  struct key_entry *k = key_entry_get(conf, section, key);
  int i;

  if (k == NULL)
    return 1;

  if (uhuru_conf_value_get_type(&k->value) != CONF_TYPE_LIST) {
    uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "cannot set key to a different type in configuration section %s: %s", section, key);
    return 1;
  }

  uhuru_conf_value_destroy(&k->value);
  uhuru_conf_value_set_list(&k->value, val, len);

  return 0;
}

int uhuru_conf_add_value(struct uhuru_conf *conf, const char *section, const char *key, struct uhuru_conf_value *value)
{
  struct key_entry *k = key_entry_add(conf, section, key);

  if (k == NULL)
    return 1;

  uhuru_conf_value_set(&k->value, value);

  return 0;
}

int uhuru_conf_add_uint(struct uhuru_conf *conf, const char *section, const char *key, unsigned int val)
{
  struct key_entry *k = key_entry_add(conf, section, key);

  if (k == NULL)
    return 1;

  uhuru_conf_value_set_int(&k->value, val);

  return 0;
}

int uhuru_conf_add_string(struct uhuru_conf *conf, const char *section, const char *key, const char *val)
{
  struct key_entry *k = key_entry_add(conf, section, key);

  if (k == NULL)
    return 1;

  uhuru_conf_value_set_string(&k->value, val);

  return 0;
}

int uhuru_conf_add_list(struct uhuru_conf *conf, const char *section, const char *key, const char **val, size_t len)
{
  struct key_entry *k = key_entry_add(conf, section, key);
  int i;

  if (k == NULL)
    return 1;

  uhuru_conf_value_set_list(&k->value, val, len);

  return 0;
}
