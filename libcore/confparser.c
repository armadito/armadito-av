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

#include <libarmadito/armadito.h>

#include "armadito-config.h"

#include "confparser.h"
#include "string_p.h"

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
	TOKEN_INTEGER,
	TOKEN_ERROR,
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

static int scanner_get_next_token(struct scanner *s, guint previous_token)
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
			} else {
				/* Handle value error cases before accept or ignore current char */
				if (previous_token == TOKEN_EQUAL_SIGN) {
					scanner_ungetc(s, c);
					return (c == '\n') ? TOKEN_NONE : TOKEN_ERROR ;
				}
				if (c == '[' || c == ']' || c == '=' || c == ',' || c == ';') {  /* may be return char in any case ? */
					g_string_append_c(s->token_text, c);
					return c;
				}
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
				return TOKEN_INTEGER;
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

struct a6o_conf_parser {
	const char *filename;
	FILE *input;
	struct scanner *scanner;
	enum token_type lookahead_token;
	jmp_buf env;
	char *current_section;
	char *current_key;
	enum a6o_conf_value_type current_value_type;
	int current_value_int;
	const char *current_value_string;
	GPtrArray *current_value_list;
	conf_parser_callback_t callback;
	void *user_data;
};

struct a6o_conf_parser *a6o_conf_parser_new(const char *filename, conf_parser_callback_t callback, void *user_data)
{
	struct a6o_conf_parser *cp = malloc(sizeof(struct a6o_conf_parser));

	cp->filename = os_strdup(filename);

#ifdef _WIN32
	fopen_s(&cp->input,filename, "r");
#else
	cp->input = fopen(filename, "r");
#endif

	if (cp->input == NULL) {
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "cannot open conf file %s", filename);

		return cp;
	}

	cp->scanner = scanner_new(cp->input);
	cp->lookahead_token = TOKEN_EOF;
	cp->current_section = NULL;
	cp->current_key = NULL;
	cp->current_value_type = CONF_TYPE_VOID;
	cp->current_value_int = 0;
	cp->current_value_string = NULL;
	cp->current_value_list = NULL;

	cp->callback = callback;
	cp->user_data = user_data;

	return cp;
}

static const char *token_str(enum token_type token)
{
	if (token == TOKEN_STRING)
		return "string";
	else if (token == TOKEN_EOF)
		return "end of file";
	else if (token < TOKEN_NONE) {
		static char tmp[2] = " ";
		tmp[0] = (char)token;
		return tmp;
	}

	return "???";
}

static void syntax_error(struct a6o_conf_parser *cp, guint token)
{
	a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "syntax error: file %s line %d column %d expecting '%s' got '%s'",
		cp->filename,
		scanner_current_line(cp->scanner),
		scanner_current_column(cp->scanner),
		token_str(token),
		token_str(cp->lookahead_token));

	longjmp(cp->env, 1);
}

static void token_accept(struct a6o_conf_parser *cp, guint token)
{
	if (cp->lookahead_token == token) {
		if (cp->lookahead_token != TOKEN_EOF)
			cp->lookahead_token = scanner_get_next_token(cp->scanner, token);
	} else
		syntax_error(cp, token);
}

static void call_callback(struct a6o_conf_parser *cp)
{
	int ret = 0;
	struct a6o_conf_value value;

	value.type = cp->current_value_type;

	switch(cp->current_value_type) {
	case CONF_TYPE_INT:
		value.v.int_v = cp->current_value_int;
		ret = (*cp->callback)(cp->current_section, cp->current_key, &value, cp->user_data);
		break;

	case CONF_TYPE_STRING:
		value.v.str_v = cp->current_value_string;
		ret = (*cp->callback)(cp->current_section, cp->current_key, &value, cp->user_data);
		free((void *)cp->current_value_string);
		cp->current_value_string = NULL;
		break;

	case CONF_TYPE_LIST:
		g_ptr_array_add(cp->current_value_list, NULL);
		value.v.list_v.values = (const char **)cp->current_value_list->pdata;
		value.v.list_v.len = cp->current_value_list->len - 1;
		ret = (*cp->callback)(cp->current_section, cp->current_key, &value, cp->user_data);
		g_ptr_array_set_size(cp->current_value_list, 0);
		break;
	}

	cp->current_value_type = CONF_TYPE_VOID;

	if (ret != 0) {
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "configuration file parser: callback return != 0 file %s line %d column %d",
			cp->filename,
			scanner_current_line(cp->scanner),
			scanner_current_column(cp->scanner));
		longjmp(cp->env, 1);
	}
}

