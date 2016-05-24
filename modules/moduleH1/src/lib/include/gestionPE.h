/**
 * @file   gestionPE.h
 *
 * implements the reading of files with MZ-PE format
 */

#ifndef GESTION_PE_H
#define GESTION_PE_H

#include "PEUtils.h"
#include "vector.h"
#include "windowsTypes.h"
#include "databases.h"
#include "h1-errors.h"

/**
 * initialize the PORTABLE_EXECUTABLE used to store the file
 * @param  Pe       a pointer on an empty PORTABLE_EXECUTABLE
 * @param  filename the file to read
 * @return          an ERROR_CODE value between :
 *                     E_FILE_NOT_FOUND, E_FILE_EMPTY, E_CALLOC_ERROR, E_READING_ERROR, E_NOT_MZ, E_NOT_PE, E_BAD_ARCHITECTURE, E_FSTAT_ERROR and UH_SUCCESS
 */
ERROR_CODE PeInit(PPORTABLE_EXECUTABLE Pe, int fd, CHAR *filename);

/**
 * free the structure
 * @param  Pe a pointer on a PORTABLE_EXECUTABLE
 */
VOID PeDestroy(PPORTABLE_EXECUTABLE Pe);

/**
* check if the file has the required format and structure according to the PE documentation
* @param  Pe the PORTABLE_EXECUTABLE representing the file
* @return    an ERROR_CODE value between : E_INVALID_STRUCTURE and UH_SUCCESS
*/
ERROR_CODE PeHasValidStructure(PPORTABLE_EXECUTABLE Pe);

/**
 * Check if a file has an entry point
 * @param  Pe the PORTABLE_EXECUTABLE representing the file
 * @return    an ERROR_CODE value between : E_NO_ENTRY_POINT, E_INVALID_ENTRY_POINT and UH_SUCCESS
 */
ERROR_CODE PeHasEntryPoint(PPORTABLE_EXECUTABLE Pe);

/**
 * Check if a file has the wanted data directory entry
 * @param  Pe                  the PORTABLE_EXECUTABLE representing the file
 * @param  imageDirectoryEntry the wanted entry
 * @return                     an ERROR_CODE value between : E_NO_ENTRY and UH_SUCCESS
 */
ERROR_CODE PeHasDataDirectory(PPORTABLE_EXECUTABLE Pe, DWORD imageDirectoryEntry);

/**
 * Extract the EAT of a file in a vector
 * @param  Pe                the PORTABLE_EXECUTABLE representing the file
 * @param  DataBase          the EAT database
 * @param  TotalSizeDataBase the size of the EAT database
 * @param  testFile          the PVECTOR where the EAT is stored
 * @return                   an ERROR_CODE value between : E_EAT_NOT_GOOD, E_EAT_EMPTY, E_CALLOC_ERROR, E_DLL_NAME_ERROR, E_FUNCTION_NAME_ERROR and UH_SUCCESS
 */
ERROR_CODE GenerateExportedFunctions(PPORTABLE_EXECUTABLE Pe, PDATABASE_NODE DataBase, DWORD TotalSizeDataBase, PVECTOR* testFile);

/**
 * Extract the IAT of a file in a vector
 * @param  Pe                the PORTABLE_EXECUTABLE representing the file
 * @param  DataBase          the IAT database
 * @param  TotalSizeDataBase the size of the IAT database
 * @param  testFile          the PVECTOR where the IAT is stored
 * @return                   an ERROR_CODE value between : E_IAT_NOT_GOOD, E_IAT_EMPTY, E_FUNCTION_NAME_ERROR, E_CALLOC_ERROR, E_DLL_NAME_ERROR and UH_SUCCESS
 */
ERROR_CODE GenerateImportedFunctions(PPORTABLE_EXECUTABLE Pe, PDATABASE_NODE DataBase, DWORD TotalSizeDataBase, PVECTOR* testFile);

#endif /* GESTION_PE_H */
