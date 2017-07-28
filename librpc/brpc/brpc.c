/* compile with:
   make brpc CFLAGS='-g -Iinclude/'
*/

#include <brpc.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"

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
#define BTYPE_OFF                    (BSIZE_OFF + BSIZE_SIZE)
#define BTYPE_SIZE                   1
#define METHOD_OFF                   (BTYPE_OFF + BTYPE_SIZE)
#define METHOD_SIZE                  1
#define ID_OFF                       (METHOD_OFF + METHOD_SIZE)
#define ID_SIZE                      4
#define ATABLE_OFF                   (ID_OFF + ID_SIZE)
#define ATABLE_MAX_ENTRIES           16
#define ATABLE_ENTRY_SIZE            2
#define ATABLE_SIZE                  (ATABLE_MAX_ENTRIES * ATABLE_ENTRY_SIZE)
#define ARG0_OFF                     (ATABLE_OFF + ATABLE_SIZE)

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

#define ATABLE_ENTRY_BITS            (ATABLE_ENTRY_SIZE * 8)
#define ATABLE_ENTRY_TYPE_BITS       2
#define ATABLE_ENTRY_OFFSET_BITS     (ATABLE_ENTRY_BITS - ATABLE_ENTRY_TYPE_BITS)
#define ATABLE_ENTRY_TYPE_SHIFT      ATABLE_ENTRY_OFF_BITS
#define ATABLE_ENTRY_OFFSET_SHIFT    0
#define MASK(B)                      (~(~0 << (B)))
#define GETBITS(V, S, B)             (((V) >> S) & MASK(B))
#define brpc_arg_type(B, I)          GETBITS(((uint16t *)((B) + ATABLE_OFF))[I], ATABLE_ENTRY_TYPE_SHIFT, ATABLE_ENTRY_TYPE_BITS)
#define brpc_arg_off(B, I)           GETBITS(((uint16t *)((B) + ATABLE_OFF))[I], ATABLE_ENTRY_OFF_SHIFT, ATABLE_ENTRY_OFF_BITS)

/* internal functions */
static void brpc_buffer_set_method(brpc_buffer_t *b, uint8_t method);
static void brpc_buffer_set_id(brpc_buffer_t *b, uint32_t id);

static size_t align_and_increment(size_t size, size_t alignment, size_t increment)
{
	while (size % alignment != 0)
		size++;
	size += increment;
	return size;
}

static size_t brpc_buffer_size(const char *fmt, va_list args)
{
	const char *p;
	size_t size = ARG0_OFF;
	int32_t i32;
	int64_t i64;
	char *s;

 	for(p = fmt; *p; p++) {
		switch(*p) {
		case 'n':
			break;
		case 'i':
			i32 = va_arg(args, int32_t);
			size = align_and_increment(size, ARG_INT32_ALIGN, ARG_INT32_SIZE);
			break;
		case 'l':
			i64 = va_arg(args, int64_t);
			size = align_and_increment(size, ARG_INT64_ALIGN, ARG_INT64_SIZE);
			break;
		case 's':
			s = va_arg(args, char *);
			size = align_and_increment(size, ARG_STR_ALIGN, strlen(s) + 1);
			break;
		default:
			return 0;
		}
	}

	return size;
}

static size_t align(size_t size, size_t alignment)
{
	while (size % alignment != 0)
		size++;
	return size;
}

/* temporary */
#define ATENTRY(T,O) 0

static void brpc_buffer_fill(brpc_buffer_t *b, const char *fmt, va_list args)
{
	const char *p;
	size_t l, off = ARG0_OFF;
	uint16_t *at = (uint16_t *)(b + ATABLE_OFF);
	int32_t i32;
	int64_t i64;
	char *s;

	for(p = fmt; *p; p++) {
		switch(*p) {
		case 'n':
			*at++ = ATENTRY(ARG_NONE, off);
			break;
		case 'i':
			i32 = va_arg(args, int32_t);
			off = align(off, ARG_INT32_ALIGN);
			*((int32_t *)(b + off)) = i32;
			*at++ = ATENTRY(ARG_INT32, off);
			off += ARG_INT32_SIZE;
			break;
		case 'l':
			i64 = va_arg(args, int64_t);
			off = align(off, ARG_INT64_ALIGN);
			*((int64_t *)(b + off)) = i64;
			*at++ = ATENTRY(ARG_INT64, off);
			off += ARG_INT64_SIZE;
			break;
		case 's':
			s = va_arg(args, char *);
			off = align(off, ARG_STR_ALIGN);
			l = strlen(s);
			strncpy(b + off, s, l);
			*at++ = ATENTRY(ARG_STR, off);
			off += l + 1;
			break;
		default:
			return;
		}
	}
}

brpc_buffer_t *brpc_buffer_new(enum brpc_buffer_type buffer_type, const char *fmt, ...)
{
	va_list args;
	size_t buffer_size;
	brpc_buffer_t *buffer;

	va_start(args, fmt);
	buffer_size = brpc_buffer_size(fmt, args);
	va_end(args);

	if (buffer_size == 0)
		return NULL;

	printf("buffer size: %ld 0x%lx\n", buffer_size, buffer_size);

	buffer = calloc(1, buffer_size);

	va_start(args, fmt);
	brpc_buffer_fill(buffer, fmt, args);
	va_end(args);

	return buffer;
}

void brpc_buffer_print(brpc_buffer_t *b)
{
}

int main(int argc, char **argv)
{
	brpc_buffer_t *b;

	b = brpc_buffer_new(REQUEST, "sissi", "foo", 66, "bar", "joe", 99);

	return 0;
}
