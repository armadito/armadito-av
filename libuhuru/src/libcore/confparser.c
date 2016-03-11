#include <libuhuru/core.h>

#include "libuhuru-config.h"

#include "confparser.h"
#include "os/string.h"

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
#include <ctype.h>
#include <string.h>

/*
 * the lexical analyzer
 */
struct scanner {
  int current_line;
  int current_column;
  int previous_column;
  FILE *input;
  GString *token_text;
};

enum token_type {
  TOKEN_EOF			= 0,
  TOKEN_LEFT_BRACE		= '[',
  TOKEN_RIGHT_BRACE		= ']',
  TOKEN_EQUAL_SIGN		= '=',
  TOKEN_COMMA			= ',',
  TOKEN_SEMI_COLON		= ';',
  TOKEN_NONE			= 256,
  TOKEN_STRING,
};

static struct scanner *scanner_new(FILE *input)
{
  struct scanner *s = malloc(sizeof(struct scanner));

  s->input = input;
  s->current_line = 1;
  s->current_column = 1;
  s->previous_column = 1;

  s->token_text = g_string_new("");

  return s;
}

static int scanner_current_line(struct scanner *s)
{
  return s->current_line;
}

static int scanner_current_column(struct scanner *s)
{
  return s->current_column;
}

static char *scanner_token_text(struct scanner *s)
{
  return s->token_text->str;
}

static int is_first_identifier(int c)
{
  return isalpha(c) || c == '_';
}

static int is_identifier(int c)
{
  return isalnum(c) || c == '-'  || c == '_';
}

static int scanner_getc(struct scanner *s)
{
  int c;

  c = getc(s->input);

  if (c == '\n') {
    s->current_line++;
    s->previous_column = s->current_column;
    s->current_column = 1;
  } else
    s->current_column++;

  return c;
}

static int scanner_ungetc(struct scanner *s, int c)
{
  if (c == '\n') {
    s->current_line--;
    s->current_column = s->previous_column;
  }
  else
    s->current_column--;

  return ungetc(c, s->input);
}

int scanner_get_next_token(struct scanner *s)
{
  int c;
  enum { 
    S_INITIAL,
    S_SPACE,
    S_COMMENT,
    S_IDENTIFIER,
    S_INTEGER,
    S_STRING,
  } state;

  state = S_INITIAL;
  g_string_set_size(s->token_text, 0);

  while(1) {
    c = scanner_getc(s);

    if (c == EOF)
      return TOKEN_EOF;

    switch(state) {

    case S_INITIAL:
      if (isblank(c))
	state = S_SPACE;
      else if (c == '#')
	state = S_COMMENT;
      else if (c == '"')
	state = S_STRING;
      else if (is_first_identifier(c)) {
	g_string_append_c(s->token_text, c);
	state = S_IDENTIFIER;
      } else if (isdigit(c)) {
	g_string_append_c(s->token_text, c);
	state = S_INTEGER;
      } else if (c == '[' || c == ']' || c == '=' || c == ',' || c == ';') {  /* may be return char in any case ? */
	g_string_append_c(s->token_text, c);
	return c;
      }
      break;

    case S_SPACE:
      if (!isspace(c)) {
	scanner_ungetc(s, c);
	state = S_INITIAL;
      }
      break;

    case S_COMMENT:
      if (c == '\n')
	state = S_INITIAL;
      break;

    case S_STRING:
      if (c != '"')
	g_string_append_c(s->token_text, c);
      else
	return TOKEN_STRING;
      break;

    case S_IDENTIFIER:
      if (is_identifier(c))
	g_string_append_c(s->token_text, c);
      else {
	scanner_ungetc(s, c);
	return TOKEN_STRING;
      }
      break;

    case S_INTEGER:
      if (isdigit(c))
	g_string_append_c(s->token_text, c);
      else {
	scanner_ungetc(s, c);
	return TOKEN_STRING;
      }
      break;

    }
  }

  return TOKEN_NONE;
}

static void scanner_free(struct scanner *s)
{
  g_string_free(s->token_text, TRUE);
  free(s);
}

