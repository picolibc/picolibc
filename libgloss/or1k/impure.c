/* impure.c. Handling of re-entrancy data structure for OpenRISC 1000.

   Copyright (C) 2014, Authors
 
   Contributor Stefan Wallentowitz <stefan.wallentowitz@tum.de>
  
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
#include "or1k-internals.h"

#include <string.h>

/* As an exception handler may also use the library, it is better to use
 * a different re-entrancy data structure for the exceptions.
 * This data structure is configured here and as part of the exception
 * handler (or1k_exception_handler) temporarily replaces the software's
 * impure data pointer.
 *
 * During initialization, the libraries standard _impure_data and the exception
 * impure data (_exception_impure_data) are initialized. Afterwards,
 * the current value _current_impure_ptr is set to _impure_ptr.
 *
 * At runtime __getreent is called to return the current reentrancy pointer,
 * which is stored in _current_impure_ptr.
 *
 * In the or1k_exception_handler the _current_impure_ptr is set to point to
 * _exception_impure_ptr. After the exception handler returned, it is set back
 * to _impure_ptr.
 */

/* Link in the external impure_data structure */
extern struct _reent *__ATTRIBUTE_IMPURE_PTR__ _impure_ptr;

#ifdef __OR1K_MULTICORE__
struct _reent **_or1k_impure_ptr;
struct _reent **_or1k_exception_impure_ptr;
struct _reent **_or1k_current_impure_ptr;
#else
struct _reent *__ATTRIBUTE_IMPURE_PTR__ _or1k_impure_ptr;

/* Create exception impure data structure */
static struct _reent _or1k_exception_impure_data = _REENT_INIT (_or1k_exception_impure_data);

/* Link to the exception impure data structure */
struct _reent *__ATTRIBUTE_IMPURE_PTR__ _or1k_exception_impure_ptr = &_or1k_exception_impure_data;

/* Link to the currently used data structure. */
struct _reent *__ATTRIBUTE_IMPURE_PTR__ _or1k_current_impure_ptr;
#endif

#ifdef __OR1K_MULTICORE__
#define OR1K_LIBC_GETREENT _or1k_current_impure_ptr[or1k_coreid()]
#else
#define OR1K_LIBC_GETREENT _or1k_current_impure_ptr
#endif

void
_or1k_libc_impure_init (void)
{
#ifdef __OR1K_MULTICORE__
	uint32_t c;

	_or1k_impure_ptr = _sbrk_r(0, sizeof(struct _reent*) * or1k_numcores());
	_or1k_exception_impure_ptr = _sbrk_r(0, sizeof(struct _reent*) * or1k_numcores());
	_or1k_current_impure_ptr = _sbrk_r(0, sizeof(struct _reent*) * or1k_numcores());

	_or1k_impure_ptr[0] = _impure_ptr;
	_REENT_INIT_PTR(_impure_ptr);
	for (c = 1; c < or1k_numcores(); c++) {
		_or1k_impure_ptr[c] = _sbrk_r(0, sizeof(struct _reent));
		_REENT_INIT_PTR(_or1k_impure_ptr[c]);
	}

	for (c = 0; c < or1k_numcores(); c++) {
		_or1k_exception_impure_ptr[c] = _sbrk_r(0, sizeof(struct _reent));
		_REENT_INIT_PTR(_or1k_exception_impure_ptr[c]);
	}

	for (c = 0; c < or1k_numcores(); c++) {
		_or1k_current_impure_ptr[c] = _or1k_impure_ptr[c];
	}
#else
	// Initialize both impure data structures
	_REENT_INIT_PTR (_impure_ptr);
	_REENT_INIT_PTR (_or1k_exception_impure_ptr);

	// Set current to standard impure pointer
	_or1k_current_impure_ptr = _impure_ptr;
#endif
}

struct _reent*
_or1k_libc_getreent(void) {
	return OR1K_LIBC_GETREENT;
}

#ifdef __OR1K_MULTICORE__
struct _or1k_reent (*_or1k_reent)[];
#else
struct _or1k_reent _or1k_reent;
#endif

void
_or1k_reent_init(void)
{
#ifdef __OR1K_MULTICORE__
	size_t memsize = sizeof(struct _or1k_reent) * or1k_numcores();
	_or1k_reent = (struct _or1k_reent*) _sbrk_r(0, memsize);
#endif
}
