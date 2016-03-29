#include <stdio.h>
#include <Windows.h>
#include <urlmon.h>

#include <wincrypt.h>
#include <libuhuru-config.h>
#include <libuhuru/libcore/stdpaths.h>

#include "json\updatedb\uh_json_parse.h"

// ---------------------------------------------------

#define CACHE_FILEPATH "modules\\DB\\dbcache"
#define DB_DESC_URL "http://uhuru.gendarmerie.fr/current/uhurudbvirus.json"
#define DB_SIG_URL "http://uhuru.gendarmerie.fr/current/uhurudbvirus.json.sig"

char * BuildCompleteDBpath(char * filename, char * module);
int CopyModulesDatabaseFiles(Package ** pkgList, int nbPackages);
void FreePackageList(Package ** pkgList, int nbPackages);
char * ConvertBytesToCharString(BYTE * hash);
int CompareHashes(BYTE * hash1, char * hash2);
int DownloadPackageFiles(Package ** packageList, int nbPackages);
Package ** ParseDescriptionFile(char * desc, int * nbPackages);
int SaveHashInCacheFile(BYTE * hash);
int CompareWithCachedHash(BYTE * hash);
BYTE * GetFileHash(char * data, int len, ALG_ID algo); // put this function in uh_crypt.c

int UpdateModulesDB(int reload);

