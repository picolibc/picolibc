/* outbyte.S -- Write one byte for OpenRISC 1000.
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

#include "include/or1k-asm.h"

.global _or1k_outbyte

.text
_or1k_outbyte:
	LOAD_SYMBOL_2_GPR(r4,_or1k_board_uart_base)
	l.lwz	r4, 0(r4)
	l.sfeq	r4, r0
	OR1K_DELAYED_NOP(l.bf .Lnouart)
.Luart:
	OR1K_DELAYED_NOP(l.j _or1k_uart_write)
.Lnouart:
	OR1K_DELAYED(
		OR1K_INST(l.nop 0x4),
		OR1K_INST(l.jr r9)
	)
