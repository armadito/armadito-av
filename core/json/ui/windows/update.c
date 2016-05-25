/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "ui\update.h"
#include <stdio.h>
#include <libarmadito.h>
//#include "libarmadito-config.h"
#include <Windows.h>
#include <libarmadito/stdpaths.h>
#include "service\service.h"
#include "updatedb\json_process.h"
#include "utils\crypt.h"
#include "utils\others.h"


#define DB_CACHE_PATH "modules\\DB\\dbcache"

#define SVC_MODE 1
#define CMD_MODE 2
#define TEST_MODE 3



char * get_db_module_path(char * filename, char * module) {

	char * completePath = NULL;
	char * dbdir = NULL;
	int dbdir_len = 0;
	int len = 0;
	int off = 0;
	char path_sep = '\0';

	if (filename == NULL || module == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: get_db_module_path :: Invalids parameters\n");
		return NULL;
	}

	dbdir = a6o_std_path(BASES_LOCATION);
	if (dbdir == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: get_db_module_path :: Can't get database directory!\n");
		return NULL;
	}

	// get path separator.
	path_sep = a6o_path_sep( );
	
	len = strnlen_s(dbdir,MAX_PATH) + strnlen_s(module,MAX_PATH) + strnlen_s(filename,MAX_PATH) + 2;	
	completePath = (char*)calloc(len+1,sizeof(char));
	completePath[len] = '\0';

	memcpy_s(completePath, len, dbdir, strnlen_s(dbdir, _MAX_PATH));	
	off += strnlen_s(dbdir, _MAX_PATH);
	completePath[off] = path_sep;
	off++;	
	memcpy_s(completePath + off, len, module, strnlen_s(module, _MAX_PATH));
	off += strnlen_s(module, _MAX_PATH);
	completePath[off] = path_sep;
	off++;
	memcpy_s(completePath + off, len, filename, strnlen_s(filename, _MAX_PATH));

	printf("[+] Debug :: get_db_module_path :: completePath = %s\n", completePath);

	return completePath;

}

// This function copy the databases files in corresponding the modules database directories.
int copy_modules_db_files(Package ** pkgList, int nbPackages ) {

	int ret = 0, i =0;
	char * dbfilepath = NULL;

	if (pkgList == NULL || nbPackages <= 0) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: copy_modules_db_files :: Invalids parameters\n");
		return -1;
	}


	for (i = 0; i < nbPackages; i++) {

		dbfilepath = get_db_module_path(pkgList[i]->displayname, pkgList[i]->licence);
		if (dbfilepath == NULL) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "[-] Warning :: copy_modules_db_files :: Can't get db file complete file path :: filename = %s :: licence = %s\n",pkgList[i]->displayname,pkgList[i]->licence);
			continue;
		}

		if (CopyFileA(pkgList[i]->cachefilename,dbfilepath,FALSE) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING,"[-] Warning :: CopyModulesDatabaseFiles :: Copying db file [%s] failed! :: GLE = %d\n",pkgList[i]->displayname ,GetLastError());			
		}

		if (dbfilepath != NULL) {
			free(dbfilepath);
			dbfilepath = NULL;
		}

	}


	return ret;
}

void FreePackageList(Package ** pkgList, int nbPackages) {
	
	int i = 0;

	if (pkgList == NULL || nbPackages <= 0) {
		//printf("[-] Error :: FreePackageList :: Invalids parameters\n");
		return;
	}

	for (i = 0; i < nbPackages; i++) {

		if (pkgList[i]->cachefilename != NULL) {
			free(pkgList[i]->cachefilename);
			pkgList[i]->cachefilename = NULL;
		}
					
		free(pkgList[i]);
		pkgList[i] = NULL;
	
	}

	free(pkgList);
	pkgList = NULL;


	return;
}

char * ConvertBytesToCharString(BYTE * hash) {

	char * string = NULL;
	int ret = 0, len = 0;
	int i = 0;
	char* tmp = NULL;
	int size = 0;


	if (hash == NULL) {
		printf("[-] Error :: ConvertBytesToChar :: Invalid parameter\n");
		return NULL;
	}

	__try {

		len = strnlen(hash, MAX_PATH);
		
		// multiply len by 2 for hexa.
		size = len * 2 +1;
		string = (char*)calloc(size+1, sizeof(char));
		string[size] = '\0';
		//printf("[+] Debug :: ConvertBytesToChar :: len = %d :: size = %d\n",len,size);

		tmp = string;

		for (i = 0; i < len; i++) {

			// Tip :: add size-i*2 to avoid heap corruption error.
			sprintf_s(tmp,size-i*2, "%02x", hash[i]);
			tmp += 2;			

		}
		//tmp = string;
		printf("[+] Debug :: ConvertBytesToChar :: string = %s\n",string);
		
	}
	__finally {

	}


	return string;

}

