#include "quarantine.h"
#include <stdio.h>
#include <libarmadito.h>
#include <Windows.h>
#include "os\dir.h"
#include "os\string.h"
#include "os\file.h"

#include "utils\others.h"

typedef int (*dirent_cb_t)(const char *full_path, enum os_file_flag flags, int entry_errno, void *data);

static enum os_file_flag dirent_flags(DWORD fileAttributes)
{
	switch(fileAttributes) {

	case FILE_ATTRIBUTE_DIRECTORY:
		return FILE_FLAG_IS_DIRECTORY;
	case FILE_ATTRIBUTE_DEVICE:
		return FILE_FLAG_IS_DEVICE;
		//case DT_LNK:
		//return FILE_FLAG_IS_LINK;
	case FILE_ATTRIBUTE_NORMAL:
		return FILE_FLAG_IS_PLAIN_FILE;
	case FILE_ATTRIBUTE_ARCHIVE:
		return FILE_FLAG_IS_PLAIN_FILE;
	default:
		return FILE_FLAG_IS_UNKNOWN;
	}
}

char *os_strerror(int errnum)
{
	char * msg = NULL;
	int size = MAXPATHLEN;

	msg = (char*)calloc(size + 1, sizeof(char));
	msg[size] = '\0';

	strerror_s(msg,size,errno);

	return msg;
}

int process_info(const char * full_path,enum os_file_flag flags, int entry_errno, void *data ) {

	int ret = 0;
	//json_object * jobj = NULL;
	json_object * jinfo = NULL;
	json_object * jfiles = NULL;
	enum json_tokener_error jerr;
	char * content = NULL;
	char * jfilecontent = NULL;
	int len = 0;

        if(strlen(str) < 5){
	   printf("[+] Debug :: Invalid full_path : %s\n",full_path);
	   return 1;
	}

	if(strcmp(str + strlen(str) - 5, ".info") != 0)
	  return 1;

	// open the file.
	printf("[+] Debug :: process_info :: %s :: data = %d\n",full_path,data);

	jfiles = (json_object *)data;

	__try {

		content = GetFileContent(full_path,&len);
		if (content == NULL || len <= 0) {
			printf("[-] Error :: process_info :: can't get info file content !\n");
			ret = -2;
			__leave;
		}

		jinfo = json_tokener_parse_verbose(content,&jerr);
		if (jinfo == NULL) {
			//printf("[-] Error :: process_info :: Parsing description file failed! :: error = %d\n",jerr);
			ret = -4;
			__leave;
		}
		//jfiles = json_object_new_object();

		/*if (!json_object_object_get_ex(jobj, "files", &jfiles)) {
		  printf("[-] Error :: process_info :: [path] not present in json object :: \n");
		  ret = -5;
		  __leave;
		  }
		*/

		// checko object type
		if (!json_object_is_type((json_object *)data, json_type_array)) {
			printf("[-] Error :: process_info :: bad object type :: \n");
			ret = -6;
			__leave;
		}

		json_object_array_add((json_object *)data, jinfo);

		//jfilecontent = json_object_to_json_string((json_object *)data);
		//printf("[+] Debug :: process_info :: jfile content %s\n\n",jfilecontent);
		printf("[+] Debug :: process_info :: ####\n");
		/*if (jobj == NULL) {

		  jobj = json_object_new_object();
		  json_object_object_add(jobj, "date", json_object_new_string(timestamp));

		  }*/


	}
	__finally {

		if (content != NULL) {
			free(content);
			content = NULL;
		}

		if (jfilecontent != NULL) {
			free(jfilecontent);
			jfilecontent = NULL;
		}

		/*if (jinfo != NULL) {
		  json_object_put(jinfo);
		  }*/

	}

	return ret;
}

