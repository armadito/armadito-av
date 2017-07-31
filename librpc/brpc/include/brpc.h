#ifndef BRPC_H
#define BRPC_H

#include <stdint.h>

typedef char brpc_buffer_t;

struct brpc_connection;

typedef int (*brpc_method_t)(struct brpc_connection *conn, const brpc_buffer_t *args, brpc_buffer_t **res);

/*
   1) compute buffer length by walking through the arguments
   2) allocate buffer
   3) fill buffer by walking again through the arguments (make 2 functions taking va_list arguments)
*/
enum brpc_buffer_type {
	REQUEST = 1,
	NOTIFY,
	RESPONSE,
	ERROR,
};

brpc_buffer_t *brpc_buffer_new(const char *fmt, ...);

int32_t brpc_buffer_get_int32(const brpc_buffer_t *b, uint8_t index, int *error);
int64_t brpc_buffer_get_int64(const brpc_buffer_t *b, uint8_t index, int *error);
char *brpc_buffer_get_str(const brpc_buffer_t *bduff, uint8_t index, int *error);


struct brpc_mapper;

struct brpc_mapper *brpc_mapper_new(void);

int brpc_mapper_add(struct brpc_mapper *mapper, const char *method, brpc_method_t method_cb);

struct brpc_connection;

#endif
