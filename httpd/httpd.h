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

#ifndef HTTPD_HTTPD_H
#define HTTPD_HTTPD_H

#include <armadito-config.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <microhttpd.h>

enum http_method {
	HTTP_METHOD_GET   = 1 << 0,
	HTTP_METHOD_POST  = 1 << 1,
	HTTP_METHOD_OTHER = 0,
};

struct httpd;

struct httpd *httpd_new(unsigned short port);

void httpd_destroy(struct httpd *h);

#ifdef DEBUG
void connection_debug(struct MHD_Connection *connection);
#endif

#endif
