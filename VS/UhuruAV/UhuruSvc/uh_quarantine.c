#include "uh_quarantine.h"
#include "others.h"
#include <string.h>
#include "json\ui\ui.h"

#define PIPE_NAME "\\\\.\\pipe\\Armadito_ondemand"
//
char * GetQuarantineCompletepath() {

	char * dirpath = NULL;
	char * completePath = NULL;
	char filepath[MAX_PATH];	
	char * ptr = NULL;
	int dir_len = 0, len= 0;
	int ret = 0;
	
	__try {

		if (!GetModuleFileNameA(NULL, (LPSTR)&filepath, MAX_PATH)) {	
			printf("[-] Error :: GetQuarantineCompletepath :: GetModuleFilename failed :: GLE = %d\n",GetLastError());
			return NULL;
		}

		// Remove the module filename from the complete file path
		ptr = strrchr(filepath,'\\');
		if (ptr == NULL) {
			printf("[-] Error :: GetQuarantineCompletepath :: No backslash found in the path\n");
			return NULL;
		}

		// calc the dir buffer length.
		dir_len = (int)(ptr - filepath);
		dirpath = (char*)(calloc(dir_len+1,sizeof(char)));
		dirpath[dir_len] = '\0';

		memcpy_s(dirpath, dir_len, filepath, dir_len);
		//printf("[+] Debug :: GetQuarantineCompletepath :: dirpath = %s\n",dirpath);

		len = dir_len + strnlen(_QUARANTINE_DIR, MAX_PATH) + 2;
		
		completePath = (char*)calloc(len+1,sizeof(char));
		completePath[len] = '\0';

		strncat_s(completePath, len, dirpath, dir_len);
		strncat_s(completePath, len, "\\", 1);
		strncat_s(completePath, len, _QUARANTINE_DIR, strnlen(_QUARANTINE_DIR, MAX_PATH));		

		printf("[+] Debug :: GetQuarantineCompletepath :: completePath = %s\n",completePath);

		
	}
	__finally {

		if (dirpath != NULL) {
			free(dirpath);
			dirpath = NULL;
		}

	}	

	return completePath;
}

char * GetTimestampString( ) {

	char * timestamp = NULL;
	SYSTEMTIME st = {0};
	int len = 0;

	// get the current system time
	GetSystemTime(&st);

	len = 15; // year + month + day + hour + minute + second
	timestamp = (char*)calloc(len+1,sizeof(char));
	timestamp[len] = '\0';
	sprintf_s(timestamp,len, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	return timestamp;
}

char * BuildQuarantineFilePath(char * filepath) {

	char * qPath = NULL;
	int len = 0;
	char * filename = NULL;
	char * timestamp  = NULL;
	char * quarantine_dir = NULL;


	if (filepath == NULL) {
		printf("[-] Error :: BuildQuarantineFilePath :: Invalid file path!\n");
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," BuildQuarantineFilePath :: Invalid file path!\n");
		return NULL;
	}

	__try {

		quarantine_dir = GetQuarantineCompletepath( );
		if (quarantine_dir == NULL) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR,"Get Quarantine complete path failed\n");
			printf("[-] Error :: BuildQuarantineFilePath :: Get Quarantine complete path failed\n");
			return NULL;
		}


		// Get the file name from the complete file path
		filename = strrchr(filepath,'\\');
		if (filename == NULL) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," BuildQuarantineFilePath!strrchr() failed :: backslash not found in the path :: %s.\n",filepath);
			printf("[-] Error :: BuildQuarantineFilePath!strrchr() failed :: backslash not found in the path :: %s.\n",filepath);
			return NULL;
		}

		// get the current system time in format // year + month + day + hour + minute + second
		timestamp = GetTimestampString( );				

		// printf("[i] Debug :: BuildQuarantineFilePath :: filename = %s\n", filename);
		// printf("[i] Debug :: BuildQuarantineFilePath :: timestamp len = %d\n",strnlen_s(timestamp,MAX_PATH) );
		len = 0;
		len = strnlen_s(quarantine_dir, MAX_PATH) + strnlen_s(filepath,MAX_PATH) + strnlen_s(timestamp,MAX_PATH) +3;
		//printf("[i] Debug :: BuildQuarantineFilePath :: len = %d\n", len);


		qPath = (char*)calloc(len + 1, sizeof(char));
		qPath[len] = '\0';

		strncat_s(qPath, len, quarantine_dir, strnlen_s(quarantine_dir, MAX_PATH));
		//printf("[i] Debug :: BuildQuarantineFilePath :: path = %s\n", qPath);		
		strncat_s(qPath, len, filename, strnlen_s(filename,MAX_PATH));
		strncat_s(qPath, len, "_", 1);
		strncat_s(qPath, len, timestamp, strnlen_s(timestamp,MAX_PATH));

		printf("[i] Debug :: BuildQuarantineFilePath :: path = %s\n", qPath);
		
	}
	__finally {

		if (timestamp != NULL) {
			free(timestamp);
			timestamp = NULL;
		}

		if (quarantine_dir != NULL) {
			free(quarantine_dir);
			quarantine_dir = NULL;
		}

	}

	return qPath;

}

