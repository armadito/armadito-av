/**
 * @file   DAVFIStaticDetection.c
 *
 * DLL creation
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "errors.h"
#include "model.h"
#include "utils.h"
#include "tfidfDetection.h"
#include "gestionPE.h"
#include "windowsTypes.h"
#include "kNN.h"
#include "databases.h"

/* dll utils */
#define 	WINAPI 				__stdcall
#define 	DLL_PROCESS_ATTACH	1
#define 	DLL_THREAD_ATTACH 	2
#define 	DLL_THREAD_DETACH 	3
#define 	DLL_PROCESS_DETACH 	0

/* global variable for the bases loading */
PMODEL modelArrayMalwareEat = NULL, modelArrayMalwareIat = NULL, modelArrayNotMalwareEat = NULL, modelArrayNotMalwareIat = NULL;
DWORD TotalSizeDataBaseEat = 0, TotalSizeDataBaseIat = 0, TotalSizeDataBaseTFIDFInf = 0, nbDocsTFIDFInf = 0, TotalSizeDataBaseTFIDFSain = 0, nbDocsTFIDFSain = 0;
PDATABASE_NODE DataBaseEat = NULL, DataBaseIat = NULL;
PTFIDF_NODE DataBaseTFIDFInf = NULL, DataBaseTFIDFSain = NULL;

/**
 * this function is for testing a file and deciding if it is a malicious file, a benign file, or something else
 * @param  fileName the file name
 * @return          an ERROR_CODE value between : UH_NOT_DECIDED, E_CALLOC_ERROR, UH_MALWARE and UH_NOT_MALWARE.
 */