/*
 * the conf file parser
 * uses a simple recursive descent parser
 */

struct uhuru_conf_parser {
  const char *filename;
  FILE *input;
  struct scanner *scanner;
  enum token_type lookahead_token;
  jmp_buf env;
  char *current_section;
  char *current_key;
  GPtrArray *current_value_list;
  conf_parser_callback_t callback;
  void *user_data;
};

struct uhuru_conf_parser *uhuru_conf_parser_new(const char *filename, conf_parser_callback_t callback, void *user_data)
{
  struct uhuru_conf_parser *cp = malloc(sizeof(struct uhuru_conf_parser));

  cp->filename = os_strdup(filename);

#ifdef WIN32
  fopen_s(&cp->input,filename, "r");
#else
  cp->input = fopen(filename, "r");
#endif

  if (cp->input == NULL) {
    uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "cannot open conf file %s", filename);

    return cp;
  }

  cp->scanner = scanner_new(cp->input);
  cp->lookahead_token = TOKEN_EOF;
  cp->current_section = NULL;
  cp->current_key = NULL;
  cp->current_value_list = NULL;

  cp->callback = callback;
  cp->user_data = user_data;

  return cp;
}

static char *token2str(guint token)
{
  switch(token) {
#define M(E) case E: return #E
    M(TOKEN_EOF);
    M(TOKEN_LEFT_BRACE);
    M(TOKEN_RIGHT_BRACE);
    M(TOKEN_EQUAL_SIGN);
    M(TOKEN_COMMA);
    M(TOKEN_SEMI_COLON);
    M(TOKEN_NONE);
    M(TOKEN_STRING);
  default:
    return "???";
  }

  return "???";
}

static void print_token(struct scanner *scanner, enum token_type token)
{
  fprintf(stderr, "%-20s %3d ", token2str(token), token);
  if (token == TOKEN_STRING) 
    fprintf(stderr, " \"%s\"", scanner_token_text(scanner));
  else
    fprintf(stderr, " %c", (token < 255) ? (char)token : '?');
  fprintf(stderr, "\n");
}

static const char *token_str(enum token_type token)
{
  if (token == TOKEN_STRING) 
    return "string";
  else if (token == TOKEN_EOF)
    return "end of file";
  else if (token < TOKEN_NONE) {
    /* memory leak, I know, but this function is called only in case of syntax error... */
    char *tmp = (char *)malloc(2);

    tmp[0] = (char)token;
    tmp[1] = '\0';

    return tmp;
  }

  return "???";
}

static void syntax_error(struct uhuru_conf_parser *cp, guint token) 
{
  uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_ERROR, "syntax error: file %s line %d column %d expecting '%s' got '%s'\n", 
	    cp->filename,
	    scanner_current_line(cp->scanner),
	    scanner_current_column(cp->scanner),
	    token_str(token), 
	    token_str(cp->lookahead_token));

  longjmp(cp->env, 1);
}

static void accept(struct uhuru_conf_parser *cp, guint token)
{
  if (cp->lookahead_token == token) {
    if (cp->lookahead_token != TOKEN_EOF)
      cp->lookahead_token = scanner_get_next_token(cp->scanner);
  } else
    syntax_error(cp, token);
}

/*
  recursive descent parser functions:
  the grammar is given in conf.h
*/
static void r_configuration(struct uhuru_conf_parser *cp);
static void r_section_list(struct uhuru_conf_parser *cp);
static void r_section(struct uhuru_conf_parser *cp);
static void r_section_name(struct uhuru_conf_parser *cp);
static void r_definition_list(struct uhuru_conf_parser *cp);
static void r_definition(struct uhuru_conf_parser *cp);
static void r_key(struct uhuru_conf_parser *cp);
static void r_opt_value_list(struct uhuru_conf_parser *cp);
static void r_value(struct uhuru_conf_parser *cp);
static void r_list_sep(struct uhuru_conf_parser *cp);

