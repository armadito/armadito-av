#ifndef _STAMP_H_
#define _STAMP_H_

#include <time.h>

static inline void stamp_now(struct timespec *s)
{
  clock_gettime(CLOCK_REALTIME_COARSE, s);
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

  if (dst->tv_nsec >= 1000000000L) {
    dst->tv_nsec -= 1000000000L;
    dst->tv_sec++;
  }

  dst->tv_sec += src->tv_sec;
}

static inline void stamp_sub(struct timespec *dst, const struct timespec *src)
{
  dst->tv_nsec -= src->tv_nsec;

  if (dst->tv_nsec < 0) {
    dst->tv_nsec += 1000000000L;
    dst->tv_sec--;
  }

  dst->tv_sec -= src->tv_sec;
}

#endif