ERROR_CODE fileAnalysis(CHAR* fileName){
	/*variable initialization*/
	PVECTOR testFileEat = NULL, testFileIat = NULL;
	ERROR_CODE infosArray[9] = {UH_NULL, UH_NULL, UH_NULL, UH_NULL, UH_NULL, UH_NULL, UH_NULL, UH_NULL, UH_NULL}; /* ERROR_CODE array for tests return values */
	PORTABLE_EXECUTABLE Pe;

	/* initialization of the Pe structure */
	infosArray[0] = PeInit(&Pe, fileName);
	/* if errors happened before the calloc */
	if (infosArray[0] == E_FILE_NOT_FOUND || infosArray[0] == E_FILE_EMPTY || infosArray[0] == E_INVALID_FILE_SIZE){
		return UH_NOT_DECIDED;
	}
	if(infosArray[0] == E_CALLOC_ERROR){
		return E_CALLOC_ERROR;
	}
	if (infosArray[0] == E_NOT_MZ){
		PeDestroy(&Pe);
		return UH_NOT_MALWARE;
	}
	if(infosArray[0] != UH_SUCCESS){
		PeDestroy(&Pe);
		return UH_NOT_DECIDED;
	}

	/////////////////////////////////////////////////////////////////
	// Place used for indev tests
	/////////////////////////////////////////////////////////////////



	/////////////////////////////////////////////////////////////////

	if (PeHasValidStructure(&Pe) == E_INVALID_STRUCTURE){
#if defined(_DEBUG) && defined(_MSC_VER)
		printf("> %s\nPeHasValidStructure(&Pe) == E_INVALID_STRUCTURE : %s\n", fileName, GetErrorCodeMsg(GetCurrentError()));
#endif
		PeDestroy(&Pe);
		return UH_MALWARE;
	}

// uncomment in order to only test the structure of the file
#if 0
	PeDestroy(&Pe);
	return UH_NOT_MALWARE;
#endif

	/*Iat, Eat and Entry point detection*/
	infosArray[1] = PeHasDataDirectory(&Pe, IMAGE_DIRECTORY_ENTRY_EXPORT);
	infosArray[2] = PeHasDataDirectory(&Pe, IMAGE_DIRECTORY_ENTRY_IMPORT);
	infosArray[3] = PeHasEntryPoint(&Pe);
	if (infosArray[3] == E_INVALID_ENTRY_POINT){
		PeDestroy(&Pe);
#if defined(_DEBUG) && defined(_MSC_VER)
		printf("> %s\ninfosArray[3] == E_INVALID_ENTRY_POINT\n", fileName);
#endif
		return UH_MALWARE;
	}

	if(infosArray[1] == E_NO_ENTRY && infosArray[2] == E_NO_ENTRY){
		PeDestroy(&Pe);
		if(infosArray[3] == UH_SUCCESS){
#if defined(_DEBUG) && defined(_MSC_VER)
			printf("> %s\ninfosArray[1] == E_NO_ENTRY && infosArray[2] == E_NO_ENTRY && infosArray[3] == UH_SUCCESS\n", fileName);
#endif
			return UH_MALWARE;
		}
		else{
#if defined(_DEBUG) && defined(_MSC_VER)
			printf("> %s\ninfosArray[1] == E_NO_ENTRY && infosArray[2] == E_NO_ENTRY && infosArray[3] != UH_SUCCESS\n", fileName);
#endif
			return UH_NOT_DECIDED;
		}
	}
	
	if(infosArray[1] != E_NO_ENTRY){ 
		/*generation of the EAT*/
		infosArray[4] = GenerateExportedFunctions(&Pe, DataBaseEat, TotalSizeDataBaseEat, &testFileEat);
		if(infosArray[4] == E_CALLOC_ERROR){
			PeDestroy(&Pe);
			return E_CALLOC_ERROR;
		}else if(infosArray[4] == E_EMPTY_VECTOR){
			if(infosArray[2] == E_NO_ENTRY){
				PeDestroy(&Pe);
#if defined(_DEBUG) && defined(_MSC_VER)
				printf("> %s\ninfosArray[4] == E_EMPTY_VECTOR && infosArray[2] == E_NO_ENTRY\n", fileName);
#endif
				return UH_NOT_DECIDED;
			}
		}
		/* the eat of some safe file can be empty but present due to some compilers */
		else if(infosArray[4] == E_EAT_EMPTY){
			if(infosArray[2] == E_NO_ENTRY){
				PeDestroy(&Pe);
				if(infosArray[3] == UH_SUCCESS){
#if defined(_DEBUG) && defined(_MSC_VER)
					printf("> %s\ninfosArray[2] == E_NO_ENTRY && infosArray[3] == UH_SUCCESS\n", fileName);
#endif
					return UH_MALWARE;
				}
				else{
#if defined(_DEBUG) && defined(_MSC_VER)
					printf("> %s\ninfosArray[2] == E_NO_ENTRY && infosArray[3] != UH_SUCCESS\n", fileName);
#endif
					return UH_NOT_DECIDED;
				}
			}
		}else if(infosArray[4] != UH_SUCCESS){
#if defined(_DEBUG) && defined(_MSC_VER)
			printf("> %s\ninfosArray[4] != UH_SUCCESS : %s\n", fileName, GetErrorCodeMsg(infosArray[4]));
#endif
			return UH_MALWARE;
		}else { /*tests on the EAT*/
			infosArray[4] = UH_SUCCESS;
			infosArray[6] = isKnownEAT(testFileEat, modelArrayMalwareEat, modelArrayNotMalwareEat);

			vectorDelete(testFileEat);

			if(infosArray[6] == UH_MALWARE){
				PeDestroy(&Pe);
#if defined(_DEBUG) && defined(_MSC_VER)
				printf("> %s\ninfosArray[6] == UH_MALWARE\n", fileName);
#endif
				return UH_MALWARE;
			}
			if(infosArray[2] == E_NO_ENTRY && infosArray[6] == UH_NOT_MALWARE){
				PeDestroy(&Pe);
#if defined(_DEBUG) && defined(_MSC_VER)
				printf("> %s\ninfosArray[2] == E_NO_ENTRY && infosArray[6] == UH_NOT_MALWARE\n", fileName);
#endif
				return UH_NOT_MALWARE;
			}
			if(infosArray[6] == UH_EAT_UNKNOWN && infosArray[2] == E_NO_ENTRY){
				PeDestroy(&Pe);
#if defined(_DEBUG) && defined(_MSC_VER)
				printf("> %s\ninfosArray[6] == UH_EAT_UNKNOWN && infosArray[2] == E_NO_ENTRY\n", fileName);
#endif
				return UH_NOT_DECIDED;
			}
		}
	}
	


	if(infosArray[2] != E_NO_ENTRY){
		infosArray[5] = GenerateImportedFunctions(&Pe, DataBaseIat, TotalSizeDataBaseIat, &testFileIat);
		if(infosArray[5] == E_EMPTY_VECTOR){
			PeDestroy(&Pe);
#if defined(_DEBUG) && defined(_MSC_VER)
			printf("> %s\ninfosArray[5] == E_EMPTY_VECTOR\n", fileName);
#endif
			return UH_NOT_DECIDED;
		}
		if(infosArray[5] == E_CALLOC_ERROR){
			PeDestroy(&Pe);
#if defined(_DEBUG) && defined(_MSC_VER)
			printf("> %s\ninfosArray[5] == E_CALLOC_ERROR\n", fileName);
#endif
			return UH_NOT_DECIDED;
		}
		if (infosArray[5] != UH_SUCCESS){
			PeDestroy(&Pe);
#if defined(_DEBUG) && defined(_MSC_VER)
			printf("> %s\ninfosArray[5] != UH_SUCCESS : %s\n", fileName, GetErrorCodeMsg(infosArray[5]));
#endif
			return UH_MALWARE;
		}else{ /*tests on the IAT*/
			infosArray[8] = tfidfTest(testFileIat,
									  DataBaseTFIDFInf, 
									  DataBaseTFIDFSain, 
									  TotalSizeDataBaseTFIDFInf, 
									  TotalSizeDataBaseTFIDFSain,
									  nbDocsTFIDFInf,
									  nbDocsTFIDFSain);
			
			if(infosArray[8] == UH_NOT_MALWARE){
				PeDestroy(&Pe);
				vectorDelete(testFileIat);
#if defined(_DEBUG) && defined(_MSC_VER)
				printf("> %s\ninfosArray[8] == UH_NOT_MALWARE\n", fileName);
#endif
				return UH_NOT_MALWARE;
			}
			if(infosArray[8] == UH_MALWARE){
				PeDestroy(&Pe);
				vectorDelete(testFileIat);
#if defined(_DEBUG) && defined(_MSC_VER)
				printf("> %s\ninfosArray[8] == UH_MALWARE\n", fileName);
#endif
				return UH_MALWARE;
			}

			infosArray[7] = hasMalwareIAT(testFileIat, modelArrayMalwareIat, modelArrayNotMalwareIat);
			vectorDelete(testFileIat);
		}
	}

	PeDestroy(&Pe);

	if(infosArray[7] == UH_MALWARE){
#if defined(_DEBUG) && defined(_MSC_VER)
		printf("> %s\ninfosArray[7] == UH_MALWARE\n", fileName);
#endif
		return UH_MALWARE;
	}

	if((infosArray[7] == UH_NOT_MALWARE && infosArray[6] != UH_MALWARE) || (infosArray[6] == UH_NOT_MALWARE && infosArray[7] != UH_MALWARE)){
#if defined(_DEBUG) && defined(_MSC_VER)
		printf("> %s\n(infosArray[7] == UH_NOT_MALWARE && infosArray[6] != UH_MALWARE) || (infosArray[6] == UH_NOT_MALWARE && infosArray[7] != UH_MALWARE) : %d : %d\n", fileName, infosArray[7], infosArray[6]);
#endif
		return UH_NOT_MALWARE;
	}

#if defined(_DEBUG) && defined(_MSC_VER)
	printf("> %s\nreturn UH_NOT_DECIDED\n", fileName);
#endif

	return UH_NOT_DECIDED;
}

