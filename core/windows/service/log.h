#include <stdio.h>
#include <stdarg.h>
#include "MessageText\uhEventProvider.h"

void uhLog(const char *fmt, ...);
void winEventHandler(enum a6o_log_domain domain, enum a6o_log_level log_level, const char *message, void *user_data);