/*
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

/************************************************************************
 *
 * anomaly_macros_rtl.h : $Revision$
 *
 * (c) Copyright 2005-2011 Analog Devices, Inc.  All rights reserved.
 *
 * This file defines macros used within the run-time libraries to enable
 * certain anomaly workarounds for the appropriate chips and silicon
 * revisions. Certain macros are defined for silicon-revision none - this
 * is to ensure behaviour is unchanged from libraries supplied with
 * earlier tools versions, where a small number of anomaly workarounds
 * were applied in all library flavours. __FORCE_LEGACY_WORKAROUNDS__
 * is defined in this case.
 *
 * This file defines macros for a subset of all anomalies that may impact
 * the run-time libraries.
 *
 ************************************************************************/


#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_2_4:"Assembly code in comment used to illustrate anomalous behaviour")
#endif /* _MISRA_RULES */

#if !defined(__SILICON_REVISION__)
#define __FORCE_LEGACY_WORKAROUNDS__
#endif


/* 050000245 - "False Hardware Error from an Access in the Shadow of a
**              Conditional Branch"
**
** If a load accesses reserved or illegal memory on the opposite control
** flow of a conditional jump to the taken path, a false hardware error
** will occur.
**
** This macro is used by System Services/Device Drivers.
**
** This is for all Blackfin LP parts.
*/
#define WA_05000245 \
  (defined(__ADSPLPBLACKFIN__) && defined(__SILICON_REVISION__))


/* 05-00-0229 - "SPI Slave Boot Mode Modifies Registers".
 * When the SPI slave boot completes, the final DMA IRQ is cleared
 * but the DMA5_CONFIG and SPI_CTL registers are not reset to their
 * default states.
 *
 * We work around this by resetting the registers to their default
 * values at the beginning of the CRT. The only issue would be when
 * users boot from flash and make use of the DMA or serial port.
 * In this case, users would need to modify the CRT.
 *
 * This problem impacts all revisions of ADSP-BF531/2/3/8/9
 */
#define WA_05000229 \
	(defined(__ADSPBLACKFIN__) && defined (__SILICON_REVISION__) && \
	 (defined(__ADSPBF531__) || defined(__ADSPBF532__) || \
	  defined(__ADSPBF533__) || defined(__ADSPBF538__) || \
	  defined(__ADSPBF539__)))


/* 05-00-0283 - "A system MMR write is stalled indefinitely when killed in a
 * particular stage".
 *
 * Where an interrupt occurs killing a stalled system MMR write, and the ISR
 * executes an SSYNC, execution execution may stall indefinitely".
 *
 * The workaround is to execute a mispredicted jump over a dummy MMR read,
 * thus killing the read. Also to avoid a system MMR write in two slots
 * after a not predicted conditional jump.
 *
 * This problem impacts:
 * BF531/2/3 - < 0.6
 * BF534/6/7 - < 0.3
 * BF538/9 - < 0.4
 * BF561/6 - < 0.5
 *
 * Since this impacts 538/9 0.3 but not 534 0.3 (the libraries that they use)
 * we have to enable this workaround for the 534 0.3 libraries (see bottom
 * two lines).
 */
#define WA_05000283 \
        (defined (__SILICON_REVISION__) && \
         (((defined(__ADSPBF531__) ||  \
            defined(__ADSPBF532__) ||  \
            defined(__ADSPBF533__)) && \
           (__SILICON_REVISION__ == 0xffff || \
            __SILICON_REVISION__ < 0x6)) || \
          ((defined(__ADSPBF534__) ||  \
            defined(__ADSPBF536__) ||  \
            defined(__ADSPBF537__)) && \
           (__SILICON_REVISION__ == 0xffff || \
            __SILICON_REVISION__ < 0x3)) || \
          ((defined(__ADSPBF538__) ||  \
            defined(__ADSPBF539__)) &&  \
           (__SILICON_REVISION__ == 0xffff || \
            __SILICON_REVISION__ < 0x4)) || \
          (defined(__ADSPBF561__)) || \
          (defined(__ADSPBF534__) && __SILICON_REVISION__ == 0x3 && \
           defined(__ADI_LIB_BUILD__))))


