#ifndef PTRARRAY_H
#define PTRARRAY_H

#include "types.h"

#include <stddef.h>

struct ptr_array;

struct ptr_array *ptr_array_new(destroy_cb_t element_destroy_cb);

void ptr_array_free(struct ptr_array *array);

void ptr_array_add(struct ptr_array *array, void *element);

size_t ptr_array_size(struct ptr_array *array);

void *ptr_array_index(struct ptr_array *array, int index);

void ptr_array_clear(struct ptr_array *array);

#endif
