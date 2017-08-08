#include <brpc.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"
#include "hash.h"

/*

     Offset +-----------------------+
          0 | message size          |
            +                       +
          1 |                       |
            +-----------------------+
          2 | message type          |
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
            +-----------------------+   * arg offset from the beginning of the message, on 14 bits
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

#define MSIZE_OFF                    0
#define MSIZE_SIZE                   2
#define MSIZE_ADDR(B)                ((uint16_t *)((B) + MSIZE_OFF))
#define MTYPE_OFF                    (MSIZE_OFF + MSIZE_SIZE)
#define MTYPE_SIZE                   1
#define MTYPE_ADDR(B)                ((char *)((B) + MTYPE_OFF))
#define METHOD_OFF                   (MTYPE_OFF + MTYPE_SIZE)
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

#define MESSAGE_TYPE_REQUEST   ((uint8_t)1)
#define MESSAGE_TYPE_RESPONSE  ((uint8_t)2)
#define MESSAGE_TYPE_ERROR     ((uint8_t)3)

typedef char brpc_data_t;

struct brpc_msg {
	struct buffer buffer;
};

static brpc_data_t *brpc_msg_get_data(const struct brpc_msg *msg)
{
	return buffer_data(&msg->buffer);
}

static uint16_t brpc_msg_get_size(const struct brpc_msg *msg)
{
	return *MSIZE_ADDR(brpc_msg_get_data(msg));
}

static void brpc_msg_set_size(struct brpc_msg *msg, uint16_t size)
{
	*MSIZE_ADDR(brpc_msg_get_data(msg)) = size;
}

static uint8_t brpc_msg_get_type(const struct brpc_msg *msg)
{
	return *MTYPE_ADDR(brpc_msg_get_data(msg));
}

static void brpc_msg_set_type(struct brpc_msg *msg, uint8_t type)
{
	*MTYPE_ADDR(brpc_msg_get_data(msg)) = type;
}

static uint8_t brpc_msg_get_method(const struct brpc_msg *msg)
{
	return *METHOD_ADDR(brpc_msg_get_data(msg));
}

static void brpc_msg_set_method(struct brpc_msg *msg, uint8_t method)
{
	*METHOD_ADDR(brpc_msg_get_data(msg)) = method;
}

static uint32_t brpc_msg_get_id(const struct brpc_msg *msg)
{
	return *ID_ADDR(brpc_msg_get_data(msg));
}

static void brpc_msg_set_id(struct brpc_msg *msg, uint32_t id)
{
	*ID_ADDR(brpc_msg_get_data(msg)) = id;
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

static size_t approximate_size(const char *fmt)
{
	const char *p;
	size_t size = ARG_OFF;

	if (fmt != NULL) {
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
	}

	return size;
}

static char *add_arg(struct buffer *b, int arg_count, uint8_t arg_type, size_t arg_size, size_t arg_alignment)
{
	char *arg_p;
	size_t gap, arg_offset;

	gap = align_gap(buffer_size(b), arg_alignment);
	if (gap != 0)
		buffer_fill(b, 0, gap);
	buffer_make_room(b, arg_size);

	arg_offset = buffer_size(b);
	if (arg_offset >= ATENTRY_MAX_OFFSET)
		return NULL;

	arg_p = buffer_end(b);
	buffer_increment(b, arg_size);

	*MSIZE_ADDR(buffer_data(b)) = buffer_size(b);
	*ATENTRY_ADDR(buffer_data(b), arg_count) = ATENTRY(arg_type, arg_offset);

	return arg_p;
}

static struct brpc_msg *brpc_msg_vnew(uint8_t msg_type, const char *fmt, va_list args)
{
	struct brpc_msg *msg;
	const char *p, *s;
	int arg_count;
	char *arg_p;
	size_t asize, l;

	asize = approximate_size(fmt);
	if (asize == 0)
		return NULL;
	msg = malloc(sizeof(struct brpc_msg));
	buffer_init(&msg->buffer, asize);
	buffer_fill(&msg->buffer, 0, ARG_OFF);

	brpc_msg_set_type(msg, msg_type);

	if (fmt != NULL) {
		for(p = fmt, arg_count = 0; *p; p++, arg_count++) {
			if (arg_count >= ATABLE_MAX_ENTRIES)
				goto ret_error;
			switch(*p) {
			case 'i':
				arg_p = add_arg(&msg->buffer, arg_count, ARG_INT32, ARG_INT32_SIZE, ARG_INT32_ALIGN);
				if (arg_p == NULL)
					goto ret_error;
				*(int32_t *)arg_p = va_arg(args, int32_t);
				break;
			case 'l':
				arg_p = add_arg(&msg->buffer, arg_count, ARG_INT64, ARG_INT64_SIZE, ARG_INT64_ALIGN);
				if (arg_p == NULL)
					goto ret_error;
				*(int64_t *)arg_p = va_arg(args, int64_t);
				break;
			case 's':
				s = va_arg(args, const char *);
				l = strlen(s) + 1;
				arg_p = add_arg(&msg->buffer, arg_count, ARG_STR, l, ARG_STR_ALIGN);
				if (arg_p == NULL)
					goto ret_error;
				strncpy(arg_p, s, l);
				break;
			default:
				goto ret_error;
			}
		}
	}

	return msg;

ret_error:
	buffer_destroy(&msg->buffer, 1);
	free(msg);
	return NULL;
}

static struct brpc_msg *brpc_msg_new(uint8_t msg_type, const char *fmt, ...)
{
	va_list args;
	struct brpc_msg *ret;

	va_start(args, fmt);
	ret = brpc_msg_vnew(msg_type, fmt, args);
	va_end(args);

	return ret;
}

static struct brpc_msg *brpc_msg_new_with_size(size_t initial_size)
{
	struct brpc_msg *msg;

	msg = malloc(sizeof(struct brpc_msg));
	buffer_init(&msg->buffer, initial_size);

	return msg;
}

static void brpc_msg_free(struct brpc_msg *msg)
{
	buffer_destroy(&msg->buffer, 1);
	free(msg);
}

static int check_arg(brpc_data_t *b, int index, uint8_t arg_type, int *error)
{
	if (index >= ATABLE_MAX_ENTRIES) {
		if (error != NULL)
			*error = BRPC_ERR_ARGC_OUT_OF_BOUND;
		return 1;
	}

	if (ATENTRY_GET_TYPE(b, index) != arg_type) {
		if (error != NULL)
			*error = BRPC_ERR_ARGUMENT_TYPE_MISMATCH;
		return 1;
	}

	return 0;
}

int brpc_msg_is_int32(const struct brpc_msg *msg, int index)
{
	brpc_data_t *b = brpc_msg_get_data(msg);

	return check_arg(b, index, ARG_INT32, NULL) == 0;
}

int brpc_msg_is_int64(const struct brpc_msg *msg, int index)
{
	brpc_data_t *b = brpc_msg_get_data(msg);

	return check_arg(b, index, ARG_INT64, NULL) == 0;
}

int brpc_msg_is_str(const struct brpc_msg *msg, int index)
{
	brpc_data_t *b = brpc_msg_get_data(msg);

	return check_arg(b, index, ARG_STR, NULL) == 0;
}

int32_t brpc_msg_get_int32(const struct brpc_msg *msg, int index, int *error)
{
	brpc_data_t *b = brpc_msg_get_data(msg);

	if (check_arg(b, index, ARG_INT32, error))
		return -1;

	return *(int32_t *)(b + ATENTRY_GET_OFFSET(b, index));
}

int64_t brpc_msg_get_int64(const struct brpc_msg *msg, int index, int *error)
{
	brpc_data_t *b = brpc_msg_get_data(msg);

	if (check_arg(b, index, ARG_INT64, error))
		return -1;

	return *(int64_t *)(b + ATENTRY_GET_OFFSET(b, index));
}

char *brpc_msg_get_str(const struct brpc_msg *msg, int index, int *error)
{
	brpc_data_t *b = brpc_msg_get_data(msg);

	if (check_arg(b, index, ARG_STR, error))
		return "";

	return (char *)(b + ATENTRY_GET_OFFSET(b, index));
}

static int count_args(brpc_data_t *b)
{
	int i;

	for (i = 0; i < ATABLE_MAX_ENTRIES; i++)
		if (ATENTRY_GET_TYPE(b, i) == ARG_NONE)
			break;

	return i;
}

int brpc_msg_add_int32(struct brpc_msg *msg, int32_t i)
{
	int arg_count = count_args(brpc_msg_get_data(msg));
	char *arg_p = add_arg(&msg->buffer, arg_count, ARG_INT32, ARG_INT32_SIZE, ARG_INT32_ALIGN);

	if (arg_p == NULL)
		return BRPC_ERR_TOO_MANY_ARGUMENTS;

	*(int32_t *)arg_p = i;

	return BRPC_OK;
}

int brpc_msg_add_int64(struct brpc_msg *msg, int64_t l)
{
	int arg_count = count_args(brpc_msg_get_data(msg));
	char *arg_p = add_arg(&msg->buffer, arg_count, ARG_INT64, ARG_INT64_SIZE, ARG_INT64_ALIGN);

	if (arg_p == NULL)
		return BRPC_ERR_TOO_MANY_ARGUMENTS;

	*(int64_t *)arg_p = l;

	return BRPC_OK;
}

int brpc_msg_add_str(struct brpc_msg *msg, const char *s)
{
	int arg_count = count_args(brpc_msg_get_data(msg));
	size_t l = strlen(s) + 1;
	char *arg_p = add_arg(&msg->buffer, arg_count, ARG_STR, l, ARG_STR_ALIGN);

	if (arg_p == NULL)
		return BRPC_ERR_TOO_MANY_ARGUMENTS;

	strncpy(arg_p, s, l);

	return BRPC_OK;
}

#ifdef DEBUG
static const char *brpc_msg_type_str(uint8_t type)
{
	switch(type) {
#define M(T) case T: return #T
		M(MESSAGE_TYPE_REQUEST);
		M(MESSAGE_TYPE_RESPONSE);
		M(MESSAGE_TYPE_ERROR);
	}

	return "unknown";
}

void brpc_msg_print(struct brpc_msg *msg)
{
	int argc = 0;
	brpc_data_t *b = brpc_msg_get_data(msg);

	fprintf(stderr, "Buffer: type %s size %d method %d id %d\n",
		brpc_msg_type_str(brpc_msg_get_type(msg)),
		brpc_msg_get_size(msg),
		brpc_msg_get_method(msg),
		brpc_msg_get_id(msg));
	while (argc < ATABLE_MAX_ENTRIES && ATENTRY_GET_TYPE(b, argc) != ARG_NONE) {
		switch(ATENTRY_GET_TYPE(b, argc)) {
		case ARG_INT32:
			fprintf(stderr, "  [%d] %d\n", argc, brpc_msg_get_int32(msg, argc, NULL));
			break;
		case ARG_INT64:
			fprintf(stderr, "  [%d] 0x%lx\n", argc, brpc_msg_get_int64(msg, argc, NULL));
			break;
		case ARG_STR:
			fprintf(stderr, "  [%d] \"%s\"\n", argc, brpc_msg_get_str(msg, argc, NULL));
			break;
		}

		argc++;
	}
}
#endif

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

#ifdef HAVE_PTHREAD
#include <pthread.h>

typedef pthread_mutex_t brpc_lock_t;

static void lock_acquire(brpc_lock_t *lock)
{
	if (pthread_mutex_lock((pthread_mutex_t *)lock))
		perror("pthread_mutex_lock");
}

static void lock_release(brpc_lock_t *lock)
{
	if (pthread_mutex_unlock((pthread_mutex_t *)lock))
		perror("pthread_mutex_lock");
}

static void lock_init(brpc_lock_t *lock)
{
	if (pthread_mutex_init((pthread_mutex_t *)lock, NULL))
		perror("pthread_mutex_init");
}

static void lock_destroy(brpc_lock_t *lock)
{

	if (pthread_mutex_destroy((pthread_mutex_t *)lock))
		perror("pthread_mutex_destroy");
}
#else
#include <windows.h>

typedef HANDLE brpc_lock_t;

static void lock_acquire(brpc_lock_t *lock)
{
	DWORD dwWaitResult;

	dwWaitResult = WaitForSingleObject(
		*lock,      // handle to mutex
		INFINITE);  // no time-out interval
}

static void lock_release(brpc_lock_t *lock)
{
	ReleaseMutex(*lock);
}

static void lock_init(brpc_lock_t *lock)
{
	*lock = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex
}

static void lock_destroy(brpc_lock_t *lock)
{
	CloseHandle(*lock);
}
#endif

struct brpc_connection {
	struct brpc_mapper *mapper;
	uint32_t current_id;
	struct hash_table *response_table;
	brpc_lock_t id_lock;
	brpc_read_cb_t read_cb;
	void *read_cb_data;
	brpc_write_cb_t write_cb;
	void *write_cb_data;
	brpc_lock_t write_lock;
	void *connection_data;
	brpc_error_handler_t error_handler;
};

struct rpc_callback_entry {
	brpc_cb_t cb;
	void *user_data;
};

struct brpc_connection *brpc_connection_new(struct brpc_mapper *mapper, void *connection_data)
{
	struct brpc_connection *conn = malloc(sizeof(struct brpc_connection));

	conn->mapper = mapper;

	conn->current_id = 1L;
	conn->response_table = hash_table_new(hash_int, equal_int, NULL, (destroy_cb_t)free);
	lock_init(&conn->id_lock);

	conn->read_cb = NULL;
	conn->read_cb_data = NULL;
	conn->write_cb = NULL;
	conn->write_cb_data = NULL;
	lock_init(&conn->write_lock);

	conn->connection_data = connection_data;
	conn->error_handler = NULL;

	return conn;
}

void brpc_connection_free(struct brpc_connection *conn)
{
	hash_table_free(conn->response_table);
	lock_destroy(&conn->id_lock);
	lock_destroy(&conn->write_lock);
	free(conn);
}

void *brpc_connection_get_data(struct brpc_connection *conn)
{
	return conn->connection_data;
}

static struct brpc_mapper *brpc_connection_get_mapper(struct brpc_connection *conn)
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

void brpc_connection_set_error_handler(struct brpc_connection *conn, brpc_error_handler_t error_handler)
{
	conn->error_handler = error_handler;
}

static brpc_error_handler_t brpc_connection_get_error_handler(struct brpc_connection *conn)
{
	return conn->error_handler;
}

static uint32_t brpc_connection_register_callback(struct brpc_connection *conn, brpc_cb_t cb, void *user_data)
{
	uint32_t id;
	struct rpc_callback_entry *entry;

	entry = malloc(sizeof(struct rpc_callback_entry));
	entry->cb = cb;
	entry->user_data = user_data;

	lock_acquire(&conn->id_lock);

	id = conn->current_id;

	conn->current_id++;

	/* insertion should always work??? */
	if (!hash_table_insert(conn->response_table, H_INT_TO_POINTER(id), entry))
		free(entry);

	lock_release(&conn->id_lock);

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
	lock_acquire(&conn->id_lock);

	entry = hash_table_search(conn->response_table, H_INT_TO_POINTER(id));

	if (entry != NULL) {
		cb = entry->cb;
		*p_user_data = entry->user_data;
		hash_table_remove(conn->response_table, H_INT_TO_POINTER(id));
	}

	lock_release(&conn->id_lock);

	return cb;
}

