/* pthread.cc: posix pthread interface for Cygwin

   Copyright 1998 Cygnus Solutions.

   Written by Marco Fuykschot <marco@ddi.nl>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include "thread.h"

extern "C" {
/*  ThreadCreation */
int
pthread_create (pthread_t * thread, const pthread_attr_t * attr, void *(*start_routine) (void *), void *arg)
{
  return __pthread_create (thread, attr, start_routine, arg);
}

int
pthread_attr_init (pthread_attr_t * attr)
{
  return __pthread_attr_init (attr);
}

int
pthread_attr_destroy (pthread_attr_t * attr)
{
  return __pthread_attr_destroy (attr);
}

int
pthread_attr_setstacksize (pthread_attr_t * attr, size_t size)
{
  return __pthread_attr_setstacksize (attr, size);
}

int
pthread_attr_getstacksize (pthread_attr_t * attr, size_t * size)
{
  return __pthread_attr_getstacksize (attr, size);
}


/*
   pthread_attr_setstackaddr(...){};
   pthread_attr_getstackaddr(...){};
 */

/* Thread Exit */
int
pthread_exit (void * value_ptr)
{
  return __pthread_exit (value_ptr);
}

int
pthread_join(pthread_t thread, void **return_val)
{
  return __pthread_join(&thread, (void **)return_val);
}

int
pthread_detach(pthread_t thread)
{
  return __pthread_detach(&thread);
}

int
pthread_suspend(pthread_t thread)
{
   return __pthread_suspend(&thread);
}

int
pthread_continue(pthread_t thread)
{
   return __pthread_continue(&thread);
}

unsigned long
pthread_getsequence_np (pthread_t * thread)
{
  return __pthread_getsequence_np (thread);
}

/* Thread SpecificData */
int
pthread_key_create (pthread_key_t * key)
{
  return __pthread_key_create (key);
}

int
pthread_key_delete (pthread_key_t * key)
{
  return __pthread_key_delete (key);
}

int
pthread_setspecific (pthread_key_t * key, const void *value)
{
  return __pthread_setspecific (key, value);
}

void *
pthread_getspecific (pthread_key_t * key)
{
  return (void *) __pthread_getspecific (key);
}

/* Thread signal */
int
pthread_kill (pthread_t * thread, int sig)
{
  return __pthread_kill (thread, sig);
}

int
pthread_sigmask (int operation, const sigset_t * set, sigset_t * old_set)
{
  return __pthread_sigmask (operation, set, old_set);
}

/*  ID */

pthread_t
pthread_self ()
{
  return __pthread_self ();
}

int
pthread_equal (pthread_t t1, pthread_t t2)
{
  return __pthread_equal (&t1, &t2);
}

/* Mutexes  */
int
pthread_mutex_init (pthread_mutex_t * mutex, const pthread_mutexattr_t * attr)
{
  return __pthread_mutex_init (mutex, attr);
}

int
pthread_mutex_lock (pthread_mutex_t * mutex)
{
  return __pthread_mutex_lock (mutex);
}

int
pthread_mutex_trylock (pthread_mutex_t * mutex)
{
  return __pthread_mutex_trylock (mutex);
}

int
pthread_mutex_unlock (pthread_mutex_t * mutex)
{
  return __pthread_mutex_unlock (mutex);
}

int
pthread_mutex_destroy (pthread_mutex_t * mutex)
{
  return __pthread_mutex_destroy (mutex);
}

/* Semaphores */
int
sem_init (sem_t * sem, int pshared, unsigned int value)
{
  return __sem_init (sem, pshared, value);
}

int
sem_destroy (sem_t * sem)
{
  return __sem_destroy (sem);
}

int
sem_wait (sem_t * sem)
{
  return __sem_wait (sem);
}

int
sem_trywait (sem_t * sem)
{
  return __sem_trywait (sem);
}

int
sem_post (sem_t * sem)
{
  return __sem_post (sem);
}
}
