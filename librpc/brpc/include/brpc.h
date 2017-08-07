#ifndef BRPC_H
#define BRPC_H

#include <stddef.h>
#include <stdint.h>
#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

/* error codes */
#define BRPC_OK                               0
#define BRPC_EOF                              1
#define BRPC_ERR_INTERNAL_ERROR               2
#define BRPC_ERR_METHOD_NOT_FOUND             3
#define BRPC_ERR_ARGC_OUT_OF_BOUND            4
#define BRPC_ERR_INVALID_ARGUMENT_TYPE        5
#define BRPC_ERR_INVALID_BUFFER_TYPE          6
#define BRPC_ERR_INVALID_RESPONSE_ID          7
#define BRPC_ERR_INVALID_ERROR_BUFFER         8

typedef char brpc_buffer_t;

brpc_buffer_t *brpc_buffer_new(const char *fmt, ...);

int32_t brpc_buffer_get_int32(const brpc_buffer_t *b, uint8_t index, int *error);
int64_t brpc_buffer_get_int64(const brpc_buffer_t *b, uint8_t index, int *error);
char *brpc_buffer_get_str(const brpc_buffer_t *b, uint8_t index, int *error);

/*
 * RPC mapper
 * handles mapping method id (a char) to method definition
 */

struct brpc_connection;

typedef int (*brpc_method_t)(struct brpc_connection *conn, const brpc_buffer_t *params, brpc_buffer_t **result);

struct brpc_mapper;

struct brpc_mapper *brpc_mapper_new(void);

int brpc_mapper_add(struct brpc_mapper *mapper, uint8_t method, brpc_method_t method_cb);

/*
  RPC connection
  handles
  - id generation and management
*/
struct brpc_connection *brpc_connection_new(struct brpc_mapper *mapper, void *data);

void brpc_connection_free(struct brpc_connection *conn);

void *brpc_connection_get_data(struct brpc_connection *conn);

typedef ssize_t (*brpc_read_cb_t)(void *buffer, size_t size, void *data);

void brpc_connection_set_read_cb(struct brpc_connection *conn, brpc_read_cb_t read_cb, void *data);

typedef ssize_t (*brpc_write_cb_t)(const void *buffer, size_t size, void *data);

void brpc_connection_set_write_cb(struct brpc_connection *conn, brpc_write_cb_t write_cb, void *data);

typedef void (*brpc_error_handler_t)(struct brpc_connection *conn, uint32_t id, int code, const char *message);

void brpc_connection_set_error_handler(struct brpc_connection *conn, brpc_error_handler_t error_handler);

int brpc_connection_process(struct brpc_connection *conn);

int brpc_notify(struct brpc_connection *conn, uint8_t method, brpc_buffer_t *params);

/* int brpc_notify(struct brpc_connection *conn, uint8_t method, const char *fmt, ...); */

typedef void (*brpc_cb_t)(const brpc_buffer_t *result, void *user_data);

int brpc_call(struct brpc_connection *conn, uint8_t method, brpc_buffer_t *params, brpc_cb_t cb, void *user_data);

/* int brpc_call(struct brpc_connection *conn, uint8_t method, brpc_cb_t cb, void *user_data, const char *fmt, ...); */

#endif
