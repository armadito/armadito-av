#include <stdio.h>
#include <stdlib.h>
#include "named_pipe_server.h"
#include <libuhuru-config.h>
#include <libuhuru/core.h>

int main(int argc, const char **argv)
{
	// load modules db etc.
	uhuru_error *err = NULL;
	uhuru * uhuru = uhuru_open(&err);
	if (uhuru == NULL){
		printf("uhuru_open() error - %s \n", err->error_message );
		return -1;
	}

	// Notes : If you intend to use a named pipe locally only, deny access to NT AUTHORITY\NETWORK or switch to local RPC.
	if (start_named_pipe_server(uhuru) < 0){
		printf("named_pipe_server - error \n");
		return -1;
	}

	return 0;
}