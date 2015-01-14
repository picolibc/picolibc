/* malloc-lock.c. Lock malloc.
 *
 * Copyright (C) 2014, Authors
 *
 * Contributor Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include <reent.h>
#include <or1k-support.h>

/* Lock calls from different cores, but allows recursive calls from the same
 * core. The lock is not only atomic to other cores calling malloc, but also
 * disables all external interrupts. This is necessary as it could otherwise
 * lead to a deadlock to interrupt while in malloc and then call it from an
 * exception. But as we want the exceptions to be flexible to use all library
 * calls and especially memory management this is necessary.
 */

// The lock. It is zero when unlocked and contains a unique value for each core.
// This value is not the core id (to avoid id zero), but the pointer value of
// the core specific struct _reent.
volatile uint32_t _or1k_malloc_lock;

// Count how often the current holder has entered the lock
volatile uint32_t _or1k_malloc_lock_cnt;
// The exception enable restore of the current mutex holder
volatile uint32_t _or1k_malloc_lock_restore;

/**
 * Recursive lock of the malloc
 */
void __malloc_lock(struct _reent *ptr) {
	uint32_t restore;
	uint32_t id;

	// Each core is identified by its struct _reent pointer
	id = (uint32_t) ptr;

	// Disable timer and interrupt exception, save TEE and IEE flag
	// temporarily to restore them later on unlock
	restore = or1k_critical_begin();

	// We cannot be disturbed by an interrupt or timer exception from here

	// Check if we currently don't hold the lock
	if (_or1k_malloc_lock != id) {
		do {
			// Repeatedly check the lock until it is set to zero
			while (_or1k_malloc_lock != 0) {}
			// .. and then try to set it atomically. As this may
			// fail, we need to repeat this
		} while (or1k_sync_cas(&_or1k_malloc_lock, 0, id) != 0);
	}

	// Store the TEE and IEE flags for later restore
	_or1k_malloc_lock_restore = restore;

	// Increment counter. The lock may be accessed recursively
	_or1k_malloc_lock_cnt++;

	return;
}

void __malloc_unlock(struct _reent *ptr) {
	// Decrement counter. The lock may be unlocked recursively
	_or1k_malloc_lock_cnt--;

	// If this was the last recursive unlock call
	if(_or1k_malloc_lock_cnt == 0){
		// We need to temporarily store the value to avoid a race
		// condition between unlocking and reading restore
		uint32_t restore = _or1k_malloc_lock_restore;
		// unset lock
		_or1k_malloc_lock = 0;
		// Restore flags
		or1k_critical_end(_or1k_malloc_lock_restore);
	}

	return;
}
