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


/* Defines. (These are correctly defined here as per
   http://www.opengroup.org/onlinepubs/7908799/xsh/pthread.h.html */

/* FIXME: this should allocate a new cond variable, and return the value  that
 would normally be written to the passed parameter of pthread_cond_init(lvalue, NULL); */
// #define PTHREAD_COND_INITIALIZER 0

#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED  1
#define PTHREAD_DESTRUCTOR_ITERATIONS 1
/* Tls has 64 items for pre win2000 - and we don't want to use them all :]
 * Before committing discuss this with the list
 */
#define PTHREAD_KEYS_MAX 32
#define PTHREAD_CREATE_DETACHED 1
/* the default : joinable */
#define PTHREAD_CREATE_JOINABLE 0


/* these shouldn't be defined here but in sys/types.
 * defining in sys/types mught also allow us to override them for the internal functions
 * more easily (internal sys/types vs external sys/type - dev thoughts on this? 
 */
  typedef void *pthread_t;
  typedef void *pthread_mutex_t;

  typedef void *pthread_key_t;
  typedef void *pthread_attr_t;
  typedef void *pthread_mutexattr_t;
  typedef void *pthread_condattr_t;
  typedef void *pthread_cond_t;

/*  ThreadCreation */
  int pthread_create (pthread_t * thread, const pthread_attr_t * attr,
		      void *(*)(void *), void *arg);
  int pthread_attr_init (pthread_attr_t * attr);
  int pthread_attr_destroy (pthread_attr_t * attr);
  int pthread_attr_setdetachstate (pthread_attr_t *, int);
  int pthread_attr_getdetachstate (const pthread_attr_t *, int *);
  int pthread_attr_setstacksize (pthread_attr_t * attr, size_t size);
  int pthread_attr_getstacksize (pthread_attr_t * attr, size_t * size);

/* Condition variables */
  int pthread_cond_broadcast (pthread_cond_t *);
  int pthread_cond_destroy (pthread_cond_t *);
  int pthread_cond_init (pthread_cond_t *, const pthread_condattr_t *);
  int pthread_cond_signal (pthread_cond_t *);
  int pthread_cond_timedwait (pthread_cond_t *,
			      pthread_mutex_t *, const struct timespec *);
  int pthread_cond_wait (pthread_cond_t *, pthread_mutex_t *);
  int pthread_condattr_destroy (pthread_condattr_t *);
  int pthread_condattr_getpshared (const pthread_condattr_t *, int *);
  int pthread_condattr_init (pthread_condattr_t *);
  int pthread_condattr_setpshared (pthread_condattr_t *, int);


/* Thread Control */
  int pthread_detach (pthread_t thread);
  int pthread_join (pthread_t thread, void **value_ptr);

/* Thread Exit */
  void pthread_exit (void *value_ptr);

/* Thread SpecificData */
  int pthread_key_create (pthread_key_t *, void (*)(void *));
  int pthread_key_delete (pthread_key_t * key);
  int pthread_setspecific (pthread_key_t key, const void *value);
  void *pthread_getspecific (pthread_key_t key);

/* Thread signal (should be in signal.h) */
  int pthread_kill (pthread_t * thread, int sig);
  int pthread_sigmask (int operation, const sigset_t * set,
		       sigset_t * old_set);

/*  ID */
  pthread_t pthread_self ();
  int pthread_equal (pthread_t t1, pthread_t t2);

/* Mutexes  */
  int pthread_mutex_init (pthread_mutex_t * mutex,
			  const pthread_mutexattr_t *);
  int pthread_mutex_lock (pthread_mutex_t * mutext);
  int pthread_mutex_trylock (pthread_mutex_t * mutext);
  int pthread_mutex_unlock (pthread_mutex_t * mutext);
  int pthread_mutex_destroy (pthread_mutex_t * mutext);

#ifdef __cplusplus
}
#endif

#endif				/* _PTHREAD_H */
