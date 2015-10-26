#ifndef _LIBUHURU_REPORTP_H_
#define _LIBUHURU_REPORTP_H_

#include <libuhuru/core.h>

void uhuru_report_init(struct uhuru_report *report, int scan_id, const char *path);

void uhuru_report_destroy(struct uhuru_report *report);

void uhuru_report_change(struct uhuru_report *report, enum uhuru_file_status status, const char *mod_name, const char *mod_report);

#endif
