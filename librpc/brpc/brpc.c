/*
  compile with:
  gcc -g -Iinclude/ -DDO_TEST_DEBUG_MAIN -o brpc buffer.c hash.c brpc.c
*/

#include <brpc.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

#include "buffer.h"
#include "hash.h"

/*

     Offset +-----------------------+
          0 | buffer size           |
            +                       +
          1 |                       |
            +-----------------------+
          2 | buffer type           |
            +-----------------------+
          3 | method                |
            +-----------------------+
          4 | id                    |
            +                       +
          5 |                       |
            +                       +
          6 |                       |
            +                       +
          7 |                       |
            +-----------------------+
          8 | arg table entry 0     | arg table entries:
            +                       + an arg table entry contains:
          9 |                       |   * arg type, on 2 bits
            +-----------------------+   * arg offset from the beginning of the buffer, on 14 bits
          A | arg table entry 1     | N is the maximum number of args (currently 16)
            +                       +
          B |                       |
            +-----------------------+
            |                       |
            .                       .
            .                       .
            .                       .
            |                       |
            +-----------------------+
            | arg table entry N-1   |
            +                       +
            |                       |
            +-----------------------+
      8+2*N | arg 0            0xce | arguments:
            + (here an int32)       + an argument can be:
            |                  0xfa |   * int32: 4 bytes long integer
            +                       +   * int64: 8 bytes long integer
            |                  0xca |   * str: (strlen+1) long string
            +                       + int arguments are aligned on their size
            |                  0xca | str which are aligned on 4 (on 32 bits arch) or 8 (on 64 bits arch)
            +-----------------------+
    8+2*N+4 | arg 1             't' |
            + (here a string)       +
    8+2*N+5 |                   'o' |
            +                       +
    8+2*N+6 |                   't' |
            +                       +
    8+2*N+7 |                   'o' |
            +                       +
    8+2*N+8 |                  '\0' | strings are null-terminated
            +                       +
    8+2*N+9 |                   PAD | and padded to align next entry
            +                       +
    8+2*N+A |                   PAD |
            +                       +
    8+2*N+B |                   PAD |
            +-----------------------+
    8+2*N+C | arg 2                 |
            .                       .
            .                       .
            .                       .

*/

#define BSIZE_OFF                    0
#define BSIZE_SIZE                   2
#define BSIZE_ADDR(B)                ((uint16_t *)((B) + BSIZE_OFF))
#define BTYPE_OFF                    (BSIZE_OFF + BSIZE_SIZE)
#define BTYPE_SIZE                   1
#define BTYPE_ADDR(B)                ((char *)((B) + BTYPE_OFF))
#define METHOD_OFF                   (BTYPE_OFF + BTYPE_SIZE)
#define METHOD_SIZE                  1
#define METHOD_ADDR(B)               ((char *)((B) + METHOD_OFF))
#define ID_OFF                       (METHOD_OFF + METHOD_SIZE)
#define ID_SIZE                      4
#define ID_ADDR(B)                   ((uint32_t *)((B) + ID_OFF))
#define ATABLE_OFF                   (ID_OFF + ID_SIZE)
#define ATABLE_MAX_ENTRIES           16
#define ATENTRY_SIZE                 2
#define ATABLE_SIZE                  (ATABLE_MAX_ENTRIES * ATENTRY_SIZE)
#define ATABLE_ADDR(B)               ((uint16_t *)((B) + ATABLE_OFF))
#define ATENTRY_ADDR(B, I)           (ATABLE_ADDR(B) + (I))
#define ARG_OFF                      (ATABLE_OFF + ATABLE_SIZE)

#define ARG_NONE                     0
#define ARG_NONE_SIZE                0
#define ARG_NONE_ALIGN               0
#define ARG_INT32                    1
#define ARG_INT32_SIZE               4
#define ARG_INT32_ALIGN              4
#define ARG_INT64                    2
#define ARG_INT64_SIZE               8
#define ARG_INT64_ALIGN              8
#define ARG_STR                      3
/* must #define it to 4 on 32 bits arch */
#define ARG_STR_ALIGN                8