char * BuildLocationFilePath(char * filepath, char * specialDir) {

	char * qPath = NULL;
	int len = 0;
	char * filename = NULL;
	char * timestamp  = NULL;
	char * location_dir = NULL;


	if (filepath == NULL) {
		printf("[-] Error :: BuildLocationFilePath :: Invalid file path!\n");
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," BuildLocationFilePath :: Invalid file path!\n");
		return NULL;
	}

	__try {

		location_dir = GetLocationCompletepath(specialDir );
		if (location_dir == NULL) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR,"Get Location complete path failed\n");
			printf("[-] Error :: BuildLocationFilePath :: Get Location complete path failed\n");
			return NULL;
		}


		// Get the file name from the complete file path
		filename = strrchr(filepath,'\\');
		if (filename == NULL) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," BuildQuarantineFilePath!strrchr() failed :: backslash not found in the path :: %s.\n",filepath);
			printf("[-] Error :: BuildLocationFilePath!strrchr() failed :: backslash not found in the path :: %s.\n",filepath);
			return NULL;
		}

		// get the current system time in format // year + month + day + hour + minute + second
		timestamp = GetTimestampString( );				

		// printf("[i] Debug :: BuildQuarantineFilePath :: filename = %s\n", filename);
		// printf("[i] Debug :: BuildQuarantineFilePath :: timestamp len = %d\n",strnlen_s(timestamp,MAX_PATH) );
		len = 0;
		len = strnlen_s(location_dir, MAX_PATH) + strnlen_s(filepath,MAX_PATH) + strnlen_s(timestamp,MAX_PATH) +3;
		//printf("[i] Debug :: BuildQuarantineFilePath :: len = %d\n", len);


		qPath = (char*)calloc(len + 1, sizeof(char));
		qPath[len] = '\0';

		strncat_s(qPath, len, location_dir, strnlen_s(location_dir, MAX_PATH));
		//printf("[i] Debug :: BuildQuarantineFilePath :: path = %s\n", qPath);		
		strncat_s(qPath, len, filename, strnlen_s(filename,MAX_PATH));
		strncat_s(qPath, len, "_", 1);
		strncat_s(qPath, len, timestamp, strnlen_s(timestamp,MAX_PATH));

		printf("[i] Debug :: BuildLocationFilePath :: path = %s\n", qPath);
		
	}
	__finally {

		if (timestamp != NULL) {
			free(timestamp);
			timestamp = NULL;
		}

		if (location_dir != NULL) {
			free(location_dir);
			location_dir = NULL;
		}

	}

	return qPath;

}

