#ifndef _LIBUMWSU_STATUSP_H_
#define _LIBUMWSU_STATUSP_H_

#include <libumwsu/status.h>

int umwsu_status_cmp(enum umwsu_status s1, enum umwsu_status s2);

void umwsu_report_init(struct umwsu_report *report, const char *path);

void umwsu_report_destroy(struct umwsu_report *report);

void umwsu_report_change(struct umwsu_report *report, enum umwsu_status status, const char *mod_name, const char *mod_report);

#endif