static void free_and_set(char **old, char *new)
{
  if (*old != NULL)
    free(*old);

  *old = os_strdup(new);
}

/* configuration : section_list */
static void r_configuration(struct uhuru_conf_parser *cp)
{
  r_section_list(cp);
}

/* section_list : section section_list | EMPTY */
static void r_section_list(struct uhuru_conf_parser *cp)
{
  if (cp->lookahead_token == TOKEN_LEFT_BRACE) {
    r_section(cp);
    r_section_list(cp);
  }
}

/* section : '[' STRING ']' definition_list */
static void r_section(struct uhuru_conf_parser *cp)
{
  accept(cp, TOKEN_LEFT_BRACE);
  r_section_name(cp);
  accept(cp, TOKEN_RIGHT_BRACE);
  r_definition_list(cp);
}

/* section_name : STRING */
static void r_section_name(struct uhuru_conf_parser *cp)
{
  /* store current section */
  free_and_set(&cp->current_section, scanner_token_text(cp->scanner));

  accept(cp, TOKEN_STRING);
}

/* definition_list: definition definition_list | EMPTY */
static void r_definition_list(struct uhuru_conf_parser *cp)
{
  if (cp->lookahead_token == TOKEN_STRING) {
    r_definition(cp);
    r_definition_list(cp);
  }
}

/* definition : key '=' value opt_value_list  */
static void r_definition(struct uhuru_conf_parser *cp)
{
  const char **argv;
  size_t length;

  r_key(cp);
  accept(cp, TOKEN_EQUAL_SIGN);
  r_value(cp);
  r_opt_value_list(cp);

  /* process stored values by calling the callback */
  g_ptr_array_add(cp->current_value_list, NULL);
  argv = (const char **)cp->current_value_list->pdata;
  length = cp->current_value_list->len - 1;
  (*cp->callback)(cp->current_section, cp->current_key, argv, length, cp->user_data);
  g_ptr_array_set_size(cp->current_value_list, 0);
}

/* key : STRING */
static void r_key(struct uhuru_conf_parser *cp)
{
  /* store current key */
  free_and_set(&cp->current_key, scanner_token_text(cp->scanner));

  accept(cp, TOKEN_STRING);
}

/* opt_value_list : list_sep value opt_value_list | EMPTY */
static void r_opt_value_list(struct uhuru_conf_parser *cp)
{
  if (cp->lookahead_token == TOKEN_COMMA || cp->lookahead_token == TOKEN_SEMI_COLON) {
    r_list_sep(cp);
    r_value(cp);
    r_opt_value_list(cp);
  }
}

/* value : STRING */
static void r_value(struct uhuru_conf_parser *cp)
{
  /* store current value */
  g_ptr_array_add(cp->current_value_list, os_strdup(scanner_token_text(cp->scanner)));

  accept(cp, TOKEN_STRING);
}

/* list_sep : ',' | ';'  */
static void r_list_sep(struct uhuru_conf_parser *cp)
{
  if (cp->lookahead_token == TOKEN_COMMA)
    accept(cp, TOKEN_COMMA);
  else
    accept(cp, TOKEN_SEMI_COLON);
}

void arg_destroy_notify(gpointer data)
{
  free(data);
}

int uhuru_conf_parser_parse(struct uhuru_conf_parser *cp)
{
  int ret = -2;

  if (cp->input == NULL)
    return -1;

  cp->lookahead_token = scanner_get_next_token(cp->scanner);

  cp->current_section = NULL;
  cp->current_key = NULL;
  cp->current_value_list = g_ptr_array_new_with_free_func(arg_destroy_notify);

  if (!setjmp(cp->env)) {
    r_configuration(cp);

    accept(cp, TOKEN_EOF);

    ret = 0;
  }

  return ret;
}

void uhuru_conf_parser_free(struct uhuru_conf_parser *cp)
{
  if (cp->input != NULL) {
    fclose(cp->input);

    scanner_free(cp->scanner);

    g_ptr_array_free(cp->current_value_list, TRUE);
  }

  free((void *)cp->filename);

  free(cp);
}
