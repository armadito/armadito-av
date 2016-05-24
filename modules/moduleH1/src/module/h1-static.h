#ifndef __H1_STATIC__
#define __H1_STATIC__

#include "uh_errors.h"

ERROR_CODE fileAnalysis(int fd, char *fileName);

short initDatabases(
	char* modelMalwareEat, 
	char* modelMalwareIat, 
	char* modelNotMalwareEat, 
	char* modelNotMalwareIat, 
	char* databaseEat, 
	char* databaseIat, 
	char* databaseTFIDFInf, 
	char* databaseTFIDFSain
	);

void freeDatabases();

int initDB(char* dbName,
	char* malicousDbName,
	char* notMalicousDbName,
	char* ElfdatabaseTFIDFInf,
	char* ElfdatabaseTFIDFSain);

void freeDB(void);

ERROR_CODE analyseElfFile(int fd, char *fileName);

#endif
