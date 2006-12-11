/* Initial boot
 *
 * Copyright (c) 2006 CodeSourcery Inc
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

	.extern __stack
	.extern __heap_end
	.weak __heap_end
	.extern __start1
	.globl __start
__start:
	/* Initialize stack */
	move.l IMM(__stack), sp
	move.l IMM(0), fp
	move.l IMM(__heap_end), d1
#if HOSTED
	/* INIT_SIM syscall.  Allows changing sp & d1.  */
	move.l IMM(1),d0
	.align 4

	/* The halt sequence must be 'nop; halt' and aligned to a 4 byte
	    boundary.  */
	nop
	halt
	
	/* This sentinel instruction value must be immediately after
	   the halt instruction.  The debugger will adjust the pc, so
	   that it is never executed.  This instruction is
	   'movec %sp,0'.  */
	.long 0x4e7bf000
#endif
	move.l d1,sp@-
	move.l fp,sp@-  /* Dummy return address */
	jmp __start1
