#include "threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

static void *block_1(void *pool_data)
{
	unsigned int seconds = random() % 2 + 1;

	printf("thread 0x%lx is going to sleep for %d seconds\n", pthread_self(), seconds);
	sleep(seconds);
	printf("thread 0x%lx has finished sleeping\n", pthread_self());

	return malloc(1);
}

static int process_1(void *pool_data, void *data)
{
	printf("thread 0x%lx is processing\n", pthread_self());

	free(data);

	return 0;
}

static int test_1(void)
{
	struct thread_pool *tp;

	tp = thread_pool_new(8, block_1, process_1, NULL);

	sleep(60);

	return 0;
}

static void *block_2(void *pool_data)
{
	int fd = *(int *)pool_data;
	char *p = malloc(1);

	if (read(fd, p, 1) <= 0) {
		perror("read");
		free(p);
		return NULL;
	}

	return p;
}

static int process_2(void *pool_data, void *data)
{
	char *p = data;

	printf("thread 0x%lx has received %c\n", pthread_self(), *p);

	free(p);

	sleep(1);

	return 0;
}

static int test_2(int n)
{
	int command_pipe[2];
	struct thread_pool *tp;
	int *pool_data;
	int i, ret;

	if (pipe(command_pipe) < 0) {
		perror("pipe failed");
		return 1;
	}

	pool_data = malloc(sizeof(int));
	*pool_data = command_pipe[0];

	tp = thread_pool_new(4, block_2, process_2, pool_data);

	for (i = 0; i < n; i++) {
		char c = 'A' + (random() % 26);

		if (write(command_pipe[1], &c, 1) < 0) {
			perror("write failed");
			return 1;
		}
		printf("[%d] %c\n", i, c);

		sleep(1);
 	}

	if ((ret = thread_pool_free(tp, 1)))
		fprintf(stderr, "thread_pool_free returned %d\n", ret);

	return ret;
}

static void usage(const char *progname)
{
	fprintf(stderr, "Usage: %s TEST_NUMBER\n", progname);
	fprintf(stderr, "  TEST_NUMBER: 1,2\n");

	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int which_test;

	if (argc < 2)
		usage(argv[0]);
	if (sscanf(argv[1], "%d", &which_test) != 1)
		usage(argv[0]);

	switch(which_test) {
	case 1:
		return test_1();
	case 2:
		return test_2(4);
	default:
		usage(argv[0]);
	}

	return 0;
}
