#ifndef _LIBUHURU_QUARANTINE_H_
#define _LIBUHURU_QUARANTINE_H_

#include <libuhuru/module.h>
#include <libuhuru/scan.h>

void quarantine_callback(struct uhuru_report *report, void *callback_data);

extern struct uhuru_module quarantine_module;

#endif
