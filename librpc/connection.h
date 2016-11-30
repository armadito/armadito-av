
#include <libarmadito-rpc/armadito-rpc.h>

#include <jansson.h>

int a6o_rpc_connection_send(struct a6o_rpc_connection *conn, json_t *obj);

size_t a6o_rpc_connection_register_callback(struct a6o_rpc_connection *conn, a6o_rpc_cb_t cb, void *user_data);