#define ATENTRY_BITS                 (ATENTRY_SIZE * 8)
#define ATENTRY_OFFSET_SHIFT         0
#define ATENTRY_OFFSET_BITS          14
#define ATENTRY_TYPE_SHIFT           ATENTRY_OFFSET_BITS
#define ATENTRY_TYPE_BITS            (ATENTRY_BITS - ATENTRY_OFFSET_BITS)
#define MASK(B)                      (~(~0 << (B)))
#define GETBITS(V, S, B)             (((V) >> S) & MASK(B))
#define ATENTRY(TYPE, OFFSET)        ((((OFFSET) & MASK(ATENTRY_OFFSET_BITS)) << ATENTRY_OFFSET_SHIFT) | (((TYPE) & MASK(ATENTRY_TYPE_BITS)) << ATENTRY_TYPE_SHIFT))
#define ATENTRY_GET_TYPE(B, I)       GETBITS(*ATENTRY_ADDR(B, I), ATENTRY_TYPE_SHIFT, ATENTRY_TYPE_BITS)
#define ATENTRY_GET_OFFSET(B, I)     GETBITS(*ATENTRY_ADDR(B, I), ATENTRY_OFFSET_SHIFT, ATENTRY_OFFSET_BITS)
#define ATENTRY_MAX_OFFSET           (1 << ATENTRY_OFFSET_BITS)

enum brpc_buffer_type {
	REQUEST = 1,
	NOTIFY,
	RESPONSE,
	ERROR,
};

static uint16_t brpc_buffer_get_size(const brpc_buffer_t *b)
{
	return *BSIZE_ADDR(b);
}

static void brpc_buffer_set_size(brpc_buffer_t *b, uint16_t size)
{
	*BSIZE_ADDR(b) = size;
}

static uint8_t brpc_buffer_get_type(const brpc_buffer_t *b)
{
	return *BTYPE_ADDR(b);
}

static void brpc_buffer_set_type(brpc_buffer_t *b, uint8_t type)
{
	*BTYPE_ADDR(b) = type;
}

static uint8_t brpc_buffer_get_method(const brpc_buffer_t *b)
{
	return *METHOD_ADDR(b);
}

static void brpc_buffer_set_method(brpc_buffer_t *b, uint8_t method)
{
	*METHOD_ADDR(b) = method;
}

static uint32_t brpc_buffer_get_id(const brpc_buffer_t *b)
{
	return *ID_ADDR(b);
}

static void brpc_buffer_set_id(brpc_buffer_t *b, uint32_t id)
{
	*ID_ADDR(b) = id;
}

static size_t align_gap(size_t offset, size_t alignment)
{
	return (offset % alignment == 0) ? 0 : alignment - offset % alignment;
}

static size_t align_and_increment(size_t offset, size_t alignment, size_t increment)
{
	return offset + align_gap(offset, increment) + increment;
}

#define APPROXIMATE_STRLEN 64

static size_t brpc_buffer_approximate_size(const char *fmt)
{
	const char *p;
	size_t size = ARG_OFF;

 	for(p = fmt; *p; p++) {
		switch(*p) {
		case 'i':
			size = align_and_increment(size, ARG_INT32_ALIGN, ARG_INT32_SIZE);
			break;
		case 'l':
			size = align_and_increment(size, ARG_INT64_ALIGN, ARG_INT64_SIZE);
			break;
		case 's':
			size = align_and_increment(size, ARG_STR_ALIGN, APPROXIMATE_STRLEN);
			break;
		default:
			return 0;
		}
	}

	return size;
}

static char *brpc_buffer_add_arg(struct buffer *b, int arg_count, uint8_t arg_type, size_t arg_size, size_t arg_alignment)
{
	char *arg_p;
	size_t arg_offset;

	buffer_fill(b, 0, align_gap(buffer_size(b), arg_alignment));
	buffer_make_room(b, arg_size);

	arg_offset = buffer_size(b);
	if (arg_offset >= ATENTRY_MAX_OFFSET)
		return NULL;

	arg_p = buffer_end(b);
	buffer_increment(b, arg_size);

	*ATENTRY_ADDR(buffer_data(b), arg_count) = ATENTRY(arg_type, arg_offset);

	return arg_p;
}

