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
 * @file   utils.h
 *
 * This file define some generic macros and constants used by the module.
 */

#ifndef UTILS_H
#define UTILS_H

#define TODO
#define TOFIX

//
// this part is used to define tools for debugging
//

// #define IN_DEBUG_MODE

/* definition of a debug macro, only working when IN_DEBUG_MODE is defined */

/* style of the debug log */
/**
===================== DBG_PRNT_MACRO_H1_STATIC =====================
Function : "In at elit justo. Sed. "
File : "Nulla nisl arcu, congue ac"
Line : "Suspendisse id ligula dictum, efficitur."
Last modification : "Donec consequat ante id lacinia."
Last compilation : "Nam dolor turpis, porttitor vitae."
Message : "Etiam nisl nibh, porta a."
===================== DBG_PRNT_MACRO_H1_STATIC =====================
**/

#if defined(IN_DEBUG_MODE)
#define DBG_MACRO_ENABLED 1
#else
#define DBG_MACRO_ENABLED 0
#endif
#define DBG_HEADER		"===================== %s =====================\n"
#define DBG_HEADER_MSG	"DBG_PRNT_MACRO_H1_STATIC"
#define DBG_FILE_INFO	"File : \"%s\"\nLine : %d\n"
#define DBG_MODIF		"Last modification : %s\n"
#define DBG_COMPILE		"Last compilation : %s at %s\n"
#define DBG_FUNC_INFO	"Function : <%s>\n"
/* for this macro, it is necessary to disable warning 4127 with hardcoded if & while condition (ie. having directly 0 or 1 in the condition)*/
#if defined(_MSC_VER)
#define DBG_PRNT(fmt, ...) \
		do {\
			__pragma( warning(push) ) \
			__pragma( warning(disable:4127) ) \
			if (DBG_MACRO_ENABLED) {\
			__pragma( warning(pop))\
				fprintf(\
					stdout, \
					DBG_HEADER DBG_FUNC_INFO DBG_FILE_INFO DBG_MODIF DBG_COMPILE "Message : " fmt DBG_HEADER, \
					DBG_HEADER_MSG, __FUNCTION__, __FILE__, __LINE__, __TIMESTAMP__, __DATE__, __TIME__, __VA_ARGS__, DBG_HEADER_MSG\
				); \
							}\
					} \
		__pragma( warning(push) ) \
		__pragma( warning(disable:4127) ) \
					while( 0 ) \
		__pragma( warning(pop))
#else
#define DBG_PRNT(fmt, ...) \
    do {\
		if (DBG_MACRO_ENABLED) {\
			fprintf(\
				stdout, \
				fmt , \
				 __VA_ARGS__\
			); \
					}\
			} \
					while( 0 )
#endif

//
// end of the part about the debugging
//

#define		TRUE					1
#define		FALSE					0
/* maximum size of a constant CHAR[] */
#define 	NAMESIZE 				2048

//
// This part present all the parameters used by the module in order to test files
//

/* number of nearest neighbors for the k-NN test */
#define 	NUMBER_OF_N_N			9
/* threshold for the iat test */
#define 	IAT_KNN_THRESHOLD		0.4
#define 	IAT_KNN_MINI_THRESHOLD 	0.75
/* threshold for the eat test */
#define 	EAT_KNN_THRESHOLD		0
/* threshold the define the status of a function*/
#define		FUNC_IS_NOT_MALWARE		0.001
#define		FUNC_IS_MALWARE			100
/* threshold the define the status of a file*/
#define		THRESHOLD_NOT_MALWARE	0.2
#define		THRESHOLD_MALWARE		5
//
// end of the part
//

#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

//
// usefull generic functions
//

#include "windowsTypes.h"

#endif /* UTILS_H */
