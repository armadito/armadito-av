#include "conf.h"
#include "uhurup.h"

#include <glib.h>
#include <stdlib.h>
#include <assert.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <setjmp.h>
#include <fcntl.h>

void conf_load(struct uhuru_module *mod)
{
  GKeyFile *key_file;
  char *filename;
  /* GError *error; */
  static char *dirs[] = { LIBUHURU_CONF_DIR, LIBUHURU_CONF_DIR "/conf.d", NULL};
  char **keys, **pkey;

  if (mod->conf_set == NULL)
    return;

  key_file = g_key_file_new();

  asprintf(&filename, "%s.conf", mod->name);

  if (!g_key_file_load_from_dirs(key_file, filename, (const gchar **)dirs, NULL, G_KEY_FILE_NONE, NULL)) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "cannot load conf file %s", mod->name);
    return;
  }

  keys = g_key_file_get_keys(key_file, mod->name, NULL, NULL);
  if (keys == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "cannot find group %s", mod->name);
    return;
  }

  for(pkey = keys; *pkey != NULL; pkey++) {
    char *value = g_key_file_get_value(key_file, mod->name, *pkey, NULL);

    assert(value != NULL);
    (*mod->conf_set)(mod->data, *pkey, value);
  }

  g_strfreev(keys);

  g_key_file_free(key_file);
  free(filename);
}

void conf_set(struct uhuru *uhuru, const char *mod_name, const char *key, const char *value)
{
  struct uhuru_module *mod = uhuru_get_module_by_name(uhuru, mod_name);

  if (mod == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "No such module: %s", mod_name);
    return;
  }

  (*mod->conf_set)(mod->data, key, value);
}

char *conf_get(struct uhuru *uhuru, const char *mod_name, const char *key)
{
  struct uhuru_module *mod = uhuru_get_module_by_name(uhuru, mod_name);

  if (mod == NULL) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "No such module: %s", mod_name);
    return;
  }

  return (*mod->conf_get)(mod->data, key);
}

static GScannerConfig conf_parser_config = {
  .cset_skip_characters = " \t\n",
  .cset_identifier_first = G_CSET_a_2_z "_" G_CSET_A_2_Z,
  .cset_identifier_nth = G_CSET_a_2_z "_0123456789" G_CSET_A_2_Z G_CSET_LATINS G_CSET_LATINC "-",
  .cpair_comment_single = "#\n",
  .case_sensitive = TRUE,		/* Should symbol lookup work case sensitive? */
  .skip_comment_multi = TRUE,		/* C like comment */
  .skip_comment_single = TRUE,		/* single line comment */
  .scan_comment_multi = TRUE,		/* scan multi line comments? */
  .scan_identifier = TRUE,
  .scan_identifier_1char = TRUE,
  .scan_identifier_NULL = FALSE,
  .scan_symbols = TRUE,
  .scan_binary = FALSE,
  .scan_octal = TRUE,
  .scan_float = FALSE,
  .scan_hex = TRUE,			/* '0x0ff0' */
  .scan_hex_dollar = FALSE,		/* '$0ff0' */
  .scan_string_sq = FALSE,		/* string: 'anything' */
  .scan_string_dq = TRUE,		/* string: "\\-escapes!\n" */
  .numbers_2_int = TRUE,		/* bin, octal, hex => int */
  .int_2_float = FALSE,			/* int => G_TOKEN_FLOAT? */
  .identifier_2_string = TRUE,		/* identifier returned as strings */
  .char_2_token = TRUE,
  .symbol_2_token = FALSE,
  .scope_0_fallback = FALSE,
  .store_int64 = FALSE,   
};

struct uhuru_conf_parser {
  GScanner *scanner;
  int fd;
  guint lookahead_token;
  jmp_buf env;
};

struct uhuru_conf_parser *uhuru_conf_parser_new(const char *filename)
{
  struct uhuru_conf_parser *cp;

  cp = (struct uhuru_conf_parser *)malloc(sizeof(struct uhuru_conf_parser));
  assert(cp != NULL);

  cp->scanner = g_scanner_new(&conf_parser_config);
  
  cp->fd = open(filename, O_RDONLY);
  if (cp->fd < 0) {
    g_log(NULL, G_LOG_LEVEL_WARNING, "cannot open conf file %s", filename);
  }

  g_scanner_input_file(cp->scanner, cp->fd);

  return cp;
}

/* not defined in glib.h */
#define G_TOKEN_SEMI_COLON  ';'

