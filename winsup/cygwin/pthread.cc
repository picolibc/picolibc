/* pthread.cc: posix pthread interface for Cygwin

   Copyright 1998, 1999, 2000, 2001 Red Hat, Inc.

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
  return __pthread_create (thread, attr, start_routine, arg);
}

int
pthread_once (pthread_once_t * once_control, void (*init_routine) (void))
{
  return __pthread_once (once_control, init_routine);
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
pthread_attr_setdetachstate (pthread_attr_t * attr, int detachstate)
{
  return __pthread_attr_setdetachstate (attr, detachstate);
}

int
pthread_attr_getdetachstate (const pthread_attr_t * attr, int *detachstate)
{
  return __pthread_attr_getdetachstate (attr, detachstate);
}


int
pthread_attr_setstacksize (pthread_attr_t * attr, size_t size)
{
  return __pthread_attr_setstacksize (attr, size);
}

int
pthread_attr_getstacksize (const pthread_attr_t * attr, size_t * size)
{
  return __pthread_attr_getstacksize (attr, size);
}

int
pthread_attr_setinheritsched (pthread_attr_t * attr, int inheritsched)
{
  return __pthread_attr_setinheritsched (attr, inheritsched);
}

int
pthread_attr_getinheritsched (const pthread_attr_t * attr, int *inheritsched)
{
  return __pthread_attr_getinheritsched (attr, inheritsched);
}

int
pthread_attr_setschedparam (pthread_attr_t * attr,
			    const struct sched_param *param)
{
  return __pthread_attr_setschedparam (attr, param);
}

int
pthread_attr_getschedparam (const pthread_attr_t * attr,
			    struct sched_param *param)
{
  return __pthread_attr_getschedparam (attr, param);
}

int
pthread_attr_setschedpolicy (pthread_attr_t * attr, int policy)
{
  return __pthread_attr_setschedpolicy (attr, policy);
}

int
pthread_attr_getschedpolicy (const pthread_attr_t * attr, int *policy)
{
  return __pthread_attr_getschedpolicy (attr, policy);
}

int
pthread_attr_setscope (pthread_attr_t * attr, int contentionscope)
{
  return __pthread_attr_setscope (attr, contentionscope);
}

int
pthread_attr_getscope (const pthread_attr_t * attr, int *contentionscope)
{
  return __pthread_attr_getscope (attr, contentionscope);
}

#ifdef _POSIX_THREAD_ATTR_STACKADDR
int
pthread_attr_setstackaddr (pthread_attr_t * attr, void *stackaddr)
{
  return __pthread_attr_setstackaddr (attr, stackaddr);
}

int
pthread_attr_getstackaddr (const pthread_attr_t * attr, void **stackaddr)
{
  return __pthread_attr_getstackaddr (attr, stackaddr);
}
#endif

/* Thread Exit */
void
pthread_exit (void *value_ptr)
{
  return __pthread_exit (value_ptr);
}

int
pthread_join (pthread_t thread, void **return_val)
{
  return __pthread_join (&thread, (void **) return_val);
}

int
pthread_detach (pthread_t thread)
{
  return __pthread_detach (&thread);
}


/* This isn't a posix call... should we keep it? */
int
pthread_suspend (pthread_t thread)
{
  return __pthread_suspend (&thread);
}

/* same */
int
pthread_continue (pthread_t thread)
{
  return __pthread_continue (&thread);
}

unsigned long
pthread_getsequence_np (pthread_t * thread)
{
  return __pthread_getsequence_np (thread);
}

/* Thread SpecificData */
int
pthread_key_create (pthread_key_t * key, void (*destructor) (void *))
{
  return __pthread_key_create (key, destructor);
}

int
pthread_key_delete (pthread_key_t key)
{
  return __pthread_key_delete (key);
}

int
pthread_setspecific (pthread_key_t key, const void *value)
{
  return __pthread_setspecific (key, value);
}

void *
pthread_getspecific (pthread_key_t key)
{
  return (void *) __pthread_getspecific (key);
}

/* Thread signal */
int
pthread_kill (pthread_t thread, int sig)
{
  return __pthread_kill (thread, sig);
}

int
pthread_sigmask (int operation, const sigset_t * set, sigset_t * old_set)
{
  return __pthread_sigmask (operation, set, old_set);
}

/*  ID */

pthread_t pthread_self ()
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

int
pthread_mutex_setprioceiling (pthread_mutex_t * mutex,
			      int prioceiling, int *old_ceiling)
{
  return __pthread_mutex_setprioceiling (mutex, prioceiling, old_ceiling);
}

