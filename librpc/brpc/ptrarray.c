#include "buffer.h"
#include "ptrarray.h"

#include <stdlib.h>

struct ptr_array {
	struct buffer elements;
	destroy_cb_t element_destroy_cb;
};

struct ptr_array *ptr_array_new(destroy_cb_t element_destroy_cb)
{
	struct ptr_array *array;

	array = malloc(sizeof(struct ptr_array));

	buffer_init(&array->elements, 0);
	array->element_destroy_cb = element_destroy_cb;

	return array;
}

void ptr_array_free(struct ptr_array *array)
{
	buffer_destroy(&array->elements, 1);
	free(array);
}

void ptr_array_add(struct ptr_array *array, void *element)
{
	buffer_make_room(&array->elements, sizeof(void *));

	*(void **)buffer_end(&array->elements) = element;

	buffer_increment(&array->elements, sizeof(void *));
}

size_t ptr_array_size(struct ptr_array *array)
{
	return buffer_size(&array->elements) / sizeof(void *);
}

void *ptr_array_index(struct ptr_array *array, int index)
{
	return *((void **)buffer_data(&array->elements) + index);
}

void ptr_array_clear(struct ptr_array *array)
{
	void **p;

	for(p = buffer_data(&array->elements); p < (void **)buffer_end(&array->elements); p++)
		(*(array->element_destroy_cb))(*p);

	buffer_clear(&array->elements);
}
