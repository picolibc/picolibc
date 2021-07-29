/* semaphore.h: POSIX semaphore interface

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <sys/types.h>

#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __INSIDE_CYGWIN__
  typedef struct __sem_t {char __dummy;} *sem_t;
#endif

#define SEM_FAILED ((sem_t *) 0)

/* Semaphores */
  int sem_init (sem_t *__sem, int __pshared, unsigned int __value);
  int sem_destroy (sem_t *__sem);
  sem_t *sem_open (const char *__name, int __oflag, ...);
  int sem_close (sem_t *__sem);
  int sem_unlink (const char *__name);
  int sem_wait (sem_t *__sem);
  int sem_trywait (sem_t *__sem);
#if __GNU_VISIBLE
  int sem_clockwait (sem_t *__sem, clockid_t __clock_id, const struct timespec *__abstime);
#endif
  int sem_timedwait (sem_t *__sem, const struct timespec *__abstime);
  int sem_post (sem_t *__sem);
  int sem_getvalue (sem_t *__sem, int *__sval);

#ifdef __cplusplus
}
#endif

#endif				/* _SEMAPHORE_H */
