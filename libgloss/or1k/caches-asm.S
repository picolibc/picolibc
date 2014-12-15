/* caches-asm.S -- cache manipulation for OpenRISC 1000.
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
/*!Function used at reset to clear and enable all caches
                                                                              */
/* -------------------------------------------------------------------------- */
	.global	_or1k_cache_init
	.type	_or1k_cache_init,@function

_or1k_cache_init:
	/* Instruction cache enable */
	/* Check if IC present and skip enabling otherwise */
	l.mfspr	r3,r0,OR1K_SPR_SYS_UPR_ADDR
	l.andi	r4,r3,OR1K_SPR_SYS_UPR_ICP_MASK
	l.sfeq	r4,r0
	OR1K_DELAYED_NOP(OR1K_INST(l.bf .Lnoic))

	/* Disable IC */
	l.mfspr	r6,r0,OR1K_SPR_SYS_SR_ADDR
	l.addi	r5,r0,-1
	l.xori	r5,r5,OR1K_SPR_SYS_SR_ICE_MASK
	l.and	r5,r6,r5
	l.mtspr	r0,r5,OR1K_SPR_SYS_SR_ADDR

	/* Establish cache block size
	If BS=0, 16;
	If BS=1, 32;
	r14 contain block size
	*/
	l.mfspr	r3,r0,OR1K_SPR_SYS_ICCFGR_ADDR
	l.andi	r4,r3,OR1K_SPR_SYS_ICCFGR_CBS_MASK
	l.srli	r7,r4,7
	l.ori	r8,r0,16
	l.sll	r14,r8,r7

	/* Establish number of cache sets
	r13 contains number of cache sets
	r7 contains log(# of cache sets)
	*/
	l.andi	r4,r3,OR1K_SPR_SYS_ICCFGR_NCS_MASK
	l.srli	r7,r4,3
	l.ori	r8,r0,1
	l.sll	r13,r8,r7

	/* Invalidate IC */
	l.addi	r6,r0,0
	l.sll	r5,r14,r7

.Linvi:	l.mtspr	r0,r6,OR1K_SPR_ICACHE_ICBIR_ADDR
	l.sfne	r6,r5
	OR1K_DELAYED(
		OR1K_INST(l.add r6,r6,r14),
		OR1K_INST(l.bf  .Linvi)
	)

	/* Enable IC */
	l.mfspr	r6,r0,OR1K_SPR_SYS_SR_ADDR
	l.ori	r6,r6,OR1K_SPR_SYS_SR_ICE_MASK
	l.mtspr	r0,r6,OR1K_SPR_SYS_SR_ADDR
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop

	/* Data cache enable */
	/* Check if DC present and skip enabling otherwise */
.Lnoic:	l.mfspr	r3,r0,OR1K_SPR_SYS_UPR_ADDR
	l.andi	r4,r3,OR1K_SPR_SYS_UPR_DCP_MASK
	l.sfeq	r4,r0
	OR1K_DELAYED_NOP(OR1K_INST(l.bf .Lnodc))
	/* Disable DC */
	l.mfspr	r6,r0,OR1K_SPR_SYS_SR_ADDR
	l.addi	r5,r0,-1
	l.xori	r5,r5,OR1K_SPR_SYS_SR_DCE_MASK
	l.and	r5,r6,r5
	l.mtspr	r0,r5,OR1K_SPR_SYS_SR_ADDR
	/* Establish cache block size
	   If BS=0, 16;
	   If BS=1, 32;
	   r14 contain block size */
	l.mfspr	r3,r0,OR1K_SPR_SYS_DCCFGR_ADDR
	l.andi	r4,r3,OR1K_SPR_SYS_DCCFGR_CBS_MASK
	l.srli	r7,r4,7
	l.ori	r8,r0,16
	l.sll	r14,r8,r7
	/* Establish number of cache sets
	   r13 contains number of cache sets
	   r7 contains log(# of cache sets) */
	l.andi	r4,r3,OR1K_SPR_SYS_ICCFGR_NCS_MASK
	l.srli	r7,r4,3
	l.ori	r8,r0,1
	l.sll	r13,r8,r7
	/* Invalidate DC */
	l.addi	r6,r0,0
	l.sll	r5,r14,r7

