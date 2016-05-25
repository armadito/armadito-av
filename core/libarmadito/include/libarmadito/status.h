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
 * \file status.h
 *
 * \brief definition of the C enum for a file scan status
 *
 * a6o_file_status defines the status of a file during or after scan
 */

#ifndef _LIBARMADITO_STATUS_H_
#define _LIBARMADITO_STATUS_H_

enum a6o_file_status {
	ARMADITO_UNDECIDED = 1,         /*!< not yet decided by scan                                */
	ARMADITO_CLEAN,                 /*!< file is clean and does not contain a malware           */
	ARMADITO_UNKNOWN_FILE_TYPE,     /*!< file type is not handled by any module                 */
	ARMADITO_EINVAL,                /*!< an invalid value was passed to scan functions          */
	ARMADITO_IERROR,                /*!< an internal error occured during file scan             */
	ARMADITO_SUSPICIOUS,            /*!< file is suspicious: not malware but not clean also     */
	ARMADITO_WHITE_LISTED,          /*!< file is while listed, i.e. guaranteed clean            */
	ARMADITO_MALWARE,               /*!< file contains a malware                                */
};

const char *a6o_file_status_str(enum a6o_file_status status);

const char *a6o_file_status_pretty_str(enum a6o_file_status status);

#endif
