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
