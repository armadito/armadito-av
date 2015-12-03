/**
 * @file   databases.c
 *
 * Databases structures and loading functions.
 */

#include "databases.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osdeps.h"

QWORD GetIdNumberFromName(PDATABASE_NODE DataBase, DWORD dwSizeDataBase, UCHAR *szDllFunctionName, DWORD dwSizeDllFunctionName){
	DWORD index;

	for (index = 0; index < dwSizeDataBase; index++){
		if (DataBase[index].DllNameLength == dwSizeDllFunctionName && strcmp((const CHAR *)DataBase[index].szDllFunctionName, (const CHAR *)szDllFunctionName) == 0){
			return DataBase[index].NumberId;
		}
	}

	return (QWORD)-2;		/* Maximum value if not found. (QWORD)-1 reserved for others things */
}

/**
 * decrypt a encrypted string
 * @param  StringToConvert the string to decrypt
 * @param  stringSize      its size
 */
VOID convertString(UCHAR* StringToConvert, DWORD stringSize){
	DWORD i;

	for (i = 0; i < stringSize; i++){
		if (StringToConvert[i] >= 128){
			StringToConvert[i] -= 128;
		}
		else{
			StringToConvert[i] += 128;
		}
	}
}

/**
 * decrypt a encrypted QWORD*
 * @param  qwTmp the QWORD* to decrypt
 */
VOID convertQword(QWORD* qwTmp){
	UCHAR qwBuffer[8];
	qwBuffer[0] = (UCHAR)(*qwTmp & 0x00000000000000ff);
	qwBuffer[1] = (UCHAR)((*qwTmp & 0x000000000000ff00) >> 8);
	qwBuffer[2] = (UCHAR)((*qwTmp & 0x0000000000ff0000) >> 16);
	qwBuffer[3] = (UCHAR)((*qwTmp & 0x00000000ff000000) >> 24);
	qwBuffer[4] = (UCHAR)((*qwTmp & 0x000000ff00000000) >> 32);
	qwBuffer[5] = (UCHAR)((*qwTmp & 0x0000ff0000000000) >> 40);
	qwBuffer[6] = (UCHAR)((*qwTmp & 0x00ff000000000000) >> 48);
	qwBuffer[7] = (UCHAR)((*qwTmp & 0xff00000000000000) >> 56);
	convertString(qwBuffer, 8);
	*qwTmp = (QWORD)qwBuffer[0] + ((QWORD)qwBuffer[1] << 8) + ((QWORD)qwBuffer[2] << 16) + ((QWORD)qwBuffer[3] << 24) +
		((QWORD)qwBuffer[4] << 32) + ((QWORD)qwBuffer[5] << 40) + ((QWORD)qwBuffer[6] << 48) + ((QWORD)qwBuffer[7] << 56);
}

/**
 * decrypt a encrypted PDWORD
 * @param  dwTmp the PDWORD to decrypt
 */
VOID convertDword(PDWORD dwTmp){
	UCHAR dwBuffer[4];
	dwBuffer[0] = (UCHAR)(*dwTmp & 0x000000ff);
	dwBuffer[1] = (UCHAR)((*dwTmp & 0x0000ff00) >> 8);
	dwBuffer[2] = (UCHAR)((*dwTmp & 0x00ff0000) >> 16);
	dwBuffer[3] = (UCHAR)((*dwTmp & 0xff000000) >> 24);
	convertString(dwBuffer, 4);
	*dwTmp = dwBuffer[0] + (dwBuffer[1] << 8) + (dwBuffer[2] << 16) + (dwBuffer[3] << 24);
}

