/*
 * Copyright (c) 2011 Aeroflex Gaisler
 *
 * BSD license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <sys/types.h>
#include <sys/lock.h>
#include <asm-leon/queue.h>
/*#include <sys/fsu_pthread_mutex.h>*/

typedef int pthread_protocol_t;
typedef
TAILQ_HEAD (pthread_queue, pthread) *
  pthread_queue_t;

#define pthread_mutex_t_defined
     typedef struct pthread_mutex
     {
       struct pthread_queue queue;
       char lock;
       struct pthread *owner;
       int flags;
       int count;
       int prioceiling;
       pthread_protocol_t protocol;
       int prev_max_ceiling_prio;
         TAILQ_ENTRY (pthread_mutex) dbglist;
       char *dbgname;
       int _fitothers[16];
     } pthread_mutex_t;

     typedef struct
     {
       int flags;
       int prioceiling;
       pthread_protocol_t protocol;
     } pthread_mutexattr_t;


     int (*__lbst_pthread_mutex_init) (pthread_mutex_t * __mutex,
				       pthread_mutexattr_t * __mutex_attr) =
  0;
     int (*__lbst_pthread_mutex_destroy) (pthread_mutex_t * __mutex) = 0;
     int (*__lbst_pthread_mutex_trylock) (pthread_mutex_t * __mutex) = 0;
     int (*__lbst_pthread_mutex_lock) (pthread_mutex_t * __mutex) = 0;
     int (*__lbst_pthread_mutex_unlock) (pthread_mutex_t * __mutex) = 0;
     int (*__lbst_pthread_mutexattr_init) (pthread_mutexattr_t * __attr) = 0;
     int (*__lbst_pthread_mutexattr_destroy) (pthread_mutexattr_t * __attr) =
  0;
     int (*__lbst_pthread_mutexattr_settype) (pthread_mutexattr_t * __attr,
					      int __kind) = 0;

     int ___st_pthread_mutex_init (mutex, attr)
     pthread_mutex_t *mutex;
     pthread_mutexattr_t *attr;
{
  if (__lbst_pthread_mutex_init)
    {
      return __lbst_pthread_mutex_init (mutex, attr);
    }
  return 0;
}

int
___st_pthread_mutex_destroy (mutex)
     pthread_mutex_t *mutex;
{
  if (__lbst_pthread_mutex_destroy)
    {
      return __lbst_pthread_mutex_destroy (mutex);
    }
  return 0;
}

int
___st_pthread_mutex_lock (mutex)
     pthread_mutex_t *mutex;
{
  if (__lbst_pthread_mutex_lock)
    {
      return __lbst_pthread_mutex_lock (mutex);
    }
  return 0;
}

int
___st_pthread_mutex_trylock (mutex)
     pthread_mutex_t *mutex;
{
  if (__lbst_pthread_mutex_trylock)
    {
      return __lbst_pthread_mutex_trylock (mutex);
    }
  return 0;
}

int
___st_pthread_mutex_unlock (mutex)
     pthread_mutex_t *mutex;
{
  if (__lbst_pthread_mutex_unlock)
    {
      return __lbst_pthread_mutex_unlock (mutex);
    }
  return 0;
}

int
___st_pthread_mutexattr_init (attr)
     pthread_mutexattr_t *attr;
{
  if (__lbst_pthread_mutexattr_init)
    {
      return __lbst_pthread_mutexattr_init (attr);
    }
  return (0);
}

int
___st_pthread_mutexattr_destroy (attr)
     pthread_mutexattr_t *attr;
{
  if (__lbst_pthread_mutexattr_destroy)
    {
      return __lbst_pthread_mutexattr_destroy (attr);
    }
  return 0;
}

int
___st_pthread_mutexattr_settype (attr, kind)
     pthread_mutexattr_t *attr;
     int kind;
{
  if (__lbst_pthread_mutexattr_settype)
    {
      return __lbst_pthread_mutexattr_settype (attr, kind);
    }
  return 0;
}

