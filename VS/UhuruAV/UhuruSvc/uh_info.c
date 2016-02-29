#include "uh_info.h"
#include "json\ui\ui.h"
#include <string.h>

#define PIPE_NAME "\\\\.\\pipe\\Armadito_ondemand"

/*
	This function display Av's information in json format.
	returns 0 on success or
	an error code (<0) on error.
*/
int get_av_info( ) {

	int ret = 0;
	struct uhuru* uhuru = NULL;
	enum uhuru_json_status status = JSON_OK;
	char * request = "{ \"av_request\":\"state\", \"id\":123, \"params\": {}}";
	int request_len = 0;
	char * response[2048] = {0};
	int response_len = 2048;

	request_len = strnlen_s(request,_MAX_PATH);

	__try {

		// Send state request to av
		status = json_handler_ui_request(PIPE_NAME, request, request_len, response, response_len);
		if (status != JSON_OK) {
			printf("[-] Error :: get_av_info :: json_handler_ui_request failed :: status= %d \n", status);
			ret = -1;
			__leave;
		}
		
		//printf("[+] Debug :: get_av_info :: response = %s\n",response);
		printf("%s\n",response);


	}
	__finally {



	}

	return ret;
}