#ifndef _NAMED_PIPE_CLIENT_
#define _NAMED_PIPE_CLIENT_

#include <windows.h>

int start_named_pipe_client(char* path, HANDLE * hPipe);
int send_message_to_IHM(HANDLE * hPipe, char * message, char** server_response);
int connect_to_IHM(int scan_id, char ** response, HANDLE * hPipe);

#endif