#include "quarantine.h"
#include <stdio.h>
#include "libuhuru-config.h"
#include <libuhuru/core.h>
#include <json.h>
#include "os\dir.h"
#include "os\string.h"
#include "os\file.h"
#include <Windows.h>
#include "others.h"

typedef void (*dirent_cb_t)(const char *full_path, enum os_file_flag flags, int entry_errno, void *data);

// libuhuru :: os/dir.h functions :: TODO :: quarantine in libuhuru.
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
	

	// todo: checks parameters



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

void qu_os_dir_map(const char *path, int recurse, dirent_cb_t dirent_cb, void * data ) {

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
		uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "Error :: NULL parameter in function os_dir_map()");
		return;
	}
	
	// We escape ? and * characters
	// escapedPath = cpp_escape_str(path);

	// Check if it is a directory
	if (!(GetFileAttributesA(path) & FILE_ATTRIBUTE_DIRECTORY)) {
		uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "Warning :: os_dir_map() :: (%s) is not a directory. ", path);
		return;
	}

	size = strlen(path) + 3;
	sPath = (char*)calloc(size + 1, sizeof(char));
	sPath[size] = '\0';
	sprintf_s(sPath, size, "%s\\*", path); // UF:: modif :: .info files only.
	//sprintf_s(sPath, size, "%s\\*.info", path); // UF:: modif :: .info files only.

	printf("[+] Debug :: os_dir_map :: sPath = %s\n",sPath);

	//g_log(NULL, UHURU_LOG_LEVEL_WARNING, "os_dir_map() :: (%s)", sPath);

	/*
	FindFirstFile note
	Be aware that some other thread or process could create or delete a file with this name between the time you query for the result and the time you act on the information. If this is a potential concern for your application, one possible solution is to use the CreateFile function with CREATE_NEW (which fails if the file exists) or OPEN_EXISTING (which fails if the file does not exist).
	*/


	fh = FindFirstFile(sPath, &fdata);
	if (fh == INVALID_HANDLE_VALUE) {
		uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "Warning :: os_dir_map() :: FindFirstFileA() failed ::  (%s) :: [%s]", os_strerror(errno),sPath);
		return;
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

	return;
}


//static void conf_load_dirent_cb(const char *full_path, enum os_file_flag flags, int entry_errno, void *data);


enum uhuru_json_status quarantine_response_cb(struct uhuru *uhuru, struct json_request *req, struct json_response *resp, void **request_data) {

	enum uhuru_json_status status = JSON_OK;
	char * quarantine_dir = "Quarantine";
	json_object * jobj = NULL;
	json_object * jinfo = NULL;
	json_object * jfiles = NULL;
	char * content = NULL;
	int count = 0;

	// TO IMPLEMENT
	printf("[+] Debug :: quarantine_response_cb...\n");

	__try {

		if ((jfiles = json_object_new_array( ))== NULL) {
			printf("[-] Error :: quarantine_response_cb :: can't create json object!\n");
			status = JSON_UNEXPECTED_ERR;
			__leave;
		}

		printf("[+] Debug :: json list content = %s \n",json_object_to_json_string(jfiles));
		

		// Get quarantine directory from module.
		// void os_dir_map(const char *path, int recurse, dirent_cb_t dirent_cb, void *data )
		qu_os_dir_map(quarantine_dir, 0, process_info, jfiles);

		count = json_object_array_length(jfiles);

		printf("[+] Debug :: count = %d\n",count);		
		//printf("[+] Debug :: json list content = %s \n",json_object_to_json_string(jfiles));


		if ((jobj = json_object_new_object()) == NULL) {
			printf("[-] Error :: quarantine_response_cb :: can't create json object!\n");
			status = JSON_UNEXPECTED_ERR;
			__leave;
		}

		if ((jinfo = json_object_new_object()) == NULL) {
			printf("[-] Error :: quarantine_response_cb :: can't create json object!\n");
			status = JSON_UNEXPECTED_ERR;
			__leave;
		}
		
		// add items
		json_object_object_add(jobj, "count", json_object_new_int(count));
		json_object_object_add(jobj, "last", json_object_new_string("1970-01-01 00:00"));
		//json_object_object_add(jobj, "info",jinfo);
		json_object_object_add(jobj, "files",jfiles);

		resp->info = jobj;
	}
	__finally {
		

	}
	

	//status = JSON_UNEXPECTED_ERR;
	return status;

}