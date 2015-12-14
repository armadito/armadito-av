#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include <glib.h>
#include <stdarg.h>

/* should not be there, but for now... */
#ifdef HAVE_GETPID
#define os_getpid getpid
#endif
#ifdef HAVE__GETPID
#include <process.h>
#define os_getpid _getpid
#endif

#define LOG_NAME "uhuru"

static enum uhuru_log_level current_max_level = UHURU_LOG_LEVEL_ERROR;
static uhuru_log_handler_t current_handler = uhuru_log_default_handler;
static void *current_handler_user_data = NULL;

void uhuru_log(enum uhuru_log_domain domain, enum uhuru_log_level level, const char *format, ...)
{
  va_list args;
  GString *buff;
  gchar *message;

  /* anything to do? */
  if (level > current_max_level)
    return;

  /* format message */
  buff = g_string_new("");
  va_start(args, format);
  g_string_vprintf(buff, format, args);
  va_end(args);

  /* call the handler with formated message */
  message = g_string_free(buff, FALSE);

  (*current_handler)(domain, level, message, current_handler_user_data);

  g_free(message);
}

void uhuru_log_set_handler(enum uhuru_log_level max_level, uhuru_log_handler_t handler, void *user_data)
{
  current_max_level = max_level;
  if (handler != NULL)
    current_handler = handler;
  else
    current_handler = uhuru_log_default_handler;

  current_handler_user_data = user_data;
}

static FILE *get_stream(enum uhuru_log_level log_level)
{
  return log_level & (UHURU_LOG_LEVEL_ERROR | UHURU_LOG_LEVEL_WARNING) ? stderr : stdout;
}

static const char *domain_str(enum uhuru_log_domain domain)
{
  switch(domain) {
  case UHURU_LOG_LIB:
    return "lib";
  case UHURU_LOG_MODULE:
    return "module";
  case UHURU_LOG_SERVICE:
    return "service";
  }

  return "";
}

const char *uhuru_log_level_str(enum uhuru_log_level log_level)
{
  switch (log_level) {
  case UHURU_LOG_LEVEL_ERROR:
    return "error";
  case UHURU_LOG_LEVEL_WARNING:
    return "warning";
  case UHURU_LOG_LEVEL_INFO:
    return "info";
  case UHURU_LOG_LEVEL_DEBUG:
    return "debug";
  case UHURU_LOG_LEVEL_NONE:
    return "";
  }

  return "";
}

void uhuru_log_default_handler(enum uhuru_log_domain domain, enum uhuru_log_level log_level, const char *message, void *user_data)
{
  FILE *stream = get_stream(log_level);
  GString *gstring = g_string_new(NULL);
  gchar *string;

  g_string_append_printf(gstring, "%s[%d]: ", LOG_NAME, os_getpid());

  if (log_level != UHURU_LOG_LEVEL_NONE)
    g_string_append_printf(gstring, "<%s> ", uhuru_log_level_str(log_level));

  g_string_append(gstring, message);
  g_string_append(gstring, "\n");


  string = g_string_free(gstring, FALSE);

  fputs(string, stream);

  g_free(string);
}