/**
 * initialize all the databases used by the module
 * @param  modelMalwareEat    the name of the file containing the eat model of malicious files
 * @param  modelMalwareIat    the name of the file containing the iat model of malicious files
 * @param  modelNotMalwareEat the name of the file containing the eat model of safe files
 * @param  modelNotMalwareIat the name of the file containing the iat model of safe files
 * @param  databaseEat        the name of the file containing the eat database
 * @param  databaseIat        the name of the file containing the iat database
 * @param  databaseTFIDFInf   the name of the file containing the tfidf database for malicious files
 * @param  databaseTFIDFSain  the name of the file containing the tfidf database for safe files
 * @return                    a 9 bits value where the highest bit is 1 if a loading failed, 0 otherwise
 *                            the others bits are attached respectively to each database (0x1 : modelMalwareEat, ... 0x80 : databaseTFIDFSain)
 *                            -if it is 0, no problems occurs
 *                            -if it is 1, then if the highest bit is set to 0, then the db name doesn't link to an existing file
 *                            				if the highest bit is set to 0, then there was a probleme during the loading of the base
 */
SHORT initDatabases(
	CHAR* modelMalwareEat, 
	CHAR* modelMalwareIat, 
	CHAR* modelNotMalwareEat, 
	CHAR* modelNotMalwareIat, 
	CHAR* databaseEat, 
	CHAR* databaseIat, 
	CHAR* databaseTFIDFInf, 
	CHAR* databaseTFIDFSain
	){
	FILE* fd = NULL;
	SHORT filesTestsFlag = 0; 

	fd = fopen(modelMalwareEat, "rb");
	if (fd == NULL){
	    filesTestsFlag += 0x1;
	}else{
		fclose(fd);
	}

	fd = fopen(modelMalwareIat, "rb");
	if (fd == NULL){
	    filesTestsFlag += 0x2;
	}else{
		fclose(fd);
	}

	fd = fopen(modelNotMalwareEat, "rb");
	if (fd == NULL){
	    filesTestsFlag += 0x4;
	}else{
		fclose(fd);
	}

	fd = fopen(modelNotMalwareIat, "rb");
	if (fd == NULL){
	    filesTestsFlag += 0x8;
	}else{
		fclose(fd);
	}

	fd = fopen(databaseEat, "rb");
	if (fd == NULL){
	    filesTestsFlag += 0x10;
	}else{
		fclose(fd);
	}

	fd = fopen(databaseIat, "rb");
	if (fd == NULL){
	    filesTestsFlag += 0x20;
	}else{
		fclose(fd);
	}

	fd = fopen(databaseTFIDFInf, "rb");
	if (fd == NULL){
	    filesTestsFlag += 0x40;
	}else{
		fclose(fd);
	}

	fd = fopen(databaseTFIDFSain, "rb");
	if (fd == NULL){
	    filesTestsFlag += 0x80;
	}else{
		fclose(fd);
	}

	if(filesTestsFlag != 0){
		return filesTestsFlag;
	}

	/*tfidf database loading*/
	DataBaseTFIDFInf = loadTFIDFBases(databaseTFIDFInf, &TotalSizeDataBaseTFIDFInf, &nbDocsTFIDFInf);
	if(DataBaseTFIDFInf == NULL){
		filesTestsFlag += 0x40;
	}
	DataBaseTFIDFSain = loadTFIDFBases(databaseTFIDFSain, &TotalSizeDataBaseTFIDFSain, &nbDocsTFIDFSain);
	if(DataBaseTFIDFSain == NULL){
		filesTestsFlag += 0x80;
	}

	/*databases initialization*/
	DataBaseEat = LoadDataBase((UCHAR*) databaseEat, &TotalSizeDataBaseEat);
	if(DataBaseEat == NULL){
		filesTestsFlag += 0x10;
	}

	DataBaseIat = LoadDataBase((UCHAR*) databaseIat, &TotalSizeDataBaseIat);
	if(DataBaseIat == NULL){
		filesTestsFlag += 0x20;
	}

	/*loading of models*/
	modelArrayMalwareEat = modelLoadFromZip(modelMalwareEat);
	if(modelArrayMalwareEat == NULL){
		filesTestsFlag += 0x1;
	}

	modelArrayNotMalwareEat = modelLoadFromZip(modelNotMalwareEat);
	if(modelArrayNotMalwareEat == NULL){
		filesTestsFlag += 0x4;
	}

	modelArrayMalwareIat = modelLoadFromZip(modelMalwareIat);
	if(modelArrayMalwareIat == NULL){
		filesTestsFlag += 0x2;
	}

	modelArrayNotMalwareIat = modelLoadFromZip(modelNotMalwareIat);
	if(modelArrayNotMalwareIat == NULL){
		filesTestsFlag += 0x8;
	}

	if(filesTestsFlag != 0){
		filesTestsFlag += 0x100;
	}

	return filesTestsFlag;
}

