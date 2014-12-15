/* interrupts-asm.S -- interrupt handling for OpenRISC 1000.
 *
 * Copyright (c) 2011, 2012, 2014 Authors
 *
 * Contributor Julius Baxter <juliusbaxter@gmail.com>
 * Contributor Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
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

/* -------------------------------------------------------------------------- */
/*!Generic interrupt handler function for or1k
                                                                              */
/* -------------------------------------------------------------------------- */

#include "include/or1k-asm.h"
#include "include/or1k-sprs.h"

	.extern _or1k_interrupt_handler_table
	.extern _or1k_interrupt_handler_data_ptr_table

/* -------------------------------------------------------------------------- */
/*!Function to call appropriate interrupt handler
                                                                              */
/* -------------------------------------------------------------------------- */

	.section .text
	.global	_or1k_interrupt_handler
	.type	_or1k_interrupt_handler,@function

_or1k_interrupt_handler:
	/* Make room on stack, save link register */
	l.addi	r1,r1,-12
	l.sw	0(r1),r9

	/* Read PICSR */
	l.mfspr	r3,r0,OR1K_SPR_PIC_PICSR_ADDR

	/* Load handler table base address */
	l.movhi r7,hi(_or1k_interrupt_handler_table)
	l.ori	r7,r7,lo(_or1k_interrupt_handler_table)
	/* Load data pointer table base address */
	l.movhi r12,hi(_or1k_interrupt_handler_data_ptr_table)
	l.ori	r12,r12,lo(_or1k_interrupt_handler_data_ptr_table)
#ifdef __OR1K_MULTICORE__
	/* Read the addresses of the arrays of cores */
	/* r7 = (*or1k_interrupt_handler_table)  */
	l.lwz	r7,0(r7)
	/* r12 = (*or1k_interrupt_handler_data_ptr_table)  */
	l.lwz	r12,0(r12)
	/* Generate offset in arrays */
	/* r14 = coreid */
	l.mfspr	r14,r0,OR1K_SPR_SYS_COREID_ADDR
	/* r14 = coreid*32*4 = off */
	l.slli	r14,r14,7
	/* r7 = (*or1k_exception_handler_table)[coreid] */
	l.add	r7,r7,r14
	/* r12 = (*or1k_exception_handler_table)[coreid] */
	l.add	r12,r12,r14
#endif

.L0:
	/* Find first set bit in PICSR */
	l.ff1	r4,r3
	/* Any bits set? */
	l.sfne	r4,r0
	/* If none, finish */
	OR1K_DELAYED_NOP(OR1K_INST(l.bnf .L2))
	/* What is IRQ function table offset? */
	l.addi	r5,r4,-1
	l.slli	r6,r5,2
	/* Add this to table bases */
	l.add	r14,r6,r7
	l.add	r13,r6,r12

	/* Fetch handler function address */
	l.lwz	r14,0(r14)

	/* Double check it's valid, compare against INTERRUPT_HANDLER_NOT_SET */
	l.sfne	r14,r0
	/* Skip if no handler: TODO: Indicate interrupt fired but no handler*/
	OR1K_DELAYED_NOP(OR1K_INST(l.bnf .L1))

	/* Pull out data pointer from table, save r3, we'll write over it */
	l.sw	4(r1),r3
	l.lwz	r3,0(r13)
	/* Call handler, save r5 in delay slot */
	OR1K_DELAYED(
		OR1K_INST(l.sw   8(r1),r5),
		OR1K_INST(l.jalr r14)
	)

	/* Reload r3,r5 */
	l.lwz	r3,4(r1)
	l.lwz	r5,8(r1)
.L1:
	/* Clear bit from PICSR, return to start of checking loop */
	l.ori	r6,r0,1
	l.sll	r6,r6,r5
	OR1K_DELAYED(
		OR1K_INST(l.xor r3,r3,r6),
		OR1K_INST(l.j   .L0)
	)

.L2:
	/* Finish up - write PICSR back, restore r9*/
	l.lwz	r9,0(r1)
	l.mtspr	r0,r3,OR1K_SPR_PIC_PICSR_ADDR
	OR1K_DELAYED(
		OR1K_INST(l.addi r1,r1,12),
		OR1K_INST(l.jr   r9)
	)

/* -------------------------------------------------------------------------- */
/*!Function to enable an interrupt handler in the PICMR
                                                                              */
/* -------------------------------------------------------------------------- */
	.global	or1k_interrupt_enable
	.type	or1k_interrupt_enable,@function

	/* r3 should have IRQ line for peripheral */
or1k_interrupt_enable:
	l.addi 	r1,r1,-4
	l.sw	0(r1),r4
	l.ori	r4,r0,0x1
	l.sll	r4,r4,r3
	l.mfspr	r3,r0,OR1K_SPR_PIC_PICMR_ADDR
	l.or	r3,r3,r4
	l.mtspr	r0,r3,OR1K_SPR_PIC_PICMR_ADDR
	l.lwz	r4,0(r1)
	OR1K_DELAYED(
		OR1K_INST(l.addi	r1,r1,4),
		OR1K_INST(l.jr	r9)
	)

/* -------------------------------------------------------------------------- */
/*!Function to disable an interrupt handler in the PICMR
                                                                              */
/* -------------------------------------------------------------------------- */
	.global	or1k_interrupt_disable
	.type	or1k_interrupt_disable,@function

	/* r3 should have IRQ line for peripheral */
or1k_interrupt_disable:
	l.addi 	r1,r1,-4
	l.sw	0(r1),r4
	l.ori	r4,r0,0x1
	l.sll	r4,r4,r3
	l.xori	r4,r4,0xffff
	l.mfspr	r3,r0,OR1K_SPR_PIC_PICMR_ADDR
	l.and	r3,r3,r4
	l.mtspr	r0,r3,OR1K_SPR_PIC_PICMR_ADDR
	l.lwz	r4,0(r1)
	OR1K_DELAYED(
		OR1K_INST(l.addi r1,r1,4),
		OR1K_INST(l.jr   r9)
	)
