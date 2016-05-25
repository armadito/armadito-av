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
 * @file   databases.h
 *
 * Databases structures and loading functions.
 */

#ifndef DATABASES_H
#define DATABASES_H

#include "windowsTypes.h"
#include <stdlib.h>

/**
 * the database is an array of DATABASE_NODE, a DATABASE_NODE is :
 * -an id for the couple
 * -the couple
 * -the size of the couple
 */
typedef struct _DATABASE_NODE {
	QWORD NumberId;
	DWORD DllNameLength;
	UCHAR* szDllFunctionName;
} DATABASE_NODE, *PDATABASE_NODE;

/**
 * search in a database for a wanted dll/func couple, and returns the associated ID
 * @param  DataBase              the database
 * @param  dwSizeDataBase        the size of the database
 * @param  szDllFunctionName     the couple
 * @param  dwSizeDllFunctionName the size of the couple
 * @return                       the associated ID or (QWORD)-2 if the couple is not in the database
 */
QWORD GetIdNumberFromName(PDATABASE_NODE DataBase, DWORD dwSizeDataBase, UCHAR *szDllFunctionName, DWORD dwSizeDllFunctionName);

/**
 * load a database from a file
 * @param  szFileName        the file containing the database
 * @param  TotalSizeDataBase _out: the size of he loaded database
 * @return                   a PDATABASE_NODE containing the database, or 0 if an error occurred
 */
PDATABASE_NODE LoadDataBase(UCHAR *szFileName, PDWORD TotalSizeDataBase);
PDATABASE_NODE LoadElfDataBase(UCHAR *szFileName, PDWORD TotalSizeDataBase);

#endif /* DATABASES_H */
