#include <libumwsu/scan.h>

#include "modulep.h"

#include <stdlib.h>

struct umw *umw_open(void)
{
  module_load_directory(LIBUMWSU_MODULES_PATH);

  return NULL;
}

enum umw_status umw_scan_file(struct umw *umw_handle, const char *path)
{
  return UMW_MALWARE;
}

enum umw_status umw_scan_dir(struct umw *umw_handle, const char *path, int recurse)
{
  return UMW_MALWARE;
}