brpc_buffer_t *brpc_buffer_new(const char *fmt, ...)
{
	va_list args;
	struct buffer *b;
	brpc_buffer_t *ret;
	const char *p, *s;
	int arg_count;
	char *arg_p;
	size_t approximate_size, l;

	approximate_size = brpc_buffer_approximate_size(fmt);
	if (approximate_size == 0)
		return NULL;
	b = buffer_new(approximate_size);
	buffer_fill(b, 0, ARG_OFF);

	va_start(args, fmt);
	for(p = fmt, arg_count = 0; *p; p++, arg_count++) {
		switch(*p) {
		case 'i':
			arg_p = brpc_buffer_add_arg(b, arg_count, ARG_INT32, ARG_INT32_SIZE, ARG_INT32_ALIGN);
			if (arg_p == NULL)
				goto ret_error;
			*(int32_t *)arg_p = va_arg(args, int32_t);
			break;
		case 'l':
			arg_p = brpc_buffer_add_arg(b, arg_count, ARG_INT64, ARG_INT64_SIZE, ARG_INT64_ALIGN);
			if (arg_p == NULL)
				goto ret_error;
			*(int64_t *)arg_p = va_arg(args, int64_t);
			break;
		case 's':
			s = va_arg(args, const char *);
			l = strlen(s) + 1;
			arg_p = brpc_buffer_add_arg(b, arg_count, ARG_STR, l, ARG_STR_ALIGN);
			if (arg_p == NULL)
				goto ret_error;
			strncpy(arg_p, s, l);
			break;
		default:
			goto ret_error;
		}
	}
	va_end(args);

	brpc_buffer_set_size(buffer_data(b), buffer_size(b));
	ret = buffer_data(b);
	buffer_free(b, 0);

	return ret;

ret_error:
	va_end(args);
	buffer_free(b, 1);
	return NULL;
}

static int brpc_buffer_check_arg(const brpc_buffer_t *b, uint8_t index, uint8_t arg_type, int *error)
{
	if (index >= ATABLE_MAX_ENTRIES) {
		if (error != NULL)
			*error = BRPC_ERR_ARGC_OUT_OF_BOUND;
		return 1;
	}

	if (ATENTRY_GET_TYPE(b, index) != arg_type) {
		if (error != NULL)
			*error = BRPC_ERR_INVALID_ARGUMENT_TYPE;
		return 1;
	}

	return 0;
}


int32_t brpc_buffer_get_int32(const brpc_buffer_t *b, uint8_t index, int *error)
{
	if (brpc_buffer_check_arg(b, index, ARG_INT32, error))
		return -1;

	return *(int32_t *)(b + ATENTRY_GET_OFFSET(b, index));
}

int64_t brpc_buffer_get_int64(const brpc_buffer_t *b, uint8_t index, int *error)
{
	if (brpc_buffer_check_arg(b, index, ARG_INT64, error))
		return -1;

	return *(int64_t *)(b + ATENTRY_GET_OFFSET(b, index));
}

char *brpc_buffer_get_str(const brpc_buffer_t *b, uint8_t index, int *error)
{
	if (brpc_buffer_check_arg(b, index, ARG_STR, error))
		return "";

	return (char *)(b + ATENTRY_GET_OFFSET(b, index));
}

#define MAPPER_MAX_METHODS 64

struct brpc_mapper {
	brpc_method_t method_table[MAPPER_MAX_METHODS];
};

struct brpc_mapper *brpc_mapper_new(void)
{
	struct brpc_mapper *m = calloc(1, sizeof(struct brpc_mapper));

	return m;
}

int brpc_mapper_add(struct brpc_mapper *mapper, uint8_t method, brpc_method_t method_cb)
{
	if (method >= MAPPER_MAX_METHODS)
		return 1;

	if (mapper->method_table[method] != NULL)
		return 1;

	mapper->method_table[method] = method_cb;

	return 0;
}

static brpc_method_t brpc_mapper_get(struct brpc_mapper *mapper, uint8_t method)
{
	if (method >= MAPPER_MAX_METHODS)
		return NULL;

	return mapper->method_table[method];
}

struct brpc_connection {
	struct brpc_mapper *mapper;
	uint32_t current_id;
	struct hash_table *response_table;
	brpc_read_cb_t read_cb;
	void *read_cb_data;
	brpc_write_cb_t write_cb;
	void *write_cb_data;
	void *connection_data;
	/* brpc_error_handler_t error_handler; */
#ifdef HAVE_PTHREAD
	pthread_mutex_t connection_mutex;
#endif
};

struct rpc_callback_entry {
	brpc_cb_t cb;
	void *user_data;
};

