#include <libarmadito.h>
#include "libarmadito-config.h"
#include "os/stdpaths.h"
#include "os/dir.h"

#include <stdlib.h>
#include <Windows.h>

#define LIBARMADITO_MODULES_PATH "modules"
#define LIBARMADITO_MODULES_DB_PATH "DB"
#define LIBARMADITO_CONF_DIR "conf"

static char * GetBinaryDirectory()
{
	char * dirpath = NULL;
	char filepath[MAX_PATH];
	char * ptr = NULL;
	int len = 0;

	if (!GetModuleFileNameA(NULL, (LPSTR)&filepath, MAX_PATH)) {
		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_ERROR, "[-] Error :: GetBinaryDirectory!GetModuleFileName() failed :: %d\n", GetLastError());
		return NULL;
	}

	// get the file name from the complete file path
	ptr = strrchr(filepath, '\\');
	if (ptr == NULL) {
		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "[-] Error :: GetBinaryDirectory!strrchr() failed :: backslash not found in the path :: %s.\n", filepath);
		return NULL;
	}

	// calc the dir buffer length.
	len = (int)(ptr - filepath);

	dirpath = (char*)(calloc(len + 1, sizeof(char)));
	dirpath[len] = '\0';

	memcpy_s(dirpath, len, filepath, len);

	return dirpath;
}


const char *os_stdpath_module()
{
	char *dirpath = NULL;
	char *modulesdir = NULL;
	int len = 0;

	dirpath = GetBinaryDirectory();
	if (dirpath == NULL)
		return NULL;

	len = strnlen_s(dirpath, _MAX_PATH) + strnlen_s(LIBARMADITO_MODULES_PATH, _MAX_PATH) + 1;
	modulesdir = (char*)calloc(len + 1, sizeof(char));
	modulesdir[len] = '\0';

	memcpy_s(modulesdir, len, dirpath, strnlen_s(dirpath, _MAX_PATH));
	modulesdir[strnlen_s(dirpath, _MAX_PATH)] = '\\';
	memcpy_s(modulesdir + strnlen_s(dirpath, _MAX_PATH) + 1, len, LIBARMADITO_MODULES_PATH, strnlen_s(LIBARMADITO_MODULES_PATH, _MAX_PATH));

	free(dirpath);

	return modulesdir;
}

const char *os_stdpath_config_file()
{
	char *dirpath = NULL;
	char *confdir = NULL;
	char * conffile = "\\armadito.conf";
	int len = 0;

	/* FIXME: error checking */
	dirpath = GetBinaryDirectory();
	if (dirpath == NULL)
		return NULL;

	len = strnlen_s(dirpath, _MAX_PATH) + strnlen_s(LIBARMADITO_CONF_DIR, _MAX_PATH) + strnlen_s(conffile, _MAX_PATH) + 1;
	confdir = (char*)calloc(len + 1, sizeof(char));
	confdir[len] = '\0';

	memcpy_s(confdir, len, dirpath, strnlen_s(dirpath, _MAX_PATH));
	confdir[strnlen_s(dirpath, _MAX_PATH)] = '\\';
	memcpy_s(confdir + strnlen_s(dirpath, _MAX_PATH) + 1, len, LIBARMADITO_CONF_DIR, strnlen_s(LIBARMADITO_CONF_DIR, _MAX_PATH));
	memcpy_s(confdir + strnlen_s(dirpath, _MAX_PATH) + strnlen_s(LIBARMADITO_CONF_DIR, _MAX_PATH) + 1, len, conffile, strnlen_s(conffile, _MAX_PATH));

	printf("Conf_file = %s ", confdir);

	free(dirpath);

	return confdir;
}

const char *os_stdpath_config_dir()
{
	char *dirpath = NULL;
	char *confdir = NULL;
	int len = 0;

	/* FIXME: error checking */
	dirpath = GetBinaryDirectory();
	if (dirpath == NULL)
		return NULL;

	len = strnlen_s(dirpath, _MAX_PATH) + strnlen_s(LIBARMADITO_CONF_DIR, _MAX_PATH) + 1;
	confdir = (char*)calloc(len + 1, sizeof(char));
	confdir[len] = '\0';

	memcpy_s(confdir, len, dirpath, strnlen_s(dirpath, _MAX_PATH));
	confdir[strnlen_s(dirpath, _MAX_PATH)] = '\\';
	memcpy_s(confdir + strnlen_s(dirpath, _MAX_PATH) + 1, len, LIBARMADITO_CONF_DIR, strnlen_s(LIBARMADITO_CONF_DIR, _MAX_PATH));

	printf("Conf_file = %s ", confdir);

	free(dirpath);
	dirpath = NULL;

	return NULL;
}

const char *os_stdpath_bases()
{
	char *dirpath = NULL;
	char *dbdir = NULL;
	int len = 0, off = 0;

	dirpath = GetBinaryDirectory();
	if (dirpath == NULL)
		return NULL;

	len = strnlen_s(dirpath, _MAX_PATH) + strnlen_s(LIBARMADITO_MODULES_PATH, _MAX_PATH) + strnlen_s(LIBARMADITO_MODULES_DB_PATH, _MAX_PATH) + 2;
	dbdir = (char*)calloc(len + 1, sizeof(char));
	dbdir[len] = '\0';

	memcpy_s(dbdir, len, dirpath, strnlen_s(dirpath, _MAX_PATH));
	off += strnlen_s(dirpath, _MAX_PATH);
	dbdir[off] = '\\';
	off++;
	memcpy_s(dbdir + off, len, LIBARMADITO_MODULES_PATH, strnlen_s(LIBARMADITO_MODULES_PATH, _MAX_PATH));
	off += strnlen_s(LIBARMADITO_MODULES_PATH, _MAX_PATH);
	dbdir[off] = '\\';
	off++;
	memcpy_s(dbdir + off, len, LIBARMADITO_MODULES_DB_PATH, strnlen_s(LIBARMADITO_MODULES_DB_PATH, _MAX_PATH));

	printf("DEBUG :: dbdir == [%s]\n", dbdir);

	free(dirpath);
	dirpath = NULL;

	return dbdir;
}

const char *os_stdpath_binary()
{
	return NULL;
}

const char *os_stdpath_tmp()
{
	return NULL;
}

char a6o_path_sep() {
	return '\\';
}