#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include <glib.h>
#include <stdarg.h>

#define G_LOG_DOMAIN "uhuru"

static GLogLevelFlags g_log_level(enum uhuru_log_level level)
{
  switch(level) {
  case UHURU_LOG_LEVEL_ERROR:
    return G_LOG_LEVEL_ERROR;
  case UHURU_LOG_LEVEL_WARNING:
    return G_LOG_LEVEL_WARNING;
  case UHURU_LOG_LEVEL_INFO:
    return G_LOG_LEVEL_INFO;
  case UHURU_LOG_LEVEL_DEBUG:
    return G_LOG_LEVEL_DEBUG;
  case UHURU_LOG_LEVEL_NONE:
    return 1 << G_LOG_LEVEL_USER_SHIFT;
  }

  return -1;
}

void uhuru_log(enum uhuru_log_domain domain, enum uhuru_log_level level, const char *format, ...)
{
  va_list args;

  va_start(args, format);

  g_logv(G_LOG_DOMAIN, g_log_level(level), format, args);

  va_end(args);
}

void uhuru_log_set_handler(enum uhuru_log_level max_level, uhuru_log_handler_t handler, void *user_data)
{
}

