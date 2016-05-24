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
