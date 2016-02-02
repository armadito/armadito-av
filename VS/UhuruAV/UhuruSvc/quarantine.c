#include "quarantine.h"


char * BuildQuarantineFilePath(char * filepath ) {

	char * qPath = NULL;
	int len = 0;
	char * filename = NULL;
	char * timestamp  = NULL;
	SYSTEMTIME st;

	if (filepath == NULL) {
		printf("[-] Error :: BuildQuarantineFilePath :: Invalid file path!\n");
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," BuildQuarantineFilePath :: Invalid file path!\n");
		return NULL;
	}

	__try {

		// Get the file name from the complete file path
		filename = strrchr(filepath,'\\');
		if (filename == NULL) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," BuildQuarantineFilePath!strrchr() failed :: backslash not found in the path :: %s.\n",filepath);
			printf("[-] Error :: BuildQuarantineFilePath!strrchr() failed :: backslash not found in the path :: %s.\n",filepath);
			return NULL;
		}

		// get the current system time
		GetSystemTime(&st);

		printf("[i] Debug :: System time = %02d/%02d/%04d %02d:%02d:%02d\n",st.wDay,st.wMonth,st.wYear, st.wHour,st.wMinute,st.wSecond);		
		//printf("[i] Debug :: System time = %04d/%02d/%02d %02d:%02d:%02d\n",st.wYear,st.wMonth,st.wDay, st.wHour,st.wMinute,st.wSecond);				

		len = 16; // year + month + day + hour + minute + second + '_'
		timestamp = (char*)calloc(len+1,sizeof(char));
		timestamp[len] = '\0';
		sprintf_s(timestamp,len, "_%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		//printf("[i] Debug :: timestamp = %s\n",timestamp);


		// printf("[i] Debug :: BuildQuarantineFilePath :: filename = %s\n", filename);
		// printf("[i] Debug :: BuildQuarantineFilePath :: timestamp len = %d\n",strnlen_s(timestamp,MAX_PATH) );
		len = 0;
		len = strnlen_s(QUARANTINE_DIR, MAX_PATH) + strnlen_s(filepath,MAX_PATH) + strnlen_s(timestamp,MAX_PATH) +2;
		//printf("[i] Debug :: BuildQuarantineFilePath :: len = %d\n", len);


		qPath = (char*)calloc(len + 1, sizeof(char));
		qPath[len] = '\0';

		strncat_s(qPath, len, QUARANTINE_DIR, strnlen_s(QUARANTINE_DIR, MAX_PATH));
		//printf("[i] Debug :: BuildQuarantineFilePath :: path = %s\n", qPath);		
		strncat_s(qPath, len, filename, strnlen_s(filename,MAX_PATH));
		strncat_s(qPath, len, timestamp, strnlen_s(timestamp,MAX_PATH));

		printf("[i] Debug :: BuildQuarantineFilePath :: path = %s\n", qPath);

		

		
	}
	__finally {

		if (timestamp != NULL) {
			free(timestamp);
			timestamp = NULL;
		}

	}

	return qPath;

}

int MoveFileInQuarantine(char * filepath) {

	int ret = 0;
	char * quarantineFilepath = NULL;

	if (filepath == NULL) {
		printf("[-] Error :: MoveFileInQuarantine :: Invalid file path!\n");
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," MoveFileInQuarantine :: Invalid file path!\n");
		return -1;
	}

	__try {

		// build quarantine file path.
		quarantineFilepath = BuildQuarantineFilePath( filepath);
		if (quarantineFilepath == NULL) {
			printf("[-] Error :: MoveFileInQuarantine :: Build Quarantine FilePath failed !\n");
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," Build Quarantine FilePath failed ! :: [%s]\n",filepath);
			ret = -2;
			__leave;
		}
		
		// Test if the quarantine folder is present.		
		if (MoveFileEx(filepath,quarantineFilepath,MOVEFILE_REPLACE_EXISTING|MOVEFILE_FAIL_IF_NOT_TRACKABLE) == FALSE){
			printf("[-] Error :: MoveFileInQuarantine :: Move file [%s] to quarantine failed !\n",filepath);
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," Move file [%s] to quarantine folder failed !\n",filepath);
			ret = -3;
			__leave;
		}

		printf("[+] Debug :: File [%s] moved to quarantine folder successfully !\n", filepath);	
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_INFO," File [%s] moved to quarantine folder successfully !\n",filepath );
		
	}
	__finally {

		if (quarantineFilepath != NULL) {
			free(quarantineFilepath);
			quarantineFilepath = NULL;
		}

	}
	


	// change file name.


	return ret;

}