#ifndef __SYS_LOCK_H__
#define __SYS_LOCK_H__

/* dummy lock routines for single-threaded aps */

typedef int _LOCK_T;
typedef int _LOCK_RECURSIVE_T;

#define __LOCK_INIT(class,lock) static int lock = 0;
#define __LOCK_INIT_RECURSIVE(class,lock) static int lock = 0;
#define __lock_init(lock) {}
#define __lock_init_recursive(lock) {}
#define __lock_close(lock) {}
#define __lock_close_recursive(lock) {}
#define __lock_acquire(lock) {}
#define __lock_acquire_recursive(lock) {}
#define __lock_try_acquire(lock) {}
#define __lock_try_acquire_recursive(lock) {}
#define __lock_release(lock) {}
#define __lock_release_recursive(lock) {}

#endif /* __SYS_LOCK_H__ */
