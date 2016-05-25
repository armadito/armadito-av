/**
 * @file   kNN.c
 *
 * kNN implementation for the tests on IAT and EAT.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "distances.h"
#include "kNN.h"
#include "table.h"
#include "utils.h"

ERROR_CODE tableStateDecide(PTABLE table){
	INT score = 0, numberOfRelevantNotMalware = 0, numberOfRelevantMalware = 0, i = 0;
	DOUBLE tmpDist = 0;
	CHAR tmpModel = 0;

	// first case : all NUMBER_OF_N_N nearest neighbors are from the same model
	for (i = 0; i < NUMBER_OF_N_N; ++i){
		tmpDist = tableElementDistance(table, i);
		tmpModel = tableElementModel(table, i);
		if (tmpDist == E_DISTANCE_ERROR || tmpModel == ELEMENT_MODEL_ERROR){
			return E_TEST_ERROR;
		}
		score += tmpModel;
	}
	if (score == NUMBER_OF_N_N*ELEMENT_MALWARE_MODEL){
		//printf("%6.2f%%\n", 100);
		return ARMADITO_IS_MALWARE;
	}
	if (score == NUMBER_OF_N_N*ELEMENT_NOT_MALWARE_MODEL){
		//printf("%6.2f%%\n", 0);
		return ARMADITO_NOT_MALWARE;
	}

	//second case : all neighbors are under the minimun similarity threshold (so have a higher distance than IAT_KNN_MINI_THRESHOLD)
	score = 0;
	for (i = 0; i < NUMBER_OF_N_N; ++i){
		tmpDist = tableElementDistance(table, i);
		tmpModel = tableElementModel(table, i);

		if (tmpDist == E_DISTANCE_ERROR || tmpModel == ELEMENT_MODEL_ERROR){
			return E_TEST_ERROR;
		}
		score += (tmpDist > IAT_KNN_MINI_THRESHOLD ? 1 : 0);
	}
	if (score == NUMBER_OF_N_N){
		//printf("N/A\n");
		return ARMADITO_NOT_DECIDED;
	}

	//third case : one or more are above the similarity threshold (so have a lower distance than IAT_KNN_THRESHOLD)
	for (i = 0; i < NUMBER_OF_N_N; ++i){
		tmpDist = tableElementDistance(table, i);
		tmpModel = tableElementModel(table, i);
		if (tmpDist == E_DISTANCE_ERROR || tmpModel == ELEMENT_MODEL_ERROR){
			tableDelete(table);
			return E_TEST_ERROR;
		}
		if (tmpDist <= IAT_KNN_THRESHOLD){
			if (tmpModel == ELEMENT_MALWARE_MODEL){
				numberOfRelevantMalware += 1;
			}
			else{
				numberOfRelevantNotMalware += 1;
			}
		}
	}
	if ((numberOfRelevantMalware + numberOfRelevantNotMalware) != 0){
		if (numberOfRelevantMalware > numberOfRelevantNotMalware){
			//printf("%6.2f%%\n", 100*((double)numberOfRelevantMalware)/((double)(numberOfRelevantMalware + numberOfRelevantNotMalware)));
			return ARMADITO_DOUBTFUL;
		}
		else{
			//printf("%6.2f%%\n", 100 - 100*((double)numberOfRelevantNotMalware)/((double)(numberOfRelevantMalware + numberOfRelevantNotMalware)));
			return ARMADITO_NOT_MALWARE;
		}
	}

	//last case : at least one NN is between IAT_KNN_MINI_THRESHOLD and IAT_KNN_THRESHOLD, but all under IAT_KNN_THRESHOLD
	numberOfRelevantNotMalware = 0;
	numberOfRelevantMalware = 0;

	for (i = 0; i < NUMBER_OF_N_N; ++i){
		tmpDist = tableElementDistance(table, i);
		tmpModel = tableElementModel(table, i);
		if (tmpDist == E_DISTANCE_ERROR || tmpModel == ELEMENT_MODEL_ERROR){
			return E_TEST_ERROR;
		}
		if (tmpModel == ELEMENT_MALWARE_MODEL){
			numberOfRelevantMalware += 1;
		}
		else{
			numberOfRelevantNotMalware += 1;
		}
	}
	if ((numberOfRelevantMalware + numberOfRelevantNotMalware) != 0){
		if (numberOfRelevantMalware > numberOfRelevantNotMalware){
			//printf("%6.2f%%\n", 100*((double)numberOfRelevantMalware)/((double)(numberOfRelevantMalware + numberOfRelevantNotMalware)));
			return ARMADITO_DOUBTFUL;
		}
		else{
			//printf("%6.2f%%\n", 100 - 100 * ((double)numberOfRelevantNotMalware) / ((double)(numberOfRelevantMalware + numberOfRelevantNotMalware)));
			return ARMADITO_NOT_MALWARE;
		}
	}

	//printf("N/A\n");
	return ARMADITO_NOT_DECIDED;
}

ERROR_CODE hasMalwareIAT(PVECTOR testFile, PMODEL modelArrayMalware, PMODEL modelArrayNotMalware){
	INT score = 0;
	DWORD i = 0, nbMalwareElements = 0;
	DOUBLE dist = E_DISTANCE_ERROR, tmpDist = 0;
	PTABLE bestTable;
	ERROR_CODE retValue;
	CHAR tmpModel = 0;

	/* the table with the k-nearest neighbors */
	bestTable = tableNew(NUMBER_OF_N_N);
	if (bestTable == NULL){
		return E_TEST_ERROR;
	}

	/* search for nearest neighbors in the not malware model */
	for (i = 0; i < modelArrayNotMalware->numberOfFile; i++){
		dist = distanceDFG(testFile, modelArrayNotMalware->modelFiles[i]);
		if (dist == E_DISTANCE_ERROR || !(0 <= dist && dist <= 1)){
			tableDelete(bestTable);
			return E_TEST_ERROR;
		}
		retValue = tableAddElement(bestTable, dist, ELEMENT_NOT_MALWARE_MODEL);
		if (retValue == E_FAILURE){
			tableDelete(bestTable);
			return E_TEST_ERROR;
		}
		if (dist == 0){
			tableDelete(bestTable);
			return ARMADITO_NOT_MALWARE;
		}
	}

	/* search for nearest neighbors in the malware model */
	for (i = 0; i < modelArrayMalware->numberOfFile; i++){
		dist = distanceDFG(testFile, modelArrayMalware->modelFiles[i]);
		if (dist == E_DISTANCE_ERROR || !(0 <= dist && dist <= 1)){
			tableDelete(bestTable);
			return E_TEST_ERROR;
		}
		retValue = tableAddElement(bestTable, dist, ELEMENT_MALWARE_MODEL);
		if (retValue == E_FAILURE){
			tableDelete(bestTable);
			return E_TEST_ERROR;
		}
		if (dist == 0){
			tableDelete(bestTable);
			return ARMADITO_IS_MALWARE;
		}
	}

	// Uncomment this in order to use the new version of the decision making for the NN table.
	/*retValue = tableStateDecide(bestTable);
	tableDelete(bestTable);
	return retValue;*/

	/*
	* we consider the distance to be relevant if inferior to the threshold
	* all the neighbors have a score (ELEMENT_MALWARE_MODEL for the malware model and ELEMENT_NOT_MALWARE_MODEL for the
	* not malware model), and we sum those scores (if the neighbor is relevant),
	*/
	for (i = 0; i < NUMBER_OF_N_N; ++i){
		tmpDist = tableElementDistance(bestTable, i);
		tmpModel = tableElementModel(bestTable, i);
		if (tmpDist == E_DISTANCE_ERROR || tmpModel == ELEMENT_MODEL_ERROR){
			tableDelete(bestTable);
			return E_TEST_ERROR;
		}
		score += (tmpDist <= IAT_KNN_THRESHOLD ? 1 : 0)*tmpModel;
	}

	TODO;
	// ajouter un test afin que les voisins avec distance > 0.75 ou 0.8 ne soient PAS DU TOUT pris en compte

	/* if the score is null, we look for all the neighbors, not only the relevant ones */
	if (score == 0){

		/*
		for (i = 0; i < NUMBER_OF_N_N; ++i){
			tmpModel = tableElementModel(bestTable, i);
			if (tmpModel == ELEMENT_MODEL_ERROR){
				tableDelete(bestTable);
				return E_TEST_ERROR;
			}
			nbMalwareElements += (tmpModel == ELEMENT_MALWARE_MODEL ? 1 : 0);
		}
		//tableShow(bestTable);
		

		/* we decide based on the majority */
		//return (nbMalwareElements >= (NUMBER_OF_N_N / 2) + 1 ? ARMADITO_IS_MALWARE : ARMADITO_NOT_MALWARE);

		// if the iat are not enough relevant to take a decision.
		DBG_PRNT("> IAT_UNDECIDED (%d)", score);  
		tableDelete(bestTable);
		return ARMADITO_NOT_DECIDED;
	}
	else {
		/* a positive score means that the file is closer to the malware model, a negative means that it is closer to the not malware model */
		//tableShow(bestTable);
		tableDelete(bestTable);
		return (score > 0 ? ARMADITO_IS_MALWARE : ARMADITO_NOT_MALWARE);
	}
}

