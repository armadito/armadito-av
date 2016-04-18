#include "../update.h"
#include <stdio.h>
#include <libarmadito.h>
//#include "libarmadito-config.h"




enum a6o_json_status update_response_cb(struct armadito *armadito, struct json_request *req, struct json_response *resp, void **request_data)
{
	enum a6o_json_status status = JSON_OK;
	
	printf("[+] Debug :: TODO (linux version) :: update_response_cb...\n");

	return status;

}
