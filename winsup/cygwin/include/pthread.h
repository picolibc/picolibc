/* pthread.h: POSIX pthread interface

   Copyright 1996, 1997, 1998 Cygnus Solutions.

   Written by Marco Fuykschot <marco@ddi.nl>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <sys/types.h>
#include <signal.h>

#ifndef _PTHREAD_H
#define _PTHREAD_H

#ifdef __cplusplus
extern "C"
{
#endif

#define TFD(n) void*(*n)(void*)

typedef int pthread_t;
typedef int pthread_mutex_t;
typedef int sem_t;

typedef struct pthread_key
  {
  }
pthread_key_t;

typedef struct pthread_attr
  {
    size_t stacksize;
  }
pthread_attr_t;

typedef struct pthread_mutexattr
  {
  }
pthread_mutexattr_t;

/*  ThreadCreation */
int pthread_create (pthread_t * thread, const pthread_attr_t * attr, TFD (function), void *arg);
int pthread_attr_init (pthread_attr_t * attr);
int pthread_attr_destroy (pthread_attr_t * attr);
int pthread_attr_setstacksize (pthread_attr_t * attr, size_t size);
int pthread_attr_getstacksize (pthread_attr_t * attr, size_t * size);

/* Thread Control */
int pthread_detach (pthread_t thread);
int pthread_join (pthread_t thread, void **value_ptr);

/* Thread Exit */
int pthread_exit (void *value_ptr);

/* Thread SpecificData */
int pthread_key_create (pthread_key_t * key);
int pthread_key_delete (pthread_key_t * key);
int pthread_setspecific (pthread_key_t * key, const void *value);
void *pthread_getspecific (pthread_key_t * key);

/* Thread signal */
int pthread_kill (pthread_t * thread, int sig);
int pthread_sigmask (int operation, const sigset_t * set, sigset_t * old_set);

/*  ID */
pthread_t pthread_self ();
int pthread_equal (pthread_t t1, pthread_t t2);

/* Mutexes  */
int pthread_mutex_init (pthread_mutex_t * mutex, const pthread_mutexattr_t *);
int pthread_mutex_lock (pthread_mutex_t * mutext);
int pthread_mutex_trylock (pthread_mutex_t * mutext);
int pthread_mutex_unlock (pthread_mutex_t * mutext);
int pthread_mutex_destroy (pthread_mutex_t * mutext);

/* Solaris Semaphores */
int sem_init (sem_t * sem, int pshared, unsigned int value);
int sem_destroy (sem_t * sem);
int sem_wait (sem_t * sem);
int sem_trywait (sem_t * sem);
int sem_post (sem_t * sem);

#ifdef __cplusplus
}
#endif

#endif /* _PTHREAD_H */
