#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "confp.h"
#include "confparser.h"
#include "os/dir.h"
#include "os/string.h"

#include <assert.h>
#include <glib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* old function, empty for now just to compile */
void conf_load_file(struct uhuru *uhuru, const char *filename)
{
  uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "conf_load_file() stub");
}

/* old function, empty for now just to compile */
void conf_load_path(struct uhuru *uhuru, const char *path)
{
  uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "conf_load_path() stub");
}

enum value_type {
  TYPE_INT,
  TYPE_STR,
  TYPE_LIST,
};

struct key_value {
  enum value_type type;
  union {
    int int_v;
    const char *str_v;
    struct {
      size_t len;
      const char **values;
    } list_v;
  } v;
};

struct key_entry {
  const char *key;
  struct key_value value;
};

struct section_entry {
  const char *section;
  GPtrArray *keys;
};

struct uhuru_conf {
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

static void key_value_set_int(struct key_value *kv, unsigned int val)
{
  kv->type = TYPE_INT;
  kv->v.int_v = val;
}

static void key_value_set_string(struct key_value *kv, const char *val)
{
  kv->type = TYPE_STR;
  kv->v.str_v = os_strdup(val);
}

static void key_value_set_list(struct key_value *kv, const char **val, size_t len)
{
  int i;

  kv->type = TYPE_LIST;
  kv->v.list_v.len = len;
  kv->v.list_v.values = malloc((len + 1)*sizeof(char *));

  for(i = 0; i < len; i++)
    kv->v.list_v.values[i] = os_strdup(val[i]);

  kv->v.list_v.values[len] = NULL;
}

static void key_value_destroy(struct key_value *kv)
{
  const char **p;

  switch (kv->type) {
  case TYPE_INT:
    break;
  case TYPE_STR:
    free((void *)kv->v.str_v);
    break;
  case TYPE_LIST:
    for(p = kv->v.list_v.values; *p != NULL; p++)
      free((void *)*p);
    free((void *)kv->v.list_v.values);
    break;
  }
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

  key_value_destroy(&k->value);

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

  return array_search(s->keys, (cmp_fun_t)key_entry_cmp, key);
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

typedef void (*uhuru_conf_fun_t)(const char *section, const char *key, struct key_value *value, void *user_data);

void uhuru_conf_apply(struct uhuru_conf *conf, uhuru_conf_fun_t fun, void *user_data)
{
  int i;

  fprintf(stderr, "conf: %d sections\n", conf->sections->len);
  
  for(i = 0; i < conf->sections->len; i++) {
    struct section_entry *s = g_ptr_array_index(conf->sections, i);
    int j;

    fprintf(stderr, "conf: section %d %s: %d keys\n", i, s->section, s->keys->len);

    for(j = 0; j < s->keys->len; j++) {
      struct key_entry *k = g_ptr_array_index(s->keys, j);
	
      (*fun)(s->section, k->key, &k->value, user_data); 
    }     
  }
}

static int conf_load_parser_cb(void *user_data, const char *section, const char *key, enum conf_parser_value_type type, ...)
{
  struct uhuru_conf *conf = (struct uhuru_conf *)user_data;
  va_list args;
  unsigned int v;
  const char *s;
  const char **argv;
  size_t len;
  int ret;

  fprintf(stderr, "parser_cb: adding section %s key %s\n", section, key);

  va_start(args, type);

  switch(type) {
  case CP_VALUE_INT:
    v = va_arg(args, unsigned int);
    ret = uhuru_conf_add_uint(conf, section, key, v);
    break;
  case CP_VALUE_STRING:
    s = va_arg(args, const char *);
    ret = uhuru_conf_add_string(conf, section, key, s);
    break;
  case CP_VALUE_LIST:    
    argv = va_arg(args, const char **);
    len = va_arg(args, size_t);
    ret = uhuru_conf_add_list(conf, section, key, argv, len);
    break;
  }

  va_end(args);

  return ret;
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

static void conf_save_fun(const char *section, const char *key, struct key_value *value, void *user_data)
{
  struct conf_save_data *data = (struct conf_save_data *)user_data;
  int n;

  if (data->p == NULL || strcmp(data->p, section)) {
    fprintf(data->f, "\n[%s]\n\n", section);
    data->p = section;
  }

  fprintf(data->f, "%s =", key);

  switch (value->type) {
  case TYPE_INT:
    fprintf(data->f, "%d", value->v.int_v);
    break;
  case TYPE_STR:
    fprintf(data->f, "\"%s\"", value->v.str_v);
    break;
  case TYPE_LIST:
    for(n = 0; n < value->v.list_v.len; n++)
      fprintf(data->f, "%s\"%s\"", (n == 0) ? " " : "; ", value->v.list_v.values[n]);
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

const char **uhuru_conf_get_sections(struct uhuru_conf *conf, size_t *length)
{
  return NULL;
}

const char **uhuru_conf_get_keys(struct uhuru_conf *conf, const char *section, size_t *length)
{
  return NULL;
}

int uhuru_conf_has_key(struct uhuru_conf *conf, const char *section, const char *key)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  return k != NULL;
}

int uhuru_conf_is_int(struct uhuru_conf *conf, const char *section, const char *key)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL)
    return 0;

  return k->value.type == TYPE_INT;
}

int uhuru_conf_is_string(struct uhuru_conf *conf, const char *section, const char *key)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL)
    return 0;

