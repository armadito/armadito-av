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

#include "tools.h"
#include <stdio.h>
#include <ui\ui.h>

#define SVC_IPC_PATH "\\\\.\\pipe\\Armadito_ondemand"

/*
	This function display Av's information in json format.
	returns 0 on success or
	an error code (<0) on error.
*/
int get_av_info( ) {

	int ret = 0;	
	enum a6o_json_status status = JSON_OK;
	char * request = "{ \"av_request\":\"state\", \"id\":123, \"params\": {}}";
	int request_len = 0;
	char * response[2048] = {0};
	int response_len = 2048;

	char default_resp[] = "{ \"av_response\": \"state\", \"id\": 123, \"status\": -1, \"info\": { \"antivirus\": { \"version\": \"0.0\", \"service\": \"off\", \"real-time-protection\": \"off\" }, \"update\": { \"status\": \"critical\", \"last-update\": \"1970-01-01 00:00\" }, \"modules\": [] } } ";

	request_len = strnlen_s(request,_MAX_PATH);

	__try {

		// Send state request to av
		status = json_handler_ui_request(SVC_IPC_PATH, request, request_len, response, response_len);
		if (status != JSON_OK) {
			a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: get_av_info :: json_handler_ui_request failed :: status= %d \n", status);
			ret = -1;
			__leave;
		}
		
		//printf("[+] Debug :: get_av_info :: response = %s\n",response);
		printf("%s\n",response);


	}
	__finally {

		if (ret < 0) {
			printf("%s\n",default_resp);
		}


	}

	return ret;
}