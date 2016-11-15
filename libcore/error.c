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

#include <libarmadito/armadito.h>

#include "armadito-config.h"

#include <core/error.h>

#include <assert.h>
#include <glib.h>
#include <stdlib.h>

a6o_error *a6o_error_new(int error_code, const char *error_message)
{
	a6o_error *e = (a6o_error *)malloc(sizeof(a6o_error));

	assert(e != NULL);

	e->error_code = error_code;
	e->error_message = error_message;

	return e;
}

void a6o_error_set(a6o_error **error, int error_code, const char *error_message)
{
	if (error == NULL)
		return;

	/* same check as in glib: if error location is already set, do not overwrite it!!! */
	if (*error != NULL) {
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "a6o_error set over the top of a previous a6o_error or uninitialized memory.\n\
This indicates a bug in someone's code. You must ensure an error is NULL before it's set.\n\
The overwriting error message was: %s", error_message);

		return;
	}

	*error = a6o_error_new(error_code, error_message);
}

void a6o_error_free(a6o_error *err)
{
	if (err != NULL)
		free(err);
}

void a6o_error_print(a6o_error *err, FILE *out)
{
	if (err == NULL)
		return;

	fprintf(out, "** Armadito: ERROR: %s\n", err->error_message);
}
