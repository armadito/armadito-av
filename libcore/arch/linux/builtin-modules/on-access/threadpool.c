
#include <pthread.h>
#include <stdlib.h>

#include "threadpool.h"

struct thread_pool {
	blocking_fun_t blocking_fun;
	process_fun_t process_fun;
	void *pool_data;
	int n_threads;
	pthread_mutex_t *mutex;
};

struct thread_pool *thread_pool_new(int n_threads, blocking_fun_t bf, process_fun_t pf, void *pool_data)
{
	struct thread_pool *tp = malloc(sizeof(struct thread_pool));

	tp->blocking_fun = bf;
	tp->process_fun = pf;
	tp->pool_data = pool_data;

	/* create the threads */

	return tp;
}

static void *thread_fun(void *arg)
{
	void *data;
	struct thread_pool *pool = (struct thread_pool *)arg;

	while (1) {
		pthread_mutex_lock(pool->mutex);
		data = (*pool->blocking_fun)(pool->pool_data);
		pthread_mutex_unlock(pool->mutex);

		if (data == NULL)
			return NULL;

		if (!(*pool->process_fun)(pool->pool_data, data))
			return NULL;
	}

	return NULL;
}

