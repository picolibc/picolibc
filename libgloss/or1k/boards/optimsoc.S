/* optimsoc.S -- Support for OpTiMSoC systems.
 *
 * Copyright (c) 2014 Authors
 *
 * Contributor Stefan Wallentowitz <stefan.wallentowitz@saunalahti.fi>
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

#include "../include/or1k-asm.h"
#include "../include/or1k-nop.h"

#define OPTIMSOC_NA_BASE 0xe0000000

#define OPTIMSOC_NA_REGS       OPTIMSOC_NA_BASE + 0x00000
#define OPTIMSOC_NA_LMEM_SIZE  OPTIMSOC_NA_REGS + 0x24
#define OPTIMSOC_NA_GMEM_SIZE  OPTIMSOC_NA_REGS + 0x1c

#define OPTIMSOC_GZLL_STRING 0x677a6c6c
#define OPTIMSOC_GZLL_IMAGE_LAYOUT 0x2000
/*
 * Define symbols to be used during startup - file is linked at compile time
 *
 */
.weak _or1k_board_mem_base
.weak _or1k_board_mem_size
.weak _or1k_board_clk_freq

_or1k_board_mem_base:	.long	0x0
_or1k_board_mem_size:	.long	0x0

_or1k_board_clk_freq:	.long	50000000

.weak _or1k_board_uart_base
.weak _or1k_board_uart_baud
.weak _or1k_board_uart_IRQ
_or1k_board_uart_base:	.long	0x0
_or1k_board_uart_baud:	.long	0
_or1k_board_uart_IRQ:	.long	0

.weak _or1k_board_exit
_or1k_board_exit:
	l.nop OR1K_NOP_K_EXIT
.Lexitloop:
	OR1K_DELAYED_NOP(l.j .Lexitloop)

.extern _or1k_heap_start

.global _or1k_board_init_early
_or1k_board_init_early:
#ifndef __OR1K_MULTICORE__
	l.nop	0x1
.die:
	OR1K_DELAYED_NOP(l.j .die)
#endif
	l.movhi	r1,hi(OPTIMSOC_NA_LMEM_SIZE)
	l.ori	r1,r1,lo(OPTIMSOC_NA_LMEM_SIZE)
	l.lwz	r1,0(r1)
	l.movhi	r2,hi(_or1k_board_mem_size)
	l.ori	r2,r2,lo(_or1k_board_mem_size)
	l.sw	0(r2),r1
	// Set stack beyond apps if single image in local memory
	// Is this a kernel running?
	l.movhi r3,hi(OPTIMSOC_GZLL_STRING)
	l.ori	r3,r3,lo(OPTIMSOC_GZLL_STRING)
	l.movhi r4,hi(OPTIMSOC_GZLL_IMAGE_LAYOUT)
	l.ori   r4,r4,lo(OPTIMSOC_GZLL_IMAGE_LAYOUT)
	l.lwz   r5,0(r4)
	l.sfeq  r3,r5
	// If not, we are done
	OR1K_DELAYED_NOP(l.bnf ._or1k_board_init_early_done)
	// Else: are we running with global memory (loading)?
	l.movhi r6,hi(OPTIMSOC_NA_GMEM_SIZE)
	l.ori   r6,r6,lo(OPTIMSOC_NA_GMEM_SIZE)
	l.lwz   r6,0(r6)
	l.sfeqi r6,0
	// If yes, we are done
	OR1K_DELAYED_NOP(l.bnf ._or1k_board_init_early_done)
	// Else: Read end of apps
	l.lwz   r7,12(r4)
	l.movhi r8,hi(_or1k_heap_start)
	l.ori   r8,r8,lo(_or1k_heap_start)
	l.sw    0(r8),r7
._or1k_board_init_early_done:
	OR1K_DELAYED_NOP(l.jr r9)

.weak _or1k_board_init
_or1k_board_init:
	OR1K_DELAYED_NOP(l.jr r9)
