/* Weak-linked stub locking functions.

   Intended that you can replace the _lock_xxx functions with your own
   lock implementation at link time, if needed.

   See comments in sys/lock.h for notes about lock initialization.
*/
#include <sys/lock.h>

void __dummy_lock(_lock_t *lock) { }
int __dummy_lock_try(_lock_t *lock) { return 0; }

void _lock_init(_lock_t *lock) __attribute__((weak, alias("__dummy_lock")));
void _lock_init_recursive(_lock_t *lock) __attribute__((weak, alias("__dummy_lock")));
void _lock_close(_lock_t *lock) __attribute__((weak, alias("__dummy_lock")));
void _lock_close_recursive(_lock_t *lock) __attribute__((weak, alias("__dummy_lock")));
void _lock_acquire(_lock_t *lock) __attribute__((weak, alias("__dummy_lock")));
void _lock_acquire_recursive(_lock_t *lock) __attribute__((weak, alias("__dummy_lock")));
int _lock_try_acquire(_lock_t *lock) __attribute__((weak, alias("__dummy_lock_try")));
int _lock_try_acquire_recursive(_lock_t *lock) __attribute__((weak, alias("__dummy_lock_try")));
void _lock_release(_lock_t *lock) __attribute__((weak, alias("__dummy_lock")));
void _lock_release_recursive(_lock_t *lock) __attribute__((weak, alias("__dummy_lock")));
