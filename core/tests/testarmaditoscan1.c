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
#include <stdio.h>

int main(int argc, char **argv)
{
	struct armadito *u = a6o_open(NULL, NULL);
	int i;

	for (i = 1; i < argc; i++) {
		enum a6o_file_status status;

		/* status = a6o_scan_file(u, argv[i]); */
		printf("%s: %s\n", argv[i], a6o_file_status_str(status));
	}

	a6o_close(u, NULL);
}
