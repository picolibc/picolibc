/* util.c -- Utility functions for OpenRISC 1000.
 *
 * Copyright (c) 2014 Authors
 *
 * Contributor Julius Baxter <julius.baxter@orsoc.se>
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

#include <stdint.h>
#include <stddef.h>
#include <reent.h>

#include "or1k-internals.h"

#ifdef __OR1K_MULTICORE__
uint32_t* *_or1k_stack_core;
uint32_t* *_or1k_exception_stack_core;
#endif

uint32_t* _or1k_stack_top;
uint32_t* _or1k_stack_bottom;

uint32_t* _or1k_exception_stack_top;
uint32_t* _or1k_exception_stack_bottom;

void _or1k_init() {
#ifdef __OR1K_MULTICORE__
	uint32_t c;

	// Initialize stacks
	_or1k_stack_core = _sbrk_r(0, sizeof(uint32_t*) * or1k_numcores());
	_or1k_exception_stack_core = _sbrk_r(0, sizeof(uint32_t*) * or1k_numcores());

	_or1k_stack_core[0] = _or1k_stack_top;
	_or1k_exception_stack_core[0] = _or1k_exception_stack_top;

	for (c = 1; c < or1k_numcores(); c++) {
		_or1k_stack_core[c] = _or1k_stack_core[c-1] - _or1k_stack_size;
		_or1k_exception_stack_core[c] = _or1k_exception_stack_core[c-1] -
				_or1k_exception_stack_size;
	}

	size_t exc_size = sizeof(void*) * or1k_numcores() * OR1K_NUM_EXCEPTIONS;
	_or1k_exception_handler_table = _sbrk_r(0, exc_size);

	size_t int_size = sizeof(void*) * or1k_numcores() * 32;
	size_t intdata_size = sizeof(void*) * or1k_numcores() * 32;
	_or1k_interrupt_handler_table = _sbrk_r(0, int_size);
	_or1k_interrupt_handler_data_ptr_table = _sbrk_r(0, intdata_size);
#endif

	_or1k_reent_init();

#ifdef __OR1K_MULTICORE__
	for (c = 0; c < or1k_numcores(); c++) {
		_or1k_exception_handler_table[c][6] = _or1k_interrupt_handler;
	}
#else
	_or1k_exception_handler_table[6] = _or1k_interrupt_handler;
#endif
}

uint32_t or1k_critical_begin() {
	uint32_t iee = or1k_interrupts_disable();
	uint32_t tee = or1k_timer_disable();
	return (iee << 1) | tee;
}

void or1k_critical_end(uint32_t restore) {
	or1k_timer_restore(restore & 0x1);
	or1k_interrupts_restore((restore >> 1) & 0x1);
}