/*
  recursive descent parser functions:
  the grammar is given in conf.h
*/
static void r_configuration(struct a6o_conf_parser *cp);
static void r_section_list(struct a6o_conf_parser *cp);
static void r_section(struct a6o_conf_parser *cp);
static void r_section_name(struct a6o_conf_parser *cp);
static void r_definition_list(struct a6o_conf_parser *cp);
static void r_definition(struct a6o_conf_parser *cp);
static void r_key(struct a6o_conf_parser *cp);
static void r_value(struct a6o_conf_parser *cp);
static void r_int_value(struct a6o_conf_parser *cp);
static void r_string_value(struct a6o_conf_parser *cp);
static void r_opt_string_list(struct a6o_conf_parser *cp);
static void r_list_string_value(struct a6o_conf_parser *cp);
static void r_list_sep(struct a6o_conf_parser *cp);

static void free_and_set(char **old, char *new)
{
	if (*old != NULL)
		free(*old);

	*old = os_strdup(new);
}

/* configuration : section_list */
static void r_configuration(struct a6o_conf_parser *cp)
{
	r_section_list(cp);
}

/* section_list : section section_list | EMPTY */
static void r_section_list(struct a6o_conf_parser *cp)
{
	if (cp->lookahead_token == TOKEN_LEFT_BRACE) {
		r_section(cp);
		r_section_list(cp);
	}
}

/* section : '[' STRING ']' definition_list */
static void r_section(struct a6o_conf_parser *cp)
{
	token_accept(cp, TOKEN_LEFT_BRACE);
	r_section_name(cp);
	token_accept(cp, TOKEN_RIGHT_BRACE);
	r_definition_list(cp);
}

/* section_name : STRING */
static void r_section_name(struct a6o_conf_parser *cp)
{
	/* store current section */
	free_and_set(&cp->current_section, scanner_token_text(cp->scanner));

	token_accept(cp, TOKEN_STRING);
}

/* definition_list: definition definition_list | EMPTY */
static void r_definition_list(struct a6o_conf_parser *cp)
{
	if (cp->lookahead_token == TOKEN_STRING) {
		r_definition(cp);
		r_definition_list(cp);
	}
}

/* definition : key '=' value */
static void r_definition(struct a6o_conf_parser *cp)
{
	r_key(cp);
	token_accept(cp, TOKEN_EQUAL_SIGN);
	r_value(cp);

	/* process stored values by calling the callback */
	call_callback(cp);
}

/* key : STRING */
static void r_key(struct a6o_conf_parser *cp)
{
	/* store current key */
	free_and_set(&cp->current_key, scanner_token_text(cp->scanner));

	token_accept(cp, TOKEN_STRING);
}

/* value : int_value | string_value opt_string_list */
static void r_value(struct a6o_conf_parser *cp)
{
	if (cp->lookahead_token == TOKEN_INTEGER) {
		r_int_value(cp);
#ifdef DEBUG
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_DEBUG, "configuration parser: file %s, %s = %d",
			cp->filename,
			cp->current_key,
			cp->current_value_int);
#endif
	}
	else if (cp->lookahead_token == TOKEN_STRING) {
		r_string_value(cp);
		r_opt_string_list(cp);
#ifdef DEBUG
		if (cp->current_value_type == CONF_TYPE_STRING)
			a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_DEBUG, "configuration parser: file %s, %s = \"%s\"",
				cp->filename,
				cp->current_key,
				cp->current_value_string);
		else {
			int i;
			GString *values = g_string_new("");
			for (i = 0; i < cp->current_value_list->len; i++, g_string_append_printf(values, ";"))
				g_string_append_printf(values, "\"%s\"", g_ptr_array_index(cp->current_value_list, i));
			a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_DEBUG, "configuration parser: file %s, %s = %s",
				cp->filename,
				cp->current_key,
				values->str);
			g_string_free(values, TRUE);
		}
