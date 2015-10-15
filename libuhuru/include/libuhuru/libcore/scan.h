#ifndef _LIBUHURU_LIBCORE_SCAN_H_
#define _LIBUHURU_LIBCORE_SCAN_H_

#include <libuhuru/common/status.h>
#include <libuhuru/libcore/handle.h>

#ifdef __cplusplus
extern "C" {
#endif

struct uhuru_report;
struct uhuru_scan;

enum uhuru_scan_flags {
  UHURU_SCAN_THREADED   = 1 << 0,
  UHURU_SCAN_RECURSE    = 1 << 1,
};

struct uhuru_scan *uhuru_scan_new(struct uhuru *uhuru, const char *path, enum uhuru_scan_flags flags);

typedef void (*uhuru_scan_callback_t)(struct uhuru_report *report, void *callback_data);

void uhuru_scan_add_callback(struct uhuru_scan *scan, uhuru_scan_callback_t callback, void *callback_data);

void uhuru_scan_run(struct uhuru_scan *scan);

void uhuru_scan_free(struct uhuru_scan *scan);

/* for on-access scan */
/* path is provided just for reporting; it can be NULL; it won't be open()ed */
enum uhuru_file_status uhuru_scan_fd(struct uhuru *uhuru, int fd, const char *path);

struct uhuru_report {
  char *path;
  enum uhuru_file_status status;
  enum uhuru_action action;
  char *mod_name;
  char *mod_report;
};

void uhuru_report_print(struct uhuru_report *report, FILE *out);

#ifdef __cplusplus
}
#endif

#endif