int qu_os_dir_map(const char *path, int recurse, dirent_cb_t dirent_cb, void * data ) {

	char * sPath = NULL, *entryPath= NULL;
	char * escapedPath = NULL;

	HANDLE fh = INVALID_HANDLE_VALUE;
	int size = 0;
	WIN32_FIND_DATAA fdata ;
	WIN32_FIND_DATAA tmp ;
	enum os_file_flag flags;
	json_object * jobj = NULL;

	// Check parameters
	if (path == NULL || dirent_cb == NULL) {
		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "Error :: NULL parameter in function os_dir_map()");
		return 1;
	}

	// We escape ? and * characters
	// escapedPath = cpp_escape_str(path);

	// Check if it is a directory
	if (!(GetFileAttributesA(path) & FILE_ATTRIBUTE_DIRECTORY)) {
		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "Warning :: os_dir_map() :: (%s) is not a directory. ", path);
		return 1;
	}

	size = strlen(path) + 3;
	sPath = (char*)calloc(size + 1, sizeof(char));
	sPath[size] = '\0';
	sprintf_s(sPath, size, "%s\\*", path); // UF:: modif :: .info files only.
	//sprintf_s(sPath, size, "%s\\*.info", path); // UF:: modif :: .info files only.

	printf("[+] Debug :: os_dir_map :: sPath = %s\n",sPath);

	//g_log(NULL, ARMADITO_LOG_LEVEL_WARNING, "os_dir_map() :: (%s)", sPath);

	/*
	  FindFirstFile note
	  Be aware that some other thread or process could create or delete a file with this name between the time you query for the result and the time you act on the information. If this is a potential concern for your application, one possible solution is to use the CreateFile function with CREATE_NEW (which fails if the file exists) or OPEN_EXISTING (which fails if the file does not exist).
	*/


	fh = FindFirstFile(sPath, &fdata);
	if (fh == INVALID_HANDLE_VALUE) {
		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "Warning :: os_dir_map() :: FindFirstFileA() failed ::  (%s) :: [%s]", os_strerror(errno),sPath);
		return 1;
	}

	while (FindNextFile(fh, &tmp) != FALSE) {


		//printf("qu_os_dir_map :::: file = %s--------------\n",tmp.cFileName);

		// exclude paths "." and ".."
		if (strncmp(tmp.cFileName,".",strlen(tmp.cFileName)) == 0 || strncmp(tmp.cFileName,"..",strlen(tmp.cFileName)) == 0)
		{
			continue;
		}

		// build the entry complete path.
		size = strlen(path)+strlen(tmp.cFileName)+2;

		entryPath = (char*)calloc(size + 1, sizeof(char));
		entryPath[size] = '\0';
		sprintf_s(entryPath, size,"%s\\%s", path, tmp.cFileName);


		// If it is a directory and we do recursive scan
		if ((GetFileAttributesA(entryPath) & FILE_ATTRIBUTE_DIRECTORY) && recurse >= 1) {
			qu_os_dir_map(entryPath, recurse, dirent_cb, data);
		}
		else {
			printf("qu_os_dir_map :::: --------------\n");
			flags = dirent_flags(tmp.dwFileAttributes);
			(*dirent_cb)(entryPath, flags, 0, data);
		}

		free(entryPath);
	}

	free(sPath);
	FindClose(fh);

	return 0;
}

json_object * quarantine_enum_files_cb(struct armadito *armadito) {

	char * quarantine_dir = NULL;
	char * conf_quarantine_dir = NULL;
	json_object * jobj = NULL;
	json_object * jinfo = NULL;
	json_object * jfiles = NULL;
	int ret = 0,count = 0;
	struct a6o_conf * conf = NULL;

	__try {

		//quarantine_dir = GetLocationCompletepath("Quarantine");
		conf = a6o_get_conf(armadito);
		if (conf == NULL) {
			printf("[-] Error :: quarantine_enum_files_cb :: can't get configuration!\n");
			ret = -1;
			__leave;
		}		

		conf_quarantine_dir = a6o_conf_get_string(conf,"quarantine","quarantine-dir");
		// TODO :: quarantine_dir = GetLocationCompletepath("Quarantine");
		if (conf_quarantine_dir == NULL) {
			printf("[-] Error :: quarantine_enum_files_cb :: can't get quarantine directory from configuration!\n");
			ret = -1;
			__leave;
		}


		quarantine_dir = GetLocationCompletepath(conf_quarantine_dir);
		if (quarantine_dir == NULL) {
			printf("[-] Error :: quarantine_enum_files_cb :: can't get quarantine complete path!\n");
			ret = -1;
			__leave;
		}
		

		if ((jfiles = json_object_new_array( ))== NULL) {
			printf("[-] Error :: quarantine_enum_files_cb :: can't create json object!\n");
			ret = -1;
			__leave;
		}

		printf("[+] Debug :: json list content = %s \n",json_object_to_json_string(jfiles));

		// Get quarantine directory from module.
		qu_os_dir_map(quarantine_dir, 0, process_info, jfiles);

		count = json_object_array_length(jfiles);

		printf("[+] Debug :: quarantine_enum_files_cb :: count = %d\n",count);
		//printf("[+] Debug :: json list content = %s \n",json_object_to_json_string(jfiles));

		if ((jobj = json_object_new_object()) == NULL) {
			printf("[-] Error :: quarantine_response_cb :: can't create json object!\n");
			ret = -2;
			__leave;
		}

		if ((jinfo = json_object_new_object()) == NULL) {
			printf("[-] Error :: quarantine_response_cb :: can't create json object!\n");
			ret = -3;
			__leave;
		}

		// add items
		json_object_object_add(jobj, "count", json_object_new_int(count));
		json_object_object_add(jobj, "last", json_object_new_string("1970-01-01 00:00"));
		json_object_object_add(jobj, "files",jfiles);
	}
	__finally {

		if (quarantine_dir != NULL) {
			free(quarantine_dir);
			quarantine_dir = NULL;
		}

	}

	return jobj;

}

