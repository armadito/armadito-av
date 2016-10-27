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

#include <stdarg.h>
#include <stdio.h>
#include <glib.h>
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#include "log.h"

#define LOG_NAME "armadito-httpd"

enum httpd_log_level {
	HTTPD_LOG_ERROR,
	HTTPD_LOG_WARNING,
	HTTPD_LOG_INFO,
	HTTPD_LOG_DEBUG,
};

static enum httpd_log_level current_log_level = HTTPD_LOG_DEBUG;
static enum log_mode current_log_mode = LOG_TO_STDERR;

#ifdef linux
static int priority_from_level(enum httpd_log_level log_level)
{
	switch (log_level) {
	case HTTPD_LOG_ERROR:
		return LOG_ERR;
	case HTTPD_LOG_WARNING:
		return LOG_WARNING;
	case HTTPD_LOG_INFO:
		return LOG_INFO;
	case HTTPD_LOG_DEBUG:
		return LOG_DEBUG;
	}

	return LOG_INFO;
}

static void system_log(enum httpd_log_level log_level, const char *message)
{
	syslog(priority_from_level(log_level), "%s", message);
}
#endif

static void stderr_log(enum httpd_log_level log_level, const char *message)
{
	fputs(message, stderr);
}

static void do_log(enum httpd_log_level level, const char *s_level, const char *format, va_list args)
{
	GString *buff;
	gchar *message;

	if (level > current_log_level)
		return;

	buff = g_string_new("");

	g_string_append_printf(buff, "[%s] ", s_level);
	g_string_append_vprintf(buff, format, args);
	g_string_append(buff, "\n");

	message = g_string_free(buff, FALSE);

	switch(current_log_mode) {
	case LOG_TO_SYSTEM_LOG:
		system_log(level, message);
		break;
	case LOG_TO_STDERR:
		stderr_log(level, message);
		break;
	default:
		break;
	}

	g_free(message);
}

#define LOG_BODY(F, L, S)			\
	va_list args;				\
	va_start(args, F);			\
	do_log(L, S, F, args);			\
	va_end(args);

void log_d(const char *format, ...)
{
	LOG_BODY(format, HTTPD_LOG_DEBUG, "debug");
}

void log_e(const char *format, ...)
{
	LOG_BODY(format, HTTPD_LOG_ERROR, "error");
}

void log_i(const char *format, ...)
{
	LOG_BODY(format, HTTPD_LOG_INFO, "info");
}

void log_w(const char *format, ...)
{
	LOG_BODY(format, HTTPD_LOG_WARNING, "warning");
}

void log_init(enum log_mode mode)
{
	current_log_mode = mode;
}
