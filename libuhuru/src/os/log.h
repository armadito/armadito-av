#ifndef _LIBUHURU_OS_LOG_H_
#define _LIBUHURU_OS_LOG_H_

#include <glib.h>
#include <Windows.h>

static void winEventHandler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data);
void log_init(const char *g_log_domain);


#endif
