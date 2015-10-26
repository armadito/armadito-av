#include <stdio.h>
#include <stdlib.h>
#include "named_pipe_server.h"


int main(int argc, const char **argv)
{
	int named_pipe = 0;

	// Notes : If you intend to use a named pipe locally only, deny access to NT AUTHORITY\NETWORK or switch to local RPC.
	named_pipe = start_named_pipe_server();

	return 0;
}
