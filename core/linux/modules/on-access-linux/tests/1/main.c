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

#include "fanotify.h"

#include <glib.h>

int main(int argc, char **argv)
{
	struct watchd *w;
	GMainLoop * loop;

	w = watchd_new();

	while(--argc)
		watchd_add(w, *++argv);

	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	watchd_free(w);

	return 0;
}
