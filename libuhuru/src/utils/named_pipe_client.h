#ifndef _NAMED_PIPE_CLIENT_
#define _NAMED_PIPE_CLIENT_

int start_named_pipe_client(char* path, char * message, char** server_response);
int connect_to_IHM(int scan_id, char ** response);

#endif