#ifndef _LIBUMWSU_SCAN_H_
#define _LIBUMWSU_SCAN_H_

#if 0
#include <magic.h>
#endif

#include <libumwsu/status.h>

#ifdef __cplusplus
extern "C" {
#endif

struct umwsu;
struct umwsu_report;

struct umwsu *umwsu_open(void);

void umwsu_set_verbose(struct umwsu *u, int verbosity);

void umwsu_print(struct umwsu *u);

void umwsu_close(struct umwsu *u);

#if 0
enum umwsu_status umwsu_scan_file(struct umwsu *umwsu_handle, magic_t magic, const char *path);
#endif

typedef void (*umwsu_scan_callback_t)(struct umwsu *umwsu_handle, struct umwsu_report *report, void **callback_data);

enum umwsu_status umwsu_scan_dir(struct umwsu *umwsu_handle, const char *path, int recurse, int threaded, umwsu_scan_callback_t callback, void **callback_data );

struct umwsu_report {
  enum umwsu_status status;
  char *path;
  char *mod_name;
  char *mod_report;
};

void umwsu_report_print(struct umwsu_report *report, FILE *out);

#ifdef __cplusplus
}
#endif

#endif