ERROR_CODE isKnownEAT(PVECTOR testFile, PMODEL modelArrayMalware, PMODEL modelArrayNotMalware){
	DWORD i = 0;
	DOUBLE dist = 0;
	DOUBLE minDist = 1;
	ERROR_CODE result = ARMADITO_EAT_UNKNOWN;

	/*search for nearest neighbors in the not malware model */
	for (i = 0; i < modelArrayNotMalware->numberOfFile; i++){
		dist = distanceDFG(testFile, modelArrayNotMalware->modelFiles[i]);
		if (dist == E_DISTANCE_ERROR || !(0 <= dist && dist <= 1)){
			return E_TEST_ERROR;
		}
		if (dist == 0){ /* if the eat is known in the model */
			return ARMADITO_NOT_MALWARE;
		}
		if (dist <= EAT_KNN_THRESHOLD && dist < minDist){
			minDist = dist;
			result = ARMADITO_NOT_MALWARE;
		}
	}

	/*search for nearest neighbors in the malware model */
	for (i = 0; i < modelArrayMalware->numberOfFile; i++){
		dist = distanceDFG(testFile, modelArrayMalware->modelFiles[i]);
		if (dist == E_DISTANCE_ERROR || !(0 <= dist && dist <= 1)){
			return E_TEST_ERROR;
		}
		if (dist == 0){ /* if the eat is known in the model */
			return ARMADITO_IS_MALWARE;
		}
		if (dist <= EAT_KNN_THRESHOLD && dist < minDist){
			minDist = dist;
			result = ARMADITO_IS_MALWARE;
		}
	}

	return result;
}
