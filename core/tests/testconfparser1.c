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

#include "confparser.h"

#include <assert.h>
#include <stdio.h>

int conf_parser_print_cb(const char *section, const char *key, struct a6o_conf_value *value, void *user_data)
{
	int i;

	fprintf(stderr, "=> section %s key %s args", section, key);

#if 0
	for(i = 0; *argv != NULL; i++, argv++)
		fprintf(stderr, " [%d] %s", i, *argv);
#endif

	fprintf(stderr, "\n");

	return 0;
}

int main(int argc, char **argv)
{
	struct a6o_conf_parser *cp;
	int ret;

	assert(argc >= 2);

	cp = a6o_conf_parser_new(argv[1], conf_parser_print_cb, NULL);

	ret = a6o_conf_parser_parse(cp);

	a6o_conf_parser_free(cp);

	return ret;
}
