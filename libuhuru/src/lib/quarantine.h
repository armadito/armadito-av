#ifndef _LIBUHURU_QUARANTINE_H_
#define _LIBUHURU_QUARANTINE_H_

void quarantine_callback(struct uhuru_report *report, void *callback_data);

extern struct uhuru_module uhuru_mod_quarantine;

#endif