json_object * quarantine_restore_file_cb(char* filename,struct armadito *armadito) {

	json_object * jfiles = NULL;
	char * quarantine_dir = NULL;
	char * conf_quarantine_dir = NULL;
	char * infopath = NULL;
	char * info = NULL;
	int info_len = 0;
	char * filepath = NULL;
	char * old_filepath = NULL;
	int len = 0;
	int ret = 0;

	enum json_tokener_error jerr;
	struct json_object  * jobj = NULL, * jobj_path = NULL;
	struct a6o_conf * conf = NULL;

	__try {

		if (filename == NULL) {
			__leave;
		}

		//quarantine_dir = GetLocationCompletepath("Quarantine");
		conf = a6o_get_conf(armadito);
		if (conf == NULL) {
			printf("[-] Error :: quarantine_restore_file_cb :: can't get configuration!\n");
			ret = -1;
			__leave;
		}

		conf_quarantine_dir = a6o_conf_get_string(conf,"quarantine","quarantine-dir");
		if (conf == NULL) {
			printf("[-] Error :: quarantine_restore_file_cb :: can't get configuration dir from configuration!\n");
			ret = -1;
			__leave;
		}

		quarantine_dir = GetLocationCompletepath(conf_quarantine_dir);
		if (quarantine_dir == NULL) {
			printf("[-] Error :: quarantine_restore_file_cb :: can't get quarantine complete path!\n");
			ret = -1;
			__leave;
		}


		// quarantine file complete path.
		len = 0;
		len = strnlen(quarantine_dir, MAX_PATH) + strnlen(filename, MAX_PATH) + 2;
		filepath = (char*)calloc(len+1,sizeof(char));
		filepath[len] = '\0';
		strncat_s(filepath, len, quarantine_dir, strnlen(quarantine_dir, MAX_PATH));
		strncat_s(filepath, len, "\\", 1);
		strncat_s(filepath, len, filename, strnlen(filename, MAX_PATH));
		printf("[+] Debug :: quarantine_restore_file_cb :: filepath = %s\n",filepath);

		// Get information file content.
		len = 0;
		len = strnlen(quarantine_dir, MAX_PATH) + strnlen(filename, MAX_PATH) + strnlen(".info", MAX_PATH) + 2;
		infopath = (char*)calloc(len+1,sizeof(char));
		infopath[len] = '\0';

		strncat_s(infopath, len, quarantine_dir, strnlen(quarantine_dir, MAX_PATH));
		strncat_s(infopath, len, "\\", 1);
		strncat_s(infopath, len, filename, strnlen(filename, MAX_PATH));
		strncat_s(infopath, len, ".info", strnlen(".info", MAX_PATH));
		printf("[+] Debug :: quarantine_restore_file_cb :: infopath = %s\n",infopath);

		info = GetFileContent(infopath, &info_len);
		if (info == NULL || info_len <= 0) {
			printf("[-] Error :: quarantine_restore_file_cb :: Can't get info file content!\n");
			ret = -3;
			__leave;
		}

		printf("[+] Debug :: quarantine_restore_file_cb :: info content = %s\n",info);

		// parse json content
		jobj =  json_tokener_parse_verbose(info,&jerr);
		if (jobj == NULL) {
			printf("[-] Error :: quarantine_restore_file_cb :: Parsing description file failed! :: error = %d\n",jerr);
			ret = -4;
			__leave;
		}

		// format verification
		if (!json_object_object_get_ex(jobj, "path", &jobj_path)) {
			printf("[-] Error :: quarantine_restore_file_cb :: [path] not present in json object :: \n");
			ret = -5;
			__leave;
		}

		if (!json_object_is_type(jobj_path, json_type_string)) {
			printf("[-] Error :: quarantine_restore_file_cb :: bad object type for [path] :: \n");
			ret = -6;
			__leave;
		}

		old_filepath = _strdup(json_object_get_string(jobj_path));
		printf("[+] Debug :: quarantine_restore_file_cb :: old filepath = %s\n", old_filepath);

		// Restore the file to its previous location.
		if (MoveFileEx(filepath,old_filepath,MOVEFILE_REPLACE_EXISTING|MOVEFILE_FAIL_IF_NOT_TRACKABLE) == FALSE){
			printf("[-] Error :: quarantine_restore_file_cb :: Move file [%s] to previous location [%s] failed ! :: GLE = %d\n",filepath,old_filepath, GetLastError());
			a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_ERROR," Move file [%s] to previous location [%s] failed ! :: GLE = %d\n",filepath, old_filepath,GetLastError());
			ret = -7;
			__leave;
		}

		// remove info file.
		if (DeleteFileA(infopath) == FALSE) {
			printf("[-] Error :: quarantine_restore_file_cb :: Delete the quarantine info file [%s] failed ! :: GLE = %d\n",infopath, GetLastError());
			a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_ERROR," Delete the quarantine info file [%s] failed ! :: GLE = %d\n",infopath, GetLastError());
			ret = -8;
			__leave;

		}

		printf("[+] Debug :: quarantine_restore_file_cb :: File [%s] restored to [%s] location successfully !\n", filepath, old_filepath);
		a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_INFO," File [%s] restored to [%s] location successfully !\n", filepath, old_filepath);

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

		if (filepath != NULL) {
			free(filepath);
			filepath = NULL;
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

	// return the new quarantine list.
	jfiles = quarantine_enum_files_cb(armadito);

	return jfiles;
}

