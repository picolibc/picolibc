/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _ASM

#include "crtstart.h"

	/* Adjust stack address in r0 for TLS */
	.macro alloc_tls_common
#ifdef STACK_TLS
	/* Allocate space for thread-local storage */
	ldr		r1, .L2
	subs		r0, r1		/* subtract TLS size */
	ldr		r1, .L3
	negs		r1, r1
	ands		r0, r1		/* mask for alignment */
#endif
	.endm

	/* Stack in sp, return in sp and r0 */
	.macro alloc_tls_sp
#ifdef STACK_TLS
	mov		r0, sp
	alloc_tls_common
	mov		sp, r0		/* set stack pointer */
#endif
	.endm

	/* Stack in r0, return in sp and r0 */
	.macro alloc_tls_r0
#ifdef STACK_TLS
	alloc_tls_common
#endif
	mov		sp, r0		/* set stack pointer */
	.endm

	.macro alloc_tls_data
#ifdef STACK_TLS
.L2:
	.word		__tls_size
.L3:
	.word		__tls_align
#endif
	.endm

#if __ARM_ARCH_PROFILE == 'M'

#ifdef STACK_TLS
	/* Entry point */
	.globl		_start
	.section	.init,"ax",%progbits
	.syntax		unified
	.thumb
	.thumb_func
	.type		_start, %function
	.align		2
_start:
	.cfi_startproc
	alloc_tls_sp
	b		_cstart
	.align		2
	alloc_tls_data
	.cfi_endproc
#endif

#else

#ifdef __PICOCRT_ENABLE_MMU

#if __ARM_ARCH >= 7 && __ARM_ARCH_PROFILE != 'R'

	/* Define an identity page table across the address space */

	.section .rodata
	.global __identity_page_table
	.balign 16384
__identity_page_table:
	.set _i, 0
	.rept MMU_NORMAL_COUNT
	.4byte (_i << 20) | MMU_NORMAL_FLAGS
	.set _i, _i + 1
	.endr
	.set _i, 0
	.rept MMU_DEVICE_COUNT
	.4byte (1 << 31) | (_i << 20) | MMU_DEVICE_FLAGS
	.set _i, _i + 1
	.endr
	.size __identity_page_table, (MMU_NORMAL_COUNT + MMU_DEVICE_COUNT) * 4

#endif /* __ARM_ARCH >= 7 && __ARM_ARCH_PROFILE != 'R' */

#endif /* __PICOCRT_ENABLE_MMU */

	/*
	 * Set up all of the shadow stack pointers. With Thumb 1 ISA
	 * we need to do this in ARM mode, so create a function to
	 * hold the code. Otherwise, place the code in-line in _start.
	 */

	.macro set_stacks
	.syntax		unified
	mrs	r2, cpsr
	mov	r1, #0x1f
	bic	r2, r1

	mov	r1, MODE_IRQ
	orr	r1, r2
	msr	cpsr_c, r1
	ldr	r0, .L1
	mov	sp, r0

	mov	r1, MODE_ABT
	orr	r1, r2
	msr	cpsr_c, r1
	sub	r0, r0, SHADOW_STACK_SIZE
	mov	sp, r0

	mov	r1, MODE_UND
	orr	r1, r2
	msr	cpsr_c, r1
	sub	r0, r0, SHADOW_STACK_SIZE
	mov	sp, r0

	mov	r1, MODE_FIQ
	orr	r1, r2
	msr	cpsr_c, r1
	sub	r0, r0, SHADOW_STACK_SIZE
	mov	sp, r0

	mov	r1, MODE_SVC
	orr	r1, r2
	msr	cpsr_c, r1
	sub	r0, r0, SHADOW_STACK_SIZE

	alloc_tls_r0
	.endm

	.macro set_stacks_data
	.align	2
.L1:
	.word	__stack
	alloc_tls_data
	.endm

	/* Generate a reference to __vector_table so we get one loaded */
	.equ		__my_vector_table, __vector_table

	/* Entry point */
	.globl		_start
	.section	.init,"ax",%progbits
	.syntax		unified
#ifdef __thumb__
	.thumb
	.thumb_func
#else
	.arm
#endif
	.type		_start, %function
_start:
	.cfi_startproc
#if __ARM_ARCH_ISA_THUMB == 1 && defined(__thumb__)
	bl		_set_stacks
#else
	set_stacks
#endif
	b		_cstart
#if __ARM_ARCH_ISA_THUMB != 1
	set_stacks_data
#endif

	.align		2
	.cfi_endproc
	.size		_start, .-_start

#if __ARM_ARCH_ISA_THUMB == 1
	.section	.init,"ax",%progbits
	.align	2
	.syntax unified
	.arm
	.type	_set_stacks, %function

_set_stacks:
	.cfi_startproc
	set_stacks
	bx	lr
	set_stacks_data
	.cfi_endproc
	.size	_set_stacks, .-_set_stacks
#endif

#endif /* __ARM_ARCH_PROFILE != 'M' */
