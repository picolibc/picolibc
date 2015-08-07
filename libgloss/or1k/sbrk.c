/* sbrk.c -- allocate space on heap on OpenRISC 1000.
 *
 * Copyright (c) 2014 Authors
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

#include "include/or1k-support.h"

extern uint32_t	end; /* Set by linker.  */
uint32_t _or1k_heap_start = &end;
uint32_t _or1k_heap_end;

void *
_sbrk_r (struct _reent * reent, ptrdiff_t incr)
{
	uint32_t	prev_heap_end;

	// This needs to be atomic

	// Disable interrupts on this core
	uint32_t sr_iee = or1k_interrupts_disable();
	uint32_t sr_tee = or1k_timer_disable();

	// Initialize heap end to end if not initialized before
	or1k_sync_cas((void*) &_or1k_heap_end, 0, (uint32_t) _or1k_heap_start);

	do {
		// Read previous heap end
		prev_heap_end = _or1k_heap_end;
		// and try to set it to the new value as long as it has changed
	} while (or1k_sync_cas((void*) &_or1k_heap_end,
			(uint32_t) prev_heap_end,
			(uint32_t) (prev_heap_end + incr)) != (uint32_t) prev_heap_end);

	// Restore interrupts on this core
	or1k_timer_restore(sr_tee);
	or1k_interrupts_restore(sr_iee);

	return (void*) prev_heap_end;
}
