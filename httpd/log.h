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

#ifndef HTTPD_LOG_H
#define HTTPD_LOG_H

/* debug */
void log_d(const char *format, ...);

/* error */
void log_e(const char *format, ...);

/* info */
void log_i(const char *format, ...);

/* warning */
void log_w(const char *format, ...);

enum log_mode {
	LOG_TO_SYSTEM_LOG,
	LOG_TO_STDERR,
};

void log_init(enum log_mode mode);

#endif
