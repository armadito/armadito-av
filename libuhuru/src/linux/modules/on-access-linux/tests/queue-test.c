#include "queue.h"
#include "stamp.h"

#include <stdio.h>

static void queue_test_print(struct queue *q)
{
  struct queue_entry *e;

  printf("-> ");

  e = q->head; 
  while (e != NULL) {
    printf("{%d, %ld.%09ld s} ", e->fd, e->timestamp.tv_sec, e->timestamp.tv_nsec);
    e = e->next;
  }

  printf("\n");

  printf("<- ");

  e = q->tail; 
  while (e != NULL) {
    printf("{%d, %ld.%ld s} ", e->fd, e->timestamp.tv_sec, e->timestamp.tv_nsec);
    e = e->prev;
  }

  printf("\n");
}

static struct timespec zero_s_5 = { 0L, 500000000L};
static struct timespec zero_s_7 = { 0L, 700000000L};
static struct timespec one_s = { 1L, 0L};

static void queue_test_push(struct queue *q)
{
  struct timespec now;

  stamp_now(&now);
  queue_push(q, 1, &now);
  queue_push(q, 2, &now);

  stamp_add(&now, &zero_s_5);
  queue_push(q, 3, &now);
  queue_push(q, 4, &now);

  stamp_add(&now, &zero_s_7);
  queue_push(q, 5, &now);
  queue_push(q, 6, &now);
}

static void queue_test_timeout_1(struct queue *q)
{
  struct timespec now;
  int fds[6];
  int n;

  stamp_now(&now);
  stamp_add(&now, &one_s);

  n = queue_pop_timeout(q, &now, fds, 6);

  printf("pop'ed %d filedesc\n", n);
}

static void queue_test(void)
{
  struct queue q;

  queue_init(&q);

  queue_test_push(&q);

  queue_test_print(&q);

  queue_test_timeout_1(&q);

  queue_test_print(&q);

  queue_destroy(&q);
}

int main(int argc, char **argv)
{
  queue_test();
}