static int brpc_connection_send_and_free(struct brpc_connection *conn, struct brpc_msg *msg)
{
	int ret = BRPC_OK;

	assert(conn->write_cb != NULL);

	lock_acquire(&conn->write_lock);

	if ((*conn->write_cb)(brpc_msg_get_data(msg), brpc_msg_get_size(msg), conn->write_cb_data) < 0)
		ret = BRPC_ERR_IO_ERROR;

	lock_release(&conn->write_lock);

	brpc_msg_free(msg);

	return ret;
}

static int brpc_connection_receive(struct brpc_connection *conn, struct brpc_msg **p_msg)
{
	ssize_t n_read, size_to_read;
	uint16_t b_size;
	struct brpc_msg *msg;

	assert(conn->read_cb != NULL);

	n_read = (*conn->read_cb)(&b_size, MSIZE_SIZE, conn->read_cb_data);
	if (n_read == 0)
		return BRPC_EOF;
	else if (n_read < MSIZE_SIZE)
		return BRPC_ERR_IO_ERROR;

	msg = brpc_msg_new_with_size(b_size);

	size_to_read = b_size - MSIZE_SIZE;
	n_read = (*conn->read_cb)(brpc_msg_get_data(msg) + MSIZE_SIZE, size_to_read, conn->read_cb_data);
	if (n_read == 0)
		return BRPC_EOF;
	else if (n_read < size_to_read)
		return BRPC_ERR_IO_ERROR;

	brpc_msg_set_size(msg, b_size);
	*p_msg = msg;

	return BRPC_OK;
}

