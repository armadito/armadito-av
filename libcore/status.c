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

#include "status_p.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int a6o_file_status_cmp(enum a6o_file_status s1, enum a6o_file_status s2)
{
	if (s1 == s2)
		return 0;

	switch(s1) {
	case A6O_FILE_UNDECIDED:
		return -1;
	case A6O_FILE_CLEAN:
		return (s2 == A6O_FILE_UNDECIDED) ? 1 : -1;
	case A6O_FILE_IERROR:
		return (s2 == A6O_FILE_UNDECIDED || s2 == A6O_FILE_CLEAN) ? 1 : -1;
	case A6O_FILE_SUSPICIOUS:
		return (s2 == A6O_FILE_UNDECIDED || s2 == A6O_FILE_CLEAN || s2 == A6O_FILE_IERROR) ? 1 : -1;
	case A6O_FILE_WHITE_LISTED:
		return (s2 == A6O_FILE_UNDECIDED || s2 == A6O_FILE_CLEAN || s2 == A6O_FILE_IERROR || s2 == A6O_FILE_SUSPICIOUS) ? 1 : -1;
	case A6O_FILE_MALWARE:
		return 1;
	}

	assert(1 == 0);

	return 0;
}

const char *a6o_file_status_str(enum a6o_file_status status)
{
	switch(status) {
#undef M
#define M(S) case S: return #S
		M(A6O_FILE_UNDECIDED);
		M(A6O_FILE_CLEAN);
		M(A6O_FILE_UNKNOWN_TYPE);
		M(A6O_FILE_EINVAL);
		M(A6O_FILE_IERROR);
		M(A6O_FILE_SUSPICIOUS);
		M(A6O_FILE_WHITE_LISTED);
		M(A6O_FILE_MALWARE);
	}

	return "UNKNOWN STATUS";
}

const char *a6o_file_status_pretty_str(enum a6o_file_status status)
{
	switch(status) {
	case A6O_FILE_UNDECIDED:
		return "undecided";
	case A6O_FILE_CLEAN:
		return "clean";
	case A6O_FILE_UNKNOWN_TYPE:
		return "ignored";
	case A6O_FILE_EINVAL:
		return "invalid argument";
	case A6O_FILE_IERROR:
		return "internal error";
	case A6O_FILE_SUSPICIOUS:
		return "suspicious";
	case A6O_FILE_WHITE_LISTED:
		return "white listed";
	case A6O_FILE_MALWARE:
		return "malware";
	}

	return "unknown status";
}

