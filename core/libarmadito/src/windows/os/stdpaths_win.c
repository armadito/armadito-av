#include <libarmadito.h>
#include "libarmadito-config.h"
#include "os/stdpaths.h"
#include "os/dir.h"

#include <stdlib.h>
#include <Windows.h>

#define LIBUHURU_MODULES_PATH "modules"
#define LIBUHURU_CONF_DIR "conf"

const char *os_stdpath_module()
{
  char *dirpath = NULL;
  char *modulesdir = NULL;
  int len = 0;

  dirpath = GetBinaryDirectory( );
  if (dirpath == NULL)
    return NULL;

  len = strnlen_s(dirpath, _MAX_PATH) + strnlen_s(LIBUHURU_MODULES_PATH, _MAX_PATH) + 1;
  modulesdir = (char*)calloc(len + 1, sizeof(char));
  modulesdir[len] = '\0';

  memcpy_s(modulesdir, len, dirpath, strnlen_s(dirpath, _MAX_PATH));
  modulesdir[strnlen_s(dirpath, _MAX_PATH)] = '\\';
  memcpy_s(modulesdir + strnlen_s(dirpath, _MAX_PATH) + 1, len, LIBUHURU_MODULES_PATH, strnlen_s(LIBUHURU_MODULES_PATH, _MAX_PATH));

  free(dirpath);
  dirpath = NULL;

  return modulesdir;
}

const char *os_stdpath_config_file()
{
  char *dirpath = NULL;
  char *confdir = NULL;
  char * conffile = "uhuru.conf";
  int len =0;

  /* FIXME: error checking */
  dirpath = GetBinaryDirectory( );
  if (dirpath == NULL)
    return NULL;

  len = strnlen_s(dirpath, _MAX_PATH) + strnlen_s(LIBUHURU_CONF_DIR, _MAX_PATH) + strnlen_s(conffile, _MAX_PATH) + 1;
  confdir = (char*)calloc(len + 1, sizeof(char));
  confdir[len] = '\0';

  memcpy_s(confdir, len, dirpath, strnlen_s(dirpath, _MAX_PATH));
  confdir[strnlen_s(dirpath, _MAX_PATH)] = '\\';
  memcpy_s(confdir + strnlen_s(dirpath, _MAX_PATH) + 1, len, LIBUHURU_CONF_DIR, strnlen_s(LIBUHURU_CONF_DIR, _MAX_PATH));
  memcpy_s(confdir + strnlen_s(dirpath, _MAX_PATH) + strnlen_s(LIBUHURU_CONF_DIR, _MAX_PATH) + 1, len, conffile, strnlen_s(conffile, _MAX_PATH));
 
  printf("Conf_file = %s ", confdir);

  free(dirpath);
  dirpath = NULL;

  return confdir;
}

const char *os_stdpath_config_dir()
{

	char *dirpath = NULL;
	char *confdir = NULL;
	int len =0;

	/* FIXME: error checking */
	dirpath = GetBinaryDirectory( );
	if (dirpath == NULL)
	return NULL;

	len = strnlen_s(dirpath, _MAX_PATH) + strnlen_s(LIBUHURU_CONF_DIR, _MAX_PATH) + 1;
	confdir = (char*)calloc(len + 1, sizeof(char));
	confdir[len] = '\0';

	memcpy_s(confdir, len, dirpath, strnlen_s(dirpath, _MAX_PATH));
	confdir[strnlen_s(dirpath, _MAX_PATH)] = '\\';
	memcpy_s(confdir + strnlen_s(dirpath, _MAX_PATH) + 1, len, LIBUHURU_CONF_DIR, strnlen_s(LIBUHURU_CONF_DIR, _MAX_PATH));
 
	printf("Conf_file = %s ", confdir);

	free(dirpath);
	dirpath = NULL;

	return NULL;
}

const char *os_stdpath_bases()
{
  return NULL;
}

const char *os_stdpath_binary()
{
  return NULL;
}

const char *os_stdpath_tmp()
{
  return NULL;
}
