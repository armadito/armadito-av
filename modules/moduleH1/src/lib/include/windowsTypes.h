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
 * @file   windowsTypes.h
 *
 * This file define a set of custom types names for more clarity in the code.
 */

#ifndef WINDOWS_TYPES_H
#define WINDOWS_TYPES_H

#include <stdint.h>

#ifndef _MSC_VER

/* void types */
typedef 	void 			VOID;
typedef 	void* 			PVOID;

/* char types */
typedef 	char 			CHAR;
typedef 	unsigned char 	UCHAR;

/* 8 bits types */
typedef 	uint8_t 		BYTE;
typedef 	uint8_t* 		PBYTE;
typedef 	uint8_t 		BOOLEAN;

/* 16 bits types */
typedef 	int16_t 		SHORT;
typedef 	uint16_t 		USHORT;
typedef 	uint16_t 		WORD;
typedef 	uint16_t* 		PWORD;

/* 32 bits types */
typedef 	uint32_t 		DWORD;
typedef 	uint32_t* 		PDWORD;
typedef 	int32_t 		LONG;
typedef 	uint32_t 		ULONG;
typedef 	int32_t 		INT;
typedef 	uint32_t 		UINT;

/* 64 bits types */
typedef 	uint64_t 		QWORD;
typedef 	uint64_t* 		PQWORD;
typedef 	uint64_t 		ULONGLONG;
typedef 	int64_t 		LONGLONG;
typedef 	uint64_t 		ULONG_PTR;

/* non-integer types */
typedef 	double 			DOUBLE;
typedef 	float 			FLOAT;

#else

#include <Windows.h>

/* define the missing type in Windows.h */
typedef		uint64_t		QWORD;

#endif

#endif /* WINDOWS_TYPES_H */