/*
	This function parses and verify package fill in the description file.
*/
int DownloadPackageFiles(Package ** packageList, int nbPackages) {

	int ret = 0, i= 1;
	HRESULT hres = S_OK;
	char * fileurl = NULL;
	char * checksum = NULL;
	char cacheFilename[MAX_PATH] = {0};
	char *content = NULL;
	unsigned char * hash = NULL;
	char * hashString = NULL;
	ALG_ID algo;
	int fsize = 0;
	int len = 0;

	if (packageList == NULL || nbPackages <= 0) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: DownloadPackageFiles :: Invalid parameter\n");
		return -1;
	}

	__try {

		for (i = 0; i < nbPackages; i++) {

			// Skip linux files.
			if (strncmp(packageList[i]->licence, "moduleH1_lin", 13) == 0) {
				a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Warning :: Skipping module H1 linux databases files.\n");
				continue;
			}

			// Download the file.
			printf("[+] Debug :: DownloadPackageFiles :: Downloading file from :: %s....\n",packageList[i]->fileurl);
			hres = URLDownloadToCacheFileA(NULL, packageList[i]->fileurl, cacheFilename,MAX_PATH, 0,NULL);
			if (FAILED(hres)) {
				a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: DownloadPackageFiles :: URLDownloadToCacheFileA failed :: error = 0x%x\n",hres);
				ret = -2;
				__leave;
			}

			packageList[i]->cachefilename = _strdup(cacheFilename);
			printf("[+] Debug :: DownloadPackageFiles :: cache filename  = %s\n", packageList[i]->cachefilename);

			

			content = GetFileContent(cacheFilename, &fsize);
			if (content == NULL) {
				a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: DownloadPackageFiles :: Get file content failed!\n");
				ret = -3;
				__leave;
			}
			

			// choose the hash algo
			if (strncmp(packageList[i]->controltype,"MD5",3) == 0) {
				algo = CALG_MD5;
			}
			else if (strncmp(packageList[i]->controltype,"SHA1",4) == 0) {
				algo = CALG_SHA1;
			}
			else if (strncmp(packageList[i]->controltype,"SHA256",3) == 0) {
				algo = CALG_SHA_256;
			}
			else {
				a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: DownloadPackageFiles :: Checksum type not supported :: %s!\n",packageList[i]->controltype);
				ret = -4;
				__leave;
			}

			// Verify the checksum of the downloaded file.
			hash = GetFileHash(content, fsize, algo);
			if (hash == NULL) {
				a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: DownloadPackageFiles :: Get file Checksum failed!\n");				
				ret = -5;
				__leave;
			}

			hashString = ConvertBytesToCharString(hash);
			if (hashString == NULL) {
				a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: DownloadPackageFiles :: Convert Bytes to Char string failed!\n");				
				ret = -6;
				__leave;
			}

			// compare checksums
			printf("[+] Debug :: DownloadPackageFiles :: checksum = %s\n",packageList[i]->controlsum);
			if (strncmp(hashString,packageList[i]->controlsum,MAX_PATH) != 0) {
				a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: DownloadPackageFiles :: Checksum control failed! :: hash = %s differents from checksum =%s\n",hashString,packageList[i]->controlsum );	
				ret = -7;
				__leave;
			}


			// clean
			if (hash != NULL) {
				free(hash);
				hash = NULL;
			}
			if (hashString != NULL) {
				free(hashString);
				hashString = NULL;
			}
			if (content != NULL) {
				free(content);
				content = NULL;
			}
			

		}

	}
	__finally {

		

		if (hash != NULL) {
			free(hash);
			hash = NULL;
		}

		if (hashString != NULL) {
			free(hashString);
			hashString = NULL;
		}

		if (content != NULL) {
			free(content);
			content = NULL;
		}

	}

	return ret;

}


