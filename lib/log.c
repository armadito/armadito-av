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

#include <errno.h>
#include <glib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_GETPID
#define os_getpid getpid
#endif
#ifdef HAVE__GETPID
#include <process.h>
#define os_getpid _getpid
#endif

#define LOG_NAME "armadito"

static enum a6o_log_level current_max_level = ARMADITO_LOG_LEVEL_WARNING;
static a6o_log_handler_t current_handler = a6o_log_default_handler;
static void *current_handler_user_data = NULL;

void a6o_log(enum a6o_log_domain domain, enum a6o_log_level level, const char *format, ...)
{
	va_list args;
	GString *buff;
	gchar *message;

	/* anything to do? */
	if (level != ARMADITO_LOG_LEVEL_NONE && level > current_max_level)
		return;

	/* format message */
	buff = g_string_new("");
	va_start(args, format);
	g_string_vprintf(buff, format, args);
	va_end(args);

	/* call the handler with formated message */
	message = g_string_free(buff, FALSE);

	(*current_handler)(domain, level, message, current_handler_user_data);

	g_free(message);

#ifndef _WIN32
	if (level & ARMADITO_LOG_LEVEL_ERROR)
		abort();
#endif
}

void a6o_log_set_handler(enum a6o_log_level max_level, a6o_log_handler_t handler, void *user_data)
{
	current_max_level = max_level;

	if (handler != NULL)
		current_handler = handler;
	else
		current_handler = a6o_log_default_handler;

	current_handler_user_data = user_data;
}

static FILE *get_stream(enum a6o_log_level log_level)
{
	return log_level & (ARMADITO_LOG_LEVEL_ERROR | ARMADITO_LOG_LEVEL_WARNING) ? stderr : stdout;
}

static const char *domain_str(enum a6o_log_domain domain)
{
	switch(domain) {
	case ARMADITO_LOG_LIB:
		return "lib";
	case ARMADITO_LOG_MODULE:
		return "module";
	case ARMADITO_LOG_SERVICE:
		return "service";
	}

	return "";
}

const char *a6o_log_level_str(enum a6o_log_level log_level)
{
	switch (log_level) {
	case ARMADITO_LOG_LEVEL_ERROR:
		return "error";
	case ARMADITO_LOG_LEVEL_WARNING:
		return "warning";
	case ARMADITO_LOG_LEVEL_INFO:
		return "info";
	case ARMADITO_LOG_LEVEL_DEBUG:
		return "debug";
	case ARMADITO_LOG_LEVEL_NONE:
		return "";
	}

	return "";
}

#ifdef HAVE_CLOCK_GETTIME
static void append_uptime(GString *gstring)
{
	struct timespec now = {0L, 0L};

	clock_gettime(CLOCK_MONOTONIC_COARSE, &now);

	g_string_append_printf(gstring, "[%6.6f] ", now.tv_sec + now.tv_nsec / 1000000000.0);
}
#else
static void append_uptime(GString *gstring)
{
}
#endif

void a6o_log_default_handler(enum a6o_log_domain domain, enum a6o_log_level log_level, const char *message, void *user_data)
{
	FILE *stream = get_stream(log_level);
	GString *gstring = g_string_new(NULL);
	gchar *string;

	append_uptime(gstring);

	g_string_append_printf(gstring, "%s[%d]: ", LOG_NAME, os_getpid());

	if (log_level != ARMADITO_LOG_LEVEL_NONE)
		g_string_append_printf(gstring, "<%s> ", a6o_log_level_str(log_level));

	g_string_append(gstring, message);
	g_string_append(gstring, "\n");

	string = g_string_free(gstring, FALSE);

	fputs(string, stream);

	g_free(string);
}