#ifdef HAVE_PTHREAD
static void brpc_connection_lock(struct brpc_connection *conn)
{
	if (pthread_mutex_lock(&conn->connection_mutex))
		perror("pthread_mutex_lock");
}

static void brpc_connection_unlock(struct brpc_connection *conn)
{
	if (pthread_mutex_unlock(&conn->connection_mutex))
		perror("pthread_mutex_unlock");
}

static void brpc_connection_lock_init(struct brpc_connection *conn)
{
	pthread_mutex_init(&conn->connection_mutex, NULL);
}

static void brpc_connection_lock_destroy(struct brpc_connection *conn)
{
	pthread_mutex_destroy(&conn->connection_mutex);
}
#else
static void brpc_connection_lock(struct brpc_connection *conn)
{
}

static void brpc_connection_unlock(struct brpc_connection *conn)
{
}

static void brpc_connection_lock_init(struct brpc_connection *conn)
{
}

static void brpc_connection_lock_destroy(struct brpc_connection *conn)
{
}
#endif

struct brpc_connection *brpc_connection_new(struct brpc_mapper *mapper, void *connection_data)
{
	struct brpc_connection *conn = malloc(sizeof(struct brpc_connection));

	conn->mapper = mapper;

	conn->current_id = 1L;
	conn->response_table = hash_table_new(hash_int, equal_int, NULL, (destroy_cb_t)free);

	conn->read_cb = NULL;
	conn->read_cb_data = NULL;
	conn->write_cb = NULL;
	conn->write_cb_data = NULL;

	conn->connection_data = connection_data;
	/* conn->error_handler = NULL; */

	brpc_connection_lock_init(conn);

	return conn;
}

void brpc_connection_free(struct brpc_connection *conn)
{
	hash_table_free(conn->response_table);
	brpc_connection_lock_destroy(conn);
	free(conn);
}

void *brpc_connection_get_data(struct brpc_connection *conn)
{
	return conn->connection_data;
}

static struct brpc_mapper *connection_get_mapper(struct brpc_connection *conn)
{
	return conn->mapper;
}

void brpc_connection_set_read_cb(struct brpc_connection *conn, brpc_read_cb_t read_cb, void *data)
{
	conn->read_cb = read_cb;
	conn->read_cb_data = data;
}

void brpc_connection_set_write_cb(struct brpc_connection *conn, brpc_write_cb_t write_cb, void *data)
{
	conn->write_cb = write_cb;
	conn->write_cb_data = data;
}

#if 0
void brpc_connection_set_error_handler(struct brpc_connection *conn, brpc_error_handler_t error_handler)
{
	conn->error_handler = error_handler;
}

brpc_error_handler_t brpc_connection_get_error_handler(struct brpc_connection *conn)
{
	return conn->error_handler;
}
#endif

static uint32_t brpc_connection_register_callback(struct brpc_connection *conn, brpc_cb_t cb, void *user_data)
{
	uint32_t id;
	struct rpc_callback_entry *entry;

	entry = malloc(sizeof(struct rpc_callback_entry));
	entry->cb = cb;
	entry->user_data = user_data;

	brpc_connection_lock(conn);

	id = conn->current_id;

	conn->current_id++;

	/* insertion should always work??? */
	if (!hash_table_insert(conn->response_table, H_INT_TO_POINTER(id), entry))
		free(entry);

	brpc_connection_unlock(conn);

	return id;
}

static brpc_cb_t brpc_connection_find_callback(struct brpc_connection *conn, uint32_t id, void **p_user_data)
{
	struct rpc_callback_entry *entry;
	brpc_cb_t cb = NULL;

	/*
	   connection must be locked before searching for the callback because hash_table_search may
	   walk through a hash table that is being modified at the same time by hash_table_insert;
	   hash_table_insert may realloc the hash table, hence corrupting the memory accessed by hash_table_search
	*/
	brpc_connection_lock(conn);

	entry = hash_table_search(conn->response_table, H_INT_TO_POINTER(id));

	if (entry != NULL) {
		cb = entry->cb;
		*p_user_data = entry->user_data;
		hash_table_remove(conn->response_table, H_INT_TO_POINTER(id));
	}

	brpc_connection_unlock(conn);

	return cb;
}

