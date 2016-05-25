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

#ifndef __H1_STATIC__
#define __H1_STATIC__

#include "h1-errors.h"

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
