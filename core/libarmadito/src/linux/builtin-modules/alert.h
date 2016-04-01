#ifndef _LIBARMADITO_ALERT_H_
#define _LIBARMADITO_ALERT_H_

#include <libarmadito.h>

void alert_callback(struct a6o_report *report, void *callback_data);

extern struct a6o_module alert_module;

#endif
