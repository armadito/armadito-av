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

#include "armadito-config.h"
#include "os/string.h"
#include "os/stdpaths.h"

#include <assert.h>
#include <stdlib.h>


const char *a6o_std_path(enum a6o_std_location location)
{
	assert(location >= 0 && location < LAST_LOCATION);

	switch(location) {
	case MODULES_LOCATION:
		return os_stdpath_module();
		break;
	case CONFIG_FILE_LOCATION:
		return os_stdpath_config_file();
		break;
	case CONFIG_DIR_LOCATION:
		return os_stdpath_config_dir();
		break;
	case BASES_LOCATION:
		return os_stdpath_bases();
		break;
	case BINARY_LOCATION:
		return os_stdpath_binary();
		break;
	case TMP_LOCATION:
		return os_stdpath_tmp();
		break;
	}

	return NULL;
}


