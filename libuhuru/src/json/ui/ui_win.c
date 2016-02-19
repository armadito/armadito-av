#include "ui_win.h"
#include <stdio.h>
#include <Windows.h>

#ifdef WIN32
enum uhuru_json_status json_handler_ui_request(const char * ip_path, const char * request, int request_len, char * response, int response_len) {

	enum uhuru_json_status status = JSON_OK;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	int cbWritten = 0;
	int cbBytesRead = 0;

	if (ip_path == NULL || request == NULL || request_len <= 0 ) {
		printf("[-] Error :: json_handler_ui_request :: invalids parameters!\n");
		return JSON_UNEXPECTED_ERR;
	}

	__try {

		// Connect to pipe.
		hPipe = CreateFile( ip_path, GENERIC_READ | GENERIC_WRITE,	0, NULL, OPEN_EXISTING, 0,NULL);
		if (hPipe == INVALID_HANDLE_VALUE) {
			printf("[-] Error :: json_handler_ui_request :: Opening GUI Pipe failed ! :: GLE = %d\n", GetLastError());
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