#ifndef __SYS_LOCK_H__
#define __SYS_LOCK_H__

#include <newlib.h>
#include <_ansi.h>

typedef unsigned int _LOCK_T;
typedef unsigned int _LOCK_RECURSIVE_T;

#define __LOCK_INIT(CLASS,LOCK) CLASS _LOCK_T LOCK = 0;
#define __LOCK_INIT_RECURSIVE(CLASS,LOCK) __LOCK_INIT(CLASS,LOCK)

#define __lock_init(LOCK) LOCK = 0
#define __lock_init_recursive(LOCK) LOCK = 0
#define __lock_close(LOCK) ((void)0)
#define __lock_close_recursive(LOCK) ((void) 0)
#define __lock_acquire(LOCK) __gcn_lock_acquire (&LOCK)
#define __lock_acquire_recursive(LOCK) \
   __gcn_lock_acquire_recursive (&LOCK)
#define __lock_try_acquire(LOCK) __gcn_try_lock_acquire (&LOCK)
#define __lock_try_acquire_recursive(LOCK) \
  __gcn_lock_try_acquire_recursive (&LOCK)
#define __lock_release(LOCK) __gcn_lock_release (&LOCK)
#define __lock_release_recursive(LOCK) \
  __gcn_lock_release_recursive (&LOCK)


int __gcn_try_lock_acquire (_LOCK_T *lock_ptr);
void __gcn_lock_acquire (_LOCK_T *lock_ptr);
void __gcn_lock_release (_LOCK_T *lock_ptr);
int __gcn_lock_try_acquire_recursive (_LOCK_T *lock_ptr);
void __gcn_lock_acquire_recursive (_LOCK_T *lock_ptr);
void __gcn_lock_release_recursive (_LOCK_T *lock_ptr);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_LOCK_H__ */
