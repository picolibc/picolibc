/* pthread.h: POSIX pthread interface

   Written by Marco Fuykschot <marco@ddi.nl>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <sys/types.h>
#include <signal.h>
#include <sched.h>
#include <time.h>

#ifndef _PTHREAD_H
#define _PTHREAD_H

#ifdef __cplusplus
extern "C"
{
#endif


/* Defines. (These are correctly defined here as per
   http://www.opengroup.org/onlinepubs/7908799/xsh/pthread.h.html */

#define PTHREAD_CANCEL_ASYNCHRONOUS 1
/* defaults are enable, deferred */
#define PTHREAD_CANCEL_ENABLE 0
#define PTHREAD_CANCEL_DEFERRED 0
#define PTHREAD_CANCEL_DISABLE 1
#define PTHREAD_CANCELED ((void *)-1)
/* this should be a value that can never be a valid address */
#define PTHREAD_COND_INITIALIZER (pthread_cond_t)21
#define PTHREAD_CREATE_DETACHED 1
/* the default : joinable */
#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_EXPLICIT_SCHED 1
#define PTHREAD_INHERIT_SCHED 0
#define PTHREAD_MUTEX_RECURSIVE 0
#define PTHREAD_MUTEX_ERRORCHECK 1
#define PTHREAD_MUTEX_NORMAL 2
#define PTHREAD_MUTEX_DEFAULT PTHREAD_MUTEX_NORMAL
/* this should be too low to ever be a valid address */
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP (pthread_mutex_t)18
#define PTHREAD_NORMAL_MUTEX_INITIALIZER_NP (pthread_mutex_t)19
#define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP (pthread_mutex_t)20
#define PTHREAD_MUTEX_INITIALIZER PTHREAD_NORMAL_MUTEX_INITIALIZER_NP
#define PTHREAD_ONCE_INIT { PTHREAD_MUTEX_INITIALIZER, 0 }
#if defined(_POSIX_THREAD_PRIO_INHERIT) && _POSIX_THREAD_PRIO_INHERIT >= 0
#define PTHREAD_PRIO_NONE 0
#define PTHREAD_PRIO_INHERIT 1
#define PTHREAD_PRIO_PROTECT 2
#endif
#define PTHREAD_PROCESS_SHARED 1
#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_RWLOCK_INITIALIZER (pthread_rwlock_t)22
/* process is the default */
#define PTHREAD_SCOPE_PROCESS 0
#define PTHREAD_SCOPE_SYSTEM 1
#define PTHREAD_BARRIER_SERIAL_THREAD (-1)

/* Register Fork Handlers */
int pthread_atfork (void (*)(void), void (*)(void), void (*)(void));

/* Attributes */
int pthread_attr_destroy (pthread_attr_t *);
int pthread_attr_getdetachstate (const pthread_attr_t *, int *);
int pthread_attr_getguardsize (const pthread_attr_t *, size_t *);
int pthread_attr_getinheritsched (const pthread_attr_t *, int *);
int pthread_attr_getschedparam (const pthread_attr_t *, struct sched_param *);
int pthread_attr_getschedpolicy (const pthread_attr_t *, int *);
int pthread_attr_getscope (const pthread_attr_t *, int *);
int pthread_attr_init (pthread_attr_t *);
int pthread_attr_setdetachstate (pthread_attr_t *, int);
int pthread_attr_setguardsize (pthread_attr_t *, size_t);
int pthread_attr_setinheritsched (pthread_attr_t *, int);
int pthread_attr_setschedparam (pthread_attr_t *, const struct sched_param *);
int pthread_attr_setschedpolicy (pthread_attr_t *, int);
int pthread_attr_setscope (pthread_attr_t *, int);

#if __POSIX_VISIBLE >= 200112
int pthread_attr_getstack (const pthread_attr_t *, void **, size_t *);
int pthread_attr_setstack (pthread_attr_t *, void *, size_t);
#endif
#if __POSIX_VISIBLE < 200809
int pthread_attr_getstackaddr (const pthread_attr_t *, void **)
    __attribute__ ((__deprecated__));
