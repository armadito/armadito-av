#include "libuhuru-config.h"
#include <libuhuru/core.h>
#include "os/string.h"
#include "os/stdpaths.h"

#include <assert.h>
#include <stdlib.h>


const char *uhuru_std_path(enum uhuru_std_location location)
{
  assert(location >= 0 && location < LAST_LOCATION);

  switch(location) {
  case MODULES_LOCATION:
    return os_stdpath_module();
    break;
  case CONFIG_FILE_LOCATION:
    return os_stdpath_config_file();
    break;
  case CONFIG_DIR_LOCATION:
    return os_stdpath_config_dir();
    break;
  case BASES_LOCATION:
    return os_stdpath_bases();
    break;
  case BINARY_LOCATION:
    return os_stdpath_binary();
    break;
  case TMP_LOCATION:
    return os_stdpath_tmp();
    break;
  }

  return NULL;
}
