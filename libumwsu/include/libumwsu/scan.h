#ifndef _LIBUMWSU_SCAN_H_
#define _LIBUMWSU_SCAN_H_

#include <magic.h>
#include <libumwsu/status.h>

#ifdef __cplusplus
extern "C" {
#endif

struct umwsu;

struct umwsu *umwsu_open(void);

void umwsu_set_verbose(struct umwsu *u, int verbosity);

void umwsu_print(struct umwsu *u);

void umwsu_close(struct umwsu *u);

enum umwsu_status umwsu_scan_file(struct umwsu *umwsu_handle, magic_t magic, const char *path, struct umwsu_report *report);

enum umwsu_status umwsu_scan_dir(struct umwsu *umwsu_handle, const char *path, int recurse, int threaded);

typedef void (*umwsu_scan_callback_t)(struct umwsu_report *report, void *user_data);

void umwsu_add_scan_callback(struct umwsu *umwsu_handle, umwsu_scan_callback_t callback, void *user_data);

#ifdef __cplusplus
}
#endif

#endif
