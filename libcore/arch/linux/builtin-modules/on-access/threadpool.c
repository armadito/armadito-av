
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "threadpool.h"

enum token_state {
	OWNED,
	AVAILABLE,
};

struct thread_pool {
	blocking_fun_t blocking_fun;
	process_fun_t process_fun;
	void *pool_data;
	int n_threads;
	pthread_t *threads;
	int canceled;
	enum token_state token;
	pthread_mutex_t mutex;
	pthread_cond_t notify;
};

static void *thread_fun(void *arg)
{
	void *data;
	struct thread_pool *pool = (struct thread_pool *)arg;
	int oldstate, oldtype;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
	fprintf(stderr, "cancel state %d\n", oldstate);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldtype);
	fprintf(stderr, "cancel type %d\n", oldtype);

	while (1) {
		pthread_mutex_lock(&pool->mutex);
		while (!pool->canceled && pool->token == OWNED) {
			fprintf(stderr, "thread 0x%lx is going to wait\n", pthread_self());
			pthread_cond_wait(&pool->notify, &pool->mutex);
		}
		if (pool->canceled) {
			fprintf(stderr, "thread 0x%lx was canceled while waiting\n", pthread_self());
			break;
		}

		pool->token = OWNED;
		pthread_mutex_unlock(&pool->mutex);

		data = (*pool->blocking_fun)(pool->pool_data);
		if (pool->canceled) {
			fprintf(stderr, "thread 0x%lx was canceled while blocking\n", pthread_self());
			break;
		}

		pthread_mutex_lock(&pool->mutex);
		pool->token = AVAILABLE;
		pthread_mutex_unlock(&pool->mutex);
		pthread_cond_signal(&pool->notify);

		if (data == NULL)
			break;

		if ((*pool->process_fun)(pool->pool_data, data))
			break;
	}

	fprintf(stderr, "thread 0x%lx is exiting\n", pthread_self());

	return NULL;
}

struct thread_pool *thread_pool_new(int n_threads, blocking_fun_t bf, process_fun_t pf, void *pool_data)
{
	struct thread_pool *tp = malloc(sizeof(struct thread_pool));
	int n;

	tp->blocking_fun = bf;
	tp->process_fun = pf;
	tp->pool_data = pool_data;
	tp->canceled = 0;
	tp->n_threads = n_threads;
	tp->threads = calloc(n_threads, sizeof(pthread_t));

	/* create the threads */
	for (n = 0; n < n_threads; n++) {
		if (pthread_create(&tp->threads[n], NULL, thread_fun, tp))
			goto ret_err;
	}

	tp->token = AVAILABLE;
	pthread_mutex_init(&tp->mutex, NULL);
	pthread_cond_init(&tp->notify, NULL);

	return tp;

ret_err:
	pthread_mutex_destroy(&tp->mutex);

	/* must deinitialize the threads */

	free(tp);
	return NULL;
}

int thread_pool_free(struct thread_pool *tp, int do_join)
{
	int n, ret = 0;

	tp->canceled = 1;
	for (n = 0; n < tp->n_threads; n++) {
		int r;

		fprintf(stderr, "canceling thread 0x%lx\n", tp->threads[n]);
		r = pthread_cancel(tp->threads[n]);
		if (r)
			ret = r;
	}

	fprintf(stderr, "after cancel: ret = %d\n", ret);

	if (!do_join)
		return ret;

	for (n = 0; n < tp->n_threads; n++) {
		int r;
		void *retval;

		fprintf(stderr, "joining thread 0x%lx\n", tp->threads[n]);
		r = pthread_join(tp->threads[n], &retval);
		if (r)
			ret = r;
	}

	return ret;
}
