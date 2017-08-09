#include "types.h"
#include "buffer.h"
#include "ptrarray.h"

#include <stdlib.h>

struct ptr_array {
	struct buffer b;
	destroy_cb_t element_destroy_cb;
};

struct ptr_array *ptr_array_new(destroy_cb_t element_destroy_cb)
{
	struct ptr_array *array;

	array = malloc(sizeof(struct ptr_array));

	buffer_init(&array->b, 0);
	array->element_destroy_cb = element_destroy_cb;

	return array;
}

void ptr_array_free(struct ptr_array *array)
{
	buffer_destroy(&array->b, 1);
	free(array);
}

void ptr_array_add(struct ptr_array *array, void *element)
{
	buffer_make_room(&array->b, sizeof(void *));

	*(void **)buffer_end(&array->b) = element;

	buffer_increment(&array->b, sizeof(void *));
}

size_t ptr_array_size(struct ptr_array *array)
{
	return buffer_size(&array->b) / sizeof(void *);
}

void *ptr_array_index(struct ptr_array *array, int index)
{
	return *((void **)buffer_data(&array->b) + index);
}

void ptr_array_clear(struct ptr_array *array)
{
	void **p;

	for(p = buffer_data(&array->b); p < (void **)buffer_end(&array->b); p++)
		(*(array->element_destroy_cb))(*p);

	buffer_clear(&array->b);
}
