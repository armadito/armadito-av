#include <libuhuru/core.h>

#include <glib.h>
#include <syslog.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

static enum uhuru_log_level get_uhuru_log_level_from_str(const char *s_log_level)
{
  if (!strcmp(s_log_level,"error") || !strcmp(s_log_level,"critical"))
    return UHURU_LOG_LEVEL_ERROR;

  if (!strcmp(s_log_level,"warning"))
    return UHURU_LOG_LEVEL_WARNING;

  if (!strcmp(s_log_level,"message") || !strcmp(s_log_level,"info"))
    return UHURU_LOG_LEVEL_INFO;

  if (!strcmp(s_log_level,"debug"))
    return UHURU_LOG_LEVEL_DEBUG;
  return -1;
}

static int priority_from_level(enum uhuru_log_level log_level)
{
  switch (log_level) {
  case UHURU_LOG_LEVEL_ERROR:
    return LOG_ERR;
  case UHURU_LOG_LEVEL_WARNING:
    return LOG_WARNING;
  case UHURU_LOG_LEVEL_INFO:
    return LOG_INFO;
  case UHURU_LOG_LEVEL_DEBUG:
    return LOG_DEBUG;
  }

  return LOG_INFO;
}
static void uhuru_syslog_handler(enum uhuru_log_domain domain, enum uhuru_log_level log_level, const char *message, void *user_data)
{
  if (log_level != UHURU_LOG_LEVEL_NONE)
    syslog(priority_from_level(log_level), "<%s> %s\n", uhuru_log_level_str(log_level), message);
  else
    syslog(priority_from_level(log_level), "%s\n", message);
}

static void log_init_with_uhuru_log(const char *s_log_level, int use_syslog)
{
  enum uhuru_log_level level = get_uhuru_log_level_from_str(s_log_level);

  if (use_syslog) {
    openlog("uhuru-av", LOG_CONS | LOG_PID, LOG_USER);
    uhuru_log_set_handler(level, uhuru_syslog_handler, NULL);
  } else
    uhuru_log_set_handler(level, uhuru_log_default_handler, NULL);
}

void log_init(const char *s_log_level, int use_syslog)
{
  log_init_with_uhuru_log(s_log_level, use_syslog);
}
