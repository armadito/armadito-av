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