  return k->value.type == TYPE_STR;
}

int uhuru_conf_is_list(struct uhuru_conf *conf, const char *section, const char *key)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL)
    return 0;

  return k->value.type == TYPE_LIST;
}

int uhuru_conf_get_uint(struct uhuru_conf *conf, const char *section, const char *key)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL || k->value.type != TYPE_INT)
    return -1;

  return k->value.v.int_v;
}

const char *uhuru_conf_get_string(struct uhuru_conf *conf, const char *section, const char *key)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL || k->value.type != TYPE_STR)
    return NULL;

  return k->value.v.str_v;
}

const char **uhuru_conf_get_list(struct uhuru_conf *conf, const char *section, const char *key, size_t *p_len)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL || k->value.type != TYPE_LIST)
    return NULL;

  if (p_len != NULL)
    *p_len = k->value.v.list_v.len;

  return k->value.v.list_v.values;
}

int uhuru_conf_set_uint(struct uhuru_conf *conf, const char *section, const char *key, unsigned int val)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL)
    return 1;

  if (k->value.type != TYPE_INT) {
    uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "cannot set key to a different type in configuration section %s: %s", section, key);
    return 1;
  }

  key_value_set_int(&k->value, val);

  return 0;
}

int uhuru_conf_set_string(struct uhuru_conf *conf, const char *section, const char *key, const char *val)
{
  struct key_entry *k = key_entry_get(conf, section, key);

  if (k == NULL)
    return 1;

  if (k->value.type != TYPE_STR) {
    uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "cannot set key to a different type in configuration section %s: %s", section, key);
    return 1;
  }

  key_value_destroy(&k->value);
  key_value_set_string(&k->value, val);

  return 0;
}

int uhuru_conf_set_list(struct uhuru_conf *conf, const char *section, const char *key, const char **val, size_t len)
{
  struct key_entry *k = key_entry_get(conf, section, key);
  int i;

  if (k == NULL)
    return 1;

  if (k->value.type != TYPE_LIST) {
    uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "cannot set key to a different type in configuration section %s: %s", section, key);
    return 1;
  }

  key_value_destroy(&k->value);
  key_value_set_list(&k->value, val, len);

  return 0;
}

int uhuru_conf_add_uint(struct uhuru_conf *conf, const char *section, const char *key, unsigned int val)
{
  struct key_entry *k = key_entry_add(conf, section, key);

  if (k == NULL)
    return 1;

  key_value_set_int(&k->value, val);

  return 0;
}

int uhuru_conf_add_string(struct uhuru_conf *conf, const char *section, const char *key, const char *val)
{
  struct key_entry *k = key_entry_add(conf, section, key);

  if (k == NULL)
    return 1;

  key_value_set_string(&k->value, val);

  return 0;
}

int uhuru_conf_add_list(struct uhuru_conf *conf, const char *section, const char *key, const char **val, size_t len)
{
  struct key_entry *k = key_entry_add(conf, section, key);
  int i;

  if (k == NULL)
    return 1;

  key_value_set_list(&k->value, val, len);

  return 0;
}
