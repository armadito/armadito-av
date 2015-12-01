#include <stdio.h>
#include <stdarg.h>
#include <libuhuru/libcore/log.h>
#include "uhEventProvider.h"

void uhLog(const char *fmt, ...);
static void winEventHandler(enum uhuru_log_domain domain, enum uhuru_log_level log_level, const char *message, void *user_data);