#endif
	} else {
		if (cp->lookahead_token == TOKEN_NONE ||cp->lookahead_token == TOKEN_EOF)
			a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "syntax error: file %s line %d column %d expecting value for %s, got none",
				cp->filename,
				scanner_current_line(cp->scanner),
				scanner_current_column(cp->scanner),
				cp->current_key);
		else
			a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "syntax error: file %s line %d column %d expecting value for %s, got '%c', do you forget to use double-quotes ?",
				cp->filename,
				scanner_current_line(cp->scanner),
				scanner_current_column(cp->scanner),
				cp->current_key,
				scanner_getc(cp->scanner));

		longjmp(cp->env, 1);
	}
}

/* int_value: INT */
static void r_int_value(struct a6o_conf_parser *cp)
{
	/* store current value */
	cp->current_value_type = CONF_TYPE_INT;
	cp->current_value_int = atoi(scanner_token_text(cp->scanner));

	token_accept(cp, TOKEN_INTEGER);
}

/* string_value: STRING */
static void r_string_value(struct a6o_conf_parser *cp)
{
	/* store current value */
	cp->current_value_type = CONF_TYPE_STRING;
	cp->current_value_string = os_strdup(scanner_token_text(cp->scanner));

	token_accept(cp, TOKEN_STRING);
}

/* opt_string_list : list_sep list_string_value opt_string_list | EMPTY */
static void r_opt_string_list(struct a6o_conf_parser *cp)
{
	if (cp->lookahead_token == TOKEN_COMMA || cp->lookahead_token == TOKEN_SEMI_COLON) {
		r_list_sep(cp);
		r_list_string_value(cp);
		r_opt_string_list(cp);
	}
}

/* list_string_value: STRING */
static void r_list_string_value(struct a6o_conf_parser *cp)
{
	/* if cp->current_value_string is not NULL, it is the first element of the list */
	if (cp->current_value_string != NULL) {
		cp->current_value_type = CONF_TYPE_LIST;

		g_ptr_array_add(cp->current_value_list, os_strdup(cp->current_value_string));
		free((void *)cp->current_value_string);
		cp->current_value_string = NULL;
	}

	g_ptr_array_add(cp->current_value_list, os_strdup(scanner_token_text(cp->scanner)));

	token_accept(cp, TOKEN_STRING);
}

/* list_sep : ',' | ';'  */
static void r_list_sep(struct a6o_conf_parser *cp)
{
	if (cp->lookahead_token == TOKEN_COMMA)
		token_accept(cp, TOKEN_COMMA);
	else
		token_accept(cp, TOKEN_SEMI_COLON);
}

void arg_destroy_notify(gpointer data)
{
	free(data);
}

int a6o_conf_parser_parse(struct a6o_conf_parser *cp)
{
	int ret = -2;

	if (cp->input == NULL)
		return -1;

	cp->lookahead_token = scanner_get_next_token(cp->scanner, TOKEN_NONE);

	cp->current_section = NULL;
	cp->current_key = NULL;
	cp->current_value_list = g_ptr_array_new_with_free_func(arg_destroy_notify);

	if (!setjmp(cp->env)) {
		r_configuration(cp);

		token_accept(cp, TOKEN_EOF);
		ret = 0;
	}

	return ret;
}

void a6o_conf_parser_free(struct a6o_conf_parser *cp)
{
	if (cp->input != NULL) {
		fclose(cp->input);

		scanner_free(cp->scanner);

		if (cp->current_section != NULL)
			free((void *)cp->current_section);

		if (cp->current_value_list != NULL)
			free((void *)cp->current_key);

		if (cp->current_value_string != NULL)
			free((void *)cp->current_value_string);

		if (cp->current_value_list != NULL)
			g_ptr_array_free(cp->current_value_list, TRUE);
	}

	free((void *)cp->filename);

	free(cp);
}