static int brpc_connection_process_request(struct brpc_connection *conn, struct brpc_msg *params)
{
	brpc_method_t method_cb = NULL;
	struct brpc_msg *result;
	struct brpc_mapper *mapper = brpc_connection_get_mapper(conn);
	uint32_t id = brpc_msg_get_id(params);
	int mth_ret;

	if (mapper != NULL)
		method_cb = brpc_mapper_get(mapper, brpc_msg_get_method(params));

	if (method_cb == NULL) {
		result = brpc_msg_new(MESSAGE_TYPE_ERROR, "is", BRPC_ERR_METHOD_NOT_FOUND, "method was not found");

		return brpc_connection_send_and_free(conn, result);
	}

	result = brpc_msg_new(MESSAGE_TYPE_RESPONSE, NULL);
	mth_ret = (*method_cb)(conn, params, result);

	if (mth_ret) {
		brpc_msg_free(result);

		result = brpc_msg_new(MESSAGE_TYPE_ERROR, "is", BRPC_ERR_METHOD_TO_CODE(mth_ret), "method returned an error");
		brpc_msg_set_id(result, id);

		return brpc_connection_send_and_free(conn, result);
	}
	else if (id != 0) {
		brpc_msg_set_id(result, id);

		return brpc_connection_send_and_free(conn, result);
	}

	return BRPC_OK;
}

