/*
 * cfe_prestart.S -- startup file for MIPS running CFE.
 * Derived from crt0.S.
 *
 * Copyright (c) 1995, 1996, 1997 Cygnus Support
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

#ifdef __mips16
/* This file contains 32 bit assembly code.  */
	.set nomips16
#endif

#include "regs.S"

/* This is for referencing addresses that are not in the .sdata or
   .sbss section under embedded-pic, or before we've set up gp.  */
#ifdef __mips_embedded_pic
# ifdef __mips64
#  define LA(t,x) la t,x-PICBASE ; daddu t,s0,t
# else
#  define LA(t,x) la t,x-PICBASE ; addu t,s0,t
# endif
#else /* __mips_embedded_pic */ 
# define LA(t,x) la t,x
#endif /* __mips_embedded_pic */ 

	.globl	__cfe_handle
	.globl	__cfe_entrypt

	.text
	.align	2

	.globl	_prestart
	.ent	_prestart
_prestart:
	.set	noreorder
#ifdef __mips_embedded_pic
	PICBASE = .+8
        bal	PICBASE
	nop
	move	s0,$31
#endif

	/* These are actually pointers, but they're guaranteed to be
	   in compatibility space and it's just easier to store them
	   as words ("unsigned int") than worry about the actual
	   pointer size of the runtime we're building.  */
	LA (v0, __cfe_handle)
	sw	a0, 0(v0)
	LA (v0, __cfe_entrypt)
	sw	a2, 0(v0)

	LA (v0, _start)
	jr	v0
	nop
	.end _prestart
