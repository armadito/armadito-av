#include <stdio.h>
#include <Windows.h>
#include <urlmon.h>

#include <wincrypt.h>
#include <libuhuru-config.h>

#include <json.h>

//#include "json_util.h"



// ---------------------------------------------------

#define CACHE_FILEPATH "modules\\DB\\dbcache"
#define DB_DESC_URL "http://uhuru.gendarmerie.fr/current/uhurudbvirus.json"
#define DB_SIG_URL "http://uhuru.gendarmerie.fr/current/uhurudbvirus.json.sig"

typedef struct packageStruct{
	char * displayname;
	char * fileurl;
	char * controlsum;
	char * controltype;
	char * licence;
	char * cachefilename;
}Package ;


int UpdateModulesDB(int reload);
BYTE * GetFileHash(char * data, int len, ALG_ID algo);