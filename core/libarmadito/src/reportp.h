#ifndef _LIBARMADITO_REPORTP_H_
#define _LIBARMADITO_REPORTP_H_

#include <libarmadito.h>

void a6o_report_init(struct a6o_report *report, int scan_id, const char *path, int progress);

void a6o_report_destroy(struct a6o_report *report);

void a6o_report_change(struct a6o_report *report, enum a6o_file_status status, const char *mod_name, const char *mod_report);

#endif