static int brpc_connection_send(struct brpc_connection *conn, const brpc_buffer_t *b)
{
	int ret = BRPC_OK;

	assert(conn->write_cb != NULL);

	brpc_connection_lock(conn);
	if ((*conn->write_cb)(b, brpc_buffer_get_size(b), conn->write_cb_data) < 0)
		ret = BRPC_ERR_INTERNAL_ERROR;
	brpc_connection_unlock(conn);

	return ret;
}

static int brpc_connection_receive(struct brpc_connection *conn, brpc_buffer_t **p_b)
{
	ssize_t n_read;
	uint16_t b_size;
	brpc_buffer_t *b;

	assert(conn->read_cb != NULL);

	n_read = (*conn->read_cb)(&b_size, BSIZE_SIZE, conn->read_cb_data);
	if (n_read == 0)
		return BRPC_EOF;
	else if (n_read < BSIZE_SIZE)
		return BRPC_ERR_INTERNAL_ERROR;

	b = malloc((size_t)b_size);
	n_read = (*conn->read_cb)(b + BSIZE_SIZE, (size_t)(b_size - BSIZE_SIZE), conn->read_cb_data);
	if (n_read == 0)
		return BRPC_EOF;
	else if (n_read < b_size - BSIZE_SIZE)
		return BRPC_ERR_INTERNAL_ERROR;

	brpc_buffer_set_size(b, b_size);
	*p_b = b;

	return BRPC_OK;
}

static int brpc_connection_process_request(struct brpc_connection *conn, brpc_buffer_t *b)
{

	/*
	  get method via mapper
	  ret = call method
	  if ret
	     format error buffer
	     send it
	     return
	  if not a notification
	     send result
	 */

	return BRPC_OK;
}

static int brpc_connection_process_result(struct brpc_connection *conn, brpc_buffer_t *b)
{
	return BRPC_OK;
}

static int brpc_connection_process_error(struct brpc_connection *conn, brpc_buffer_t *b)
{
	return BRPC_OK;
}

int brpc_connection_process(struct brpc_connection *conn)
{
	int ret;
	brpc_buffer_t *b;

	if ((ret = brpc_connection_receive(conn, &b)))
		return ret;

	switch(brpc_buffer_get_type(b)) {
	case REQUEST:
		ret = brpc_connection_process_request(conn, b);
		break;
	case RESPONSE:
		ret = brpc_connection_process_result(conn, b);
		break;
	case ERROR:
		ret = brpc_connection_process_error(conn, b);
		break;
	default:
		ret = BRPC_ERR_INVALID_BUFFER_TYPE;
		break;
	}

	free(b);

	return ret;
}

int brpc_notify(struct brpc_connection *conn, uint8_t method, const brpc_buffer_t *params)
{
	return brpc_call(conn, method, params, NULL, NULL);
}

int brpc_call(struct brpc_connection *conn, uint8_t method, const brpc_buffer_t *params, brpc_cb_t cb, void *user_data)
{
	uint32_t id = 0;

	if (cb != NULL)
		id = brpc_connection_register_callback(conn, cb, user_data);

	return brpc_connection_send(conn, params);
}

#ifdef DO_TEST_DEBUG_MAIN

void brpc_buffer_print(brpc_buffer_t *b)
{
	int argc = 0;

	while (argc < ATABLE_MAX_ENTRIES && ATENTRY_GET_TYPE(b, argc) != ARG_NONE) {
		switch(ATENTRY_GET_TYPE(b, argc)) {
		case ARG_INT32:
			printf("[%d] %d\n", argc, brpc_buffer_get_int32(b, argc, NULL));
			break;
		case ARG_INT64:
			printf("[%d] 0x%lx\n", argc, brpc_buffer_get_int64(b, argc, NULL));
			break;
		case ARG_STR:
			printf("[%d] \"%s\"\n", argc, brpc_buffer_get_str(b, argc, NULL));
			break;
		}

		argc++;
	}
}

#include <assert.h>

int main(int argc, char **argv)
{
	brpc_buffer_t *b;

	b = brpc_buffer_new("sissil", "foo", 66, "bar", "joe", 99, 0xdeadbeefcacafaceL);
	assert(b != NULL);
	brpc_buffer_print(b);

	b = brpc_buffer_new("zob", "foo", 66, "bar", "joe", 99);
	assert(b != NULL);
	brpc_buffer_print(b);

	return 0;
}

#endif
