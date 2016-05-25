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

/**
 * \file stdpaths.h
 *
 * \brief definition of Armadito standard paths, for modules, configuration...
 *
 * The std_path() function returns for accessing standard paths.
 *
 * This file defines functions to query standard locations on the local filesystem, such as
 * - modules binaries directory
 * - configuration file
 * - configuration directory
 * - bases directory
 *
 * Standard locations are platform dependant.
 *
 */

#ifndef _LIBARMADITO_STDPATHS_H_
#define _LIBARMADITO_STDPATHS_H_

enum a6o_std_location {
	MODULES_LOCATION,
	CONFIG_FILE_LOCATION,
	CONFIG_DIR_LOCATION,
	BASES_LOCATION,
	BINARY_LOCATION,
	TMP_LOCATION,
	LAST_LOCATION,
};

const char *a6o_std_path(enum a6o_std_location location);

char a6o_path_sep(void);

#endif