int pthread_attr_setstackaddr (pthread_attr_t *, void *)
    __attribute__ ((__deprecated__));
#endif
int pthread_attr_getstacksize (const pthread_attr_t *, size_t *);
int pthread_attr_setstacksize (pthread_attr_t *, size_t);

int pthread_cancel (pthread_t);
/* Macros for cleanup_push and pop;
 * The function definitions are
void pthread_cleanup_push (void (*routine)(void*), void *arg);
void pthread_cleanup_pop (int execute);
*/
typedef void (*__cleanup_routine_type) (void *);
typedef struct _pthread_cleanup_handler
{
  __cleanup_routine_type function;
  void *arg;
  struct _pthread_cleanup_handler *next;
} __pthread_cleanup_handler;

void _pthread_cleanup_push (__pthread_cleanup_handler *handler);
void _pthread_cleanup_pop (int execute);

#define pthread_cleanup_push(_fn, _arg) { __pthread_cleanup_handler __cleanup_handler = \
					 { _fn, _arg, NULL }; \
					 _pthread_cleanup_push( &__cleanup_handler );
#define pthread_cleanup_pop(_execute) _pthread_cleanup_pop( _execute ); }

/* Condition variables */
int pthread_cond_broadcast (pthread_cond_t *);
int pthread_cond_destroy (pthread_cond_t *);
int pthread_cond_init (pthread_cond_t *, const pthread_condattr_t *);
int pthread_cond_signal (pthread_cond_t *);
int pthread_cond_timedwait (pthread_cond_t *,
			    pthread_mutex_t *, const struct timespec *);
int pthread_cond_wait (pthread_cond_t *, pthread_mutex_t *);
int pthread_condattr_destroy (pthread_condattr_t *);
int pthread_condattr_getclock (const pthread_condattr_t *, clockid_t *);
int pthread_condattr_getpshared (const pthread_condattr_t *, int *);
int pthread_condattr_init (pthread_condattr_t *);
int pthread_condattr_setclock (pthread_condattr_t *, clockid_t);
int pthread_condattr_setpshared (pthread_condattr_t *, int);

/* Barriers */
#if __POSIX_VISIBLE >= 200112
int pthread_barrierattr_init (pthread_barrierattr_t *);
int pthread_barrierattr_setpshared (pthread_barrierattr_t *, int);
int pthread_barrierattr_getpshared (const pthread_barrierattr_t *, int *);
int pthread_barrierattr_destroy (pthread_barrierattr_t *);
int pthread_barrier_init (pthread_barrier_t *,
                          const pthread_barrierattr_t *, unsigned);
int pthread_barrier_destroy (pthread_barrier_t *);
int pthread_barrier_wait (pthread_barrier_t *);
#endif

/* Threads */
int pthread_create (pthread_t *, const pthread_attr_t *,
		    void *(*)(void *), void *);
int pthread_detach (pthread_t);
int pthread_equal (pthread_t, pthread_t);
void pthread_exit (void *) __attribute__ ((__noreturn__));
#if __POSIX_VISIBLE >= 200112
int pthread_getcpuclockid (pthread_t, clockid_t *);
#endif
int pthread_getschedparam (pthread_t, int *, struct sched_param *);
void *pthread_getspecific (pthread_key_t);
int pthread_join (pthread_t, void **);
int pthread_key_create (pthread_key_t *, void (*)(void *));
int pthread_key_delete (pthread_key_t);

