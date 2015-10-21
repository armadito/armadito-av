#ifndef _NAMED_PIPE_SERVER_
#define _NAMED_PIPE_SERVER_

#include <windows.h> 
#include "json.h"

int start_named_pipe_server();
DWORD WINAPI InstanceThread(LPVOID);
VOID GetAnswerToRequest(LPTSTR, LPTSTR, LPDWORD);

#endif