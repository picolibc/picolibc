/* Copyright (c) 2002 Jeff Johnston  <jjohnstn@redhat.com> */
#ifndef __SYS_LOCK_H__
#define __SYS_LOCK_H__

#include <sys/cdefs.h>

#ifdef __SINGLE_THREAD

/* dummy lock routines for single-threaded aps */

#define __LOCK_INIT(lock)
#define __LOCK_INIT_RECURSIVE(lock)
#define __lock_init(lock) ((void) 0)
#define __lock_init_recursive(lock) ((void) 0)
#define __lock_close(lock) ((void) 0)
#define __lock_close_recursive(lock) ((void) 0)
#define __lock_acquire(lock) ((void) 0)
#define __lock_acquire_recursive(lock) ((void) 0)
#define __lock_release(lock) ((void) 0)
#define __lock_release_recursive(lock) ((void) 0)

#else

_BEGIN_STD_C

struct __lock;
typedef struct __lock * _LOCK_T;

#define _LOCK_RECURSIVE_T _LOCK_T

#define __LOCK_INIT(lock) extern struct __lock __lock_ ## lock;
#define __LOCK_INIT_RECURSIVE(lock) __LOCK_INIT(lock)

void __retarget_lock_init(_LOCK_T *lock);
void __retarget_lock_init_recursive(_LOCK_T *lock);
void __retarget_lock_close(_LOCK_T lock);
void __retarget_lock_close_recursive(_LOCK_T lock);
void __retarget_lock_acquire(_LOCK_T lock);
void __retarget_lock_acquire_recursive(_LOCK_T lock);
void __retarget_lock_release(_LOCK_T lock);
void __retarget_lock_release_recursive(_LOCK_T lock);

#define __lock_init(lock) __retarget_lock_init(&lock)
#define __lock_init_recursive(lock) __retarget_lock_init_recursive(&lock)
#define __lock_close(lock) __retarget_lock_close(lock)
#define __lock_close_recursive(lock) __retarget_lock_close_recursive(lock)
#define __lock_acquire(lock) __retarget_lock_acquire(lock)
#define __lock_acquire_recursive(lock) __retarget_lock_acquire_recursive(lock)
#define __lock_release(lock) __retarget_lock_release(lock)
#define __lock_release_recursive(lock) __retarget_lock_release_recursive(lock)

_END_STD_C

#endif /* !defined(__SINGLE_THREAD) */

#define __LIBC_LOCK()	__lock_acquire_recursive(&__lock___libc_recursive_mutex)
#define __LIBC_UNLOCK()	__lock_release_recursive(&__lock___libc_recursive_mutex)
__LOCK_INIT_RECURSIVE(__libc_recursive_mutex)

#endif /* __SYS_LOCK_H__ */
