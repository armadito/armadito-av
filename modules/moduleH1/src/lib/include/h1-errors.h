/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito module H1.

Armadito module H1 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito module H1 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Armadito module H1.  If not, see <http://www.gnu.org/licenses/>.

***/

/**
 * @file   errors.h
 *
 * This file define a set of custom errors code used by the module.
 */

#ifndef ERRORS_H
#define ERRORS_H

#include "windowsTypes.h"

typedef enum _ERROR_CODE{
	/* _ERROR_CODE initialization value */
	ARMADITO_NULL = 000,
	/* no error return value : 1xx */

	ARMADITO_SUCCESS = 100,
	ARMADITO_IS_MALWARE = 101,
	ARMADITO_NOT_MALWARE = 102,
	ARMADITO_EAT_UNKNOWN = 103,
	ARMADITO_TFIDF_UNKNOWN = 104,
	ARMADITO_NOT_DECIDED = 105,
	ARMADITO_DOUBTFUL = 106,
	ARMADITO_UNSUPPORTED_FILE = 107,
	/* global errors :         2xx */

	E_READING_ERROR = 201,
	E_TEST_ERROR = 202,
	E_DISTANCE_ERROR = 203,
	E_FAILURE = 204,
	E_CALLOC_ERROR = 205,
	E_FILE_NOT_FOUND = 206,
	E_FILE_EMPTY = 207,
	E_FSTAT_ERROR = 208,
	/* pe errors :             3xx */

	E_NOT_MZ = 300,
	E_NOT_PE = 301,
	E_BAD_ARCHITECTURE = 302,
	ARMADITO_NO_SECTIONS = 303,
	E_NO_ENTRY_POINT = 305,
	E_EAT_EMPTY = 306,
	E_IAT_EMPTY = 307,
	E_SECTION_ERROR = 308,
	E_DLL_NAME_ERROR = 309,
	E_NAME_ERROR = 310,
	E_CHECKSUM_ERROR = 311,
	E_IAT_NOT_GOOD = 312,
	E_EAT_NOT_GOOD = 313,
	E_FUNCTION_NAME_ERROR = 314,
	E_NO_ENTRY = 315,
	ARMADITO_INVALID_SECTION_NAME = 316,
	E_INVALID_ENTRY_POINT = 320,
	E_INVALID_STUB = 321,
	E_INVALID_TIMESTAMP = 322,
	E_FIELDS_WITH_INVALID_VALUE = 323,
	E_INVALID_SIZE_OPT_HEADER = 324,
	E_EAT_INVALID_TIMESTAMP = 325,
	E_IAT_INVALID_TIMESTAMP = 326,
	E_EMPTY_VECTOR = 327,
	E_INVALID_STRUCTURE = 328,
	E_INVALID_NUMBER_RVA_SIZES = 329,
	E_INVALID_FILE_SIZE = 330,
	E_HEADER_NOT_GOOD = 331,
	E_INVALID_S_F_ALIGNMENT = 332,
	ARMADITO_INVALID_SECTION = 333,
	/* elf errors : 		   4xx */

	E_NOT_ELF = 400,
	E_SYMBOL_TABLE_EMPTY = 401,
	E_BAD_FORMAT = 402,
	E_NO_KNOWN_SYMBOLS = 404,
} ERROR_CODE, *PERROR_CODE;

VOID SetCurrentError(ERROR_CODE error);
ERROR_CODE GetCurrentError();
CHAR* GetErrorCodeMsg(ERROR_CODE error);
const char *error_code_str(ERROR_CODE e);

#endif /* ERRORS_H */
