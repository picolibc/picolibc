# Locking in Picolibc

When newlib-multithread and newlib-retargetable-locking are enabled,
Picolibc uses the following locking API. Picolibc provides stubs for
all of these functions that do not actually perform any locking, so
that a single-threaded application can still use a Picolibc compiled
with locking enabled. An application needing locking must provide a
real implementation of the locking API.

## Where Picolibc uses locking

Picolibc has a single global lock for APIs that share global
data. That includes:

 * malloc family
 * onexit/atexit
 * arc4random
 * functions using timezones (localtime, et al)
 * legacy stdio struct reent globals

Tinystdio (the default stdio) uses per-file locks for the buffered
POSIX file backend, but it doesn't require any locks for the bulk of
the implementation. It uses atomic exchanges to handle the one
reentrancy issue related to ungetc/getc.

The legacy stdio implementation is full of locking, and has per-file
locks for every operation.

## Configuration options controlling locking

There are two configuration options related to locking:

 * newlib-retargetable-locking. When 'true', locking operations are
   enabled and performed by the retargetable locking API described below.

 * newlib-multithread. When 'true', the library is built with
   multithreading support enabled and uses the locking operations.

Picolibc inherits these options from newlib, and they're so
interrelated as to make them effectively co-dependent, so users must
set them to the same value.

## Retargetable locking API

When newlib-multithread and newlib-retargetable-locking are enabled
enabled, Picolibc uses the following interface. Picolibc provides
stubs for all of these functions that do not perform locking, so a
single threaded application can still use a library compiled to enable
locking. An application needing locking would provide a real
implementation of the API.

This API requires recursive mutexes; if the underlying implementation
only provides non-recursive mutexes, a suitable wrapper implementing
recursive mutexes will be required for the recursive APIs. All APIs
involving recursive mutexes contain `recursive` in their names.

In this section, *the locking implementation* refers to the external
implementation of this API.

### `struct __lock; typedef struct __lock *_LOCK_T;`

This struct is only referenced by picolibc, not defined. The
locking implementation may define it as necessary.

### `extern struct __lock __libc_recursive_mutex;`

This is the single global lock used for most libc locking. It must be
defined in the locking implementation in such a way as to not require
any runtime initialization.

### `void __retarget_lock_init(_LOCK_T *lock)`

This is used by tinystdio to initialize the lock in a newly allocated
FILE.

### `void __retarget_lock_acquire(_LOCK_T lock)`

Acquire a non-recursive mutex. A thread will only acquire the mutex once.

### `void __retarget_lock_release(_LOCK_T lock)`

Release a non-recursive mutex.

### `void __retarget_lock_close(_LOCK_T lock)`

This is used by tinystdio to de-initialize a lock from a FILE which is
being closed.

### `void __retarget_lock_init_recursive(_LOCK_T *lock)`

This is used by the legacy stdio code to initialize the lock in a
newly allocated FILE.

### `void __retarget_lock_acquire_recursive(_LOCK_T lock)`

Acquire a recursive mutex. A thread may acquire a recursive
mutex multiple times.

### `void __retarget_lock_release_recursive(_LOCK_T lock)`

Release a recursive mutex. A thread thread must release the mutex as
many times as it has acquired it before the mutex is unlocked.

### `void __retarget_lock_close_recursive(_LOCK_T lock)`

This is used by the legacy stdio code to de-initialize a lock from a
FILE which is being closed.
