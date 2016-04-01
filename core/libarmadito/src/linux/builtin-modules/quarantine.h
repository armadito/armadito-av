#ifndef _LIBARMADITO_QUARANTINE_H_
#define _LIBARMADITO_QUARANTINE_H_

#include <libarmadito.h>

extern struct a6o_module quarantine_module;

void quarantine_callback(struct a6o_report *report, void *callback_data);

#endif
