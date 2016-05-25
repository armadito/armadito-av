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

#include <libarmadito.h>
#include "ui.h"
#include <stdio.h>
#include <Windows.h>

#ifdef WIN32
enum a6o_json_status json_handler_ui_request(const char * ip_path, const char * request, int request_len, char * response, int response_len) {

	enum a6o_json_status status = JSON_OK;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	int cbWritten = 0;
	int cbBytesRead = 0;

	if (ip_path == NULL || request == NULL || request_len <= 0 ) {
		a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: json_handler_ui_request :: invalids parameters!\n");
		return JSON_UNEXPECTED_ERR;
	}

	__try {

		// Connect to pipe.
		hPipe = CreateFile( ip_path, GENERIC_READ | GENERIC_WRITE,	0, NULL, OPEN_EXISTING, 0,NULL);
		if (hPipe == INVALID_HANDLE_VALUE) {
			a6o_log(ARMADITO_LOG_SERVICE,ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: json_handler_ui_request :: Opening Pipe [%s] failed ! :: GLE = %d\n",ip_path,GetLastError());
			status = JSON_UNEXPECTED_ERR;
			__leave;
		}

		// write request to GUI.
		if ( (WriteFile(hPipe, request, request_len, &cbWritten, NULL) == FALSE ) || cbWritten <= 0) {
			printf("[-] Error :: json_handler_ui_request :: Write in GUI pipe failed with error :: %d \n",GetLastError());
			status = JSON_REQUEST_FAILED;
			__leave;
		}

		// get IHM response
		if ((ReadFile(hPipe, response, response_len,&cbBytesRead,NULL) == FALSE) || cbBytesRead <=0) {
			printf("[-] Error :: json_handler_ui_request :: Read in GUI pipe failed with error :: %d \n",GetLastError());
			status = JSON_REQUEST_FAILED;
			__leave;
		}
		response[cbBytesRead] = '\0';
		//printf("[+] Debug :: json_handler_ui_request :: GUI request = %s ::\n",req_len, request);


	}
	__finally {

		if (hPipe != INVALID_HANDLE_VALUE) {
			CloseHandle(hPipe);
			hPipe = INVALID_HANDLE_VALUE;
		}

	}

	return status;

}
#endif