int WriteQuarantineInfoFile(char * oldfilepath, char * quarantinePath, struct uhuru_report * uh_report) {

	int ret = 0;
	char * info_path = NULL;
	char * alert_path = NULL;
	int len = 0;
	char * alert_tmp = NULL;
	char * content = NULL;
	char * timestamp = NULL;
	HANDLE fh = INVALID_HANDLE_VALUE;
	int written = 0;
	struct json_object *jobj = NULL;
	

	if (oldfilepath == NULL || quarantinePath == NULL ) {
		printf("[-] Error :: WriteQuarantineInfoFile :: invalid parameter\n");
		return -1;
	}

	__try {

		// build information file path for quarantine.
		len = strnlen_s(quarantinePath ,MAX_PATH) + strnlen_s(".info" ,MAX_PATH) +1;
		info_path = (char*)calloc(len + 1, sizeof(char));
		info_path[len] = '\0';

		strncat_s(info_path, len, quarantinePath, strnlen_s(quarantinePath ,MAX_PATH));
		strncat_s(info_path, len, ".info", strnlen_s(".info" ,MAX_PATH));

		printf("[+] Debug :: WriteQuarantineInfoFile :: info file path = %s\n",info_path);

		// build information file path for quarantine.
		alert_tmp = BuildLocationFilePath(oldfilepath, ALERT_DIR);
		len = 0;
		len = strnlen_s(alert_tmp ,MAX_PATH) + strnlen_s(".info" ,MAX_PATH) +1;
		alert_path = (char*)calloc(len + 1, sizeof(char));
		alert_path[len] = '\0';

		strncat_s(alert_path, len, alert_tmp, strnlen_s(alert_tmp ,MAX_PATH));
		strncat_s(alert_path, len, ".info", strnlen_s(".info" ,MAX_PATH));

		
		printf("[+] Debug :: WriteQuarantineInfoFile :: alert file path = %s\n",alert_path);


		// create content. Ex: {"date":"20160224142724","path":"C:\\Quarantine\\malware.txt","module":"clamav"}

		timestamp = GetTimestampString();

		jobj = json_object_new_object();
		json_object_object_add(jobj, "date", json_object_new_string(timestamp));
		json_object_object_add(jobj, "path", json_object_new_string(oldfilepath));

		if (uh_report->mod_name != NULL) {
			json_object_object_add(jobj, "module", json_object_new_string(uh_report->mod_name));
		}
		else {
			json_object_object_add(jobj, "module", json_object_new_string("black_list"));
		}

		if (uh_report->mod_report != NULL) {
			json_object_object_add(jobj, "desc", json_object_new_string(uh_report->mod_report));
		}
		else {
			json_object_object_add(jobj, "desc", json_object_new_string("uh_malware"));
		}
		
		content = json_object_to_json_string(jobj);
		if (content == NULL) {
			printf("[-] Error :: WriteQuarantineInfoFile :: can't build info content\n");
			ret = -2;
			__leave;
		}
		//printf("[+] Debug :: json content = %s \n",content);
		
		
		// write in file.
		/*if ((ret = json_object_to_file(info_path, jobj)) != 0) {
			printf("[-] Error :: WriteQuarantineInfoFile :: writing information file failed !\n",ret);
			__leave;
		}*/

		fh = CreateFile(info_path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (fh == INVALID_HANDLE_VALUE) {
			ret = -3;
			printf("[-] Error :: WriteQuarantineInfoFile :: can't open the info file :: GLE = %d\n",GetLastError());
			__leave;
		}

		len = strnlen(content,2048);

		if (WriteFile(fh, content, len, &written, NULL) == FALSE) {
			ret = -3;
			printf("[-] Error :: WriteQuarantineInfoFile :: write failed :: GLE = %d\n",GetLastError());
			__leave;
		}

		if (fh != INVALID_HANDLE_VALUE) {
			CloseHandle(fh);
			fh = INVALID_HANDLE_VALUE;
		}

		// copy info file in alert
		if (CopyFile(info_path,alert_path,TRUE)  == FALSE){
			//ret = -4;
			printf("[-] Error :: WriteQuarantineInfoFile :: Copy info file in alert dir failed :: GLE = %d\n",GetLastError());
			//__leave;
		}			

	}
	__finally {

		if (info_path != NULL) {
			free(info_path);
			info_path = NULL;
		}

		if (alert_path != NULL) {
			free(alert_path);
			alert_path = NULL;
		}

		if (timestamp != NULL) {
			free(timestamp);
			timestamp = NULL;
		}

		if (jobj != NULL) {
			json_object_put(jobj);
		}
		
		if (fh != INVALID_HANDLE_VALUE) {
			CloseHandle(fh);
			fh = INVALID_HANDLE_VALUE;
		}
		
	}

	return  ret;

}


int MoveFileInQuarantine(char * filepath, struct uhuru_report * uh_report) {

	int ret = 0;
	char * quarantineFilepath = NULL;

	if (filepath == NULL) {
		printf("[-] Error :: MoveFileInQuarantine :: Invalid file path!\n");
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," MoveFileInQuarantine :: Invalid file path!\n");
		return -1;
	}

	__try {

		// build quarantine file path.
		quarantineFilepath = BuildQuarantineFilePath(filepath);
		if (quarantineFilepath == NULL) {
			printf("[-] Error :: MoveFileInQuarantine :: Build Quarantine FilePath failed !\n");
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," Build Quarantine FilePath failed ! :: [%s]\n",filepath);
			ret = -2;
			__leave;
		}

		// Write info file
		if (WriteQuarantineInfoFile(filepath, quarantineFilepath, uh_report) != 0) {
			ret = -3;
			printf("[-] Error :: MoveFileInQuarantine :: Write Quarantine Information File failed!\n");
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," Write Quarantine Information File failed! :: [%s] :: [%s]\n",filepath, quarantineFilepath);
			__leave;
		}
		
		// Test if the quarantine folder is present.		
		if (MoveFileEx(filepath,quarantineFilepath,MOVEFILE_REPLACE_EXISTING|MOVEFILE_FAIL_IF_NOT_TRACKABLE|MOVEFILE_COPY_ALLOWED) == FALSE){
			printf("[-] Error :: MoveFileInQuarantine :: Move file [%s] to quarantine failed ! :: GLE = %d\n",filepath,GetLastError());
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," Move file [%s] to quarantine folder failed !\n",filepath);
			ret = -4;
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


