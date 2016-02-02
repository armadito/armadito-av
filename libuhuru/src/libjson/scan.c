#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "uhurujson.h"
#include "scan.h"

#ifdef linux
#include "net/unixsockclient.h"
#define DEFAULT_SOCKET_PATH   "/tmp/.uhuru-ihm"
#endif

#include <json.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifdef linux
#include <pthread.h>
#include <sys/time.h>
#endif

struct scan_data {
  struct uhuru *uhuru;
  const char *path;
  int scan_id;
  time_t last_send_time;
  int last_send_progress;
};

#ifdef linux
static time_t get_milliseconds(void)
{
  struct timeval now;

  if (gettimeofday(&now, NULL) < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "error getting time IHM (%s)", strerror(errno));
    return 0;
  }

  return now.tv_sec * 1000 + now.tv_usec / 1000;
}
#endif

#ifdef WIN32
#error must implement get_milliseconds on windows
#if 0
Note: gettimeofday on windows:
#include <time.h>
#include <sys/timeb.h>
int gettimeofday (struct timeval *tp, void *tz)
{
struct _timeb timebuffer;
_ftime (&timebuffer);
tp->tv_sec = timebuffer.time;
tp->tv_usec = timebuffer.millitm * 1000;
return 0;
}
#endif
#endif
/*
  JSON object fields examples:

  "path": "C://cygwin64//home//malware.exe", 
  "scan_status": "malware", 
  "scan_action": "alert+quarantine", 
  "mod_name": "clamav", 
  "mod_report": "Trojan.Mal32-77"
*/

static struct json_object *report_json(struct uhuru_report *report)
{
  struct json_object *j_report;

  j_report = json_object_new_object();

  json_object_object_add(j_report, "progress", json_object_new_int(report->progress));

  if (report->path != NULL)
    json_object_object_add(j_report, "path", json_object_new_string(report->path));

  json_object_object_add(j_report, "scan_status", json_object_new_string(uhuru_file_status_pretty_str(report->status)));
  json_object_object_add(j_report, "scan_action", json_object_new_string(uhuru_action_pretty_str(report->action)));

  if (report->mod_name != NULL)
    json_object_object_add(j_report, "mod_name", json_object_new_string(report->mod_name));

  if (report->mod_report != NULL)
    json_object_object_add(j_report, "mod_report", json_object_new_string(report->mod_report));

  return j_report;
}

/*
{ "ihm_request":"scan", 
  "id":123, 
  "params": {
    "progress": "70",
    "path": "C://cygwin64//home//malware.exe", 
    "scan_status": "malware", 
    "scan_action": "alert+quarantine", 
    "mod_name": "clamav", 
    "mod_report": "Trojan.Mal32-77"
  }
}
*/

#define SEND_PERIOD 200  /* milliseconds */

static void scan_callback(struct uhuru_report *report, void *callback_data)
{
  int fd;
  struct json_object *j_request;
  struct scan_data *scan_data = (struct scan_data *)callback_data;
  const char *req;
  size_t req_len;
  time_t now;

  now = get_milliseconds();
  if (report->status == UHURU_CLEAN
      && report->progress != 100
      && scan_data->last_send_progress != REPORT_PROGRESS_UNKNOWN
      && scan_data->last_send_progress == report->progress
      && scan_data->last_send_time != 0
      && (now - scan_data->last_send_time) < SEND_PERIOD)
    return;

  fd = unix_client_connect(DEFAULT_SOCKET_PATH, 10);

  if (fd < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error connecting to IHM (%s)", strerror(errno));
    return;
  }

  j_request = json_object_new_object();
  json_object_object_add(j_request, "ihm_request", json_object_new_string("scan"));
  json_object_object_add(j_request, "id", json_object_new_int(report->scan_id));
  json_object_object_add(j_request, "params", report_json(report));

  req = json_object_to_json_string(j_request);

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "scan_callback: sending req = %s", req);

  if (write(fd, req, strlen(req)) < 0)
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error writing JSON response (%s)", strerror(errno));

  if (write(fd, "\r\n\r\n", 4) < 0)
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error writing JSON response (%s)", strerror(errno));

  scan_data->last_send_time = now;
  scan_data->last_send_progress = report->progress;

  close(fd);
}

static void *scan_thread_fun(void *arg)
{
  struct scan_data *scan_data = (struct scan_data *)arg;
  struct uhuru_on_demand *on_demand;

  on_demand = uhuru_on_demand_new(scan_data->uhuru, scan_data->scan_id, scan_data->path, UHURU_SCAN_RECURSE | UHURU_SCAN_THREADED);

  uhuru_scan_add_callback(uhuru_on_demand_get_scan(on_demand), scan_callback, scan_data);

  uhuru_on_demand_run(on_demand);

  free(arg);

  uhuru_log(UHURU_LOG_SERVICE, UHURU_LOG_LEVEL_DEBUG, "JSON: scan thread terminated");

  return NULL;
}


enum uhuru_json_status scan_request_cb(const char *request, int id, struct json_object *params, struct uhuru *uhuru, struct json_object **p_info, const char **p_error_message)
{
  struct json_object *j_path;
  const char *path;
  struct scan_data *scan_data;
  pthread_t scan_thread;

#ifdef DEBUG
  uhuru_log(UHURU_LOG_SERVICE, UHURU_LOG_LEVEL_DEBUG, "JSON: scan cb called");
#endif

  /* check if 'params' object contains key "path" with a string value */
  if (!json_object_object_get_ex(params, "path", &j_path)
      || !json_object_is_type(j_path, json_type_string))
    return JSON_INVALID_REQUEST;

  /* future fields of the 'params' object: */
  /* "mode": "personnalized", */
  /* "configuration": "toto1", */

  *p_info = NULL;

  path = json_object_get_string(j_path);

  uhuru_log(UHURU_LOG_SERVICE, UHURU_LOG_LEVEL_DEBUG, "JSON: scan path = %s", path);

  scan_data = malloc(sizeof(struct scan_data));
  scan_data->uhuru = uhuru;
  scan_data->scan_id = id;
  scan_data->path = path;
  scan_data->last_send_time = 0L;
  scan_data->last_send_progress = REPORT_PROGRESS_UNKNOWN;

#ifdef linux
  if (pthread_create(&scan_thread, NULL, scan_thread_fun, scan_data)) {
    uhuru_log(UHURU_LOG_SERVICE, UHURU_LOG_LEVEL_ERROR, "JSON: cannot create scan thread (%s)", strerror(errno));
    return JSON_REQUEST_FAILED;
  }
#endif
#ifdef WIN32
#error must add thread creation
#endif

  return JSON_OK;
}

