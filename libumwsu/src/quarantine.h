#ifndef _LIBUMWSU_QUARANTINE_H_
#define _LIBUMWSU_QUARANTINE_H_

void quarantine_callback(struct umwsu_report *report, void *callback_data);

extern struct umwsu_module umwsu_mod_quarantine;

#endif
