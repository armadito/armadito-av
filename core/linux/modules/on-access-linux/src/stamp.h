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

#ifndef _STAMP_H_
#define _STAMP_H_

#include <time.h>

/* for nanosleep */
/* all times are in nanoseconds */
#define ONE_NANOSECOND  (1L)
#define ONE_MICROSECOND (1000L * ONE_NANOSECOND)
#define ONE_MILLISECOND (1000L * ONE_MICROSECOND)
#define ONE_SECOND      (1000L * ONE_MILLISECOND)

static inline void stamp_now(struct timespec *s)
{
	clock_gettime(CLOCK_MONOTONIC_COARSE, s);
}

static inline int stamp_cmp(struct timespec *s1, struct timespec *s2)
{
	if (s1->tv_sec < s2->tv_sec)
		return -1;
	else if (s1->tv_sec > s2->tv_sec)
		return 1;
	else if (s1->tv_nsec < s2->tv_nsec)
		return -1;
	else if (s1->tv_nsec > s2->tv_nsec)
		return 1;

	return 0;
}

static inline void stamp_cpy(struct timespec *dst, const struct timespec *src)
{
	dst->tv_sec = src->tv_sec;
	dst->tv_nsec = src->tv_nsec;
}

static inline void stamp_add(struct timespec *dst, const struct timespec *src)
{
	dst->tv_nsec += src->tv_nsec;

	if (dst->tv_nsec >= ONE_SECOND) {
		dst->tv_nsec -= ONE_SECOND;
		dst->tv_sec++;
	}

	dst->tv_sec += src->tv_sec;
}

static inline void stamp_sub(struct timespec *dst, const struct timespec *src)
{
	dst->tv_nsec -= src->tv_nsec;

	if (dst->tv_nsec < 0) {
		dst->tv_nsec += ONE_SECOND;
		dst->tv_sec--;
	}

	dst->tv_sec -= src->tv_sec;
}

#endif
