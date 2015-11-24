#include <glib.h>
#include <syslog.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

static const char *level_str(GLogLevelFlags log_level)
{
  switch (log_level & G_LOG_LEVEL_MASK) {
  case G_LOG_LEVEL_ERROR:
    return "error";
  case G_LOG_LEVEL_CRITICAL:
    return "critical";
  case G_LOG_LEVEL_WARNING:
    return "warning";
  case G_LOG_LEVEL_MESSAGE:
    return "message";
  case G_LOG_LEVEL_INFO:
    return "info";
  case G_LOG_LEVEL_DEBUG:
    return "debug";
  }

  return "";
}

static int priority_from_level(GLogLevelFlags log_level)
{
  switch (log_level & G_LOG_LEVEL_MASK) {
  case G_LOG_LEVEL_ERROR:
    return LOG_ERR;
  case G_LOG_LEVEL_CRITICAL:
    return LOG_CRIT;
  case G_LOG_LEVEL_WARNING:
    return LOG_WARNING;
  case G_LOG_LEVEL_MESSAGE:
    return LOG_NOTICE;
  case G_LOG_LEVEL_INFO:
    return LOG_INFO;
  case G_LOG_LEVEL_DEBUG:
    return LOG_DEBUG;
  }

  return LOG_INFO;
}

static void syslog_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
  if (log_level < (1 << G_LOG_LEVEL_USER_SHIFT))
    syslog(priority_from_level(log_level), "<%s> %s\n", level_str(log_level), message);
  else
    syslog(priority_from_level(log_level), "%s\n", message);
}

static FILE *get_stream(GLogLevelFlags log_level)
{
  gboolean to_stderr = TRUE;

  switch (log_level & G_LOG_LEVEL_MASK) {
    case G_LOG_LEVEL_INFO:
    case G_LOG_LEVEL_DEBUG:
      to_stderr = FALSE;
  }

  return to_stderr ? stderr : stdout;
}

static void stderrout_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
  FILE *stream = get_stream(log_level);
  GString *gstring = g_string_new(NULL);
  gchar *string;

  if (log_domain)
     g_string_append_printf(gstring, "%s[%d]: ", log_domain, getpid());

  if (log_level < (1 << G_LOG_LEVEL_USER_SHIFT))
    g_string_append_printf(gstring, "<%s> ", level_str(log_level));

  g_string_append(gstring, message);
  g_string_append (gstring, "\n");

  string = g_string_free(gstring, FALSE);

  fputs(string, stream);

  g_free (string);
}

static GLogLevelFlags get_log_level_from_str(const char *s_log_level)
{
  GLogLevelFlags default_flags = (1 << G_LOG_LEVEL_USER_SHIFT);

  if (!strcmp(s_log_level,"error"))
    return default_flags | G_LOG_LEVEL_ERROR;
  if (!strcmp(s_log_level,"critical"))
    return default_flags | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL;
  if (!strcmp(s_log_level,"warning"))
    return default_flags | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING;
  if (!strcmp(s_log_level,"message"))
    return default_flags | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE;
  if (!strcmp(s_log_level,"info"))
    return default_flags | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO;
  if (!strcmp(s_log_level,"debug"))
    return default_flags | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG;

  return -1;
}

void log_init(const char *s_log_level, int use_syslog)
{
  GLogLevelFlags flags, fatal_flags;
  
  flags = get_log_level_from_str(s_log_level) | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION;

  if (use_syslog) {
    openlog("uhuru-av", LOG_CONS | LOG_PID, LOG_USER);
    g_log_set_handler(G_LOG_DOMAIN, flags, syslog_handler, NULL);
  } else
    g_log_set_handler(G_LOG_DOMAIN, flags, stderrout_handler, NULL);

  fatal_flags = g_log_set_fatal_mask(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR);
}
