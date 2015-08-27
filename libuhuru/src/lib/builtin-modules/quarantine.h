#ifndef _LIBUHURU_QUARANTINE_H_
#define _LIBUHURU_QUARANTINE_H_

#include <libuhuru/module.h>
#include <libuhuru/scan.h>

extern struct uhuru_module quarantine_module;

void quarantine_callback(struct uhuru_report *report, void *callback_data);

#endif
