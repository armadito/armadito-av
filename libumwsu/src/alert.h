#ifndef _LIBUMWSU_ALERT_H_
#define _LIBUMWSU_ALERT_H_

struct alert;

struct alert *alert_new(int must_lock);

void alert_callback(struct umwsu_report *report, void *callback_data);

void alert_send(struct alert *a);

void alert_free(struct alert *a);

#endif
