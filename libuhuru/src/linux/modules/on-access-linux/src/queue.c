#include "queue.h"
#include "stamp.h"

#include <alloca.h>
#include <stdlib.h>

static struct queue_entry *queue_entry_new(int fd, struct timespec *timestamp)
{
  struct queue_entry *e = malloc(sizeof(struct queue_entry));

  e->fd = fd;
  stamp_cpy(&e->timestamp, timestamp);
  e->prev = e->next = NULL;

  return e;
}

void queue_init(struct queue *q)
{
  q->head = NULL;
  q->tail = NULL;

  pthread_mutex_init(&q->queue_lock, NULL);
}

void queue_destroy(struct queue *q)
{
  /* FIXME: empty the queue */
  pthread_mutex_destroy(&q->queue_lock);
}

static inline void queue_lock(struct queue *q)
{
  pthread_mutex_lock(&q->queue_lock);
}

static inline void queue_unlock(struct queue *q)
{
  pthread_mutex_unlock(&q->queue_lock);
}

void queue_push(struct queue *q, int fd, struct timespec *timestamp)
{
  struct queue_entry *new_entry = queue_entry_new(fd, timestamp);

  queue_lock(q);

  if (q->head == NULL) {
    q->head = new_entry;
    q->tail = new_entry;
  } else {
    q->head->prev = new_entry;
    new_entry->next = q->head;
    q->head = new_entry;
  }

  queue_unlock(q);
}

static inline void queue_remove(struct queue *q, struct queue_entry *e)
{
  if (e->prev != NULL)
    e->prev->next = e->next;
  else
    q->head = e->next;

  if (e->next != NULL)
    e->next->prev = e->prev;
  else
    q->tail = e->prev;
}

int queue_pop(struct queue *q, int fd)
{
  struct queue_entry *e;

  queue_lock(q);

  e = q->head; 
  while (e != NULL & e->fd != fd)
    e = e->next;

  if (e == NULL) {
    queue_unlock(q);
    return 0;
  }

  queue_remove(q, e);

  queue_unlock(q);

  free(e);

  return 1;
}

int queue_pop_timeout(struct queue *q, struct timespec *before, int *fds, int n_fd)
{
  struct queue_entry *e;
  struct queue_entry **to_free;
  int count = 0, i;

  to_free = alloca(n_fd * sizeof(struct queue_entry *));

  queue_lock(q);

  e = q->tail; 
  while (e != NULL && count < n_fd) {
    if (stamp_cmp(&e->timestamp, before) <= 0) {
      queue_remove(q, e);
      to_free[count] = e;
      fds[count] = e->fd;
      count++;
    }

    e = e->prev;
  }

  queue_unlock(q);

  for(i = 0; i < count; i++)
    free(to_free[i]);

  return count;
}