PDATABASE_NODE LoadDataBase(UCHAR *szFileName, PDWORD TotalSizeDataBase){
	FILE *DL = NULL;
	DWORD dwNumberOfBytesWriten = 0;
	PDATABASE_NODE DataBase = NULL;
	DWORD dwAccDB = 0, ui = 0;
	QWORD qwTmpNumberId;
	DWORD dwTmpDllNameLength;
	UCHAR* szTmpDllFunctionName;
	DWORD dwTmpTotalSizeDataBase;

	DL = os_fopen((CHAR *)szFileName, "rb");
	if (DL == NULL){
		return NULL;
	}

	if (fread(&(dwTmpTotalSizeDataBase), sizeof(DWORD), 1, DL) == 0){
		fclose(DL);
		return NULL;
	}
	convertDword(&dwTmpTotalSizeDataBase);
	*TotalSizeDataBase = dwTmpTotalSizeDataBase;

	DataBase = (PDATABASE_NODE)calloc((*TotalSizeDataBase), sizeof(DATABASE_NODE));
	if (DataBase == NULL){
		fclose(DL);
		return NULL;
	}

	for (dwAccDB = 0; dwAccDB < (*TotalSizeDataBase); dwAccDB++){
		if (fread(&(qwTmpNumberId), 1, sizeof(QWORD), DL) == 0){
			printf("TOFIX\n");
		}
		if (fread(&(dwTmpDllNameLength), 1, sizeof(DWORD), DL) == 0){
			printf("TOFIX\n");
		}
		convertDword(&dwTmpDllNameLength);
		convertQword(&qwTmpNumberId);
		DataBase[dwAccDB].NumberId = qwTmpNumberId;
		DataBase[dwAccDB].DllNameLength = dwTmpDllNameLength;

		szTmpDllFunctionName = (UCHAR *)calloc(DataBase[dwAccDB].DllNameLength + 1, sizeof(UCHAR));
		if (szTmpDllFunctionName == NULL){
			for (ui = 0; ui < dwAccDB; ui++){
				free(DataBase[ui].szDllFunctionName);
			}
			free(DataBase);
			fclose(DL);
			return NULL;
		}

		dwNumberOfBytesWriten = (DWORD)fread(szTmpDllFunctionName, DataBase[dwAccDB].DllNameLength + 1, sizeof(UCHAR), DL);
		convertString(szTmpDllFunctionName, DataBase[dwAccDB].DllNameLength + 1);
		DataBase[dwAccDB].szDllFunctionName = szTmpDllFunctionName;
		if (dwNumberOfBytesWriten == 0){
			for (ui = 0; ui <= dwAccDB; ui++){
				free(DataBase[ui].szDllFunctionName);
			}
			free(DataBase);
			fclose(DL);
			return NULL;
		}
	}

	fclose(DL);
	return DataBase;
}

PDATABASE_NODE LoadElfDataBase(UCHAR *szFileName, PDWORD TotalSizeDataBase){
	FILE *DL = NULL;
	DWORD dwNumberOfBytesWriten = 0;
	PDATABASE_NODE DataBase = NULL;
	DWORD dwAccDB = 0, ui = 0;
	QWORD qwTmpNumberId;
	DWORD dwTmpDllNameLength;
	UCHAR* szTmpDllFunctionName;
	DWORD dwTmpTotalSizeDataBase;

	DL = os_fopen((CHAR *)szFileName, "rb");
	if (DL == NULL){
		return NULL;
	}
	fseek(DL, 8, SEEK_SET);

	if (fread(&(dwTmpTotalSizeDataBase), sizeof(DWORD), 1, DL) == 0){
		fclose(DL);
		return NULL;
	}
	/*convertDword(&dwTmpTotalSizeDataBase);*/
	*TotalSizeDataBase = dwTmpTotalSizeDataBase;

	DataBase = (PDATABASE_NODE)calloc((*TotalSizeDataBase), sizeof(DATABASE_NODE));
	if (DataBase == NULL){
		fclose(DL);
		return NULL;
	}

	for (dwAccDB = 0; dwAccDB < (*TotalSizeDataBase); dwAccDB++){
		if (fread(&(qwTmpNumberId), 1, sizeof(QWORD), DL) == 0){
			printf("TOFIX\n");
		}
		if (fread(&(dwTmpDllNameLength), 1, sizeof(DWORD), DL) == 0){
			printf("TOFIX\n");
		}
		/*convertDword(&dwTmpDllNameLength);
		convertQword(&qwTmpNumberId);*/
		DataBase[dwAccDB].NumberId = qwTmpNumberId;
		DataBase[dwAccDB].DllNameLength = dwTmpDllNameLength;

		szTmpDllFunctionName = (UCHAR *)calloc(DataBase[dwAccDB].DllNameLength + 1, sizeof(UCHAR));
		if (szTmpDllFunctionName == NULL){
			for (ui = 0; ui < dwAccDB; ui++){
				free(DataBase[ui].szDllFunctionName);
			}
			free(DataBase);
			fclose(DL);
			return NULL;
		}

		dwNumberOfBytesWriten = (DWORD)fread(szTmpDllFunctionName, DataBase[dwAccDB].DllNameLength + 1, sizeof(UCHAR), DL);
		/*convertString(szTmpDllFunctionName, DataBase[dwAccDB].DllNameLength+1);*/
		DataBase[dwAccDB].szDllFunctionName = szTmpDllFunctionName;
		if (dwNumberOfBytesWriten == 0){
			for (ui = 0; ui <= dwAccDB; ui++){
				free(DataBase[ui].szDllFunctionName);
			}
			free(DataBase);
			fclose(DL);
			return NULL;
		}
	}

	fclose(DL);
	return DataBase;
}
