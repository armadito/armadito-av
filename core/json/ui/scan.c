/***

Copyright (C) 2015, 2016, 2017 Teclib'

This file is part of Armadito.

Armadito is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito.  If not, see <http://www.gnu.org/licenses/>.

***/

#include <libarmadito.h>

#include "config/libarmadito-config.h"

#include "jsonhandler.h"
#include "scan.h"
#include "ui.h"
#ifndef _WIN32
#include "debug.h"
#else
#include <sys/timeb.h>
#endif

#include "os/string.h"

#include <assert.h>
#include <json.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef linux
#include <sys/time.h>

static time_t get_milliseconds(void)
{
	struct timeval now;

	if (gettimeofday(&now, NULL) < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, "error getting time IHM (%s)", strerror(errno));
		return 0;
	}

	return now.tv_sec * 1000 + now.tv_usec / 1000;
}
#endif

#ifdef _WIN32
// get_milliseconds on windows
time_t get_milliseconds( ) {

	time_t ms = 0;
	struct _timeb tb;

	_ftime64_s(&tb);		
	ms = tb.time * 1000 + tb.millitm;
	
	/*
	//Debug print
	printf("[+] Debug :: tb.time = %d\n", tb.time);
	printf("[+] Debug :: tb.millitm = %u\n", tb.millitm);
	printf("[+] Debug :: ms = %u milliseconds\n", ms);
	*/

	return ms;
}
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

  "progress": "70",
  "malware_count": "22",
  "suspicious_count": "10",
  "scanned_count : "40",
  "path": "C://cygwin64//home//malware.exe",
  "scan_status": "malware",
  "scan_action": "alert+quarantine",
  "mod_name": "clamav",
  "mod_report": "Trojan.Mal32-77"
*/

static struct json_object *report_json(struct a6o_report *report)
{
	struct json_object *j_report;

	j_report = json_object_new_object();

	json_object_object_add(j_report, "progress", json_object_new_int(report->progress));
	json_object_object_add(j_report, "malware_count", json_object_new_int(report->malware_count));
	json_object_object_add(j_report, "suspicious_count", json_object_new_int(report->suspicious_count));
	json_object_object_add(j_report, "scanned_count", json_object_new_int(report->scanned_count));

	if (report->path != NULL)
		json_object_object_add(j_report, "path", json_object_new_string(report->path));

	json_object_object_add(j_report, "scan_status", json_object_new_string(a6o_file_status_pretty_str(report->status)));
	json_object_object_add(j_report, "scan_action", json_object_new_string(a6o_action_pretty_str(report->action)));

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
  "malware_count": "22",
  "suspicious_count": "10",
  "scanned_count : "40",
  "path": "C://cygwin64//home//malware.exe",
  "scan_status": "malware",
  "scan_action": "alert+quarantine",
  "mod_name": "clamav",
  "mod_report": "Trojan.Mal32-77"
  }
  }
*/

struct scan_data {
	/* this parameter will allow the scan to connect back to the client (IHM or command-line tool) */
	/* value is unix socket path on linux, named pipe path on windows */
	const char *ui_ipc_path;
	time_t last_send_time;
	int last_send_progress;
	struct a6o_on_demand *on_demand;
};

#define SEND_PERIOD 200  /* milliseconds */
#define RESPONSE_BUFFER_SIZE 1024

static void scan_callback(struct a6o_report *report, void *callback_data)
{
	struct json_object *j_request;
	struct scan_data *scan_data = (struct scan_data *)callback_data;
	const char *req;
	char resp[RESPONSE_BUFFER_SIZE];
	time_t now = 0;
	enum a6o_json_status status = JSON_OK;

	now = get_milliseconds();

	if (report->status == ARMADITO_CLEAN
		&& report->progress != 100
		&& scan_data->last_send_progress != REPORT_PROGRESS_UNKNOWN
		&& scan_data->last_send_progress == report->progress
		&& scan_data->last_send_time != 0
		&& (now - scan_data->last_send_time) < SEND_PERIOD)
		return;

	j_request = json_object_new_object();
	json_object_object_add(j_request, "ihm_request", json_object_new_string("scan"));
	json_object_object_add(j_request, "id", json_object_new_int(report->scan_id));
	json_object_object_add(j_request, "params", report_json(report));

#ifndef _WIN32
	jobj_debug(j_request, "IHM request");
#endif

	req = json_object_to_json_string(j_request);

	/* ui exchange using platform specific function */
	status = json_handler_ui_request(scan_data->ui_ipc_path, req, strlen(req), resp, sizeof(resp));
	if (status != JSON_OK) {
		printf("[-] Error :: scan_callback :: fail to send request to GUI");
		if(req != NULL)
		   printf("= %s", req);
		printf("\n\n");
	}

	scan_data->last_send_time = now;
	scan_data->last_send_progress = report->progress;

        assert(json_object_put(j_request));
}

enum a6o_json_status scan_response_cb(struct armadito *armadito, struct json_request *req, struct json_response *resp, void **request_data)
{
	struct json_object *j_path, *j_ui_ipc_path;
	struct scan_data *scan_data;

#ifdef DEBUG
	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_DEBUG, "JSON: scan cb called");
#endif

	/* check if 'params' object contains key "path_to_scan" with a string value */
	if (!json_object_object_get_ex(req->params, "path_to_scan", &j_path)
		|| !json_object_is_type(j_path, json_type_string))
		return JSON_INVALID_REQUEST;

	/* check if 'params' object contains key "ui_ipc_path" with a string value */
	if (!json_object_object_get_ex(req->params, "ui_ipc_path", &j_ui_ipc_path)
		|| !json_object_is_type(j_ui_ipc_path, json_type_string))
		return JSON_INVALID_REQUEST;

	/* future fields of the 'params' object: */
	/* "mode": "personnalized", */
	/* "configuration": "toto1", */

	scan_data = malloc(sizeof(struct scan_data));
	scan_data->last_send_time = 0L;
	scan_data->last_send_progress = REPORT_PROGRESS_UNKNOWN;
	scan_data->ui_ipc_path = os_strdup(json_object_get_string(j_ui_ipc_path));
	scan_data->on_demand = a6o_on_demand_new(armadito, req->id, json_object_get_string(j_path), ARMADITO_SCAN_RECURSE | ARMADITO_SCAN_THREADED);

	a6o_scan_add_callback(a6o_on_demand_get_scan(scan_data->on_demand), scan_callback, scan_data);

	*request_data = scan_data;

	return JSON_OK;
}

void scan_process_cb(struct armadito *armadito, void *request_data)
{
	struct scan_data *scan_data = (struct scan_data *)request_data;

	a6o_on_demand_run(scan_data->on_demand);

	a6o_on_demand_free(scan_data->on_demand);

	free(scan_data);
}

enum a6o_json_status scan_cancel_response_cb(struct armadito *armadito, struct json_request *req, struct json_response *resp, void **request_data)
{
#ifdef DEBUG
  a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_DEBUG, "JSON: scan cancel cb called");
#endif

  a6o_on_demand_cancel(NULL);

  return JSON_OK;
}


