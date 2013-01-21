/* sys/lock.h

   Copyright 2004, 2005 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_LOCK_H_
#define _SYS_LOCK_H_

typedef void *_LOCK_T;
#define _LOCK_RECURSIVE_T _LOCK_T

/*
 * This must match cygwins PTHREAD_XXX_MUTEX_INITIALIZER_NP which are
 * defined in <pthread.h>
 */
#define _LOCK_T_RECURSIVE_INITIALIZER ((_LOCK_T)18)
#define _LOCK_T_INITIALIZER ((_LOCK_T)19)

#define __LOCK_INIT(CLASS,NAME) \
  CLASS _LOCK_T NAME = _LOCK_T_INITIALIZER;
#define __LOCK_INIT_RECURSIVE(CLASS,NAME) \
  CLASS _LOCK_T NAME = _LOCK_T_RECURSIVE_INITIALIZER;

#define __lock_init(__lock) __cygwin_lock_init(&__lock)
#define __lock_init_recursive(__lock) __cygwin_lock_init_recursive(&__lock)
#define __lock_close(__lock) __cygwin_lock_fini(&__lock)
#define __lock_close_recursive(__lock) __cygwin_lock_fini(&__lock)
#define __lock_acquire(__lock) __cygwin_lock_lock(&__lock)
#define __lock_acquire_recursive(__lock) __cygwin_lock_lock(&__lock)
#define __lock_try_acquire(lock) __cygwin_lock_trylock(&__lock)
#define __lock_try_acquire_recursive(lock) __cygwin_lock_trylock(&__lock)
#define __lock_release(__lock) __cygwin_lock_unlock(&__lock)
#define __lock_release_recursive(__lock) __cygwin_lock_unlock(&__lock)

#ifdef __cplusplus
extern "C"
{
#endif
void __cygwin_lock_init(_LOCK_T *);
void __cygwin_lock_init_recursive(_LOCK_T *);
void __cygwin_lock_fini(_LOCK_T *);
void __cygwin_lock_lock(_LOCK_T *);
int __cygwin_lock_trylock(_LOCK_T *);
void __cygwin_lock_unlock(_LOCK_T *);
#ifdef __cplusplus
}
#endif

#endif