static const char *token_str(guint token)
{
  if (token == G_TOKEN_IDENTIFIER) 
    return "identifier";
  else if (token == G_TOKEN_STRING) 
    return "string";
  else if (token == G_TOKEN_INT)
    return "integer";
  else if (token < G_TOKEN_NONE) {
    /* memory leak, I know, I know, but this function is called only in case of syntax error... */
    char *tmp = (char *)malloc(2);

    tmp[0] = (char)token;
    tmp[1] = '\0';

    return tmp;
  }

  return "???";
}

static void syntax_error(struct uhuru_conf_parser *cp, guint token) 
{
  g_log(NULL, G_LOG_LEVEL_WARNING, "syntax error: at line %d position %d expecting '%s' got '%s'\n", 
	g_scanner_cur_line(cp->scanner),
	g_scanner_cur_position(cp->scanner),
	token_str(token), 
	token_str(cp->lookahead_token));

  longjmp(cp->env, 1);
}

static void accept(struct uhuru_conf_parser *cp, guint token)
{
  if (cp->lookahead_token == token) {
    if (cp->lookahead_token != G_TOKEN_EOF)
      cp->lookahead_token = g_scanner_get_next_token(cp->scanner);
  } else
    syntax_error(cp, token);
}

/*
  the grammar:

  configuration : group_list
  group_list : group group_list | EMPTY
  group : '[' STRING ']' definition_list
  definition_list: definition definition_list | EMPTY
  definition : STRING '=' value opt_value_list 
  opt_value_list : list_sep value opt_value_list | EMPTY
  value : STRING | INT
  list_sep : ',' | ';' 
*/

static void r_configuration(struct uhuru_conf_parser *cp);
static void r_group_list(struct uhuru_conf_parser *cp);
static void r_group(struct uhuru_conf_parser *cp);
static void r_definition_list(struct uhuru_conf_parser *cp);
static void r_definition(struct uhuru_conf_parser *cp);
static void r_opt_value_list(struct uhuru_conf_parser *cp);
static void r_value(struct uhuru_conf_parser *cp);
static void r_list_sep(struct uhuru_conf_parser *cp);

/* configuration : group_list */
static void r_configuration(struct uhuru_conf_parser *cp)
{
  r_group_list(cp);
}

/* group_list : group group_list | EMPTY */
static void r_group_list(struct uhuru_conf_parser *cp)
{
  if (cp->lookahead_token == G_TOKEN_LEFT_BRACE) {
    r_group(cp);
    r_group_list(cp);
  }
}

/* group : '[' STRING ']' definition_list */
static void r_group(struct uhuru_conf_parser *cp)
{
  accept(cp, G_TOKEN_LEFT_BRACE);
  accept(cp, G_TOKEN_STRING);
  accept(cp, G_TOKEN_RIGHT_BRACE);
  r_definition_list(cp);
}

/* definition_list: definition definition_list | EMPTY */
static void r_definition_list(struct uhuru_conf_parser *cp)
{
  if (cp->lookahead_token == G_TOKEN_STRING) {
    r_definition(cp);
    r_definition_list(cp);
  }
}

/* definition : STRING '=' value opt_value_list  */
static void r_definition(struct uhuru_conf_parser *cp)
{
  accept(cp, G_TOKEN_STRING);
  accept(cp, G_TOKEN_EQUAL_SIGN);
  r_value(cp);
  r_opt_value_list(cp);
}

/* opt_value_list : list_sep value opt_value_list | EMPTY */
static void r_opt_value_list(struct uhuru_conf_parser *cp)
{
  if (cp->lookahead_token == G_TOKEN_COMMA || cp->lookahead_token == G_TOKEN_SEMI_COLON) {
    r_list_sep(cp);
    r_value(cp);
    r_opt_value_list(cp);
  }
}

/* value : STRING | INT */
static void r_value(struct uhuru_conf_parser *cp)
{
  if (cp->lookahead_token == G_TOKEN_STRING)
    accept(cp, G_TOKEN_STRING);
  else
    accept(cp, G_TOKEN_INT);
}

/* list_sep : ',' | ';'  */
static void r_list_sep(struct uhuru_conf_parser *cp)
{
  if (cp->lookahead_token == G_TOKEN_COMMA)
    accept(cp, G_TOKEN_COMMA);
  else
    accept(cp, G_TOKEN_SEMI_COLON);
}


int uhuru_conf_parser_parse(struct uhuru_conf_parser *cp)
{
  int ret = 1;

  cp->lookahead_token = g_scanner_get_next_token(cp->scanner);

  if (!setjmp(cp->env)) {
    r_configuration(cp);

    accept(cp, G_TOKEN_EOF);

    ret = 0;
  }

  return ret;
}

void uhuru_conf_parser_free(struct uhuru_conf_parser *cp)
{
  g_scanner_destroy(cp->scanner);

  close(cp->fd);

  free(cp);
}

