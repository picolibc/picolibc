/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2019 Keith Packard
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

#define SPARC_NWIN	8

#define PSR_VER_BIT                     24
#define PSR_PIL_BIT                      8

#define PSR_VER                         (0xf << PSR_VER_BIT)
#define PSR_EF                          (1 << 12)
#define PSR_S                           (1 <<  7)
#define PSR_PS                          (1 <<  6)
#define PSR_ET                          (1 <<  5)
#define PSR_PIL                         (0xf << PSR_PIL_BIT)
#define PSR_CWP                         0x1f

#define TRAP(func) \
	rd	%psr, %l0				; \
	sethi	%hi(_sparc_ ## func ## _isr), %l4 	;\
	jmp	%l4+%lo(_sparc_ ## func ## _isr) 	;\
	rd	%tbr, %l6 				;

#define TRAP_IN_TRAP \
	ta	0x00		; \
	nop			; \
	nop			; \
	nop			;

#define TRAP_HALT(name) \
	TRAP(name) 	      ; \
	.weak	_sparc_ ## name ## _isr	      ; \
	.equ	_sparc_ ## name ## _isr, _sparc_halt_isr ;

	.section	.data.init.enter
	.globl		__weak_interrupt_vector
__weak_interrupt_vector:
	TRAP(reset)
	TRAP_HALT(instruction_access_exception)
	TRAP_HALT(illegal_instruction)
	TRAP_HALT(priveledged_instruction)
	TRAP_HALT(fp_disabled)
#ifdef _FLAT
	TRAP_HALT(window_overflow)
	TRAP_HALT(window_underflow)
#else
	TRAP(window_overflow)
	TRAP(window_underflow)
#endif
	TRAP_HALT(mem_address_not_aligned)
	TRAP_HALT(fp_exception)
	TRAP_HALT(data_access_exception)
	TRAP_HALT(tag_overflow)
	TRAP_HALT(watchpoint_detected)
	TRAP_HALT(trap_0c)
	TRAP_HALT(trap_0d)
	TRAP_HALT(trap_0e)
	TRAP_HALT(trap_0f)
	TRAP_HALT(trap_10)

	. = __weak_interrupt_vector + 0x80 * 16

	TRAP_HALT(syscall)
	TRAP_HALT(breakpoint)
	TRAP_HALT(divide_by_zero)
#ifdef _FLAT
	TRAP_HALT(window_flush)
#else
	TRAP(window_flush)
#endif
	TRAP_HALT(clean_windows)
	.size	__weak_interrupt_vector, . - __weak_interrupt_vector
	.weak	__interrupt_vector
	.equ	__interrupt_vector, __weak_interrupt_vector

	.text
	.align 4

	.global _sparc_reset_isr
	.type 	_sparc_reset_isr, #function
_sparc_reset_isr:
	sethi	%hi(_start), %g4
	jmp	%g4+%lo(_start)
	nop
	.size	_sparc_reset_isr, .-_sparc_reset_isr

	.global	_sparc_halt_isr
	.type	_sparc_halt_isr, #function
_sparc_halt_isr:
	ta	0x00
	ba	_sparc_halt_isr
	nop
	.size	_sparc_halt_isr, .-_sparc_halt_isr

	.global _sparc_window_overflow_isr
	.type 	_sparc_window_overflow_isr, #function
_sparc_window_overflow_isr:
	/* Enter the window to be stored. */
	save
	/* Save local register set. */
	std	%l0, [%sp + 0x00]
	std	%l2, [%sp + 0x08]
	std	%l4, [%sp + 0x10]
	rd	%wim, %l3
	std	%l6, [%sp + 0x18]
	/* l2 := WIM << (NWIN-1) */
	sll	%l3, (SPARC_NWIN-1), %l2
	/* Save input register set. */
	std	%i0, [%sp + 0x20]
	/* l3 := WIM >> 1 */
	srl	%l3, 1, %l3
	std	%i2, [%sp + 0x28]
	/* WIM := (WIM >> 1) ^ (WIM << (NWIN-1)) */
	wr	%l3, %l2, %wim
	/* NOTE: 3 instruction before restore (delayed write instruction) */
	std	%i4, [%sp + 0x30]
	nop
	std	%i6, [%sp + 0x38]
	/* Go back to trap window. */
	restore
	/* Re-execute save. */
	jmp	%l1
	rett	%l2
	.size	_sparc_window_overflow_isr, .-_sparc_window_overflow_isr

	.global _sparc_window_underflow_isr
	.type 	_sparc_window_underflow_isr, #function
_sparc_window_underflow_isr:
	rd	%wim, %l3
	/* l4 := WIM << 1 */
	sll	%l3, 1, %l4
	/* l5 := WIM >> (NWIN-1) */
	srl	%l3, (SPARC_NWIN-1), %l5
	/* WIM := (WIM << 1) ^ (WIM >> (NWIN-1)) */
	wr	%l4, %l5, %wim
	/* WIM is implicitly read so nops are needed. */
	nop
	nop
	nop

	/* Enter the window to restore requires two restore instructions. */
	restore
	restore
	ldd	[%sp + 0x00], %l0
	ldd	[%sp + 0x08], %l2
	ldd	[%sp + 0x10], %l4
	ldd	[%sp + 0x18], %l6
	ldd	[%sp + 0x20], %i0
	ldd	[%sp + 0x28], %i2
	ldd	[%sp + 0x30], %i4
	ldd	[%sp + 0x38], %i6
	/* Go back to the trap window. */
	save
	save
	/* Re-execute restore. */
	jmp	%l1
	rett	%l2
	.size	_sparc_window_underflow_isr, .-_sparc_window_underflow_isr
	
	.global _sparc_window_flush_isr
	.type 	_sparc_window_flush_isr, #function
_sparc_window_flush_isr:
	/*
	 * push a few registers which are needed later to the stack using
	 * sp from the window we trapped from (which is fp in this window)
	 */
	sub	%fp, 0x10, %fp
	std	%l0, [%fp + 0x40 + 0x00]
	st	%l2, [%fp + 0x40 + 0x08]
	st	%g1, [%fp + 0x40 + 0x0c]

	restore
	/* In window where we trapped from. This window will not be flushed. */

	/* Set highest processor interrupt level and enable traps. */
	rd	%psr, %g1
	or	%g1, PSR_PIL, %g1
	wr	%g1, PSR_ET, %psr
	nop
	nop

	/* Execute "save" NWINDOWS-1 times. */
	set     SPARC_NWIN-2, %g1
1:
	save
	cmp	%g1, %g0
	bne	1b
	sub	%g1, 1, %g1

	/* Execute "restore" NWINDOWS-1 times. */
	set     SPARC_NWIN-2, %g1
2:
	restore
	cmp	%g1, %g0
	bne	2b
	sub	%g1, 1, %g1

	save

	/* pop registers from stack which are used for the trap return */
	ldd	[%fp + 0x40 + 0x00], %l0
	ld	[%fp + 0x40 + 0x08], %l2
	ld	[%fp + 0x40 + 0x0c], %g1
	add	%fp, 0x10, %fp

	/* Restore %psr as it was on trap entry. */
	wr	%l0, %psr
	nop
	nop
	nop

	jmp	%l2
	rett	%l2 + 4
	.size	_sparc_window_flush_isr, .-_sparc_window_flush_isr