/**
 * free all the memory used by the databases
 */
VOID freeDatabases(){
	DWORD ui = 0;

	modelDelete(modelArrayNotMalwareIat);
	modelArrayNotMalwareIat = NULL;
	modelDelete(modelArrayMalwareIat);
	modelArrayMalwareIat = NULL;
	modelDelete(modelArrayMalwareEat);
	modelArrayMalwareEat = NULL;
	modelDelete(modelArrayNotMalwareEat);
	modelArrayNotMalwareEat = NULL;
	for(ui = 0; ui < TotalSizeDataBaseEat; ui++){
		free(DataBaseEat[ui].szDllFunctionName);
		DataBaseEat[ui].szDllFunctionName = NULL;
	}
	free(DataBaseEat);
	DataBaseEat = NULL;
	for(ui = 0; ui < TotalSizeDataBaseIat; ui++){
		free(DataBaseIat[ui].szDllFunctionName);
		DataBaseIat[ui].szDllFunctionName = NULL;
	}
	free(DataBaseIat);
	DataBaseIat = NULL;
	free(DataBaseTFIDFInf);
	DataBaseTFIDFInf = NULL;
	free(DataBaseTFIDFSain);
	DataBaseTFIDFSain = NULL;
}

#ifdef _WIN32
/* hinstDLL and lpvReserved are unused, by needed by the windows API */
INT WINAPI DllMain(PVOID hinstDLL, DWORD fdwReason, PVOID lpvReserved){
	switch (fdwReason){
		case DLL_PROCESS_ATTACH:{
			break;
		}

		case DLL_THREAD_ATTACH:{
			break;
		}

		case DLL_THREAD_DETACH:{
			break;
		}

		case DLL_PROCESS_DETACH:{
			break;
		}
	}

	return TRUE;
}
#endif