void PrintPackageList(Package ** list, int len) {

	int i = 0;

	if (list == NULL || len <= 0) {
		printf("[-] Error :: PrintPackageList :: Invalid parameter\n");
		return;
	}

	for (i = 0; i < len; i++) {

		printf("\n[+] Debug :: PrintPackageList :: PACKAGE %d\n",i);
		printf("[+] Debug :: PrintPackageList :: \n");
		printf("[+] Debug :: PrintPackageList :: displayName = %s\n",list[i]->displayname);
		printf("[+] Debug :: PrintPackageList :: fileurl = %s\n",list[i]->fileurl);
		printf("[+] Debug :: PrintPackageList :: controlsum = %s\n",list[i]->controlsum);
		printf("[+] Debug :: PrintPackageList :: controltype = %s\n",list[i]->controltype);
		printf("[+] Debug :: PrintPackageList :: licence = %s\n",list[i]->licence);
		printf("[+] Debug :: PrintPackageList :: cache = %s\n",list[i]->cachefilename);

	}

	return;

}

/*
	This function parses the descrption file and download the databases files.
*/
Package ** ParseDescriptionFile(char * desc, int * nbPackages){
	
	int ret = 0, i = 0;
	enum json_tokener_error error;
	struct json_object * jObj = NULL;
	struct json_object * jSub = NULL;
	char *key = NULL;
	struct json_object *val = NULL;
	struct lh_entry *entrykey = NULL;
	struct lh_entry *entry_nextkey = NULL;
	Package ** pkgList = NULL;
	int len = 0;


	if (desc == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: ParseDescriptionFile :: Invalid parameter!\n");
		return NULL ; 
	}
	
	__try {
	
		//printf("[+] Debug :: ParseDescriptionFile :: JSON parsing...\n");
		
		// parse description file (json parsing)
		jObj =  json_tokener_parse_verbose(desc,&error);
		if (jObj == NULL) {			
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: ParseDescriptionFile :: Parsing description file failed! :: error = %d\n",error);
			ret = -2;
			__leave;
		}		

		// This function returns the number of packages.
		ret = json_parse_obj_rec(jObj,NULL);
		if (ret <= 0 ) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: ParseDescriptionFile :: parsing json object failed! ::nbpackages %d :: pkgList %d\n",ret,pkgList);
			return NULL;
		}


		// allocate memory for package List
		pkgList = (Package**)calloc(ret,sizeof(Package*));
		if (pkgList == NULL) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: ParseDescriptionFile :: Memory allocation failed!\n");
			return NULL;
		}

		for (i = 0; i < ret; i++) {

			pkgList[i] = (Package*)calloc(1,sizeof(Package));

			if (pkgList[i] == NULL) {
				a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: ParseDescriptionFile :: Memory allocation failed!\n");
				free(pkgList);
				return NULL;
			}
		}

		// fill package list
		ret = json_parse_obj_rec(jObj,pkgList);
		if (ret <= 0 ) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: ParseDescriptionFile :: parsing json object failed! ::nbpackages %d :: pkgList %d\n",ret,pkgList);
			return NULL;
		}


		// 
		*nbPackages = ret;

	}
	__finally {

		
	}

	
	return pkgList;
}

int SaveHashInCacheFile(BYTE * hash) {

	int ret = 0;
	HANDLE hFile = NULL;
	int len =0, writn=0;
	char * db_cachefile = NULL;

	if (hash == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: SaveHashInCacheFile :: Invalid parameters!\n");
		return -1 ; 
	}

	__try {

		// Get the cached hash.		
		db_cachefile = GetLocationCompletepath(DB_CACHE_PATH);
		if (db_cachefile == NULL) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR, "[-] Error :: Can't get db cache file complete path!\n");
			ret = -1;
			__leave;
		}

		// TODO :: Create hidden file
		hFile = CreateFileA(db_cachefile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: SaveHashInCacheFile :: Creating the cache file failed! :: error = %d\n",GetLastError());
			ret = -2;
			__leave;
		}

		// Write hash in the cache file.
		len = strnlen_s(hash,MAX_PATH);
		//printf("[i] Debug :: SaveHashInCacheFile :: len = %d\n",len);

		if (WriteFile(hFile, hash, len, &writn, NULL) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: SaveHashInCacheFile :: Writing in the cache file failed! :: error = %d\n", GetLastError( ));
			ret = -3;
			__leave;
		}

		printf("[+] Debug :: SaveHashInCacheFile :: Cache file created successfully!\n");


	}
	__finally {

		if (hFile != INVALID_HANDLE_VALUE && hFile != NULL) {
			CloseHandle(hFile);
			hFile = NULL;
		}

		if (db_cachefile != NULL) {
			free(db_cachefile);
			db_cachefile = NULL;
		}

	}

	return ret;
}


