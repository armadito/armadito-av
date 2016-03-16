#include "queue.h"
#include "stamp.h"

#include <alloca.h>
#include <stdlib.h>
#include <string.h>

static struct queue_node *queue_node_new(int fd, struct timespec *timestamp)
{
  struct queue_node *n = malloc(sizeof(struct queue_node));

  n->entry.fd = fd;
  stamp_cpy(&n->entry.timestamp, timestamp);

  n->prev = n->next = NULL;

  return n;
}

struct queue *queue_new(void)
{
  struct queue *q = malloc(sizeof(struct queue));

  q->head = NULL;
  q->tail = NULL;

  pthread_mutex_init(&q->queue_lock, NULL);

  return q;
}

void queue_free(struct queue *q)
{
  /* FIXME: empty the queue */
  pthread_mutex_destroy(&q->queue_lock);

  free(q);
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
  struct queue_node *new_entry = queue_node_new(fd, timestamp);

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

static inline void queue_remove(struct queue *q, struct queue_node *n)
{
  if (n->prev != NULL)
    n->prev->next = n->next;
  else
    q->head = n->next;

  if (n->next != NULL)
    n->next->prev = n->prev;
  else
    q->tail = n->prev;
}

int queue_pop_fd(struct queue *q, int fd, struct queue_entry *pop_entry)
{
  struct queue_node *n;

  queue_lock(q);

  n = q->head; 
  while (n != NULL & n->entry.fd != fd)
    n = n->next;

  if (n == NULL) {
    queue_unlock(q);
    return 0;
  }

  queue_remove(q, n);

  queue_unlock(q);

  *pop_entry = n->entry;

  free(n);

  return 1;
}

int queue_pop_timeout(struct queue *q, struct timespec *before, struct queue_entry *pop_entries, int n_entries)
{
  struct queue_node *n;
  struct queue_node **to_free;
  int count = 0, i;

  to_free = alloca(n_entries * sizeof(struct queue_node *));

  queue_lock(q);

  n = q->tail; 
  while (n != NULL && count < n_entries) {
    if (stamp_cmp(&n->entry.timestamp, before) <= 0) {
      queue_remove(q, n);
      to_free[count] = n;
      pop_entries[count] = n->entry;
      count++;
    }

    n = n->prev;
  }

  queue_unlock(q);

  for(i = 0; i < count; i++)
    free(to_free[i]);

  return count;
}
