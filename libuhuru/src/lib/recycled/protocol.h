#ifndef _LIBUHURU_PROTOCOL_H_
#define _LIBUHURU_PROTOCOL_H_

struct protocol_handler;

struct protocol_handler *protocol_handler_new(int input_fd, int output_fd);

void protocol_handler_free(struct protocol_handler *handler);

char *protocol_handler_get_msg(struct protocol_handler *handler);

char *protocol_handler_get_header(struct protocol_handler *handler, const char *key);

typedef void (*protocol_handler_cb_t)(struct protocol_handler *handler, void *data);

int protocol_handler_add_callback(struct protocol_handler *handler, const char *cmd, protocol_handler_cb_t cb, void *data);

int protocol_handler_receive(struct protocol_handler *handler);

int protocol_handler_send_msg(struct protocol_handler *handler, const char *msg, ...);

#endif
