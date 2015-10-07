#ifndef _LIBUHURU_INFO_H_
#define _LIBUHURU_INFO_H_

#include <libuhuru/status.h>
#include <libuhuru/handle.h>

#ifdef __cplusplus
extern "C" {
#endif

enum uhuru_update_status {
  UHURU_UPDATE_OK,
  UHURU_UPDATE_LATE,
  UHURU_UPDATE_CRITICAL,
  UHURU_UPDATE_NON_AVAILABLE,
};

struct uhuru_base_info {
  const char *name;
  /* UTC and ISO 8601 date */
  const char *date;
  const char *version;
  unsigned int signature_count;
  const char *full_path;
};

struct uhuru_module_info {
  const char *name;
  enum uhuru_update_status mod_status;
  /* UTC and ISO 8601 date time */
  const char *update_date;
  /* NULL terminated array of pointers to struct base_info */
  struct uhuru_base_info **base_infos;
};

struct uhuru_info {
  enum uhuru_update_status global_status;
  /* NULL terminated array of pointers to struct uhuru_module_info */
  struct uhuru_module_info **module_infos;
};

struct uhuru_info *uhuru_info_new(struct uhuru *uhuru);

void uhuru_info_to_stdout(struct uhuru_info *info);
void uhuru_info_free(struct uhuru_info *info);

#ifdef __cplusplus
}
#endif

#endif
