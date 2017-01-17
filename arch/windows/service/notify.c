/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "notify.h"
#include "scan_ondemand.h"


//#define UI_IPC_PATH "\\\\.\\pipe\\armadito-UI"
#define MAX_LEN 2048

char * notif_type_to_str(enum notif_type type) {

	switch (type) {
		case NOTIF_INFO:
			return "INFO";
			break;
		case NOTIF_WARNING:
			return "WARNING";
			break;
		case NOTIF_ERROR:
			return "ERROR";
			break;
		default:
			break;
	}

	return "NONE";
}

int send_notif(enum notif_type type, char * message) {

	int ret = 0;
	json_object * jobj = NULL;
	json_object * jparams = NULL;
	char * notif = NULL;
	int notif_len = 0;
	char response[2048] = {0};
	int response_len = 2048;
	enum a6o_json_status status = JSON_OK;


	if (message == NULL) {
		return -1;
	}

	__try {

		if ((jobj = json_object_new_object()) == NULL) {
			printf("[-] Error :: send_notif :: can't create json object!\n");
			ret = -2;
			__leave;
		}

		if ((jparams = json_object_new_object()) == NULL) {
			printf("[-] Error :: send_notif :: can't create json object!\n");
			ret = -3;
			__leave;
		}

		/*
		{
			"ihm_request":"notification", 
			"id": 0,	
			"status":0,
			"params":{
				"type":"WARNING",
				"message":"Malware detected!"
			}
		}
		*/

		json_object_object_add(jobj, "ihm_request", json_object_new_string("notification"));
		json_object_object_add(jobj, "id", json_object_new_int(0));
		json_object_object_add(jobj, "status", json_object_new_int(0));

		json_object_object_add(jparams, "type", json_object_new_string(notif_type_to_str(type)));
		json_object_object_add(jparams, "message", json_object_new_string(message));
		json_object_object_add(jobj, "params", jparams);
			
		notif = json_object_get_string(jobj);
		notif_len = strnlen(notif,MAX_LEN);
		
		status = json_handler_ui_request(UI_IPC_PATH, notif, notif_len, response, response_len);
		if (status != JSON_OK) {
			//printf("[-] Warning :: send_notif :: notification not sent/received correctly!\n");
			a6o_log(A6O_LOG_SERVICE,A6O_LOG_LEVEL_WARNING,"[-] Warning :: send_notif :: notification not sent/received correctly!\n");
			ret = -4;
			__leave;
		}

		printf("[+] Debug :: send_notif :: notification = %s\n",notif);



	}
	__finally {

		if (jobj != NULL) {
			json_object_put(jobj);
		}

		if (jparams != NULL) {
			json_object_put(jparams);
		}
	}


	return ret;

}