/* 05-00-0311 - Erroneous Flag (GPIO) Pin Operations under Specific Sequences
**
**  Impacted:
**     ADSP-BF53[123] - 0.0-0.5 (fixed in 0.6)
**
** Use by System Services/Device Drivers.
*/
#define WA_05000311 \
  (defined(__ADSPBF533_FAMILY__) && \
   (defined(__SILICON_REVISION__) && \
    (__SILICON_REVISION__ <= 0x5 || __SILICON_REVISION__ == 0xffff)))


/* 05-00-0312 - Errors when SSYNC, CSYNC, or Loads to LT, LB and LC Registers
**              Are Interrupted
**
**  Impacted:
**     ADSP-BF53[123]   - 0.0-0.5 (fixed in 0.6)
**     ADSP-BF53[467]   - all supported revisions
**     ADSP-BF53[89]    - 0.0-0.4 (fixed in 0.5)
**     ADSP-BF561       - all supported revisions
**     ADSP-BF54[24789] - 0.0 (fixed in 0.1)
**
** Used by VDK
*/
#define WA_05000312 \
        (defined(__SILICON_REVISION__) && \
         ((defined(__ADSPBF533_FAMILY__) &&  \
           (__SILICON_REVISION__ <= 0x5 || __SILICON_REVISION__ == 0xffff)) || \
          (defined(__ADSPBF537_FAMILY__)) ||  \
          (defined(__ADSPBF538_FAMILY__) &&  \
           (__SILICON_REVISION__ <= 0x4 || __SILICON_REVISION__ == 0xffff)) || \
          (defined(__ADSPBF548_FAMILY__) && \
           (__SILICON_REVISION__ == 0x0 || __SILICON_REVISION__ == 0xffff)) || \
          (defined(__ADSPBF561_FAMILY__))))


/* 05-00-0323 - Erroneous Flag (GPIO) Pin Operations under Specific Sequences
**
**  Impacted:
**     ADSP-BF561 - all supported revisions
**
** Use by System Services/Device Drivers.
*/
#define WA_05000323 \
  (defined(__ADSPBF561__) && defined(__SILICON_REVISION__))


/* 05-00-0365 - DMAs that Go Urgent during Tight Core Writes to External
**              Memory Are Blocked
**
**  Impacted:
**     ADSP-BF54[24789] - all supported revisions
**     ADSP-BF54[24789]M - all supported revisions
**
** Use by System Services/Device Drivers.
*/
#define WA_05000365 \
  ((defined(__ADSPBF548_FAMILY__) || defined(__ADSPBF548M_FAMILY__)) && \
   defined(__SILICON_REVISION__))


/* 05-00-0371 - Possible RETS Register Corruption when Subroutine Is under
**              5 Cycles in Duration
**
** This problem impacts:
** BF531/2/3 - 0.0-0.5 (fixed in 0.6)
** BF534/6/7 - 0.0-0.3
** BF538/9   - 0.0-0.4 (fixed in 0.5)
** BF561     - 0.0-0.5
** BF542/4/7/8/9  - 0.0-0.1 (fixed in 0.2)
** BF523/5/7 - 0.0-0.1 (fixed in 0.2)
**
*/
#define WA_05000371 \
  (defined(__SILICON_REVISION__) && \
   ((defined(__ADSPBF533_FAMILY__) && \
     (__SILICON_REVISION__ <= 0x5 || __SILICON_REVISION__ == 0xffff)) || \
    (defined(__ADSPBF537_FAMILY__) && \
     (__SILICON_REVISION__ <= 0x3 || __SILICON_REVISION__ == 0xffff)) || \
    (defined(__ADSPBF538_FAMILY__) && \
     (__SILICON_REVISION__ <= 0x4 || __SILICON_REVISION__ == 0xffff)) || \
    (defined(__ADSPBF548_FAMILY__) && \
     (__SILICON_REVISION__ <= 0x1 || __SILICON_REVISION__ == 0xffff)) || \
    (defined(__ADSPBF527_FAMILY__) && \
     (__SILICON_REVISION__ <= 0x1 || __SILICON_REVISION__ == 0xffff)) || \
    (defined(__ADSPBF561__) || defined(__ADSPBF566__))))