/*
	This function compare the hash with the cached hash.
	returns 0 if the hash are equal.
	returns 1 if there is no cache yet (put this hash in the cache).
	returns error code (<0) on error.

*/
int CompareWithCachedHash(BYTE * hash) {

	int ret = 0;
	BYTE * cachedHash = NULL;
	char *db_cachefile = NULL;
	HANDLE hFile = NULL;
	LARGE_INTEGER fileSize = {0};
	int read = 0,size = 0, i = 0;
	
	if (hash == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: CompareWithCachedHash :: Invalid parameters!\n");
		return -1 ; 
	}
	
	__try {
		
		// Get the cached hash.		
		db_cachefile = GetLocationCompletepath(DB_CACHE_PATH);
		if (db_cachefile == NULL) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR, "[-] Error :: Can't get db cache file complete path!\n");
			ret = -1;
			__leave;
		}

		cachedHash = GetFileContent_b(db_cachefile,&size);
		if (cachedHash == NULL || size <= 0) {
			ret = 1;
			__leave;			
		}

		//printf("[+] Debug :: CompareWithCachedHash :: Comparing hash=%s with cache=%s\n",hash,cachedHash);
		
		printf("[+] Debug :: CompareWithCachedHash :: Hash value  = ");
		print_hexa(hash, size);
		printf("\n");

		printf("[+] Debug :: CompareWithCachedHash :: Cache value = ");
		print_hexa(cachedHash, size);
		printf("\n");		

		if (strncmp(hash, cachedHash, size) != 0) {
			ret = 1;
		}

	}
	__finally {

		if (hFile != INVALID_HANDLE_VALUE && hFile != NULL) {
			CloseHandle(hFile);
			hFile = NULL;
		}

		if (cachedHash != NULL) {
			free(cachedHash);
			cachedHash = NULL;
		}

		if (db_cachefile != NULL) {
			free(db_cachefile);
			db_cachefile = NULL;
		}

	}
	
	return ret;
}

