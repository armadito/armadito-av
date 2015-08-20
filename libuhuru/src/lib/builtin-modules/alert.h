#ifndef _LIBUHURU_ALERT_H_
#define _LIBUHURU_ALERT_H_

#include <libuhuru/scan.h>
#include "modulep.h"

void alert_callback(struct uhuru_report *report, void *callback_data);

extern struct uhuru_module uhuru_mod_alert;

#endif
