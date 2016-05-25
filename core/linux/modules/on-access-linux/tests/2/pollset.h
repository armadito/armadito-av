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

#ifndef _POLLSET_H_
#define _POLLSET_H_

struct poll_set;

struct poll_set *poll_set_new(void);

typedef void (*poll_cb_t)(void *user_data);

int poll_set_add_fd(struct poll_set *s, int fd, poll_cb_t cb, void *user_data);

int poll_set_loop(struct poll_set *s);

#endif