#include <asm-leon/elfmacro.h>

weak_alias (___st_pthread_mutex_init, __st_pthread_mutex_init)
weak_alias (___st_pthread_mutex_destroy, __st_pthread_mutex_destroy)
weak_alias (___st_pthread_mutex_trylock, __st_pthread_mutex_trylock)
weak_alias (___st_pthread_mutex_lock, __st_pthread_mutex_lock)
weak_alias (___st_pthread_mutex_unlock, __st_pthread_mutex_unlock)
weak_alias (___st_pthread_mutexattr_init, __st_pthread_mutexattr_init)
weak_alias (___st_pthread_mutexattr_destroy, __st_pthread_mutexattr_destroy)
weak_alias (___st_pthread_mutexattr_settype, __st_pthread_mutexattr_settype)
/* /\* #ifndef weak_extern *\/ */
/* /\* #define weak_extern(symbol) _weak_extern (symbol) *\/ */
/* /\* #define _weak_extern(symbol) asm (".weak " #symbol); *\/ */
/* /\* #endif *\/ */
/* /\* weak_extern (__pthread_mutex_init) *\/ */
/* /\* weak_extern (__pthread_mutex_destroy) *\/ */
/* /\* weak_extern (__pthread_mutex_lock) *\/ */
/* /\* weak_extern (__pthread_mutex_trylock) *\/ */
/* /\* weak_extern (__pthread_mutex_unlock) *\/ */
/* /\* weak_extern (__pthread_mutexattr_init) *\/ */
/* /\* weak_extern (__pthread_mutexattr_destroy) *\/ */
/* /\* weak_extern (__pthread_mutexattr_settype) *\/ */
/* /\* weak_extern (__pthread_once) *\/ */
/* /\* weak_extern (__pthread_initialize) *\/ */
/* /\* Initialize the named lock variable, leaving it in a consistent, unlocked */
/*    state.  *\/ */
/* #define __libc_lock_init(NAME) \ */
/*   (__pthread_mutex_init != NULL ? __pthread_mutex_init (&(NAME), NULL) : 0); */
/* /\* Same as last but this time we initialize a recursive mutex.  *\/ */
/* #define __libc_lock_init_recursive(NAME) \ */
/*   do {									      \ */
/*     if (__pthread_mutex_init != NULL)					      \ */
/*       {									      \ */
/* 	pthread_mutexattr_t __attr;					      \ */
/* 	__pthread_mutexattr_init (&__attr);				      \ */
/* 	__pthread_mutexattr_settype (&__attr, PTHREAD_MUTEX_RECURSIVE_NP); \ */
/* 	__pthread_mutex_init (&(NAME), &__attr);			      \ */
/* 	__pthread_mutexattr_destroy (&__attr);				      \ */
/*       }									      \ */
/*   } while (0); */
/* /\* Finalize the named lock variable, which must be locked.  It cannot be */
/*    used again until __libc_lock_init is called again on it.  This must be */
/*    called on a lock variable before the containing storage is reused.  *\/ */
/* //#define __libc_lock_fini(NAME)              (__pthread_mutex_destroy != NULL ? __pthread_mutex_destroy (&(NAME)) : 0) */
/* #define __libc_lock_fini(NAME)              (__st_pthread_mutex_destroy (&(NAME))) */
/* /\* Finalize recursive named lock.  *\/ */
/* #define __libc_lock_fini_recursive(NAME)     __libc_lock_fini (NAME) */
/* /\* Lock the named lock variable.  *\/ */
/* //#define __libc_lock_lock(NAME)              (__pthread_mutex_lock != NULL ? __pthread_mutex_lock (&(NAME)) : 0) */
/* #define __libc_lock_lock(NAME)              (__st_pthread_mutex_lock (&(NAME))) */
/* /\* Lock the recursive named lock variable.  *\/ */
/* #define __libc_lock_lock_recursive(NAME)     __libc_lock_lock (NAME) */
/* /\* Try to lock the named lock variable.  *\/ */
/* //#define __libc_lock_trylock(NAME)           (__pthread_mutex_trylock != NULL ? __pthread_mutex_trylock (&(NAME)) : 0) */
/* #define __libc_lock_trylock(NAME)           (__st_pthread_mutex_trylock (&(NAME))) */
/* /\* Try to lock the recursive named lock variable.  *\/ */
/* #define __libc_lock_trylock_recursive(NAME)  __libc_lock_trylock (NAME) */
/* /\* Unlock the named lock variable.  *\/ */
/* //#define __libc_lock_unlock(NAME)            (__pthread_mutex_unlock != NULL ? __pthread_mutex_unlock (&(NAME)) : 0) */
/* #define __libc_lock_unlock(NAME)            (__st_pthread_mutex_unlock (&(NAME))) */
/* /\* Unlock the recursive named lock variable.  *\/ */
/* #define __libc_lock_unlock_recursive(NAME)   __libc_lock_unlock (NAME) */
/* extern int __st_pthread_mutex_init        (pthread_mutex_t *__mutex, pthread_mutexattr_t *__mutex_attr); */
/* extern int __st_pthread_mutex_destroy     (pthread_mutex_t *__mutex); */
/* extern int __st_pthread_mutex_trylock     (pthread_mutex_t *__mutex); */
/* extern int __st_pthread_mutex_lock        (pthread_mutex_t *__mutex); */
/* extern int __st_pthread_mutex_unlock      (pthread_mutex_t *__mutex); */
/* extern int __st_pthread_mutexattr_init    (pthread_mutexattr_t *__attr); */
/* extern int __st_pthread_mutexattr_destroy (pthread_mutexattr_t *__attr); */
/* extern int __st_pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind); */
/* /\* /\\* Functions that are used by this file and are internal to the GNU C library.  *\\/ *\/ */
/* /\* extern int __pthread_mutex_init        (pthread_mutex_t *__mutex, pthread_mutexattr_t *__mutex_attr); *\/ */
/* /\* extern int __pthread_mutex_destroy     (pthread_mutex_t *__mutex); *\/ */
/* /\* extern int __pthread_mutex_trylock     (pthread_mutex_t *__mutex); *\/ */
/* /\* extern int __pthread_mutex_lock        (pthread_mutex_t *__mutex); *\/ */
/* /\* extern int __pthread_mutex_unlock      (pthread_mutex_t *__mutex); *\/ */
/* /\* extern int __pthread_mutexattr_init    (pthread_mutexattr_t *__attr); *\/ */
/* /\* extern int __pthread_mutexattr_destroy (pthread_mutexattr_t *__attr); *\/ */
/* /\* extern int __pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind); *\/ */
/* /\* /\\* Make the pthread functions weak so that we can elide them from *\/ */
/* /\*    single-threaded processes.  *\\/ *\/ */
/* /\* #ifndef weak_extern *\/ */
/* /\* #define weak_extern(symbol) _weak_extern (symbol) *\/ */
/* /\* #define _weak_extern(symbol) asm (".weak " #symbol); *\/ */
/* /\* #endif *\/ */
/* /\* weak_extern (__pthread_mutex_init) *\/ */
/* /\* weak_extern (__pthread_mutex_destroy) *\/ */
/* /\* weak_extern (__pthread_mutex_lock) *\/ */
/* /\* weak_extern (__pthread_mutex_trylock) *\/ */
/* /\* weak_extern (__pthread_mutex_unlock) *\/ */
/* /\* weak_extern (__pthread_mutexattr_init) *\/ */
/* /\* weak_extern (__pthread_mutexattr_destroy) *\/ */
/* /\* weak_extern (__pthread_mutexattr_settype) *\/ */
/* /\* weak_extern (__pthread_once) *\/ */
/* /\* weak_extern (__pthread_initialize) *\/ */
