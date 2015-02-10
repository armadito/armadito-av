#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

struct protocol_command;

char *protocol_command_cmd(struct protocol_command *cmd);
char *protocol_command_header(struct protocol_command *cmd, const char *key);

struct protocol_handler;

typedef void (*protocol_handler_cb_t)(struct protocol_command *cmd, void *data);

struct protocol_handler *protocol_handler_new(void);

int protocol_handler_add_callback(struct protocol_handler *handler, protocol_handler_cb_t cb, void *data);

int protocol_handler_input_char(struct protocol_handler *handler, char c);

#endif