json_object * quarantine_delete_file_cb(char* filename,struct armadito *armadito) {

	json_object * jfiles = NULL;
	char * quarantine_dir = NULL;
	char * conf_quarantine_dir = NULL;
	char * infopath = NULL;
	char * filepath = NULL;
	int len = 0;
	int ret = 0;
	struct a6o_conf * conf = NULL;

	__try {

		if (filename == NULL) {
			__leave;
		}

		conf = a6o_get_conf(armadito);
		if (conf == NULL) {
			printf("[-] Error :: quarantine_delete_file_cb :: can't get configuration!\n");
			ret = -1;
			__leave;
		}

		conf_quarantine_dir = a6o_conf_get_string(conf,"quarantine","quarantine-dir");
		if (conf == NULL) {
			printf("[-] Error :: quarantine_delete_file_cb :: can't get configuration dir from configuration!\n");
			ret = -1;
			__leave;
		}

		quarantine_dir = GetLocationCompletepath(conf_quarantine_dir);
		if (quarantine_dir == NULL) {
			printf("[-] Error :: quarantine_delete_file_cb :: can't get quarantine complete path!\n");
			ret = -1;
			__leave;
		}


		// quarantine file complete path.
		len = 0;
		len = strnlen(quarantine_dir, MAX_PATH) + strnlen(filename, MAX_PATH) + 2;
		filepath = (char*)calloc(len+1,sizeof(char));
		filepath[len] = '\0';
		strncat_s(filepath, len, quarantine_dir, strnlen(quarantine_dir, MAX_PATH));
		strncat_s(filepath, len, "\\", 1);
		strncat_s(filepath, len, filename, strnlen(filename, MAX_PATH));
		printf("[+] Debug :: quarantine_delete_file_cb :: filepath = %s\n",filepath);

		// Get information file content.
		len = 0;
		len = strnlen(quarantine_dir, MAX_PATH) + strnlen(filename, MAX_PATH) + strnlen(".info", MAX_PATH) + 2;
		infopath = (char*)calloc(len+1,sizeof(char));
		infopath[len] = '\0';

		strncat_s(infopath, len, quarantine_dir, strnlen(quarantine_dir, MAX_PATH));
		strncat_s(infopath, len, "\\", 1);
		strncat_s(infopath, len, filename, strnlen(filename, MAX_PATH));
		strncat_s(infopath, len, ".info", strnlen(".info", MAX_PATH));
		printf("[+] Debug :: quarantine_delete_file_cb :: infopath = %s\n",infopath);

		// delete qurantine file.
		if (DeleteFileA(filepath) == FALSE) {			
			a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_ERROR," Delete the quarantine file [%s] failed ! :: GLE = %d\n",filepath, GetLastError());
			ret = -7;
			__leave;
		}

		// delete info file.
		if (DeleteFileA(infopath) == FALSE) {			
			a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_ERROR," Delete the quarantine info file [%s] failed ! :: GLE = %d\n",infopath, GetLastError());
			ret = -8;
			__leave;

		}

		printf("[+] Debug :: quarantine_delete_file_cb :: File [%s] deleted successfully !\n", filepath);
		a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_INFO," File [%s] deleted successfully !\n", filepath);

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

		if (filepath != NULL) {
			free(filepath);
			filepath = NULL;
		}

	}

	// return the new quarantine list.
	jfiles = quarantine_enum_files_cb(armadito);

	return jfiles;
}

