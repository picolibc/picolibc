/* linux-crt0 -- Startup routines for the Linux BSP.
 *
 * Copyright (c) 2011 Edgar E. Iglesias
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

        .section .text
        .globl _start
        .ent _start
        .type _start, @function
_start:
	la	r13, r0, _SDA_BASE_
	la	r2, r0, _SDA2_BASE_

	brlid	r15, __init
	nop

	lwi	r5, r1, 0
	addik	r6, r1, 4

	# Add argc * 4.
	addk	r7, r5, r5
	addk	r7, r7, r7

	brlid	r15, main
	# Now add 4 + r1 (i.e r6) in the delayslot.
	addk	r7, r7, r6

	brlid   r15, exit
        addik   r5, r3, 0
	.size _start, . - _start
        .end _start

	/* Replacement for the GCC provided crti.S. This one avoids the
	   setup of stack protection regs (which result in privilieged
	   insn exceptions when running in user-space).  */
        .section .init, "ax"
	.global __init
	.align 2
__init:
	addik   r1, r1, -8
	sw      r15, r0, r1

	.section .fini, "ax"
	.global __fini
	.align 2
__fini:
	addik   r1, r1, -8
	sw      r15, r0, r1
