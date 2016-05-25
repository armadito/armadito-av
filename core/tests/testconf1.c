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

#include <libarmadito.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	struct a6o_conf *conf;
	a6o_error *error = NULL;
	const char **sections, **p;
	size_t len;

	assert(argc > 2);

	conf = a6o_conf_new();

	if (a6o_conf_load_file(conf, argv[1], &error))
		return 1;

	if (a6o_conf_save_file(conf, argv[2], &error))
		return 2;

	sections = a6o_conf_get_sections(conf, &len);

	printf("== sections\n");

	for(p = sections; *p != NULL; p++)
		printf("section %s\n", *p);

	for(p = sections; *p != NULL; p++)
		free((void *)*p);

	free((void *)sections);

	a6o_conf_free(conf);

	return 0;
}
