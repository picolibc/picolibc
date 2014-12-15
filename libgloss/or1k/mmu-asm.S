/* mmu-asm.S -- MMU handling for OpenRISC 1000.
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

/* -------------------------------------------------------------------------- */
/*!Function to control MMU
                                                                              */
/* -------------------------------------------------------------------------- */

#include "include/or1k-asm.h"
#include "include/or1k-sprs.h"

/* MMU control functions always switch MMU control with a l.rfe to return
   from function */
	.section .text

	.global or1k_dmmu_enable
or1k_dmmu_enable:
	l.mfspr	r3,r0,OR1K_SPR_SYS_SR_ADDR
	l.ori	r3,r3,OR1K_SPR_SYS_SR_DME_MASK
	l.mtspr	r0,r3,OR1K_SPR_SYS_ESR_BASE
	l.mtspr	r0,r9,OR1K_SPR_SYS_EPCR_BASE
	OR1K_DELAYED_NOP(OR1K_INST(l.rfe))


	.global or1k_dmmu_disable
or1k_dmmu_disable:
	l.ori	r3,r0,OR1K_SPR_SYS_SR_DME_MASK
	l.xori	r4,r3,0xffff
	l.mfspr	r3,r0,OR1K_SPR_SYS_SR_ADDR
	l.and	r3,r4,r3
	l.mtspr	r0,r3,OR1K_SPR_SYS_ESR_BASE
	l.mtspr	r0,r9,OR1K_SPR_SYS_EPCR_BASE
	OR1K_DELAYED_NOP(OR1K_INST(l.rfe))


	.global or1k_immu_enable
or1k_immu_enable:
	l.mfspr	r3,r0,OR1K_SPR_SYS_SR_ADDR
	l.ori	r3,r3,OR1K_SPR_SYS_SR_IME_MASK
	l.mtspr	r0,r3,OR1K_SPR_SYS_ESR_BASE
	l.mtspr	r0,r9,OR1K_SPR_SYS_EPCR_BASE
	OR1K_DELAYED_NOP(OR1K_INST(l.rfe))

	.global or1k_immu_disable
or1k_immu_disable:
	l.ori	r3,r0,OR1K_SPR_SYS_SR_IME_MASK
	l.xori	r4,r3,0xffff
	l.mfspr	r3,r0,OR1K_SPR_SYS_SR_ADDR
	l.and	r3,r4,r3
	l.mtspr	r0,r3,OR1K_SPR_SYS_ESR_BASE
	l.mtspr	r0,r9,OR1K_SPR_SYS_EPCR_BASE
	OR1K_DELAYED_NOP(OR1K_INST(l.rfe))
