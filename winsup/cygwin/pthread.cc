/* pthread.cc: posix pthread interface for Cygwin

   Copyright 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2007 Red Hat, Inc.

   Originally written by Marco Fuykschot <marco@ddi.nl>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include "thread.h"

extern "C"
{
/*  ThreadCreation */
int
pthread_create (pthread_t * thread, const pthread_attr_t * attr,
		void *(*start_routine) (void *), void *arg)
{
  return pthread::create (thread, attr, start_routine, arg);
}

int
pthread_once (pthread_once_t * once_control, void (*init_routine) (void))
{
  return pthread::once (once_control, init_routine);
}

int
pthread_atfork (void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
  return pthread::atfork (prepare, parent, child);
}

/* Thread Exit */
void
pthread_exit (void *value_ptr)
{
  return pthread::self ()->exit (value_ptr);
}

int
pthread_join (pthread_t thread, void **return_val)
{
  return pthread::join (&thread, (void **) return_val);
}

int
pthread_detach (pthread_t thread)
{
  return pthread::detach (&thread);
}


/* This isn't a posix call... should we keep it? */
int
pthread_suspend (pthread_t thread)
{
  return pthread::suspend (&thread);
}

/* same */
int
pthread_continue (pthread_t thread)
{
  return pthread::resume (&thread);
}

unsigned long
pthread_getsequence_np (pthread_t * thread)
{
  if (!pthread::is_good_object (thread))
    return EINVAL;
  return (*thread)->getsequence_np ();
}

/*  ID */

pthread_t pthread_self ()
{
  return pthread::self ();
}

/* Mutexes  */
int
pthread_mutex_init (pthread_mutex_t * mutex, const pthread_mutexattr_t * attr)
{
  return pthread_mutex::init (mutex, attr);
}

/* Synchronisation */
int
pthread_cond_init (pthread_cond_t * cond, const pthread_condattr_t * attr)
{
  return pthread_cond::init (cond, attr);
}

/* RW Locks */
int
pthread_rwlock_init (pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)
{
  return pthread_rwlock::init (rwlock, attr);
}

/* Cancelability */

int
pthread_cancel (pthread_t thread)
{
  return pthread::cancel (thread);
}

int
pthread_setcancelstate (int state, int *oldstate)
{
  return pthread::self ()->setcancelstate (state, oldstate);
}

int
pthread_setcanceltype (int type, int *oldtype)
{
  return pthread::self ()->setcanceltype (type, oldtype);
}

void
pthread_testcancel ()
{
  pthread::self ()->testcancel ();
}

void
_pthread_cleanup_push (__pthread_cleanup_handler *handler)
{
  pthread::self ()->push_cleanup_handler (handler);
}

void
_pthread_cleanup_pop (int execute)
{
  pthread::self ()->pop_cleanup_handler (execute);
}

/* Semaphores */
int
sem_init (sem_t * sem, int pshared, unsigned int value)
{
  return semaphore::init (sem, pshared, value);
}

int
sem_destroy (sem_t * sem)
{
  return semaphore::destroy (sem);
}

int
sem_wait (sem_t * sem)
{
  return semaphore::wait (sem);
}

int
sem_trywait (sem_t * sem)
{
  return semaphore::trywait (sem);
}

int
sem_timedwait (sem_t * sem, const struct timespec *abstime)
{
  return semaphore::timedwait (sem, abstime);
}

int
sem_post (sem_t * sem)
{
  return semaphore::post (sem);
}

int
sem_getvalue (sem_t * sem, int *sval)
{
  return semaphore::getvalue (sem, sval);
}

}
