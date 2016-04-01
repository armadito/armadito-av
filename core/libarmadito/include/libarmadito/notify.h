#ifndef LIBARMADITO_NOTIFY_H_
#define LIBARMADITO_NOTIFY_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

enum notif_type {
	NONE = 0,
	NOTIF_INFO,
	NOTIF_WARNING,
	NOTIF_ERROR
};

typedef int (*a6o_notify_handler_t)(enum notif_type type, const char *message);

void a6o_notify_set_handler(a6o_notify_handler_t handler);
int a6o_notify_default_handler(enum notif_type type, const char *message);
int a6o_notify(enum notif_type type, const char *format, ...);

#endif
