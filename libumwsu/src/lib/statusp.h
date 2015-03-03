#ifndef _LIBUMWSU_STATUSP_H_
#define _LIBUMWSU_STATUSP_H_

#include <libumwsu/status.h>

enum umwsu_status umwsu_status_from_str(const char *s);

int umwsu_status_cmp(enum umwsu_status s1, enum umwsu_status s2);

#endif
