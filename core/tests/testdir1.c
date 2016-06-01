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

#include "os/dir.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *flag_str(enum os_file_flag flag)
{
#define M(F) case F: return #F
	switch(flag) {
		M(FILE_FLAG_IS_ERROR);
		M(FILE_FLAG_IS_DIRECTORY);
		M(FILE_FLAG_IS_PLAIN_FILE);
		M(FILE_FLAG_IS_LINK);
		M(FILE_FLAG_IS_DEVICE);
		M(FILE_FLAG_IS_IPC);
		M(FILE_FLAG_IS_UNKNOWN);
	}

	return "arghhh";
}

static int test_dirent_cb(const char *full_path, enum os_file_flag flags, int entry_errno, void *data)
{
	printf("path: %s flags: %s errno: %d (%s)\n", full_path, flag_str(flags), entry_errno, strerror(entry_errno));
	return 0;
}

int main(int argc, char **argv)
{
	assert(argc >= 2);

	os_dir_map(argv[1], 1, test_dirent_cb, NULL);
}
