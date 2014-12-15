/* exceptions-asm.S -- exception handling for OpenRISC 1000.
 *
 * Copyright (c) 2011, 2014 Authors
 *
 * Contributor Julius Baxter <juliusbaxter@gmail.com>
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
#include "include/or1k-sprs.h"

/* -------------------------------------------------------------------------- */
/*!Generic exception handler function
                                                                              */
/* -------------------------------------------------------------------------- */
// Warning - this must be the same as specified in crt0.S
#define EXCEPTION_STACK_SIZE 128+128

	.extern _or1k_exception_handler_table

/* -------------------------------------------------------------------------- */
/*!Function to call appropriate exception handler
                                                                              */
/* -------------------------------------------------------------------------- */
	.section .text
	.global	_or1k_exception_handler
	.type	_or1k_exception_handler,@function

	/*
	r3 = address of exception vector
	r4 = address where exception occurred
	*/

#define GPR_BUF_OFFSET(x) (x << 2)

_or1k_exception_handler:
	/* Store remainder of state (r3,r4 stored in vector entry)*/
	l.sw	GPR_BUF_OFFSET(2)(r1),r2
	l.sw	GPR_BUF_OFFSET(5)(r1),r5
	l.sw	GPR_BUF_OFFSET(6)(r1),r6
	l.sw	GPR_BUF_OFFSET(7)(r1),r7
	l.sw	GPR_BUF_OFFSET(8)(r1),r8
	l.sw	GPR_BUF_OFFSET(9)(r1),r9
	l.sw	GPR_BUF_OFFSET(10)(r1),r10
	l.sw	GPR_BUF_OFFSET(11)(r1),r11
	l.sw	GPR_BUF_OFFSET(12)(r1),r12
	l.sw	GPR_BUF_OFFSET(13)(r1),r13
	l.sw	GPR_BUF_OFFSET(14)(r1),r14
	l.sw	GPR_BUF_OFFSET(15)(r1),r15
	l.sw	GPR_BUF_OFFSET(16)(r1),r16
	l.sw	GPR_BUF_OFFSET(17)(r1),r17
	l.sw	GPR_BUF_OFFSET(18)(r1),r18
	l.sw	GPR_BUF_OFFSET(19)(r1),r19
	l.sw	GPR_BUF_OFFSET(20)(r1),r20
	l.sw	GPR_BUF_OFFSET(21)(r1),r21
	l.sw	GPR_BUF_OFFSET(22)(r1),r22
	l.sw	GPR_BUF_OFFSET(23)(r1),r23
	l.sw	GPR_BUF_OFFSET(24)(r1),r24
	l.sw	GPR_BUF_OFFSET(25)(r1),r25
	l.sw	GPR_BUF_OFFSET(26)(r1),r26
	l.sw	GPR_BUF_OFFSET(27)(r1),r27
	l.sw	GPR_BUF_OFFSET(28)(r1),r28
	l.sw	GPR_BUF_OFFSET(29)(r1),r29
	l.sw	GPR_BUF_OFFSET(30)(r1),r30
	l.sw	GPR_BUF_OFFSET(31)(r1),r31

	/* Replace impure pointer for exception */
	l.movhi	r20,hi(_or1k_exception_impure_ptr)
	l.ori	r20,r20,lo(_or1k_exception_impure_ptr)
#ifdef __OR1K_MULTICORE__
	l.lwz	r20,0(r20)
	l.mfspr	r22,r0,OR1K_SPR_SYS_COREID_ADDR
	l.slli	r22,r22,2
	l.add	r20,r20,r22
#endif
	l.lwz	r20,0(r20)

	l.movhi	r21,hi(_or1k_current_impure_ptr)
	l.ori	r21,r21,lo(_or1k_current_impure_ptr)
#ifdef __OR1K_MULTICORE__
	l.lwz	r21,0(r21)
	l.add	r21,r21,r22
#endif
	l.sw	0(r21),r20

	/* Determine offset in table of exception handler using r3*/
	l.andi	r13,r3,0xff00
	l.srli	r13,r13,6
	/* Substract 2 words, as we have no vector at 0 and no reset handler */
	l.addi	r13,r13,-8
	/* r13 now contains offset in or1k_exception_handler_table for
	   function
	*/
	/* Get or1k_exception_handler_table address */
	l.movhi	r14,hi(_or1k_exception_handler_table)
	l.ori	r14,r14,lo(_or1k_exception_handler_table)
