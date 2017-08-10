
Inspired by flatbuffer or flexbuffer:
https://google.github.io/flatbuffers/
https://google.github.io/flatbuffers/flexbuffers.html

Needs for IPC:

* between process that updates rules and process(es) that scan files
* between process that uploads files to server and process(es) that scan files
* etc

Current implementation uses an implementation in C of JSON-RPC (http://www.jsonrpc.org/). However, it was designed for communication between a GUI (implemented in Python or whatever) and the scan daemon. JSON-RPC is too heavy for lightweight IPC and requires an external dependency (jansson), thus augmenting the attack surface.

Practically, only integers (int or enum) and C strings are necessary. Things that are not necessary: arrays, pointers and embedded structs and unions.


A possible implementation:

Example:

struct foo {
       int i;
       char *p;
       enum E e;
       char *q;
};

Converting struct -> buffer:

struct foo s;

                    +-------------+
                    | i           |
                    +-------------+
                    | p o---------------+
                    +-------------+     |
                    | e           |     |
                    +-------------+     |
                    | q o----------------------------+
                    +-------------+     |            |
String pool start   |             |<----+            |
                    |             |                  |
                    |             |                  |
                    | ...         | strlen(s->p) + 1 |
                    |             |                  |
                    |             |                  |
                    | \0          |                  |
                    +-------------+                  |
                    |             |<-----------------+
                    |             |
                    |             |
                    | ...         | strlen(s->q) + 1
                    |             |
                    | \0          |
                    +-------------+

Only one allocation, one one string copy when creating the buffer.
One allocation, no copy when receiving the buffer.

Note: The string pool should be aligned.

## IPC buffer

IPC buffer contains:

buffer length (2 bytes)
ipc_type (1 byte) equivalent to JSON-RPC : 0: Request, 1: Response, 2: Error
method (1 byte)  equivalent to JSON-RPC method
id  (4 bytes) (equivalent to JSON-RPC id)
data_types (8 bytes, 2 bits per type 0: None, 1: Int, 2: Str) => 32 args max
 if on 4 bytes, idem => 16 args max (probably enough)
data:
int (8 bytes)
str (8 bytes)

problem: pointers alignment. If int are 4 bytes, must calculate offsets for each access...

!!! In fact, there are no pointers inside the buffer, only offsets for strings. So no need for 8 bytes alignment.

Possibility: specify types on creation (avoid to store int and pointers and copy int later)
in this case functions to set args are:
int ipc_builder_set_int(struct ipc_builder *b, uint8_t off, int i);

int ipc_builder_set_str(struct ipc_builder *b, uint8_t off, const char *s);

Other solution: make a varargs function:

struct ipc_builder *ipc_builder_new(enum ipc_type ipc_type, uint8_t method, uint32_t id, const char *fmt, ...);

Like:
ipc_builder_new(IPC_REQUEST, IPC_RELOAD_BASES, 42, "sissi", "foo", 66, "bar", "joe", 99);

This function do that:
1) compute buffer length by walking through the arguments
2) allocate buffer
3) fill buffer by walking again through the arguments (make 2 functions taking va_list arguments)

Other possibility: declare the argument types when registering methods, so that the check is done automatically

typedef void (*lrpc_cb_t)(lrpc_buffer_t *buff, void *user_data);

int lrpc_call(struct lrpc_connection *conn, uint8_t method, lrpc_cb_t cb, void *user_data, const char *fmt, ...);

for example:
lrpc_call(MTH_RELOAD_BASES, NULL, NULL, "sissi", "foo", 66, "bar", "joe", 99);

for example:
lrpc_call(MTH_ADD, add_cb, NULL, "ii", 66, 99);

Problem: if arguments types are declared when registering methods, they do not appear in the arguments.
For examples:
lrpc_call(MTH_RELOAD_BASES, NULL, NULL, "foo", 66, "bar", "joe", 99);

lrpc_declare_method(mapper, MTH_ADD, "ii");
Or declare the callback in the same call:
lrpc_declare_method(mapper, MTH_ADD, "ii", add_cb, "i"); Nicer?

int lrpc_call(struct lrpc_connection *conn, uint8_t method, void *user_data, ...);
For example:
lrpc_call(MTH_ADD, NULL, 66, 99);
Not nice, the user_data comes from nowhere.
May be set the user_data when installing the callback? Flexible enough?


int lrpc_buffer_get_int(lrpc_buffer_t *buff, uint8_t off, int *error);
char *lrpc_buffer_get_str(lrpc_buffer_t *buff, uint8_t off, int *error);


How to allocate method return value?

By declaring the returned values types when declaring the method? In this case the method C function gets as arguments:

int method(struct brpc_connection *conn, brpc_buffer_t *args, brpc_buffer_t *res);

the args buffer is filled
the res buffer is just initialized, not filled
Implies possible incoherence between the initialization of the buffer and its filling
=> better??? to fill the buffer when creating it, using brpc_buffer_new(const char *fmt, ...)
In this case, the method signature is:
int method(struct brpc_connection *conn, brpc_buffer_t *args, brpc_buffer_t **res);

but incoherence can be checked and signaled, refusing to set values that do not match.

Buffers are non-mutable, because changing a string can change buffer size, hence non accepted!

Best way to ensure non-mutability is to provide only a constructor and no _set_() functions!!!




IPC buffer layout (new version):

If data do not start at boundaries that are multiples of 4,8, then offsets must be computed when creating the buffer and stored in the buffer.

=> sort of vtable, table of MAX_ARGS entries of fixed size (8 or 16 bits?)

Data types:
id fmt   
0 ('n'): None, occupies no space
1 ('i'): int32, 4 bytes, aligned on 4 bytes
2 ('l'): int64, 8 bytes, aligned on 8 bytes
3 ('s'): str, variable space, aligned on 8 bytes on 64 bits, 4 bytes on 32 bits


buffer length (2 bytes)
ipc_type (1 byte) equivalent to JSON-RPC : 0: Request, 1: Response, 2: Error
method (1 byte)  equivalent to JSON-RPC method

id (4 bytes) (equivalent to JSON-RPC id)

vtable:
16 bits per entry:
2 bits for type => 4 types
14 bits for offset => offset < 16384 ok



data:
int (8 bytes)
str (8 bytes)
