/* semaphore.h: POSIX semaphore interface

   Copyright 2001, 2003 Red Hat, Inc.

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

#define SEM_FAILED 0
#define SEM_VALUE_MAX 1147483648

/* Semaphores */
  int sem_init (sem_t * sem, int pshared, unsigned int value);
  int sem_destroy (sem_t * sem);
  sem_t *sem_open (const char *name, int oflag, ...);
  int sem_close (sem_t *sem);
  int sem_wait (sem_t * sem);
  int sem_trywait (sem_t * sem);
  int sem_timedwait (sem_t * sem, const struct timespec *abstime);
  int sem_post (sem_t * sem);
  int sem_getvalue (sem_t * sem, int *sval);

#ifdef __cplusplus
}
#endif

#endif				/* _SEMAPHORE_H */
