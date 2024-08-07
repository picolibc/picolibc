/* Copyright (c) 2002 Jeff Johnston  <jjohnstn@redhat.com> */
#ifndef __SYS_LOCK_H__
#define __SYS_LOCK_H__

/* dummy lock routines for single-threaded aps */

#include <sys/cdefs.h>

#if !defined(_RETARGETABLE_LOCKING)

typedef int _LOCK_T;
typedef int _LOCK_RECURSIVE_T;

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

#ifdef __cplusplus
extern "C" {
#endif

struct __lock;
typedef struct __lock * _LOCK_T;
#define _LOCK_RECURSIVE_T _LOCK_T

#define __LOCK_INIT(lock) extern struct __lock __lock_ ## lock;
#define __LOCK_INIT_RECURSIVE(lock) __LOCK_INIT(lock)

extern void __retarget_lock_init(_LOCK_T *lock);
#define __lock_init(lock) __retarget_lock_init(&lock)
extern void __retarget_lock_init_recursive(_LOCK_T *lock);
#define __lock_init_recursive(lock) __retarget_lock_init_recursive(&lock)
extern void __retarget_lock_close(_LOCK_T lock);
#define __lock_close(lock) __retarget_lock_close(lock)
extern void __retarget_lock_close_recursive(_LOCK_T lock);
#define __lock_close_recursive(lock) __retarget_lock_close_recursive(lock)
extern void __retarget_lock_acquire(_LOCK_T lock);
#define __lock_acquire(lock) __retarget_lock_acquire(lock)
extern void __retarget_lock_acquire_recursive(_LOCK_T lock);
#define __lock_acquire_recursive(lock) __retarget_lock_acquire_recursive(lock)
extern void __retarget_lock_release(_LOCK_T lock);
#define __lock_release(lock) __retarget_lock_release(lock)
extern void __retarget_lock_release_recursive(_LOCK_T lock);
#define __lock_release_recursive(lock) __retarget_lock_release_recursive(lock)

#ifdef __cplusplus
}
#endif

#endif /* !defined(_RETARGETABLE_LOCKING) */

#define __LIBC_LOCK()	__lock_acquire_recursive(&__lock___libc_recursive_mutex)
#define __LIBC_UNLOCK()	__lock_release_recursive(&__lock___libc_recursive_mutex)
__LOCK_INIT_RECURSIVE(__libc_recursive_mutex)

#endif /* __SYS_LOCK_H__ */
