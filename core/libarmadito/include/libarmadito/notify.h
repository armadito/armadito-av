#ifndef _NOTIFY_H_
#define _NOTIFY_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

enum notif_type {
	NONE = 0,
	NOTIF_INFO,
	NOTIF_WARNING,
	NOTIF_ERROR
};

typedef int (*uhuru_notify_handler_t)(enum notif_type type, const char *message);

void uhuru_notify_set_handler(uhuru_notify_handler_t handler);
int uhuru_notify_default_handler(enum notif_type type, const char *message);
int uhuru_notify(enum notif_type type, const char *format, ...);

#endif