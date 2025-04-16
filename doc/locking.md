# Locking in Picolibc

Picolibc uses locking and atomics to protect global state between
re-entrant calls. The amount of global state protected by locks has
been minimized so that applications can avoid needing them for
most common operations.

## Where Picolibc uses locking

Picolibc has a single global lock for APIs that share global
data. That includes:

 * malloc family
 * onexit/atexit
 * arc4random
 * getenv/setenv
 * functions using timezones (localtime, et al)
 * legacy stdio globals

Tinystdio (the default stdio) uses per-file locks for the buffered
POSIX file backend, but it doesn't require any locks for the bulk of
the implementation. You can enable POSIX-compliant locking in
tinystdio with -Dstdio-locking-true. That will prevent I/O from stdio
operations from interleaving between threads.

The legacy stdio implementation is full of locking, and has per-file
locks for every operation.

## Where Picolibc uses atomics

Picolibc also uses atomics to protect other data structures while
avoiding the need for locking:

 * getc/ungetc in tinystdio
 * signal handling

## Configuration options controlling locking

There is only one configuration option related to locking:

 * single-thread. When 'true', all locking operations are elided from
   the library. Re-entrant usage of the library will result in
   undefined behavior.

Atomics will still be used as defined above, so disabling locking
will still allow safe re-entrancy for those parts of the library.

## Retargetable locking API

When -Dsingle-thread is not selected, Picolibc uses the following
interface. Picolibc provides stubs for all of these functions that do
not perform locking, so a single threaded application can still use a
library compiled to enable locking. An application needing locking
would provide a real implementation of the API.

This API requires recursive mutexes; if the underlying implementation
only provides non-recursive mutexes, a suitable wrapper implementing
recursive mutexes will be required for the recursive APIs. All APIs
involving recursive mutexes contain `recursive` in their names.

In this section, *the locking implementation* refers to the external
implementation of this API.

When implementing this API every function and variable must be
defined. If not, the application will fail to link as the default
implementation will be used to satisfy any missing symbol and other
symbols will collide with the application versions.

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