.Linvd:	l.mtspr	r0,r6,OR1K_SPR_DCACHE_DCBIR_ADDR
	l.sfne	r6,r5
	OR1K_DELAYED(
		OR1K_INST(l.add r6,r6,r14),
		OR1K_INST(l.bf  .Linvd)
	)
	/* Enable DC */
	l.mfspr	r6,r0,OR1K_SPR_SYS_SR_ADDR
	l.ori	r6,r6,OR1K_SPR_SYS_SR_DCE_MASK
	l.mtspr	r0,r6,OR1K_SPR_SYS_SR_ADDR

.Lnodc:
	/* Return */
	OR1K_DELAYED_NOP(OR1K_INST(l.jr r9))

/* -------------------------------------------------------------------------- */
/*!Function to enable instruction cache
                                                                              */
/* -------------------------------------------------------------------------- */

	.global	or1k_icache_enable
	.type	or1k_icache_enable,@function

or1k_icache_enable:
	/* Enable IC */
	l.mfspr	r13,r0,OR1K_SPR_SYS_SR_ADDR
	l.ori	r13,r13,OR1K_SPR_SYS_SR_ICE_MASK
	l.mtspr	r0,r13,OR1K_SPR_SYS_SR_ADDR
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	OR1K_DELAYED_NOP(OR1K_INST(l.jr r9))

/* -------------------------------------------------------------------------- */
/*!Function to disable instruction cache
                                                                              */
/* -------------------------------------------------------------------------- */
	.global	or1k_icache_disable
	.type	or1k_icache_disable,@function

or1k_icache_disable:
	/* Disable IC */
	l.mfspr	r13,r0,OR1K_SPR_SYS_SR_ADDR
	l.addi	r12,r0,-1
	l.xori	r12,r12,OR1K_SPR_SYS_SR_ICE_MASK
	l.and	r12,r13,r12
	l.mtspr	r0,r12,OR1K_SPR_SYS_SR_ADDR
	OR1K_DELAYED_NOP(OR1K_INST(l.jr r9))

/* -------------------------------------------------------------------------- */
/*!Function to flush address of instruction cache
                                                                              */
/* -------------------------------------------------------------------------- */
	.global	or1k_icache_flush
	.type	or1k_icache_flush,@function

or1k_icache_flush:
	OR1K_DELAYED(
		OR1K_INST(l.mtspr r0,r3,OR1K_SPR_ICACHE_ICBIR_ADDR),
		/* Push r3 into IC invalidate reg */
		OR1K_INST(l.jr    r9)
	)


/* -------------------------------------------------------------------------- */
/*!Function to enable data cache
                                                                              */
/* -------------------------------------------------------------------------- */
	.global	or1k_dcache_enable
	.type	or1k_dcache_enable,@function

or1k_dcache_enable:
	/* Enable DC */
	l.mfspr	r13,r0,OR1K_SPR_SYS_SR_ADDR
	l.ori	r13,r13,OR1K_SPR_SYS_SR_DCE_MASK
	l.mtspr	r0,r13,OR1K_SPR_SYS_SR_ADDR
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	OR1K_DELAYED_NOP(OR1K_INST(l.jr r9))

/* -------------------------------------------------------------------------- */
/*!Function to disable data cache
                                                                              */
/* -------------------------------------------------------------------------- */
	.global	or1k_dcache_disable
	.type	or1k_dcache_disable,@function

or1k_dcache_disable:
	/* Disable DC */
	l.mfspr	r13,r0,OR1K_SPR_SYS_SR_ADDR
	l.addi	r12,r0,-1
	l.xori	r12,r12,OR1K_SPR_SYS_SR_DCE_MASK
	l.and	r12,r13,r12
	l.mtspr	r0,r12,OR1K_SPR_SYS_SR_ADDR
	OR1K_DELAYED_NOP(OR1K_INST(l.jr r9))

/* -------------------------------------------------------------------------- */
/*!Function to flush address of data cache
                                                                              */
/* -------------------------------------------------------------------------- */
	.global	or1k_dcache_flush
	.type	or1k_dcache_flush,@function

or1k_dcache_flush:
	OR1K_DELAYED(
		OR1K_INST(l.mtspr r0,r3,OR1K_SPR_DCACHE_DCBIR_ADDR),
		/* Push r3 into DC invalidate reg */
		OR1K_INST(l.jr    r9)
	)
