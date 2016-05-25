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
 * @file   kNN.h
 *
 * kNN implementation for the tests on IAT and EAT.
 */

#ifndef KNN_H
#define KNN_H

#include "vector.h"
#include "model.h"
#include "h1-errors.h"
#include "windowsTypes.h"

/**
 * decide if the testing file IAT is closer to the model A or the model B with the k-NN method
 * @param  k 					the number of nearest neighbours
 * @param  testFile 			the file we want to test the IAT
 * @param  modelArrayMalware 	the first model
 * @param  modelArrayNotMalware the second model
 * @return             			ARMADITO_IS_MALWARE, ARMADITO_NOT_MALWARE or E_TEST_ERROR
 */
ERROR_CODE hasMalwareIAT(PVECTOR testFile, PMODEL modelArrayMalware, PMODEL modelArrayNotMalware);

/**
 * test a file to decide if its EAT is closer to the A model or the B model, or is unknown
 * @param  testFile 			the file we want to test the EAT
 * @param  modelArrayMalware 	the first model
 * @param  modelArrayNotMalware the second model
 * @return             			ARMADITO_IS_MALWARE, ARMADITO_NOT_MALWARE, ARMADITO_EAT_UNKNOWN or E_TEST_ERROR
 */
ERROR_CODE isKnownEAT(PVECTOR testFile, PMODEL modelArrayMalware, PMODEL modelArrayNotMalware);

#endif /* KNN_H */