/* 05-00-0412 - "TESTSET Instruction Causes Data Corruption with Writeback Data
 * Cache Enabled"
 *
 * If you use the testset instruction to operate on L2 memory and you have data
 * in external memory that is cached using WB mode, data in external memory
 * and/or L2 memory can be corrupted.
 *
 * Workaround: Either do not use writeback cache or precede the TESTSET
 * instruction with an SSYNC instruction. If preceding the TESTSET instruction
 * by an SSYNC instruction, do the following:
 *
 *   CLI R0
 *   R1 = [P0]  // perform a dummy read to make sure CPLB is installed
 *   NOP
 *   NOP
 *   SSYNC
 *   TESTSET (P0)
 *   STI R0
 *
 * This problem impacts:
 * BF561/6 - rev 0.0-0.5
 *
 */
#define WA_05000412 \
        (defined (__SILICON_REVISION__) && defined(__ADSPBF561__))


/* 05-00-0426 - Speculative Fetches of Indirect-Pointer Instructions Can
**              Cause False Hardware Errors
**
**
** A false hardware error is generated if there is an indirect jump or
** call through a pointer which may point to reserved or illegal memory
** on the opposite control flow of a conditional jump to the taken path.
** This commonly occurs when using function pointers, which can be
** invalid (e.g., set to -1).
**
** Workaround: If instruction cache is on or the ICPLBs are enabled,
** this anomaly does not apply. If instruction cache is off and ICPLBs
** are disabled, the indirect pointer instructions must be 2 instructions
** away from the branch instruction, which can be implemented using NOPs:
**
**
**  Impacted:
**     All parts and revisions other than BF535 based parts.
**
** Used by System Services/Device Drivers.
*/
#define WA_05000426 \
   (defined(__ADSPLPBLACKFIN__) && defined(__SILICON_REVISION__))


/* 05-00-0428 - "Lost/Corrupted Write to L2 Memory Following Speculative Read
 * by Core B from L2 Memory"
 *
 * This issue occurs only when the accesses are performed by core B of a BF561.
 *
 * When a write to internal L2 memory follows a speculative read from internal
 * L2 memory, the L2 write may be lost or corrupted. For this anomaly to occur,
 * the speculative read must be caused by a read in the shadow of a branch. The
 * accesses do not have to be consecutive accesses. In other words, the problem
 * can occur even if there are multiple instructions between the speculative
 * read and the write, as shown in the following example:
 *
 *   R1 = 1; R2 = 1;
 *   CC = R1 == R2;
 *   IF CC JUMP X;  // Always true...
 *   R0 = [P0];     // If any of these three loads accesses L2 memory from Core
 *   R1 = [P1];     // B, speculative execution in the pipeline causes the
 *   R2 = [P2];     // anomaly trigger condition.
 *   X:
 *   ...            // Any number of instructions...
 *   [P0] = R0;  // This write can be corrupted or lost.
 *
 * The issue does not occur if the speculative read access is caused by an
 * interrupt or exception.
 *
 * The workaround required depends upon the conditional branch instruction.
 * If the evaluated condition is true and the branch is predicted, then the
 * workaround is to ensure that the target instruction is not be a load
 * instruction, for example:
 *
 *   IF CC JUMP X (BP);
 *   ...
 *   X: <load that might be from L2 memory>
 *
 * If the evaluated condition is false and the branch is not predicted, then
 * the workaround is to make sure that none of the three instructions that
 * are executed after the conditional JUMP are load instructions, for example:
 *
 *   IF CC JUMP ...;
 *   <load that might be from L2 memory>
 *   <load that might be from L2 memory>
 *   <load that might be from L2 memory>
 *
 * This problem impacts:
 * BF561 - rev 0.4,0.5
 *
 */
#define WA_05000428 \
        (defined(__SILICON_REVISION__) && \
         defined(__ADSPBF561__)        && \
         ((__SILICON_REVISION__ == 0xffff) || \
          (__SILICON_REVISION__ == 0x4)    || \
          (__SILICON_REVISION__ == 0x5)))


/* 05-00-0443 - IFLUSH Instruction at End of Hardware Loop Causes Infinite Stall
**
**  Impacted:
**     All parts and revisions other than BF535 based parts.
**
** Used by System Services/Device Drivers.
*/
#define WA_05000443 \
   (defined(__ADSPLPBLACKFIN__) && defined(__SILICON_REVISION__))


/* 16-00-0005 - "Using L1 Instruction Cache with Parity Enabled is Unreliable."
**
** Using L1 instruction cache with parity enabled is unreliable and may cause
** unpredictable results.
**
** Impacted:
**    BF6xx.
*/
#define WA_16000005 \
  (defined(__ADSPBF60x__) && defined(__SILICON_REVISION__))

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

