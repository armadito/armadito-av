#ifndef _TRACE_H_
#define _TRACE_H_

static inline float get_uptime(void)
{
  float uptime;
  FILE *proc_uptime_file = fopen("/proc/uptime", "r");

  fscanf(proc_uptime_file, "%f", &uptime);

  fclose(proc_uptime_file);

  return uptime;
}

#define trace_log(which,level,format, ...) uhuru_log(which, UHURU_LOG_LEVEL_DEBUG, "[%5.6f] " format, get_uptime(), ## __VA_ARGS__)

#endif
