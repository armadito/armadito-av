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

#ifndef BRPC_TESTS_UNIX_H
#define BRPC_TESTS_UNIX_H

#include <unistd.h>

#define SOCKET_PATH "@/tmp/brpctest.socket"

int unix_server_listen(const char *socket_path);

int unix_client_connect(const char *socket_path, int max_retry);

ssize_t unix_fd_write_cb(const void *buffer, size_t size, void *data);

ssize_t unix_fd_read_cb(void *buffer, size_t size, void *data);

#endif
