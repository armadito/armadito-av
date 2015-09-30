/**
 * @file   kNN.c
 *
 * kNN implementation for the tests on IAT and EAT.
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
 
#include "distances.h"
#include "kNN.h"
#include "table.h"
#include "utils.h"

ERROR_CODE hasMalwareIAT(PVECTOR testFile, PMODEL modelArrayMalware, PMODEL modelArrayNotMalware){
	INT score = 0;
	DWORD i = 0, nbMalwareElements = 0;
	DOUBLE dist = E_DISTANCE_ERROR, tmpDist = 0;
	PTABLE bestTable;
	ERROR_CODE retValue;
	CHAR tmpModel = 0;

	/* the table with the k-nearest neighbors */
	bestTable = tableNew(NUMBER_OF_N_N);
	if(bestTable == NULL){
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
			return UH_NOT_MALWARE;
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
		if(retValue == E_FAILURE){
			tableDelete(bestTable);
			return E_TEST_ERROR;
		}
		if (dist == 0){
			tableDelete(bestTable);
			return UH_MALWARE;
		}
	}

	/*
	* we consider the distance to be relevant if inferior to the threshold
	* all the neighbors have a score (ELEMENT_MALWARE_MODEL for the malware model and ELEMENT_NOT_MALWARE_MODEL for the 
	* not malware model), and we sum those scores (if the neighbor is relevant),
	*/
	for (i = 0; i < NUMBER_OF_N_N; ++i){
		tmpDist = tableElementDistance(bestTable, i);
		tmpModel = tableElementModel(bestTable, i);
		if(tmpDist == E_DISTANCE_ERROR || tmpModel == ELEMENT_MODEL_ERROR){
			tableDelete(bestTable);
			return E_TEST_ERROR;
		}
		score += (tmpDist <= IAT_KNN_THRESHOLD ? 1 : 0)*tmpModel;
	}

	TODO;
	// ajouter un test afin que les voisins avec distance > 0.75 ou 0.8 ne soient PAS DU TOUT pris en compte

	/* if the score is null, we look for all the neighbors, not only the relevant ones */
	if(score == 0){
		for (i = 0; i < NUMBER_OF_N_N; ++i){
			tmpModel = tableElementModel(bestTable, i);
			if(tmpModel == ELEMENT_MODEL_ERROR){
				tableDelete(bestTable);
				return E_TEST_ERROR;
			}
			nbMalwareElements += (tmpModel == ELEMENT_MALWARE_MODEL ? 1 : 0);
		}
		tableShow(bestTable);
		tableDelete(bestTable);

		/* we decide based on the majority */
		return (nbMalwareElements >= (NUMBER_OF_N_N / 2) + 1 ? UH_MALWARE : UH_NOT_MALWARE);
	}else {
		/* a positive score means that the file is closer to the malware model, a negative means that it is closer to the not malware model */
		tableShow(bestTable);
		tableDelete(bestTable);
		return (score > 0 ? UH_MALWARE : UH_NOT_MALWARE);
	}	
}

ERROR_CODE isKnownEAT(PVECTOR testFile, PMODEL modelArrayMalware, PMODEL modelArrayNotMalware){
	DWORD i = 0;
	DOUBLE dist = 0;
	DOUBLE minDist = 1;
	ERROR_CODE result = UH_EAT_UNKNOWN;

	/*search for nearest neighbors in the not malware model */
	for (i = 0; i < modelArrayNotMalware->numberOfFile; i++){
		dist = distanceDFG(testFile, modelArrayNotMalware->modelFiles[i]);
		if (dist == E_DISTANCE_ERROR || !(0 <= dist && dist <= 1)){
			return E_TEST_ERROR;
		}
		if(dist == 0){ /* if the eat is known in the model */
			return UH_NOT_MALWARE;
		}
		if(dist <= EAT_KNN_THRESHOLD && dist < minDist){
			minDist = dist;
			result = UH_NOT_MALWARE;
		}
	}

	/*search for nearest neighbors in the malware model */
	for (i = 0; i < modelArrayMalware->numberOfFile; i++){
		dist = distanceDFG(testFile, modelArrayMalware->modelFiles[i]);
		if (dist == E_DISTANCE_ERROR || !(0 <= dist && dist <= 1)){
			return E_TEST_ERROR;
		}
		if (dist == 0){ /* if the eat is known in the model */
			return UH_MALWARE;
		}
		if (dist <= EAT_KNN_THRESHOLD && dist < minDist){
			minDist = dist;
			result = UH_MALWARE;
		}
	}

	return result;
}
