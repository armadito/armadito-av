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

#ifndef _DAEMON_SERVER_H_
#define _DAEMON_SERVER_H_

enum ipc_type {
	OLD_IPC,
	JSON_IPC,
};

struct server;

struct server *server_new(struct armadito *armadito, int server_sock, enum ipc_type ipc_type);

int server_get_poll_fd(struct server *s);

void server_cb(void *user_data);

#endif
