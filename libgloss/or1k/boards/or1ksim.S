/* or1ksim.S -- Support for or1ksim.
 *
 * Copyright (c) 2011 Authors
 *
 * Contributor Julius Baxter <juliusbaxter@gmail.com>
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

/*
 * Define symbols to be used during startup - file is linked at compile time
 *
 */
.weak _or1k_board_mem_base
.weak _or1k_board_mem_size
.weak _or1k_board_clk_freq
	
_or1k_board_mem_base:	.long	0x0
_or1k_board_mem_size:	.long	0x800000

_or1k_board_clk_freq:	.long	100000000
	
/* Peripheral information - Set base to 0 if not present*/
.weak _or1k_board_uart_base
.weak _or1k_board_uart_baud
.weak _or1k_board_uart_IRQ
	
_or1k_board_uart_base:	.long	0x0
_or1k_board_uart_baud:	.long	115200
_or1k_board_uart_IRQ:	.long	13
	
.weak _or1k_board_exit
_or1k_board_exit:
	l.nop OR1K_NOP_K_EXIT_QUIET
.Lexitloop:
	OR1K_DELAYED_NOP(l.j .Lexitloop)

.global _or1k_board_init_early
_or1k_board_init_early:
	OR1K_DELAYED_NOP(l.jr r9)

.weak _or1k_board_init
_or1k_board_init:
	OR1K_DELAYED_NOP(l.jr r9)

