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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static dev_t get_dev_id(const char *path)
{
	struct stat buf;

	if (stat(path, &buf) < 0) {
		fprintf(stderr, "stat() failed (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return buf.st_dev;
}

static void test(const char *path, dev_t slash_id)
{
	dev_t dev_id;

	dev_id = get_dev_id(path);

	printf("%s (%d) vs / (%d): %s\n", path, (unsigned int)dev_id, (unsigned int)slash_id, dev_id == slash_id ? "same device" : "different device");
}

int main(int argc, char **argv)
{
	dev_t slash_id;
	int i;

	slash_id = get_dev_id("/");

	for (i = 1; i < argc; i++)
		test(argv[i], slash_id);

	return 0;
}
