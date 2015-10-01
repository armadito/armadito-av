short fileAnalysis(char* fileName);

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

short analyseElfFile(char* fileName);