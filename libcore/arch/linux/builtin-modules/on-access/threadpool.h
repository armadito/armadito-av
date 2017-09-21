


struct thread_pool;

typedef void *(*blocking_fun_t)(void *pool_data);

typedef int (*process_fun_t)(void *pool_data, void *data);

struct thread_pool *thread_pool_new(int n_threads, blocking_fun_t bf, process_fun_t pf, void *pool_data);