int update_modules_db(struct armadito * armadito) {

	int ret = 0, res = 0;
	HRESULT hres = S_OK;
	char sig_filename[MAX_PATH] = {0};
	char desc_filename[MAX_PATH] = {0};
	BYTE * hash = NULL;
	HANDLE hFile = NULL, hTestFile = NULL;
	char * desc = NULL;
	char * test = NULL;
	int len = 0, desc_size = 0, read = 0;
	LARGE_INTEGER fileSize = {0};
	Package ** packageList = NULL;
	int nbPackages = 0;
	int mode = CMD_MODE;



	__try {

		// download the database description from update server.
		// use URLDownloadToCacheFile() or URLDownloadToFile().		
		hres = URLDownloadToCacheFileA(NULL, DB_DESC_URL, desc_filename, MAX_PATH, 0,NULL);
		if (FAILED(hres)) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: UpdateModulesDB :: db description download failed! :: error =  0x%x\n",hres);
			ret = -1;
			__leave;
		}

		printf("[+] Debug :: UpdateModulesDB :: description file downloaded successfully!\n");


		// Download signature file for the description file.
		hres = URLDownloadToCacheFileA(NULL, DB_SIG_URL, sig_filename, MAX_PATH, 0,NULL);
		if (FAILED(hres)) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: UpdateModulesDB :: URLDownloadToCacheFileA failed :: error = 0x%x\n",hres);
			ret = -2;
			__leave;
		}

		printf("[+] Debug :: UpdateModulesDB :: signature file downloaded successfully!\n");

		// verify signature.
		if ( verify_file_signature(desc_filename,sig_filename) < 0) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: DownloadPackageFiles :: Verify file signature failed !\n",hres);
			ret = -3;
			__leave;
		}

		printf("[+] Debug :: UpdateModulesDB :: File Signature verified successfully !\n");

		// Get description file content.
		desc = GetFileContent(desc_filename, &desc_size);
		if (desc == NULL || desc_size <= 0) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: UpdateModulesDB :: Get description file content failed !\n");
			ret = -4;
			__leave;
		}

		//printf("[+] Debug :: UpdateModulesDB :: desc = \n%s\n", desc);

		// calc the hash of the description file.
		hash = GetFileHash(desc,desc_size,CALG_MD5);
		if (hash == NULL) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: UpdateModulesDB :: Get Description File Hash failed!\n");
			ret = -5;
			__leave;
		}

		// Compare with the cached hash.
		res = CompareWithCachedHash(hash);
		if (res < 0) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: UpdateModulesDB :: Compare With Cached Hash failed!\n");
			ret = -6;
			__leave;
		}

		// if the hash are equal then leave
		if (res == 0) {
			printf("[+] Debug :: UpdateModulesDB :: Database is already up to date!\n");
			a6o_notify(NOTIF_INFO, "Database already up to date!");
			__leave;
		}

		// Parse description file and extract package list.
		if ((packageList = ParseDescriptionFile(desc,&nbPackages)) == NULL) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: UpdateModulesDB :: Parsing Description file failed!\n");
			res = -7;
			__leave;
		}
		/*PrintPackageList(packageList, nbPackages);
		if (reload == 0)
			__leave;
			*/

		printf("\n");
		if (DownloadPackageFiles(packageList, nbPackages) < 0) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: UpdateModulesDB :: Parsing Description file failed!\n");
			res = -7;
			__leave;
		}

		//PrintPackageList(packageList, nbPackages);


		// Unload the service (unloadProcedure).
		if (mode == SVC_MODE) {

			// Pause the service.
			if (ServicePause( ) < 0) {
				a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: UpdateModulesDB :: Pausing the service failed!\n");
				ret = -8;
				__leave;
			}

		}
		else if (mode == CMD_MODE) {

			// unload the av service.
			ret = ServiceUnloadProcedure( );
			if (ret != 0) {
				a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_ERROR, " Service unloaded with errors during pause.\n");
				ret = -8;
			}

		}		

		printf("[+] Debug :: UpdateModulesDB :: Armadito service suspended successfully!\n");

		printf("\n\n");
		// Copy databases files to the right places.
		if (copy_modules_db_files(packageList, nbPackages) != 0) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: UpdateModulesDB :: Copy Databases files finished with errors!\n");
		}		


		if (mode == SVC_MODE) {

			// Resume the service.
			if (ServiceContinue( ) < 0) {
				a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: UpdateModulesDB :: Resuming the service failed!\n");
				ret = -10;
				__leave;
			}

		}
		else if (mode == CMD_MODE) {

			// reload the av service.
			ret = ServiceLoadProcedure(SVC_MODE);
			if (ret != 0) {
				a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_ERROR, " Service loaded with errors during pause.\n");
				ret = -10;
			}

		}

		printf("[+] Debug :: UpdateModulesDB :: Armadito service resumed successfully!\n");

		// Save hash in cache file.
		res = SaveHashInCacheFile(hash);
		

		printf("[+] Debug :: UpdateModulesDB :: Modules Database updated successfully!\n");
		a6o_notify(NOTIF_INFO, "Modules Database updated successfully!");

	
	}
	__finally {

		if (hFile != INVALID_HANDLE_VALUE && hFile != NULL) {
			CloseHandle(hFile);
			hFile = NULL;
		}

		// Delete the file.
		/*if (DeleteFileA(filename) == FALSE) {
			printf("[-] Error :: UpdateModulesDB :: Delete file failed! :: GLE = %d\n",GetLastError());
			ret = 1;
		}*/

		if (desc != NULL) {
			free(desc);
			desc = NULL;
		}

		if (test != NULL) {
			free(test);
			test = NULL;
		}

		if (hash != NULL) {
			free(hash);
			hash = NULL;
		}

		FreePackageList(packageList, nbPackages);

	}

	
	return ret;
}

enum a6o_json_status update_response_cb(struct armadito *armadito, struct json_request *req, struct json_response *resp, void **request_data)
{
	enum a6o_json_status status = JSON_OK;
	json_object * jaction = NULL;
	json_object * jfname = NULL;
	char * action = NULL;
	char * fname = NULL;
	char * filename = NULL;
	int len = 0;
	int ret = 0;

	
	printf("[+] Debug :: update_response_cb...\n");
	ret = update_modules_db(armadito);
	if (ret != 0) {
		status = JSON_UNEXPECTED_ERR;
	}

	return status;

}
