#include "queue.h"
#include "stamp.h"

#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
    printf("{%d, %ld.%09ld s} ", e->fd, e->timestamp.tv_sec, e->timestamp.tv_nsec);
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

static void queue_test_1(void)
{
  struct queue *q;

  q = queue_new();

  queue_test_push(q);

  queue_test_print(q);

  queue_test_timeout_1(q);

  queue_test_print(q);

  queue_free(q);
}

#define PUSH_COUNT 50
#define STATEBUFSZ 256

static void *push_thread_fun(void *arg)
{
  struct queue *q = (struct queue *)arg;
  int i;
  char statebuf[STATEBUFSZ];
  struct random_data buf;

  memset(statebuf, 0, STATEBUFSZ);
  memset(&buf, 0, sizeof(buf));
	
  if (initstate_r(0xdeadbeef, statebuf, STATEBUFSZ, &buf)) {
    fprintf(stderr, "initstate_r failed (%s)\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  for(i = 0; i < PUSH_COUNT; i++) {
    int fd = 42;
    struct timespec now;

    printf("=== push iteration %d\n", i);
    stamp_now(&now);

    random_r(&buf, &fd);
    fd %= 128;

    queue_push(q, fd, &now);

    queue_test_print(q);
  }

  printf("push thread terminated\n");

  return NULL;
}

static void *timeout_thread_fun(void *arg)
{
  struct queue *q = (struct queue *)arg;

  return NULL;
}

static void queue_test_2(void)
{
  struct queue *q;
  pthread_t push_thread, timeout_thread;

  q = queue_new();

  if (pthread_create(&push_thread, NULL, push_thread_fun, q)) {
    fprintf(stderr, "pthread_create failed (%s)\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (pthread_create(&timeout_thread, NULL, timeout_thread_fun, q)) {
    fprintf(stderr, "pthread_create failed (%s)\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  printf("joining push thread\n");
  pthread_join(push_thread, NULL);
  printf("push thread joined\n");

  printf("joining timeout thread\n");
  pthread_join(timeout_thread, NULL);
  printf("timeout thread joined\n");

  queue_free(q);
}

int main(int argc, char **argv)
{
  /* queue_test_1(); */
  queue_test_2();
}

