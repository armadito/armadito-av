#include <stdio.h>
#include "scan.h"
#include <windows.h> 
#include <conio.h>
#include <libuhuru-config.h>
#include <libuhuru/core.h>
#include <libuhuru/ipc.h>
#include "utils/json.h"

#include "named_pipe_client.h"

struct named_pipe_info {
	HANDLE * hPipe;
};

static void scan_callback(struct uhuru_report *report, void *callback_data)
{
	struct named_pipe_info *np_info = (struct named_pipe_info *)callback_data;
	char * response;
	const char * message;

	printf(" scan_callback !\n");

	message = json_get_report_msg(report);

	if (send_message_to_IHM(np_info->hPipe, (char*)message, &response) < 0){
		printf("Error when writing callback msg on Pipe");
		return;
	}

	// Traiter la réponse ici

}

int start_new_scan(struct new_scan* scan, uhuru* uhuru)
{
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	
	struct named_pipe_info *np_info;
	struct uhuru_scan *u_scan;


	if (scan != NULL && scan->scan_id > 0 && scan->scan_path != NULL){

		printf("\n\n #### Start scanning (%d) %s ####\n", scan->scan_id, scan->scan_path);

		if (connect_to_IHM(scan->scan_id, &hPipe) < 0){
			printf("Error when trying to connect to \\\\.\\pipe\\IHM_scan_%d \n", scan->scan_id);
			return -1;
		}

		np_info = (struct named_pipe_info*)malloc(sizeof(struct named_pipe_info));

		// We need the Pipe Handle in callback functions
		np_info->hPipe = &hPipe;
	
		printf("--- uhuru_scan_new() ---\n");
		u_scan = uhuru_scan_new(uhuru, 0, scan->scan_path, UHURU_SCAN_RECURSE);

		printf("--- uhuru_scan_add_callback() ---\n");
		uhuru_scan_add_callback(u_scan, scan_callback, np_info);

		printf("--- uhuru_scan_run() ---\n");
		uhuru_scan_run(u_scan);

		printf("--- uhuru_scan_free() ---\n");


		// TODO : Try fake callback


		// free and close all
		uhuru_scan_free(u_scan);
		closeConnection_to_IHM(&hPipe);
		free(np_info);
	} 


	return 0;
}