int RestoreFileFromQuarantine(char * filename) {

	int ret = 0;
	char * quarantine_dir = NULL;
	char * infopath = NULL;
	char * info = NULL;
	int info_len = 0;
	char * filepath = NULL;
	char * old_filepath = NULL;
	int len = 0;

	enum json_tokener_error jerr;
	struct json_object  * jobj = NULL, * jobj_path = NULL;

	if (filename == NULL) {
		printf("[-] Error :: RestoreFileFromQuarantine :: invalid parameter\n");
		return -1;
	}

	__try {

		// Get Quarantine directory path.
		quarantine_dir = GetQuarantineCompletepath( );
		if (quarantine_dir == NULL) {
			printf("[-] Error :: RestoreFileFromQuarantine :: Can't get the Quarantine folder path.\n");
			ret = -2;
			__leave;
		}

		// quarntine file complete path.
		len = 0;
		len = strnlen(quarantine_dir, MAX_PATH) + strnlen(filename, MAX_PATH) + 2;		
		filepath = (char*)calloc(len+1,sizeof(char));
		filepath[len] = '\0';
		strncat_s(filepath, len, quarantine_dir, strnlen(quarantine_dir, MAX_PATH));
		strncat_s(filepath, len, "\\", 1);		
		strncat_s(filepath, len, filename, strnlen(filename, MAX_PATH));			
		printf("[+] Debug :: RestoreFileFromQuarantine :: filepath = %s\n",filepath);


		// Get information file content.
		len = 0;
		len = strnlen(quarantine_dir, MAX_PATH) + strnlen(filename, MAX_PATH) + strnlen(".info", MAX_PATH) + 2;		
		infopath = (char*)calloc(len+1,sizeof(char));
		infopath[len] = '\0';

		strncat_s(infopath, len, quarantine_dir, strnlen(quarantine_dir, MAX_PATH));
		strncat_s(infopath, len, "\\", 1);		
		strncat_s(infopath, len, filename, strnlen(filename, MAX_PATH));
		strncat_s(infopath, len, ".info", strnlen(".info", MAX_PATH));		
		printf("[+] Debug :: RestoreFileFromQuarantine :: infopath = %s\n",infopath);

		info = GetFileContent(infopath, &info_len);
		if (info == NULL || info_len <= 0) {
			printf("[-] Error :: RestoreFileFromQuarantine :: Can't get info file content!\n");
			ret = -3;
			__leave;
		}

		printf("[+] Debug :: RestoreFileFromQuarantine :: info content = %s\n",info);

		// parse json content
		jobj =  json_tokener_parse_verbose(info,&jerr);
		if (jobj == NULL) {			
			printf("[-] Error :: RestoreFileFromQuarantine :: Parsing description file failed! :: error = %d\n",jerr);
			ret = -4;
			__leave;
		}


		// format verification
		if (!json_object_object_get_ex(jobj, "path", &jobj_path)) {
			printf("[-] Error :: RestoreFileFromQuarantine :: [path] not present in json object :: \n");
			ret = -5;
			__leave;
		}

		if (!json_object_is_type(jobj_path, json_type_string)) {
			printf("[-] Error :: RestoreFileFromQuarantine :: bad object type for [path] :: \n");
			ret = -6;
			__leave;
		}

		old_filepath = _strdup(json_object_get_string(jobj_path));
		printf("[+] Debug :: RestoreFileFromQuarantine :: old filepath = %s\n", old_filepath);
		
		 
		// Restore the file to its previous location.
		if (MoveFileEx(filepath,old_filepath,MOVEFILE_REPLACE_EXISTING|MOVEFILE_FAIL_IF_NOT_TRACKABLE) == FALSE){
			printf("[-] Error :: RestoreFileFromQuarantine :: Move file [%s] to previous location [%s] failed ! :: GLE = %d\n",filepath,old_filepath, GetLastError());
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," Move file [%s] to previous location [%s] failed ! :: GLE = %d\n",filepath, old_filepath,GetLastError());
			ret = -7;
			__leave;
		}

		// remove info file.
		if (DeleteFileA(infopath) == FALSE) {
			printf("[-] Error :: RestoreFileFromQuarantine :: Delete the quarantine info file [%s] failed ! :: GLE = %d\n",infopath, GetLastError());
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR," Delete the quarantine info file [%s] failed ! :: GLE = %d\n",infopath, GetLastError());
			ret = -8;
			__leave;

		}

		printf("[+] Debug :: RestoreFileFromQuarantine :: File [%s] restored to [%s] location successfully !\n", filepath, old_filepath);	
		uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_INFO," File [%s] restored to [%s] location successfully !\n", filepath, old_filepath);


		

	}
	__finally {
		
		if (quarantine_dir != NULL) {
			free(quarantine_dir);
			quarantine_dir = NULL;
		}

		if (infopath != NULL) {
			free(infopath);
			infopath = NULL;
		}

		if (info != NULL) {
			free(info);
			info = NULL;
		}

		if (old_filepath != NULL) {
			free(old_filepath);
			old_filepath = NULL;
		}

		if (jobj != NULL) {
			json_object_put(jobj);
		}
		if (jobj_path != NULL) {
			json_object_put(jobj_path);
		}
	}
	

	return ret;

}


