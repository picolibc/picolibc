/* Copyright (c) 2016 Thomas Preud'homme <thomas.preudhomme@arm.com> */
/*
FUNCTION
<<__retarget_lock_init>>, <<__retarget_lock_init_recursive>>, <<__retarget_lock_close>>, <<__retarget_lock_close_recursive>>, <<__retarget_lock_acquire>>, <<__retarget_lock_acquire_recursive>>, <<__retarget_lock_release>>, <<__retarget_lock_release_recursive>>---locking routines

INDEX
	__lock___sfp_recursive_mutex
INDEX
	__lock___atexit_recursive_mutex
INDEX
	__lock___at_quick_exit_mutex
INDEX
	__lock___malloc_recursive_mutex
INDEX
	__lock___env_recursive_mutex
INDEX
	__lock___tz_mutex
INDEX
	__lock___arc4random_mutex

INDEX
	__retarget_lock_init
INDEX
	__retarget_lock_init_recursive
INDEX
	__retarget_lock_close
INDEX
	__retarget_lock_close_recursive
INDEX
	__retarget_lock_acquire
INDEX
	__retarget_lock_acquire_recursive
INDEX
	__retarget_lock_release
INDEX
	__retarget_lock_release_recursive

SYNOPSIS
	#include <lock.h>
	struct __lock __lock___sfp_recursive_mutex;
	struct __lock __lock___atexit_recursive_mutex;
	struct __lock __lock___at_quick_exit_mutex;
	struct __lock __lock___malloc_recursive_mutex;
	struct __lock __lock___env_recursive_mutex;
	struct __lock __lock___tz_mutex;
	struct __lock __lock___arc4random_mutex;

	void __retarget_lock_init (_LOCK_T * <[lock_ptr]>);
	void __retarget_lock_init_recursive (_LOCK_T * <[lock_ptr]>);
	void __retarget_lock_close (_LOCK_T <[lock]>);
	void __retarget_lock_close_recursive (_LOCK_T <[lock]>);
	void __retarget_lock_acquire (_LOCK_T <[lock]>);
	void __retarget_lock_acquire_recursive (_LOCK_T <[lock]>);
	void __retarget_lock_release (_LOCK_T <[lock]>);
	void __retarget_lock_release_recursive (_LOCK_T <[lock]>);

DESCRIPTION
Newlib was configured to allow the target platform to provide the locking
routines and static locks at link time.  As such, a dummy default
implementation of these routines and static locks is provided for
single-threaded application to link successfully out of the box on bare-metal
systems.

For multi-threaded applications the target platform is required to provide
an implementation for @strong{all} these routines and static locks.  If some
routines or static locks are missing, the link will fail with doubly defined
symbols.

PORTABILITY
These locking routines and static lock are newlib-specific.  Supporting OS
subroutines are required for linking multi-threaded applications.
*/

/* dummy lock routines and static locks for single-threaded apps */

#include <sys/lock.h>

#ifndef __SINGLE_THREAD

struct __lock {
  char unused;
};

struct __lock __lock___libc_recursive_mutex;

void
__retarget_lock_init (_LOCK_T *lock)
{
  (void) lock;
}

void
__retarget_lock_init_recursive(_LOCK_T *lock)
{
  (void) lock;
}

void
__retarget_lock_close(_LOCK_T lock)
{
  (void) lock;
}

void
__retarget_lock_close_recursive(_LOCK_T lock)
{
  (void) lock;
}

void
__retarget_lock_acquire (_LOCK_T lock)
{
  (void) lock;
}

void
__retarget_lock_acquire_recursive (_LOCK_T lock)
{
  (void) lock;
}

void
__retarget_lock_release (_LOCK_T lock)
{
  (void) lock;
}

void
__retarget_lock_release_recursive (_LOCK_T lock)
{
  (void) lock;
}

#endif /* __SINGLE_THREAD */
