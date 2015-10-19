#ifndef _LIBUHURU_STATUSP_H_
#define _LIBUHURU_STATUSP_H_

#include <libuhuru/core.h>

int uhuru_file_status_cmp(enum uhuru_file_status s1, enum uhuru_file_status s2);

const char *uhuru_file_status_str(enum uhuru_file_status status);

const char *uhuru_file_status_pretty_str(enum uhuru_file_status status);

#endif