int
pthread_mutex_getprioceiling (const pthread_mutex_t * mutex, int *prioceiling)
{
  return __pthread_mutex_getprioceiling (mutex, prioceiling);
}



int
pthread_mutexattr_destroy (pthread_mutexattr_t * attr)
{
  return __pthread_mutexattr_destroy (attr);
}

int
pthread_mutexattr_getprioceiling (const pthread_mutexattr_t * attr,
				  int *prioceiling)
{
  return __pthread_mutexattr_getprioceiling (attr, prioceiling);
}

int
pthread_mutexattr_getprotocol (const pthread_mutexattr_t * attr,
			       int *protocol)
{
  return __pthread_mutexattr_getprotocol (attr, protocol);
}

int
pthread_mutexattr_getpshared (const pthread_mutexattr_t * attr, int *pshared)
{
  return __pthread_mutexattr_getpshared (attr, pshared);
}

int
pthread_mutexattr_gettype (const pthread_mutexattr_t * attr, int *type)
{
  return __pthread_mutexattr_gettype (attr, type);
}

int
pthread_mutexattr_init (pthread_mutexattr_t * attr)
{
  return __pthread_mutexattr_init (attr);
}

int
pthread_mutexattr_setprioceiling (pthread_mutexattr_t * attr, int prioceiling)
{
  return __pthread_mutexattr_setprioceiling (attr, prioceiling);
}

int
pthread_mutexattr_setprotocol (pthread_mutexattr_t * attr, int protocol)
{
  return __pthread_mutexattr_setprotocol (attr, protocol);
}

int
pthread_mutexattr_setpshared (pthread_mutexattr_t * attr, int pshared)
{
  return __pthread_mutexattr_setpshared (attr, pshared);
}

int
pthread_mutexattr_settype (pthread_mutexattr_t * attr, int type)
{
  return __pthread_mutexattr_settype (attr, type);
}

/* Synchronisation */

int
pthread_cond_destroy (pthread_cond_t * cond)
{
  return __pthread_cond_destroy (cond);
}

int
pthread_cond_init (pthread_cond_t * cond, const pthread_condattr_t * attr)
{
  return __pthread_cond_init (cond, attr);
}

int
pthread_cond_signal (pthread_cond_t * cond)
{
  return __pthread_cond_signal (cond);
}

int
pthread_cond_broadcast (pthread_cond_t * cond)
{
  return __pthread_cond_broadcast (cond);
}

int
pthread_cond_timedwait (pthread_cond_t * cond,
			pthread_mutex_t * mutex,
			const struct timespec *abstime)
{
  return __pthread_cond_timedwait (cond, mutex, abstime);
}

int
pthread_cond_wait (pthread_cond_t * cond, pthread_mutex_t * mutex)
{
  return __pthread_cond_wait (cond, mutex);
}

int
pthread_condattr_init (pthread_condattr_t * condattr)
{
  return __pthread_condattr_init (condattr);
}

int
pthread_condattr_destroy (pthread_condattr_t * condattr)
{
  return __pthread_condattr_destroy (condattr);
}

int
pthread_condattr_getpshared (const pthread_condattr_t * attr, int *pshared)
{
  return __pthread_condattr_getpshared (attr, pshared);
}

int
pthread_condattr_setpshared (pthread_condattr_t * attr, int pshared)
{
  return __pthread_condattr_setpshared (attr, pshared);
}

/* Scheduling */

int
pthread_getconcurrency (void)
{
  return __pthread_getconcurrency ();
}

int
pthread_setconcurrency (int new_level)
{
  return __pthread_setconcurrency (new_level);
}




int
pthread_getschedparam (pthread_t thread, int *policy,
		       struct sched_param *param)
{
  return __pthread_getschedparam (thread, policy, param);
}

int
pthread_setschedparam (pthread_t thread, int policy,
		       const struct sched_param *param)
{
  return __pthread_setschedparam (thread, policy, param);
}


/* Cancelability */

int
pthread_cancel (pthread_t thread)
{
  return __pthread_cancel (thread);
}



int
pthread_setcancelstate (int state, int *oldstate)
{
  return __pthread_setcancelstate (state, oldstate);
}

int
pthread_setcanceltype (int type, int *oldtype)
{
  return __pthread_setcanceltype (type, oldtype);
}

void
pthread_testcancel (void)
{
  __pthread_testcancel ();
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
