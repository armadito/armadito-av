#include "..\osdeps.h"
#include <libarmadito.h>

#define MAX_PATH 260


char * get_db_module_path(char * filename, char * module) {

	char * completePath = NULL;
	char * dbdir = NULL;
	int dbdir_len = 0;
	int len = 0;
	int off = 0;
	char path_sep = '\0';

	if (filename == NULL || module == NULL) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: get_db_module_path :: Invalids parameters\n");
		return NULL;
	}

	dbdir = a6o_std_path(BASES_LOCATION);
	if (dbdir == NULL) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: get_db_module_path :: Can't get database directory!\n");
		return NULL;
	}

	// get path separator.
	path_sep = a6o_path_sep( );
	
	len = strnlen(dbdir,MAX_PATH) + strnlen(module,MAX_PATH) + strnlen(filename,MAX_PATH) + 2;	
	completePath = (char*)calloc(len+1,sizeof(char));
	completePath[len] = '\0';

	memcpy(completePath, dbdir, strnlen(dbdir, MAX_PATH));	
	off += strnlen(dbdir, MAX_PATH);
	completePath[off] = path_sep;
	off++;	
	memcpy(completePath + off, module, strnlen(module, MAX_PATH));
	off += strnlen(module, MAX_PATH);
	completePath[off] = path_sep;
	off++;
	memcpy(completePath + off,filename, strnlen(filename, MAX_PATH));

	//printf("[+] Debug :: get_db_module_path :: completePath = %s\n", completePath);

	return completePath;

}
