/**
 * @file   tfidfDetection.c
 *
 * tfidf testing.
 */

#include <stdio.h>
#include <stdlib.h>
#include "tfidfDetection.h"

PTFIDF_NODE loadTFIDFBases(CHAR *szFileName, PDWORD TotalSizeDataBase, PDWORD nbDocsInBase){
	FILE *DL = NULL;
	PTFIDF_NODE DataBase = NULL;
	DWORD dwAccDB = 0;
	DWORD tmp;

	DL = fopen(szFileName, "rb");
	if(DL == NULL){
		perror(szFileName);
		return NULL;
	}
	
	if(fread(TotalSizeDataBase, sizeof(DWORD), 1, DL) == 0){
		fclose(DL);
		return NULL;
	}

	if(fread(nbDocsInBase, sizeof(DWORD), 1, DL) == 0){
		fclose(DL);
		return NULL;
	}
	DataBase = (PTFIDF_NODE)calloc((*TotalSizeDataBase), sizeof(TFIDF_NODE));
	if(DataBase == NULL){
		return NULL;
	}

	for(dwAccDB = 0; dwAccDB < (*TotalSizeDataBase); dwAccDB++){
		if(fread(&(DataBase[dwAccDB].NumberId), 1, sizeof(QWORD), DL) == 0){
			free(DataBase);
			fclose(DL);
			return NULL;
		}
		if(fread(&(DataBase[dwAccDB].OccursNumber), 1, sizeof(DWORD), DL) == 0){
			free(DataBase);
			fclose(DL);
			return NULL;
		}
		// read the null char written in order to delimitate
		// TOFIX : remove this in the db file
		fread(&tmp, 1, 1, DL);
	}

	fclose(DL);
	return DataBase;
}

/**
 * search in a database for a wanted function, and returns the associated frequence
 * @param  DB       the database
 * @param  szDB     the size of the database
 * @param  nbDocs   number of documents in the database
 * @param  numberID the id of the function
 * @return          the frequence of the function, or 1/nbdocs if the function is not in the database
 */
DOUBLE GetFreqFromNumberId(PTFIDF_NODE DB, DWORD szDB, DWORD nbDocs, QWORD numberID){
	DWORD index;

	for(index = 0; index < szDB; index++){
		if(DB[index].NumberId == numberID){
			return ((DOUBLE)DB[index].OccursNumber)/((DOUBLE)nbDocs);
		}
	}

	return ((DOUBLE)1)/((DOUBLE)nbDocs);		/* Maximum value if not found. */
}

ERROR_CODE tfidfTest(PVECTOR testFile, 
	PTFIDF_NODE DBInf, 
	PTFIDF_NODE DBSain, 
	DWORD TotalSizeDataBaseTFIDFInf, 
	DWORD TotalSizeDataBaseTFIDFSain,
	DWORD nbDocsTFIDFInf,
	DWORD nbDocsTFIDFSain){
	DOUBLE sum = 0, tmp = 0, tmp_x = 0, tmp_y = 0;
	DWORD i = 0, j = 0;
	DWORD vecSize = 0;
	VEC_TYPE vecValue = 0;

	vecSize = vectorGetSize(testFile);
	if(vecSize == (DWORD)-1){
		return E_TEST_ERROR;
	}

	for (i = 0; i < vecSize; ++i){
		/* for each function, we compute the ratio of its freq in the malware base and its freq in the not malware base */
		vecValue = vectorGetValue(testFile, i);
		if(vecValue == (VEC_TYPE)-1){
			return E_TEST_ERROR;
		}
		tmp_x = (GetFreqFromNumberId(DBInf, TotalSizeDataBaseTFIDFInf, nbDocsTFIDFInf, vecValue));
		tmp_y = (GetFreqFromNumberId(DBSain, TotalSizeDataBaseTFIDFSain, nbDocsTFIDFSain, vecValue));
		tmp = tmp_x/tmp_y;
		if(tmp < 0.001 || tmp > 100){
			sum += tmp;
			j++;
		}
	}


	if(sum == 0 || j == 0){
		return UH_TFIDF_UNKNOWN;
	}

	sum = sum/((DOUBLE) j);
	if(sum < 0.2){
		return UH_NOT_MALWARE;
	}else if(sum > 5){
		return UH_MALWARE;
	}else{
		return UH_TFIDF_UNKNOWN;
	}
}
