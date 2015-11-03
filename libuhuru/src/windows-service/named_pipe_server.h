#ifndef _NAMED_PIPE_SERVER_
#define _NAMED_PIPE_SERVER_

#include <windows.h> 
#include <libuhuru-config.h>
#include <libuhuru/core.h>
#include "utils/json.h"
#include "scan.h"

struct thread_parameters {
	HANDLE hPipe;
	uhuru* uhuru;
};

int start_named_pipe_server(uhuru* uhuru);
DWORD WINAPI InstanceThread(LPVOID);
VOID GetAnswerToRequest(LPTSTR, LPTSTR, LPDWORD, struct new_scan* scan);

#endif