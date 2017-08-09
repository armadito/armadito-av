#include "ptrarray.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void destroy_cb(void *p)
{
	printf("destroying pointer %p\n", p);
}

static void test_ptr_array(int argc, char **argv)
{
	struct ptr_array *array = ptr_array_new(destroy_cb);
	int i;

	for(i = 1; i < argc; i++)
		ptr_array_add(array, argv[i]);

	assert(ptr_array_size(array) == argc - 1);

	for(i = 0; i < ptr_array_size(array); i++)
		assert(!strcmp(ptr_array_index(array, i), argv[i + 1]));

	ptr_array_clear(array);

	assert(ptr_array_size(array) == 0);

	ptr_array_free(array);
}

int main(int argc, char **argv)
{
	test_ptr_array(argc, argv);

	return 0;
}