static int brpc_connection_process_result(struct brpc_connection *conn, struct brpc_msg *result)
{
	brpc_cb_t cb;
	void *user_data;

	cb = brpc_connection_find_callback(conn, brpc_msg_get_id(result), &user_data);

	if (cb == NULL)
		return BRPC_ERR_INVALID_RESPONSE_ID;

	(*cb)(result, user_data);

	return BRPC_OK;
}

static int brpc_connection_process_error(struct brpc_connection *conn, struct brpc_msg *b)
{
	uint32_t id = brpc_msg_get_id(b);
	int error = BRPC_OK;
	uint32_t code;
	char *message;
	brpc_error_handler_t error_handler = brpc_connection_get_error_handler(conn);

	code = brpc_msg_get_int32(b, 0, &error);
	if (error)
		return BRPC_ERR_INVALID_ERROR_MESSAGE;

	message = brpc_msg_get_str(b, 1, &error);
	if (error)
		return BRPC_ERR_INVALID_ERROR_MESSAGE;

	if (error_handler != NULL)
		(*error_handler)(conn, id, code, message);

	return BRPC_OK;
}

int brpc_connection_process(struct brpc_connection *conn)
{
	int ret;
	struct brpc_msg *b;

	if ((ret = brpc_connection_receive(conn, &b)))
		return ret;

#ifdef DEBUG
	brpc_msg_print(b);
#endif

	switch(brpc_msg_get_type(b)) {
	case MESSAGE_TYPE_REQUEST:
		ret = brpc_connection_process_request(conn, b);
		break;
	case MESSAGE_TYPE_RESPONSE:
		ret = brpc_connection_process_result(conn, b);
		break;
	case MESSAGE_TYPE_ERROR:
		ret = brpc_connection_process_error(conn, b);
		break;
	default:
		ret = BRPC_ERR_INVALID_MESSAGE_TYPE;
		break;
	}

	brpc_msg_free(b);

	return ret;
}

static int brpc_vcall(struct brpc_connection *conn, uint8_t method, brpc_cb_t cb, void *user_data, const char *fmt, va_list args)
{
	uint32_t id = 0;
	struct brpc_msg *params = brpc_msg_vnew(MESSAGE_TYPE_REQUEST, fmt, args);
	int ret;

	if (cb != NULL)
		id = brpc_connection_register_callback(conn, cb, user_data);

	brpc_msg_set_method(params, method);
	brpc_msg_set_id(params, id);

#ifdef DEBUG
	brpc_msg_print(params);
#endif

	return brpc_connection_send_and_free(conn, params);
}

int brpc_notify(struct brpc_connection *conn, uint8_t method, const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = brpc_vcall(conn, method, NULL, NULL, fmt, args);
	va_end(args);

	return ret;
}

int brpc_call(struct brpc_connection *conn, uint8_t method, brpc_cb_t cb, void *user_data, const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = brpc_vcall(conn, method, cb, user_data, fmt, args);
	va_end(args);

	return ret;
}
