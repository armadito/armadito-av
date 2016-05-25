/**
 * @file   model.h
 *
 */

#ifndef MODEL_H
#define MODEL_H

#include "vector.h"
#include "windowsTypes.h"

typedef struct {
	DWORD numberOfFile;
	PVECTOR* modelFiles;
} MODEL, *PMODEL;

/**
 * Load a folder into a PMODEL variable
 * @param  dirName the folder to load
 * @return         the PMODEL, or NULL if error
 */
PMODEL modelLoad(CHAR* dirName);

/**
 * Load a zip into a PMODEL variable
 * @param  zipName the zip to load
 * @return         the PMODEL, or NULL if error
 */
PMODEL modelLoadFromZip(CHAR* zipName);

/**
 * Memory freeing of the model
 * @param  M the model
 */
VOID modelDelete(PMODEL M);

#endif /* MODEL_H */
