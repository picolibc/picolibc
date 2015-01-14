/* getreent.S. Return reentrancy pointer.

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

#include "or1k-asm.h"

	.extern	_or1k_libc_getreent

	.global	__getreent
	.type	__getreent,@function

__getreent:
	OR1K_DELAYED_NOP(OR1K_INST(l.j _or1k_libc_getreent))
