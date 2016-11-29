
#include <jansson.h>

struct a6o_rpc_connection;

int rpc_connection_send(struct a6o_rpc_connection *conn, json_t *obj);

