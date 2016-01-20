#include <stdio.h>
#include <Windows.h>
#include <urlmon.h>

#include <wincrypt.h>
#include <libuhuru-config.h>

#include "json.h"



// ---------------------------------------------------

#define CACHE_FILEPATH "modules\\DB\\dbcache"

typedef struct packageStruct{
	char * displayname;
	char * fileurl;
	char * controlsum;
	char * controltype;
	char * licence;
	char * cachefilename;
}Package ;


int UpdateModulesDB(int cmdLineMode);
BYTE * GetFileHash(char * data, int len, ALG_ID algo);