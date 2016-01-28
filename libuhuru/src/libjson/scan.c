#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "uhurujson.h"
#include "scan.h"

#include <json.h>
#include <stdlib.h>
#include <string.h>

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

static void scan_callback(struct uhuru_report *report, void *callback_data)
{
#if 0
  char * response;
  const char * message;
  HANDLE * hPipe = (HANDLE*)malloc(sizeof(HANDLE));

  if (connect_to_IHM(report->scan_id, hPipe) < 0){
    printf("Error when trying to connect to \\\\.\\pipe\\IHM_scan_%d \n", report->scan_id);
    return;
  }

  message = json_get_report_msg(report);

  if (send_message_to_IHM(hPipe, (char*)message, &response) < 0){
    printf("Error when writing callback msg on Pipe");
    return;
  }

  // Traiter la réponse ici
  closeConnection_to_IHM(hPipe);
#endif
}

enum uhuru_json_status scan_request_cb(const char *request, int id, struct json_object *params, struct uhuru *uhuru, struct json_object **p_info, const char **p_error_message)
{
  struct json_object *j_path;
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

  on_demand = uhuru_on_demand_new(uhuru, id, json_object_get_string(j_path), UHURU_SCAN_RECURSE);

  uhuru_scan_add_callback(uhuru_on_demand_get_scan(on_demand), scan_callback, p_info);

  uhuru_on_demand_run(on_demand);

  return JSON_OK;
}

