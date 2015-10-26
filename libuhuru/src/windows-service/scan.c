#include <stdio.h>
#include "scan.h"
#include <windows.h> 
#include <conio.h>

#include "named_pipe_client.h"

int start_new_scan(struct new_scan* scan)
{
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	char * response;

	if (scan != NULL && scan->scan_id > 0 && scan->scan_path != NULL){
		printf("\n\n #### Start scanning (%d) %s ####\n", scan->scan_id, scan->scan_path);
		
		// CALL libuhuru here

		if (connect_to_IHM(scan->scan_id, &hPipe) < 0){
			printf("Error when trying to connect to \\.\pipe\IHM_scan_%d \n", scan->scan_id);
			return -1;
		}

		send_message_to_IHM(&hPipe, "{\"message\":\"ok\"}", &response);
		closeConnection_to_IHM(&hPipe);
	} 

	return 0;
}
