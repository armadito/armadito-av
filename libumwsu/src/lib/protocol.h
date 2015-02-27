#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

struct protocol_handler;

struct protocol_handler *protocol_handler_new(int input_fd, int output_fd);

void protocol_handler_free(struct protocol_handler *handler);

char *protocol_handler_cmd(struct protocol_handler *handler);

char *protocol_handler_header_value(struct protocol_handler *handler, const char *header_key);

typedef void (*protocol_handler_cb_t)(struct protocol_handler *handler, void *data);

int protocol_handler_add_callback(struct protocol_handler *handler, const char *cmd, protocol_handler_cb_t cb, void *data);

int protocol_handler_input(struct protocol_handler *handler);

int protocol_handler_input_buffer(struct protocol_handler *handler, char *buff, int len);

int protocol_handler_output_message(struct protocol_handler *handler, const char *cmd, ...);

#endif