/* Mutex's */
int pthread_mutex_destroy (pthread_mutex_t *);
int pthread_mutex_getprioceiling (const pthread_mutex_t *, int *);
int pthread_mutex_init (pthread_mutex_t *, const pthread_mutexattr_t *);
int pthread_mutex_lock (pthread_mutex_t *);
int pthread_mutex_setprioceiling (pthread_mutex_t *, int, int *);
int pthread_mutex_timedlock (pthread_mutex_t *, const struct timespec *);
int pthread_mutex_trylock (pthread_mutex_t *);
int pthread_mutex_unlock (pthread_mutex_t *);
int pthread_mutexattr_destroy (pthread_mutexattr_t *);
int pthread_mutexattr_getprioceiling (const pthread_mutexattr_t *, int *);
int pthread_mutexattr_getprotocol (const pthread_mutexattr_t *, int *);
int pthread_mutexattr_getpshared (const pthread_mutexattr_t *, int *);
int pthread_mutexattr_gettype (const pthread_mutexattr_t *, int *);
int pthread_mutexattr_init (pthread_mutexattr_t *);
int pthread_mutexattr_setprioceiling (pthread_mutexattr_t *, int);
int pthread_mutexattr_setprotocol (pthread_mutexattr_t *, int);
int pthread_mutexattr_setpshared (pthread_mutexattr_t *, int);
int pthread_mutexattr_settype (pthread_mutexattr_t *, int);

/* Spinlocks */
#if __POSIX_VISIBLE >= 200112
int pthread_spin_destroy (pthread_spinlock_t *);
int pthread_spin_init (pthread_spinlock_t *, int);
int pthread_spin_lock (pthread_spinlock_t *);
int pthread_spin_trylock (pthread_spinlock_t *);
int pthread_spin_unlock (pthread_spinlock_t *);
#endif

/* RW Locks */
#if __XSI_VISIBLE >= 500 || __POSIX_VISIBLE >= 200112 || __cplusplus >= 201402L
int pthread_rwlock_destroy (pthread_rwlock_t *rwlock);
int pthread_rwlock_init (pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);
int pthread_rwlock_rdlock (pthread_rwlock_t *rwlock);
int pthread_rwlock_timedrdlock (pthread_rwlock_t *rwlock,
				const struct timespec *abstime);
int pthread_rwlock_tryrdlock (pthread_rwlock_t *rwlock);
int pthread_rwlock_wrlock (pthread_rwlock_t *rwlock);
int pthread_rwlock_timedwrlock (pthread_rwlock_t *rwlock,
				const struct timespec *abstime);
int pthread_rwlock_trywrlock (pthread_rwlock_t *rwlock);
int pthread_rwlock_unlock (pthread_rwlock_t *rwlock);
int pthread_rwlockattr_init (pthread_rwlockattr_t *rwlockattr);
int pthread_rwlockattr_getpshared (const pthread_rwlockattr_t *attr,
				   int *pshared);
int pthread_rwlockattr_setpshared (pthread_rwlockattr_t *attr, int pshared);
int pthread_rwlockattr_destroy (pthread_rwlockattr_t *rwlockattr);
#endif

int pthread_once (pthread_once_t *, void (*)(void));

#if __XSI_VISIBLE >= 500
/* Concurrency levels - X/Open interface */
int pthread_getconcurrency (void);
int pthread_setconcurrency (int);
#endif


pthread_t pthread_self (void);
int pthread_setcancelstate (int, int *);
int pthread_setcanceltype (int, int *);
int pthread_setschedparam (pthread_t, int, const struct sched_param *);
int pthread_setschedprio (pthread_t, int);
int pthread_setspecific (pthread_key_t, const void *);
void pthread_testcancel (void);

/* Non posix calls */

#if __GNU_VISIBLE
int pthread_getaffinity_np (pthread_t, size_t, cpu_set_t *);
int pthread_getattr_np (pthread_t, pthread_attr_t *);
int pthread_getname_np (pthread_t, char *, size_t) __attribute__((__nonnull__(2)));
int pthread_setaffinity_np (pthread_t, size_t, const cpu_set_t *);
int pthread_setname_np (pthread_t, const char *) __attribute__((__nonnull__(2)));
int pthread_sigqueue (pthread_t *, int, const union sigval);
int pthread_timedjoin_np (pthread_t, void **, const struct timespec *);
int pthread_tryjoin_np (pthread_t, void **);
int pthread_yield (void);
#endif
#if __MISC_VISIBLE /* HP-UX, others? */
int pthread_suspend (pthread_t);
int pthread_continue (pthread_t);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _PTHREAD_H */
