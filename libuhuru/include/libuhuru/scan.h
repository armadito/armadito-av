#ifndef _LIBUHURU_SCAN_H_
#define _LIBUHURU_SCAN_H_

#include <libuhuru/status.h>

#ifdef __cplusplus
extern "C" {
#endif

struct uhuru;
struct uhuru_report;
struct uhuru_scan;

struct uhuru *uhuru_open(int is_remote);

void uhuru_print(struct uhuru *u);

void uhuru_close(struct uhuru *u);

enum uhuru_scan_flags {
  UHURU_SCAN_THREADED   = 1 << 0,
  UHURU_SCAN_RECURSE    = 1 << 1,
};

enum uhuru_scan_status {
  UHURU_SCAN_OK = 1,
  UHURU_SCAN_CANNOT_CONNECT,
  UHURU_SCAN_CONTINUE,
  UHURU_SCAN_COMPLETED,
};

struct uhuru_scan *uhuru_scan_new(struct uhuru *uhuru, const char *path, enum uhuru_scan_flags flags);

typedef void (*uhuru_scan_callback_t)(struct uhuru_report *report, void *callback_data);

void uhuru_scan_add_callback(struct uhuru_scan *scan, uhuru_scan_callback_t callback, void *callback_data);

enum uhuru_scan_status uhuru_scan_start(struct uhuru_scan *scan);

int uhuru_scan_get_poll_fd(struct uhuru_scan *scan);
enum uhuru_scan_status uhuru_scan_run(struct uhuru_scan *scan);

/* enum uhuru_scan_status uhuru_scan_wait_for_completion(struct uhuru_scan *scan); */

void uhuru_scan_free(struct uhuru_scan *scan);

enum uhuru_action {
  UHURU_ACTION_NONE         = 0,
  UHURU_ACTION_ALERT        = 1 << 1,
  UHURU_ACTION_QUARANTINE   = 1 << 2,
  UHURU_ACTION_REMOVE       = 1 << 3,
};

/* const char *uhuru_action_str(enum uhuru_action action); */
const char *uhuru_action_pretty_str(enum uhuru_action action);

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
