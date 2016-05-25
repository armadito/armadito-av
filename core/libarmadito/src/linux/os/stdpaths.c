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
#include "libarmadito-config.h"

#include "os/stdpaths.h"

#include <string.h>

const char *os_stdpath_module()
{
	return strdup(LIBARMADITO_MODULES_PATH);
}

const char *os_stdpath_config_file()
{
	return strdup(LIBARMADITO_CONF_DIR "/armadito.conf");
}

const char *os_stdpath_config_dir()
{
	return strdup(LIBARMADITO_CONF_DIR "/conf.d");
}

const char *os_stdpath_bases()
{
	return strdup(LIBARMADITO_BASES_DIR);
}

const char *os_stdpath_binary()
{
	return NULL;
}

const char *os_stdpath_tmp()
{
	return NULL;
}

char a6o_path_sep(void)
{
	return '/';
}

