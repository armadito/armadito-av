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

/**
 * \file status.h
 *
 * \brief definition of the C enum for a file scan status
 *
 * a6o_file_status defines the status of a file during or after scan
 */

#ifndef __LIBARMADITO_STATUS_H_
#define __LIBARMADITO_STATUS_H_

enum a6o_file_status {
	A6O_FILE_UNDECIDED = 1,         /*!< not yet decided by scan                                */
	A6O_FILE_CLEAN,                 /*!< file is clean and does not contain a malware           */
	A6O_FILE_UNKNOWN_TYPE,          /*!< file type is not handled by any module                 */
	A6O_FILE_EINVAL,                /*!< an invalid value was passed to scan functions          */
	A6O_FILE_IERROR,                /*!< an internal error occured during file scan             */
	A6O_FILE_SUSPICIOUS,            /*!< file is suspicious: not malware but not clean also     */
	A6O_FILE_WHITE_LISTED,          /*!< file is while listed, i.e. guaranteed clean            */
	A6O_FILE_MALWARE,               /*!< file contains a malware                                */
};

#endif
