/***

Copyright (C) 2015, 2016, 2017 Teclib'

This file is part of Armadito.

Armadito is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito.  If not, see <http://www.gnu.org/licenses/>.

***/

#include <libarmadito.h>

#include <glib.h>
#include <syslog.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

static enum a6o_log_level get_a6o_log_level_from_str(const char *s_log_level)
{
	if (!strcmp(s_log_level,"error"))
		return ARMADITO_LOG_LEVEL_ERROR;

	if (!strcmp(s_log_level,"warning"))
		return ARMADITO_LOG_LEVEL_WARNING;

	if (!strcmp(s_log_level,"info"))
		return ARMADITO_LOG_LEVEL_INFO;

	if (!strcmp(s_log_level,"debug"))
		return ARMADITO_LOG_LEVEL_DEBUG;
	return -1;
}

static int priority_from_level(enum a6o_log_level log_level)
{
	switch (log_level) {
	case ARMADITO_LOG_LEVEL_ERROR:
		return LOG_ERR;
	case ARMADITO_LOG_LEVEL_WARNING:
		return LOG_WARNING;
	case ARMADITO_LOG_LEVEL_INFO:
		return LOG_INFO;
	case ARMADITO_LOG_LEVEL_DEBUG:
		return LOG_DEBUG;
	}

	return LOG_INFO;
}
static void a6o_syslog_handler(enum a6o_log_domain domain, enum a6o_log_level log_level, const char *message, void *user_data)
{
	if (log_level != ARMADITO_LOG_LEVEL_NONE)
		syslog(priority_from_level(log_level), "<%s> %s\n", a6o_log_level_str(log_level), message);
	else
		syslog(priority_from_level(log_level), "%s\n", message);
}

static void log_init_with_a6o_log(const char *s_log_level, int use_syslog)
{
	enum a6o_log_level level = get_a6o_log_level_from_str(s_log_level);

	if (use_syslog) {
		openlog("armadito-av", LOG_CONS | LOG_PID, LOG_USER);
		a6o_log_set_handler(level, a6o_syslog_handler, NULL);
	} else
		a6o_log_set_handler(level, a6o_log_default_handler, NULL);
}

void log_init(const char *s_log_level, int use_syslog)
{
	log_init_with_a6o_log(s_log_level, use_syslog);
}
