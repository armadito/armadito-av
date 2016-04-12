#include <libarmadito.h>
#include "libarmadito-config.h"

#include "os/stdpaths.h"

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

char a6o_path_sep(void)
{
	return '/';
}

