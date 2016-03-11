#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "conf.h"
#include "uhurup.h"
#include "confparser.h"
#include "os/dir.h"
#include "os/string.h"

#include <assert.h>
#include <glib.h>
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

struct dict {
  GHashTable *bindings;
};

static struct dict *dict_new(GDestroyNotify value_free_fun)
{
  struct dict *d = malloc(sizeof(struct dict));

  d->bindings = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)free, value_free_fun);

  return d;
}     

static void dict_free(struct dict *d)
{
  g_hash_table_destroy(d->bindings);
  free(d);
}

static void *dict_lookup(struct dict *d, const char *name)
{
  return g_hash_table_lookup(d->bindings, name);
}

static int dict_insert(struct dict *d, const char *name, void *value)
{
  if (g_hash_table_contains(d->bindings, name))
    return 1;
      
  g_hash_table_insert(d->bindings, os_strdup(name), value);
  
  return 0;
}

typedef void (*dict_fun_t)(const char *key, void *value, void *user_data);

static void dict_foreach(struct dict *d, dict_fun_t fun, void *user_data)
{
  g_hash_table_foreach(d->bindings, (GHFunc)fun, user_data);
}

struct uhuru_conf {
  struct dict *sections;
};

struct uhuru_conf *uhuru_conf_new(void)
{
  struct uhuru_conf *c = malloc(sizeof(struct uhuru_conf));

  c->sections = dict_new((GDestroyNotify)dict_free);

  return c;
}

void uhuru_conf_free(struct uhuru_conf *conf)
{
  dict_free(conf->sections);
  free(conf);
}

static void value_free(gpointer data)
{
  g_ptr_array_free((GPtrArray *)data, TRUE);
}

static int conf_load_parser_cb(const char *section_name, const char *key, const char **value, size_t length, void *user_data)
{
  struct uhuru_conf *conf = (struct uhuru_conf *)user_data;
  struct dict *section;
  GPtrArray *value_array;
  int i;

  section = dict_lookup(conf->sections, section_name);

  if (section == NULL) {
    section = dict_new(value_free);
    dict_insert(conf->sections, section_name, section);
  }

  if (dict_lookup(section, key) != NULL) {
    uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "duplicate key '%s' in section '%s'", key, section_name);
    return 1;
  }

  value_array = g_ptr_array_new();
  for(i = 0; i < length; i++)
    g_ptr_array_add(value_array, os_strdup(value[i]));

  dict_insert(section, key, value_array);

  return 0;
}

int uhuru_conf_load_file(struct uhuru_conf *conf, const char *path, uhuru_error **error)
{
  struct uhuru_conf_parser *cp = uhuru_conf_parser_new(path, conf_load_parser_cb, conf);

  uhuru_conf_parser_parse(cp);

  uhuru_conf_parser_free(cp);

  return 0;
}

static void key_print_fun(const char *key, void *value, void *user_data)
{
  FILE *f = (FILE *)user_data;
  GPtrArray *values = (GPtrArray *)value;
  int i;
  const char **argv = (const char **)values->pdata;

  fprintf(f, "\"%s\"=", key);

  for(i = 0; i < values->len; i++)
    fprintf(f, "%c\"%s\" ", (i == 0) ? ' ' : ';', argv[i]);

  fprintf(f, "\n");
}

static void section_print_fun(const char *key, void *value, void *user_data)
{
  FILE *f = (FILE *)user_data;
  struct dict *section = (struct dict *)value;

  fprintf(f, "\n[%s]\n\n", key);

  dict_foreach(section, key_print_fun, f);
}

int uhuru_conf_save_file(struct uhuru_conf *conf, const char *path, uhuru_error **error)
{
  FILE *f;

  if (!strcmp(path, "-"))
    f = stdout;
  else
    f = fopen(path, "w");

  dict_foreach(conf->sections, section_print_fun, f);

  fclose(f);
  
  return 0;
}

const char **uhuru_conf_get_sections(struct uhuru_conf *conf, size_t *length)
{
  return NULL;
}

const char **uhuru_conf_get_keys(struct uhuru_conf *conf, const char *section_name, size_t *length)
{
  return NULL;
}

int uhuru_conf_is_int(struct uhuru_conf *conf, const char *section_name, const char *key)
{
  return 0;
}

int uhuru_conf_is_string(struct uhuru_conf *conf, const char *section_name, const char *key)
{
  return 0;
}

int uhuru_conf_is_list(struct uhuru_conf *conf, const char *section_name, const char *key)
{
  return 0;
}

int uhuru_conf_get_uint(struct uhuru_conf *conf, const char *section_name, const char *key)
{
  return -1;
}

const char *uhuru_conf_get_string(struct uhuru_conf *conf, const char *section_name, const char *key)
{
  return NULL;
}

const char **uhuru_conf_get_list(struct uhuru_conf *conf, const char *section_name, const char *key, size_t *length)
{
  return NULL;
}

void uhuru_conf_set_uint(struct uhuru_conf *conf, const char *section_name, const char *key, unsigned int value)
{
}

void uhuru_conf_set_string(struct uhuru_conf *conf, const char *section_name, const char *key, const char *value)
{
}

void uhuru_conf_set_list(struct uhuru_conf *conf, const char *section_name, const char *key, const char **list, size_t length)
{
}
