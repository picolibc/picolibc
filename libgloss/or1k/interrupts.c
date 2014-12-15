/* interrupts.c -- interrupt handling for OpenRISC 1000.
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

#include "include/or1k-support.h"
#include "include/or1k-sprs.h"
#include <stdint.h>

#include "or1k-internals.h"

#ifdef __OR1K_MULTICORE__
or1k_interrupt_handler_table_t *_or1k_interrupt_handler_table;
or1k_interrupt_handler_data_ptr_table_t *_or1k_interrupt_handler_data_ptr_table;
#else
or1k_interrupt_handler_table_t _or1k_interrupt_handler_table;
or1k_interrupt_handler_data_ptr_table_t _or1k_interrupt_handler_data_ptr_table;
#endif

void or1k_interrupt_handler_add(uint32_t id,
		or1k_interrupt_handler_fptr handler,
		void *data_ptr)
{
#ifdef __OR1K_MULTICORE__
	_or1k_interrupt_handler_table[or1k_coreid()][id] = handler;
	_or1k_interrupt_handler_data_ptr_table[or1k_coreid()][id] = (uint32_t) data_ptr;
#else
	_or1k_interrupt_handler_table[id] = handler;
	_or1k_interrupt_handler_data_ptr_table[id] = (uint32_t) data_ptr;
#endif
}

void
or1k_interrupts_enable(void)
{
	uint32_t sr = or1k_mfspr(OR1K_SPR_SYS_SR_ADDR);
	sr = OR1K_SPR_SYS_SR_IEE_SET(sr, 1);
	or1k_mtspr(OR1K_SPR_SYS_SR_ADDR, sr);
}

uint32_t
or1k_interrupts_disable(void)
{
	uint32_t oldsr, newsr;
	oldsr= or1k_mfspr(OR1K_SPR_SYS_SR_ADDR);
	newsr = OR1K_SPR_SYS_SR_IEE_SET(oldsr, 0);
	or1k_mtspr(OR1K_SPR_SYS_SR_ADDR, newsr);
	return OR1K_SPR_SYS_SR_IEE_GET(oldsr);
}

void
or1k_interrupts_restore(uint32_t sr_iee)
{
	uint32_t sr = or1k_mfspr(OR1K_SPR_SYS_SR_ADDR);
	sr = OR1K_SPR_SYS_SR_IEE_SET(sr, sr_iee);
	or1k_mtspr(OR1K_SPR_SYS_SR_ADDR, sr);
}
