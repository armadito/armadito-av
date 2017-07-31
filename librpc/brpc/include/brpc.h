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


#if 0
struct brpc_mapper;
struct brpc_mapper *brpc_mapper_new(void);


int brpc_mapper_add(struct brpc_mapper *mapper, const char *method, brpc_method_t method_cb);

struct brpc_connection;



struct ipc_builder *ipc_builder_new(enum ipc_type ipc_type, uint8_t method, uint32_t id);

int ipc_builder_add_int(struct ipc_builder *b, int i);

int ipc_builder_add_str(struct ipc_builder *b, const char *s);

char *ipc_builder_end(struct ipc_builder *b);

/* may be buffer is returned by ipc_builder_end() */
char *ipc_builder_get_buffer(struct ipc_builder *b);

struct ipc_builder *ipc_builder_new_from_data(const char *data);

enum ipc_type ipc_builder_get_ipc_type(struct ipc_builder *b);

uint8_t ipc_builder_get_method(struct ipc_builder *b);

uint32_t ipc_builder_get_id(struct ipc_builder *b);

/* can check argument types using fmt */
int ipc_builder_check_types(struct ipc_builder *b, const char *fmt);

uint8_t ipc_builder_get_arg_count(struct ipc_builder *b);

int ipc_builder_get_int(struct ipc_builder *b, uint8_t off, int *arg);

int ipc_builder_get_str(struct ipc_builder *b, uint8_t off, char **arg);

#endif

#endif
