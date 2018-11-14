/* Copyright (c) 2009, 2010, 2011, 2012 ARM Ltd.  All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. The name of the company may not be used to endorse or promote
    products derived from this software without specific prior written
    permission.

 THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include "newlib.h"
#include "svc.h"

/* ANSI concatenation macros.  */
#define CONCAT(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a ## b

#ifdef __USER_LABEL_PREFIX__
#define FUNCTION( name ) CONCAT (__USER_LABEL_PREFIX__, name)
#else
#error __USER_LABEL_PREFIX is not defined
#endif


	.text
	.align 2
_init_vectors:
        /* Installs a table of exception vectors to catch and handle all
           exceptions by terminating the process with a diagnostic.  */
	adr	x0, vectors
	msr	vbar_el3, x0
	msr	vbar_el2, x0
	msr	vbar_el1, x0
	ret

curr_sp0_sync:
curr_sp0_irq:
curr_sp0_fiq:
curr_sp0_serror:
curr_spx_sync:
curr_spx_irq:
curr_spx_fiq:
curr_spx_serror:
lower_a64_sync:
lower_a64_irq:
lower_a64_fiq:
lower_a64_serror:
lower_a32_sync:
lower_a32_irq:
lower_a32_fiq:
lower_a32_serror:
	mov	x0, 2
	adr	x1, .LC3
	mov	x2, 26
	bl	FUNCTION (write)
	mov	x0,  126
	b	FUNCTION (exit)		/* Cannot return.  */
.LC3:
	.string "Terminated by exception.\n"

	.macro	ventry	label
	.align	7
	b	\label
	.endm

	/* AArch64 Exception Model -- 3.5.5 Exception Vectors.  */

	.align	12
vectors:
	/* Current EL with SP0.  */
	ventry	curr_sp0_sync		/* Synchronous  */
	ventry	curr_sp0_irq		/* Irq/vIRQ  */
	ventry	curr_sp0_fiq		/* Fiq/vFIQ  */
	ventry	curr_sp0_serror		/* SError/VSError  */

	/* Current EL with SPx.  */
	ventry	curr_spx_sync		/* Synchronous  */
	ventry	curr_spx_irq		/* IRQ/vIRQ  */
	ventry	curr_spx_fiq		/* FIQ/vFIQ  */
	ventry	curr_spx_serror		/* SError/VSError  */

	/* Lower EL using AArch64.  */
	ventry	lower_a64_sync		/* Synchronous  */
	ventry	lower_a64_irq		/* IRQ/vIRQ  */
	ventry	lower_a64_fiq		/* FIQ/vFIQ  */
	ventry	lower_a64_serror	/* SError/VSError  */

	/* Lower EL using AArch32.  */
	ventry	lower_a32_sync		/* Synchronous  */
	ventry	lower_a32_irq		/* IRQ/vIRQ  */
	ventry	lower_a32_fiq		/* FIQ/vFIQ  */
	ventry	lower_a32_serror	/* SError/VSError  */


	.text
	.align 2
_flat_map:
	/* Page table setup (identity mapping).  */
	adrp	x0, ttb
	add	x0, x0, :lo12:ttb
	msr	ttbr0_el3, x0
	adr	x1, .				/* phys address */
	bic	x1, x1, #(1 << 30) - 1		/* 1GB block alignment */
	add	x2, x0, x1, lsr #(30 - 3)	/* offset in level 1 page
						   table */
	mov	x3, #0x401			/* page table attributes
						   (AF, block) */
	orr	x1, x1, x3
	mov	x3, #(1 << 30)			/* 1GB block */
	str	x1, [x2], #8			/* 1st GB */
	add	x1, x1, x3
	str	x1, [x2]			/* 2nd GB */

	/* Setup/enable the MMU.  */

	/* RES1, RES1, 40-bit PA, 39-bit VA, inner/outer cacheable WB */
	ldr	x0, =(1 << 31) | (1 << 23) | (2 << 16) | 25 | (3 << 10) | (3 << 8)
	msr	tcr_el3, x0

	mov	x0, #0xee			/* Inner/outer cacheable WB */
	msr	mair_el3, x0
	isb

	mrs	x0, sctlr_el3
	ldr	x1, =0x100d			/* bits I(12) SA(3) C(2) M(0) */
	bic	x0, x0, #(1 << 1)		/* clear bit A(1) */
	bic	x0, x0, #(1 << 19)		/* clear WXN */
	orr	x0, x0, x1			/* set bits */

	dsb	sy
	msr	sctlr_el3, x0
	isb

	/* Determine if SVE is available.  */
       mrs      x0, id_aa64pfr0_el1
       tbz      x0, 32, .Lnosve

       /* set up CPTR_EL3.TZ to 1.  */
       mrs     x0, cptr_el3

       /* TZ is bit 8 of CPTR_EL3.  */
       orr     x0, x0, #0x100
       msr     cptr_el3, x0
       isb

       /* set up vector lenght in ZCR_EL3 (4 LSB).  */
       mov     x2, #0xF

       /* Try to set the maximum value supported by the architecture (2048).
	  SVE Arch.

	    "If this field is set to a value that is not supported by the
	    implementation then reading the register must return the highest
	    supported vector length that is less than the value written."  */
       mrs     x1, s3_6_c1_c2_0                /* mrs  x1, zcr_el3.  */
       bfi     x1, x2, 0, 4
       msr     s3_6_c1_c2_0, x1                /* msr  zcr_el3, x1.  */
       isb
.Lnosve:
	ret

	.data
	.align	12
ttb:
	.space	4096, 0


	.text
	.align 2
	.global	FUNCTION (_cpu_init_hook)
	.type	FUNCTION (_cpu_init_hook), %function
	.cfi_sections	.debug_frame
FUNCTION (_cpu_init_hook):
	.cfi_startproc
	str	x30, [sp, -16]!
	.cfi_def_cfa_offset 16
	.cfi_offset 30, -16
	bl	_init_vectors
	bl	_flat_map
	ldr	x30, [sp], 16
	.cfi_restore 30
	ret
	.cfi_endproc
	.size	FUNCTION (_cpu_init_hook), .-FUNCTION (_cpu_init_hook)