#ifdef __OR1K_MULTICORE__
	/* Read the address of the array of cores */
	/* r14 = (*or1k_exception_handler_table)  */
	l.lwz	r14,0(r14)
	/* Generate core offset in array (off = coreid*30*4 = coreid*120) */
	/* r15 = coreid */
	l.mfspr	r15,r0,OR1K_SPR_SYS_COREID_ADDR
	/* r16 = coreid * 128 */
	l.slli	r16,r15,7
	/* r15 = coreid * 8 */
	l.slli	r15,r15,3
	/* r15 = coreid*128 - coreid*8 = coreid*120 = off */
	l.sub	r15,r16,r15
	/* r14 = (*or1k_exception_handler_table)[coreid] =  r14 + off */
	l.add	r14,r14,r15
#endif
	/* r14 now contains base of exception handler table */
	/* add offset of exception vector */
	l.add	r14,r14,r13
	/* load handler address from table */
	l.lwz	r13, 0(r14)

	/* Check to see if this handler has been set yet */
	l.sfne	r13,r0
	OR1K_DELAYED_NOP(OR1K_INST(l.bnf exception_exit))

	/* Call exception handler, copy EPCR to r3 */
	OR1K_DELAYED(
		OR1K_INST(l.or   r3,r4,r4),
		OR1K_INST(l.jalr r13)
	)

	/* Restore impure pointer */
	l.movhi	r20,hi(_or1k_impure_ptr)
	l.ori	r20,r20,lo(_or1k_impure_ptr)
#ifdef __OR1K_MULTICORE__
	l.lwz	r20,0(r20)
	l.mfspr	r22,r0,OR1K_SPR_SYS_COREID_ADDR
	l.slli	r22,r22,2
	l.add	r20,r20,r22
#endif
	l.lwz	r20,0(r20)

	l.movhi	r21,hi(_or1k_current_impure_ptr)
	l.ori	r21,r21,lo(_or1k_current_impure_ptr)
#ifdef __OR1K_MULTICORE__
	l.lwz	r21,0(r21)
	l.add	r21,r21,r22
#endif
	l.sw	0(r21),r20

	/* Restore state */
	l.lwz	r2,GPR_BUF_OFFSET(2)(r1)
	l.lwz	r3,GPR_BUF_OFFSET(3)(r1)
	l.lwz	r4,GPR_BUF_OFFSET(4)(r1)
	l.lwz	r5,GPR_BUF_OFFSET(5)(r1)
	l.lwz	r6,GPR_BUF_OFFSET(6)(r1)
	l.lwz	r7,GPR_BUF_OFFSET(7)(r1)
	l.lwz	r8,GPR_BUF_OFFSET(8)(r1)
	l.lwz	r9,GPR_BUF_OFFSET(9)(r1)
	l.lwz	r10,GPR_BUF_OFFSET(10)(r1)
	l.lwz	r11,GPR_BUF_OFFSET(11)(r1)
	l.lwz	r12,GPR_BUF_OFFSET(12)(r1)
	l.lwz	r13,GPR_BUF_OFFSET(13)(r1)
	l.lwz	r14,GPR_BUF_OFFSET(14)(r1)
	l.lwz	r15,GPR_BUF_OFFSET(15)(r1)
	l.lwz	r16,GPR_BUF_OFFSET(16)(r1)
	l.lwz	r17,GPR_BUF_OFFSET(17)(r1)
	l.lwz	r18,GPR_BUF_OFFSET(18)(r1)
	l.lwz	r19,GPR_BUF_OFFSET(19)(r1)
	l.lwz	r20,GPR_BUF_OFFSET(20)(r1)
	l.lwz	r21,GPR_BUF_OFFSET(21)(r1)
	l.lwz	r22,GPR_BUF_OFFSET(22)(r1)
	l.lwz	r23,GPR_BUF_OFFSET(23)(r1)
	l.lwz	r24,GPR_BUF_OFFSET(24)(r1)
	l.lwz	r25,GPR_BUF_OFFSET(25)(r1)
	l.lwz	r26,GPR_BUF_OFFSET(26)(r1)
	l.lwz	r27,GPR_BUF_OFFSET(27)(r1)
	l.lwz	r28,GPR_BUF_OFFSET(28)(r1)
	l.lwz	r29,GPR_BUF_OFFSET(29)(r1)
	l.lwz	r30,GPR_BUF_OFFSET(30)(r1)
	l.lwz	r31,GPR_BUF_OFFSET(31)(r1)

	// Restore original stack
	l.lwz	r1,GPR_BUF_OFFSET(1)(r1)

	l.rfe
	l.nop

exception_exit:
	/* Exception handler not set, exit */
	OR1K_DELAYED(
		OR1K_INST(l.or  r3,r4,r4),
		OR1K_INST(l.jal exit)
	)
