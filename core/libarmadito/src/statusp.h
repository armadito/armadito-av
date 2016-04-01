#ifndef _LIBARMADITO_STATUSP_H_
#define _LIBARMADITO_STATUSP_H_

#include <libarmadito.h>

int a6o_file_status_cmp(enum a6o_file_status s1, enum a6o_file_status s2);

const char *a6o_file_status_str(enum a6o_file_status status);

const char *a6o_file_status_pretty_str(enum a6o_file_status status);

#endif
