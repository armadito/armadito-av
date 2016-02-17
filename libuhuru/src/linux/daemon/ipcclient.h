#ifndef _IPCCLIENT_H_
#define _IPCCLIENT_H_

#include <libuhuru/core.h>

struct ipc_client;

struct ipc_client *ipc_client_new(int sock, struct uhuru *uhuru);

void ipc_client_free(struct ipc_client *cl);

int ipc_client_process(struct ipc_client *cl);

#endif
