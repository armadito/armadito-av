#include <libuhuru/core.h>
#include "libuhuru-config.h"

#include "os/stdpaths.h"

#include <string.h>

const char *os_stdpath_module()
{
  return strdup(LIBUHURU_MODULES_PATH);
}

const char *os_stdpath_config_file()
{
  return strdup(LIBUHURU_CONF_DIR "/uhuru.conf");
}

const char *os_stdpath_config_dir()
{
  return strdup(LIBUHURU_CONF_DIR "/conf.d");
}

const char *os_stdpath_bases()
{
  return strdup(LIBUHURU_BASES_DIR);
}

const char *os_stdpath_binary()
{
  return NULL;
}

const char *os_stdpath_tmp()
{
  return NULL;
}
