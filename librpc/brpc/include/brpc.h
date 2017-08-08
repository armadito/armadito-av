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

/* status and error codes */
#define BRPC_OK                               0      /* no error */
#define BRPC_EOF                              1      /* reading connection returned 0 bytes, other end has hanged up */
#define BRPC_ERR_IO_ERROR                     2      /* an error occured reading/writing connection */
#define BRPC_ERR_METHOD_NOT_FOUND             3      /* the invoked method (either call or notify) does not exist */
#define BRPC_ERR_ARGC_OUT_OF_BOUND            4      /* when retrieving argument by index, index is greater than effective # of arguments */
#define BRPC_ERR_ARGUMENT_TYPE_MISMATCH       5      /* when retrieving argument by index, wanted argument type does not match type in the message */
#define BRPC_ERR_TOO_MANY_ARGUMENTS           6      /* when retrieving argument by index, index is greater than effective # of arguments */
#define BRPC_ERR_INVALID_MESSAGE_TYPE         7      /* the message type is neither request, response or error */
#define BRPC_ERR_INVALID_RESPONSE_ID          8      /* the id of the response is not registered */
#define BRPC_ERR_INVALID_ERROR_MESSAGE        9      /* the error message does not contain an int32 (code) and a str (message) */
#define BRPC_ERR_METHOD_ERROR                 0x80   /* mask for method specific error code */

#define BRPC_ERR_IS_METHOD_ERROR(C) ((C) & BRPC_ERR_METHOD_ERROR)

#define BRPC_ERR_METHOD_TO_CODE(E) (BRPC_ERR_METHOD_ERROR | ((E) & 0x7f))
#define BRPC_ERR_CODE_TO_METHOD(C) (~BRPC_ERR_METHOD_ERROR & (C))

struct brpc_msg;

int brpc_msg_is_int32(const struct brpc_msg *msg, int index);
int brpc_msg_is_int64(const struct brpc_msg *msg, int index);
int brpc_msg_is_str(const struct brpc_msg *msg, int index);

int32_t brpc_msg_get_int32(const struct brpc_msg *msg, int index, int *error);
int64_t brpc_msg_get_int64(const struct brpc_msg *msg, int index, int *error);
char *brpc_msg_get_str(const struct brpc_msg *msg, int index, int *error);

int brpc_msg_add_int32(struct brpc_msg *msg, int32_t i);
int brpc_msg_add_int64(struct brpc_msg *msg, int64_t l);
int brpc_msg_add_str(struct brpc_msg *msg, const char *s);

/*
 * RPC mapper
 * handles mapping method id (a char) to method definition
 */

struct brpc_connection;

typedef int (*brpc_method_t)(struct brpc_connection *conn, const struct brpc_msg *params, struct brpc_msg *result);

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


/* method call/notify */

int brpc_notify(struct brpc_connection *conn, uint8_t method, const char *fmt, ...);

typedef void (*brpc_cb_t)(const struct brpc_msg *result, void *user_data);

int brpc_call(struct brpc_connection *conn, uint8_t method, brpc_cb_t cb, void *user_data, const char *fmt, ...);

#endif
