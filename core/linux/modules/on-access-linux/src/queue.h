/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <time.h>
#include <pthread.h>

struct queue_entry {
	int fd;
	struct timespec timestamp;
};

struct queue_node {
	struct queue_entry entry;
	struct queue_node *prev, *next;
};

struct queue {
	struct queue_node *head, *tail;
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
int queue_pop_fd(struct queue *q, int fd, struct queue_entry *pop_entry);

/**
   Atomically dequeue n_fd maximum file descriptors that have timeout'ed
   Place the dequeued file descriptors + path + timestamp in 'entries'
   Returns the number of entries that have been dequeued
*/
int queue_pop_timeout(struct queue *q, struct timespec *before, struct queue_entry *pop_entries, int n_entries);

#endif