int EnumQuarantine( ) {

	int ret = 0;
	enum uhuru_json_status status = JSON_OK;
	char * request = "{ \"av_request\":\"quarantine\", \"id\":123, \"params\": {\"action\":\"enum\"}}";
	int request_len = 0;
	char response[4096] = {0};
	int response_len = 4096;

	

	__try {

		request_len = strnlen_s(request,_MAX_PATH);
		
		status = json_handler_ui_request(PIPE_NAME, request, request_len, response, response_len);
		if (status != JSON_OK) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR,"[-] Error :: EnumQuarantine :: json_handler_ui_request failed :: status= %d \n", status);
			ret = -1;
			__leave;
		}

		printf("av_response = %s\n",response);

	}
	__finally {

	}

	return ret;
}

int ui_restore_quarantine_file(char * filename) {

	int ret = 0;
	enum uhuru_json_status status = JSON_OK;
	char * request = "{ \"av_request\":\"quarantine\", \"id\":123, \"params\": {\"action\":\"restore\" , \"fname\":\"UH_EICAR - Copie (3).txt_20160302142554\"}}";
	int request_len = 0;
	char response[4096] = {0};
	int response_len = 4096;


	__try {

		request_len = strnlen_s(request,_MAX_PATH);
		
		status = json_handler_ui_request(PIPE_NAME, request, request_len, response, response_len);
		if (status != JSON_OK) {
			uhuru_log(UHURU_LOG_SERVICE,UHURU_LOG_LEVEL_ERROR,"[-] Error :: EnumQuarantine :: json_handler_ui_request failed :: status= %d \n", status);
			ret = -1;
			__leave;
		}

		printf("av_response = %s\n",response);

	}
	__finally {

	}

	return ret;

}