#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

struct protocol_handler;

struct protocol_handler *protocol_handler_new(void);

typedef void (*protocol_handler_cb_t)(struct protocol_handler *handler, void *data);

int protocol_handler_add_callback(struct protocol_handler *handler, const char *cmd, protocol_handler_cb_t cb, void *data);

int protocol_handler_input_char(struct protocol_handler *handler, char c);

char *protocol_handler_cmd(struct protocol_handler *handler);

char *protocol_handler_header_value(struct protocol_handler *handler, const char *header_key);

#endif
