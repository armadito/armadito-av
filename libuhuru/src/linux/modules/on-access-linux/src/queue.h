#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <time.h>
#include <pthread.h>

struct queue_entry {
  int fd;
  struct timespec timestamp;
  struct queue_entry *prev, *next;
};

struct queue {
  struct queue_entry *head, *tail;
  pthread_mutex_t queue_lock;
};

struct queue *queue_new(void);

void queue_free(struct queue *q);

/**
   Atomically enqueue a file descriptor in the queue
 */
void queue_push(struct queue *q, int fd, struct timespec *timestamp);

/**
   Atomically dequeue a file descriptor in the queue
   Returns 1 if file descriptor was in the queue, 0 if not
 */
int queue_pop(struct queue *q, int fd);

/**
   Atomically dequeue n_fd maximum file descriptors that have timeout'ed
   Place the dequeued file descriptors in fds
   Returns the number of file descriptors that have been dequeued 
*/
int queue_pop_timeout(struct queue *q, struct timespec *before, int *fds, int n_fd);

#endif
