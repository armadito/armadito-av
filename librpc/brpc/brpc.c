/* compile with:
   gcc -g -Iinclude/ -o brpc buffer.c brpc.c
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
#define ATENTRY_SIZE                 2
#define ATABLE_SIZE                  (ATABLE_MAX_ENTRIES * ATENTRY_SIZE)
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
#define ATENTRY_TYPE_BITS            2
#define ATENTRY_OFFSET_BITS          (ATENTRY_BITS - ATENTRY_TYPE_BITS)
#define ATENTRY_TYPE_SHIFT           ATENTRY_OFFSET_BITS
#define ATENTRY_OFFSET_SHIFT         0
#define MASK(B)                      (~(~0 << (B)))
#define GETBITS(V, S, B)             (((V) >> S) & MASK(B))
#define brpc_arg_type(B, I)          GETBITS(((uint16t *)((B) + ATABLE_OFF))[I], ATENTRY_TYPE_SHIFT, ATENTRY_TYPE_BITS)
#define brpc_arg_off(B, I)           GETBITS(((uint16t *)((B) + ATABLE_OFF))[I], ATENTRY_OFF_SHIFT, ATENTRY_OFF_BITS)
#define ATENTRY(TYPE, OFFSET)        ((((OFFSET) & MASK(ATENTRY_OFFSET_BITS)) << ATENTRY_OFFSET_SHIFT) | (((TYPE) & MASK(ATENTRY_TYPE_BITS)) << ATENTRY_TYPE_SHIFT))

/* internal functions */
static void brpc_buffer_set_method(brpc_buffer_t *b, uint8_t method);
static void brpc_buffer_set_id(brpc_buffer_t *b, uint32_t id);

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

brpc_buffer_t *brpc_buffer_new(enum brpc_buffer_type buffer_type, const char *fmt, ...)
{
	va_list args;
	struct buffer *b;
	brpc_buffer_t *ret;
	const char *p, *s;
	uint16_t off;
	int arg_count;
	size_t approximate_size;

	approximate_size = brpc_buffer_approximate_size(fmt);
	if (approximate_size == 0)
		return NULL;
	b = buffer_new(approximate_size);
	buffer_fill(b, 0, ARG_OFF);

	va_start(args, fmt);
	for(p = fmt, arg_count = 0; *p; p++, arg_count++) {
		switch(*p) {
		case 'i':
			buffer_fill(b, 0, align_gap(buffer_size(b), ARG_INT32_ALIGN));
			buffer_make_room(b, ARG_INT32_SIZE);
			*(int32_t *)buffer_end(b) = va_arg(args, int32_t);
			((uint16_t *)(buffer_data(b) + ATABLE_OFF))[arg_count] = ATENTRY(ARG_INT32, buffer_size(b));
			buffer_increment(b, ARG_INT32_SIZE);
			break;
		case 'l':
			buffer_fill(b, 0, align_gap(buffer_size(b), ARG_INT64_ALIGN));
			buffer_make_room(b, ARG_INT64_SIZE);
			*(int64_t *)buffer_end(b) = va_arg(args, int64_t);
			((uint16_t *)(buffer_data(b) + ATABLE_OFF))[arg_count] = ATENTRY(ARG_INT64, buffer_size(b));
			buffer_increment(b, ARG_INT64_SIZE);
			break;
		case 's':
			buffer_fill(b, 0, align_gap(buffer_size(b), ARG_STR_ALIGN));
			s = va_arg(args, const char *);
			off = buffer_size(b);
			buffer_append(b, s, strlen(s) + 1);
			((uint16_t *)(buffer_data(b) + ATABLE_OFF))[arg_count] = ATENTRY(ARG_STR, off);
			break;
		default:
			return NULL;
		}
	}
	va_end(args);

	ret = buffer_data(b);
	buffer_free(b, 0);

	return ret;
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
