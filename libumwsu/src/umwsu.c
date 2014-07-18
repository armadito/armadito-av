#include <libumwsu/umwsu.h>

#include "dir.h"
#include "module.h"

#include <stdio.h>
#include <magic.h>

static void load_entry(const char *full_path, void *data)
{
  magic_t *p = (magic_t *)data;

  printf("%s: %s\n", full_path, magic_file(*p, full_path));
}

static int load_all_modules(void)
{
  magic_t *p;

  p = (magic_t *)malloc(sizeof(magic_t));

  *p = magic_open(MAGIC_NONE);
  magic_load(*p, NULL);

  dir_map(LIBUMWSU_MODULES_PATH, 0, load_entry, p);

  magic_close(*p);
  free(p);

  return 0;
}

struct umw *umw_open(void)
{
  load_all_modules();

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
