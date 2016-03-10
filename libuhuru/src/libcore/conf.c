#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "conf.h"
#include "uhurup.h"
#include "confparser.h"
#include "os/dir.h"

#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if 0
finally, values are untyped and stored as arrays of strings
  enum uhuru_conf_value_type {
    TYPE_INT,
    TYPE_STRING,
    TYPE_LIST,
  };

  struct uhuru_conf_value {
    enum uhuru_conf_value_type type;
    union {
      int int_value;
      const char *string_value;
      struct {
	size_t length;
	const char **values;
      } list_value;
    };
  };
#endif

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

struct conf_entry {
  const char *key;
  GPtrArray *value;
};

struct conf_section {
  GPtrArray *keys;
};

struct uhuru_conf {
  GPtrArray *sections;
};

static struct conf_section *conf_section_get(struct uhuru_conf *conf, const char *section)
{
}

struct uhuru_conf *uhuru_conf_new(void)
{
  return NULL;
}

void uhuru_conf_free(struct uhuru_conf *conf)
{
}

static void conf_load_parser_cb(const char *section, const char *key, const char **argv, size_t length, void *user_data)
{
}

int uhuru_conf_load_file(struct uhuru_conf *conf, const char *path, uhuru_error **error)
{
  return 0;
}

int uhuru_conf_save_file(struct uhuru_conf *conf, const char *path, uhuru_error **error)
{
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

const char *uhuru_conf_get_value(struct uhuru_conf *conf, const char *section, const char *key)
{
  return NULL;
}

const char **uhuru_conf_get_list(struct uhuru_conf *conf, const char *section, const char *key, size_t *length)
{
  return NULL;
}

void uhuru_conf_set_value(struct uhuru_conf *conf, const char *section, const char *key, const char *value)
{
}

void uhuru_conf_set_list(struct uhuru_conf *conf, const char *section, const char *key, const char **list, size_t length)
{
}