enum a6o_json_status quarantine_response_cb(struct armadito *armadito, struct json_request *req, struct json_response *resp, void **request_data)
{
	enum a6o_json_status status = JSON_OK;
	json_object * jaction = NULL;
	json_object * jfname = NULL;
	char * action = NULL;
	char * fname = NULL;
	char * filename = NULL;
	int len = 0;

	// TO IMPLEMENT
	printf("[+] Debug :: quarantine_response_cb...\n");

	//printf("[+] Debug :: quarantine_response_cb :: req parameters = %s\n",json_object_to_json_string(jfiles));

	// Determine action to perform
	json_object_object_get_ex(req->params, "action", &jaction);

	// verify type.
	if (!json_object_is_type(jaction, json_type_string)) {
		printf("[-] Error :: quarantine_response_cb :: bad object type :: \n");
		return JSON_UNEXPECTED_ERR;
	}

	printf("[+] Debug :: quarantine_response_cb :: req parameters = %s\n",json_object_to_json_string(jaction));

	if (json_object_to_json_string(jaction) != NULL && (strncmp(json_object_to_json_string(jaction),"\"enum\"",6) == 0)) {

		resp->info = quarantine_enum_files_cb(armadito);
		if (resp->info == NULL) {
			status = JSON_REQUEST_FAILED;
		}

	}
	else if (json_object_to_json_string(jaction) != NULL && (strncmp(json_object_to_json_string(jaction),"\"restore\"",9) == 0)) {

		json_object_object_get_ex(req->params, "fname", &jfname);
		if (!json_object_is_type(jfname, json_type_string)) {
			printf("[-] Error :: quarantine_response_cb :: bad object type :: \n");
			return JSON_UNEXPECTED_ERR;
		}

		// remove "" from fname;
		fname = json_object_to_json_string(jfname);
		len = strnlen(fname,MAX_PATH);
		filename = (char*)calloc(len, sizeof(char));
		strncpy_s(filename, len,fname+1,len);
		filename[len-2] = '\0';

		//printf("[+] Debug :: quarantine_response_cb :: req parameters = %s\n",fname);
		printf("[+] Debug :: quarantine_response_cb :: req parameters = %s\n",filename);

		resp->info = quarantine_restore_file_cb(filename,armadito);
		if (resp->info == NULL) {
			status = JSON_REQUEST_FAILED;
		}

	}// delete action
	else if (json_object_to_json_string(jaction) != NULL && (strncmp(json_object_to_json_string(jaction),"\"delete\"",8) == 0)) {

		json_object_object_get_ex(req->params, "fname", &jfname);
		if (!json_object_is_type(jfname, json_type_string)) {
			printf("[-] Error :: quarantine_response_cb :: bad object type :: \n");
			return JSON_UNEXPECTED_ERR;
		}

		// remove "" from fname;
		fname = json_object_to_json_string(jfname);
		len = strnlen(fname,MAX_PATH);
		filename = (char*)calloc(len, sizeof(char));
		strncpy_s(filename, len,fname+1,len);
		filename[len-2] = '\0';

		//printf("[+] Debug :: quarantine_response_cb :: req parameters = %s\n",fname);
		printf("[+] Debug :: quarantine_response_cb :: req parameters = %s\n",filename);

		resp->info = quarantine_delete_file_cb(filename,armadito);
		if (resp->info == NULL) {
			status = JSON_REQUEST_FAILED;
		}

	}
	else {
		printf("[-] Error :: quarantine_response_cb action not defined :: %s\n",json_object_to_json_string(jaction));
		status = JSON_INVALID_REQUEST;
	}

	if (filename != NULL) {
		free(filename);
		filename = NULL;
	}

	return status;

}
