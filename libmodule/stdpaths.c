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

#include <libarmadito/armadito.h>

#include "armadito-config.h"

static const char *os_stdpath_module();
static const char *os_stdpath_config_file();
static const char *os_stdpath_config_dir();
static const char *os_stdpath_bases();
static const char *os_stdpath_binary();
static const char *os_stdpath_tmp();

A6O_API const char *a6o_std_path(enum a6o_std_location location)
{
	switch(location) {
	case A6O_LOCATION_MODULES:
		return os_stdpath_module();
		break;
	case A6O_LOCATION_CONFIG_FILE:
		return os_stdpath_config_file();
		break;
	case A6O_LOCATION_CONFIG_DIR:
		return os_stdpath_config_dir();
		break;
	case A6O_LOCATION_BASES:
		return os_stdpath_bases();
		break;
	case A6O_LOCATION_BINARY:
		return os_stdpath_binary();
		break;
	case A6O_LOCATION_TMP:
		return os_stdpath_tmp();
		break;
	}

	return NULL;
}

#ifdef linux
#include <string.h>

const char *os_stdpath_module()
{
	return strdup(LIBARMADITO_MODULES_PATH);
}

const char *os_stdpath_config_file()
{
	return strdup(LIBARMADITO_CONF_DIR "/armadito.conf");
}

const char *os_stdpath_config_dir()
{
	return strdup(LIBARMADITO_CONF_DIR "/conf.d");
}

const char *os_stdpath_bases()
{
	return strdup(LIBARMADITO_BASES_DIR);
}

const char *os_stdpath_binary()
{
	return NULL;
}

const char *os_stdpath_tmp()
{
	return NULL;
}

A6O_API const char *a6o_path_sep(void)
{
	return "/";
}
#endif

#ifdef _WIN32
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
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_ERROR, "[-] Error :: GetBinaryDirectory!GetModuleFileName() failed :: %d\n", GetLastError());
		return NULL;
	}

	// get the file name from the complete file path
	ptr = strrchr(filepath, '\\');
	if (ptr == NULL) {
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "[-] Error :: GetBinaryDirectory!strrchr() failed :: backslash not found in the path :: %s.\n", filepath);
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

	//printf("DEBUG :: dbdir == [%s]\n", dbdir);

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

A6O_API const char *a6o_path_sep() {
	return "\\";
}

#endif
