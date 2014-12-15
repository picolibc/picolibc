#include "include/or1k-asm.h"
#include "include/or1k-sprs.h"

	.section .text


	.global	or1k_has_multicore_support
	.type	or1k_has_multicore_support,@function
or1k_has_multicore_support:
#ifdef __OR1K_MULTICORE__
	// Return 1
	OR1K_DELAYED(
		OR1K_INST(l.ori r11,r0,1),
		OR1K_INST(l.jr  r9)
	)
#else
	// Return 0
	OR1K_DELAYED(
		OR1K_INST(l.or r11,r0,r0),
		OR1K_INST(l.jr r9)
	)
#endif

	.global	or1k_coreid
	.type	or1k_coreid,@function
or1k_coreid:
#ifdef __OR1K_MULTICORE__
	// Return SPR with core identifier
	OR1K_DELAYED(
		OR1K_INST(l.mfspr r11,r0,OR1K_SPR_SYS_COREID_ADDR),
		OR1K_INST(l.jr    r9)
	)
#else
	// Return 0
	OR1K_DELAYED(
		OR1K_INST(l.or r11,r0,r0),
		OR1K_INST(l.jr r9)
	)
#endif

	.global	or1k_numcores
	.type	or1k_numcores,@function
or1k_numcores:
#ifdef __OR1K_MULTICORE__
	// Return SPR with number of cores
	OR1K_DELAYED(
		OR1K_INST(l.mfspr r11,r0,OR1K_SPR_SYS_NUMCORES_ADDR),
		OR1K_INST(l.jr    r9)
	)
#else
	// Return 1
	OR1K_DELAYED(
		OR1K_INST(l.ori r11,r0,1),
		OR1K_INST(l.jr r9)
	)
#endif

	.global	or1k_sync_ll
	.type	or1k_sync_ll,@function
or1k_sync_ll:
#ifdef __OR1K_MULTICORE__
	// Load word atomic
	OR1K_DELAYED(
		OR1K_INST(l.lwa r11, 0(r3)),
		OR1K_INST(l.jr  r9)
	)
#else
	// Simply load word, TODO: throw exception? which?
	OR1K_DELAYED(
		OR1K_INST(l.lwz r11, 0(r3)),
		OR1K_INST(l.jr  r9)
	)
#endif

	.global	or1k_sync_sc
	.type	or1k_sync_sc,@function
or1k_sync_sc:
#ifdef __OR1K_MULTICORE__
	// swa sets the flag if it was succesfull
	// Store the value to address and set flag
	l.swa 0(r3),r4
	OR1K_DELAYED(
		// Set return to success speculatively (may go to delay slot)
		OR1K_INST(l.ori r11,r0,1),
		// If the swa was successfull, jump to end
		OR1K_INST(l.bf  .or1k_sync_sc_done)
	)
	// If the swa was not successfull, set
	l.or r11,r0,r0
.or1k_sync_sc_done:
	OR1K_DELAYED_NOP(OR1K_INST(l.jr r9))
#else
	// Simply store word, TODO: throw exception? which?
	OR1K_DELAYED(
		OR1K_INST(l.sw 0(r3),r4),
		OR1K_INST(l.jr r9)
	)
#endif


	.global or1k_sync_cas
	.type	or1k_sync_sc,@function
or1k_sync_cas:
#ifdef __OR1K_MULTICORE__
	/* Load linked address value to return register */
	l.lwa	r11,0(r3)
	/* Compare value to parameter */
	l.sfeq	r11,r4
	/* If not equal: abort and return the read value */
	OR1K_DELAYED_NOP(OR1K_INST(l.bnf .or1k_sync_cas_done))
	/* If compare was successfull: try writing */
	l.swa	0(r3),r5
	/* If writing was not successful: restart */
	OR1K_DELAYED_NOP(OR1K_INST(l.bnf or1k_sync_cas))
.or1k_sync_cas_done:
	/* Return value is the original read value */
	OR1K_DELAYED_NOP(OR1K_INST(l.jr r9))
#else
	// Non-atomic CAS, TODO: throw exception? which?
	l.lwz	r11,0(r3)
	l.sfeq	r11,r4
	OR1K_DELAYED_NOP(OR1K_INST(l.bnf .or1k_sync_cas_done))
	l.sw	0(r3),r5
.or1k_sync_cas_done:
	OR1K_DELAYED_NOP(OR1K_INST(l.jr r9))
#endif

	.global or1k_sync_tsl
	.type	or1k_sync_tsl,@function
or1k_sync_tsl:
	l.or	r4,r0,r0
	OR1K_DELAYED(
		OR1K_INST(l.addi r5,r0,1),
		OR1K_INST(l.j    or1k_sync_cas)
	)
