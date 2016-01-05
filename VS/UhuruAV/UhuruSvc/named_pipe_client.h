#ifndef _NAMED_PIPE_CLIENT_
#define _NAMED_PIPE_CLIENT_

#include <Windows.h>

int start_named_pipe_client(char* path, HANDLE * hPipe);
int send_message_to_IHM(HANDLE * hPipe, char * message, char** server_response);
int connect_to_IHM(int scan_id, HANDLE * hPipe);
int closeConnection_to_IHM(HANDLE * hPipe);

#endif