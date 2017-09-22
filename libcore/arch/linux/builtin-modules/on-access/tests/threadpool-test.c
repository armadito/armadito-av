#include "threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

static void *block(void *pool_data)
{
	unsigned int seconds = random() % 2 + 1;

	printf("thread 0x%lx is going to sleep for %d seconds\n", pthread_self(), seconds);

	sleep(seconds);

	printf("thread 0x%lx has finished sleeping\n", pthread_self());

	return malloc(1);
}

static int process(void *pool_data, void *data)
{
	printf("thread 0x%lx is processing\n", pthread_self());

	free(data);

	return 0;
}

static int test_1(void)
{
	struct thread_pool *tp;

	tp = thread_pool_new(8, block, process, NULL);

	sleep(60);

	return 0;
}

int main(int argc, char **argv)
{
	return test_1();
}
