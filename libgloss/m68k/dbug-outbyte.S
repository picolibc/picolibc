/*
 * dbug-outbyte.S -- 
 *
 * Copyright (c) 1996 Cygnus Support
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

#include "asm.h"

	.text
	.global SYM (outbyte)
	.global SYM (putDebugChar)

/*
 * outbyte -- sends a byte out the serial port
 *	d0 - contains the byte to be sent
 */
	.text
	.align	2
SYM (putDebugChar):		/* symbol name used by m68k-stub */
SYM (outbyte):
	movel	sp@(4),d1
	movl	IMM(0x13),d0
	trap	IMM(15)
	rts
