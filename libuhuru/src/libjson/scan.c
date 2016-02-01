#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "uhurujson.h"
#include "scan.h"
#ifdef linux
#include "net/unixsockclient.h"
#endif

#include <json.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_SOCKET_PATH   "/tmp/.uhuru-ihm"

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

static void scan_callback(struct uhuru_report *report, void *callback_data)
{
  int fd;
  struct json_object *j_request;
  const char *req;
  size_t req_len;

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "scan_callback");

  fd = unix_client_connect(DEFAULT_SOCKET_PATH, 10);

  if (fd < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error connecting to IHM (%s)", strerror(errno));
    return;
  }

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "scan_callback: fd = %d", fd);

  j_request = json_object_new_object();
  json_object_object_add(j_request, "ihm_request", json_object_new_string("scan"));
  json_object_object_add(j_request, "id", json_object_new_int(report->scan_id));
  json_object_object_add(j_request, "params", report_json(report));

  req = json_object_to_json_string(j_request);

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "scan_callback: req = %s", req);

  if (write(fd, req, strlen(req)) < 0)
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error writing JSON response (%s)", strerror(errno));

  close(fd);
}

enum uhuru_json_status scan_request_cb(const char *request, int id, struct json_object *params, struct uhuru *uhuru, struct json_object **p_info, const char **p_error_message)
{
  struct json_object *j_path;
  const char *path;
  struct uhuru_on_demand *on_demand;

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

  on_demand = uhuru_on_demand_new(uhuru, id, path, UHURU_SCAN_RECURSE);

  uhuru_scan_add_callback(uhuru_on_demand_get_scan(on_demand), scan_callback, NULL);

  uhuru_on_demand_run(on_demand);

  return JSON_OK;
}

