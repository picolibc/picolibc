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
 * Copyright (C) 2008 Analog Devices, Inc.
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


#if !defined(__SILICON_REVISION__)
#define __FORCE_LEGACY_WORKAROUNDS__
#endif


/* 05-00-0096 - PREFETCH, FLUSH, and FLUSHINV must be followed by a CSYNC
**
**  ADSP-BF531/2/3 - revs 0.0-0.1,
**  ADSP-BF561 - revs 0.0-0.1 (not supported in VDSP++ 4.0)
**
*/
#define WA_05000096 \
  ((defined(__ADSPBF531__) ||  \
    defined(__ADSPBF532__) ||  \
    defined(__ADSPBF533__) ||  \
    defined(__ADSPBF561__)) && \
  ((defined(__SILICON_REVISION__) && \
   (__SILICON_REVISION__ == 0xffff || __SILICON_REVISION__ <= 0x1)) || \
   defined(__FORCE_LEGACY_WORKAROUNDS__)))


/* 05-00-0109 - Reserved bits in SYSCFG register not set at power on
**
**  ADSP-BF531/2/3 - revs 0.0-0.2 (fixed 0.3)
**  ADSP-BF561 - revs 0.0-0.2 (fixed 0.3. 0.0, 0.1 not supported in VDSP++ 4.0)
**
** Changes to start code.
*/
#define WA_05000109 \
  ((defined(__ADSPBF531__) ||  \
    defined(__ADSPBF532__) ||  \
    defined(__ADSPBF533__) ||  \
    defined(__ADSPBF561__)) && \
  ((defined(__SILICON_REVISION__) && \
   (__SILICON_REVISION__ == 0xffff || __SILICON_REVISION__ <= 0x2)) || \
   defined(__FORCE_LEGACY_WORKAROUNDS__)))


/* 05-00-0123 - DTEST_COMMAND initiated memory access may be incorrect if
** data cache or DMA is active.
**
**  ADSP-BF531/2/3 - revs 0.1-0.2 (fixed 0.3)
**  ADSP-BF561 - revs 0.0-0.2 (0.0 and 0.1 not supported in VDSP++ 4.0)
*/
#define WA_05000123 \
  ((defined(__ADSPBF531__) ||  \
    defined(__ADSPBF532__) ||  \
    defined(__ADSPBF533__) ||  \
    defined(__ADSPBF561__)) && \
  ((defined(__SILICON_REVISION__) && \
   (__SILICON_REVISION__ == 0xffff || __SILICON_REVISION__ <= 0x2)) || \
   defined(__FORCE_LEGACY_WORKAROUNDS__)))


/* 05-00-0125 - Erroneous exception when enabling cache
**
**  ADSP-BF531/2/3 - revs 0.1-0.2 (fixed 0.3)
**  ADSP-BF561 - revs 0.0-0.2 (0.0 and 0.1 not supported in VDSP++ 4.0)
**
*/
#define WA_05000125 \
  ((defined(__ADSPBF531__) ||  \
    defined(__ADSPBF532__) ||  \
    defined(__ADSPBF533__) ||  \
    defined(__ADSPBF561__)) && \
  ((defined(__SILICON_REVISION__) && \
   (__SILICON_REVISION__ == 0xffff || __SILICON_REVISION__ <= 0x2)) || \
   defined(__FORCE_LEGACY_WORKAROUNDS__)))


/* 05-00-0137 - DMEM_CONTROL<12> is not set on Reset
**
**  ADSP-BF531/2/3 - revs 0.0-0.2 (fixed 0.3)
**
** Changes to start code.
**
*/
#define WA_05000137 \
  ((defined(__ADSPBF531__) ||  \
    defined(__ADSPBF532__) ||  \
    defined(__ADSPBF533__)) && \
  ((defined(__SILICON_REVISION__) && \
   (__SILICON_REVISION__ == 0xffff || __SILICON_REVISION__ <= 0x2)) || \
   defined(__FORCE_LEGACY_WORKAROUNDS__)))


/* 05-00-0158 - "Boot fails when data cache enabled: Data from a Data Cache
** fill can be  corrupted after or during instruction DMA if certain core
** stalls exist"
**
**  Impacted:
**    BF533/3/1 : 0.0-0.4 (fixed 0.5)
**
** The workaround we have only works for si-revisions >= 0.3. No workaround for
** ealier revisions.
*/
#define WA_05000158 \
  ((defined(__ADSPBF531__) ||  \
    defined(__ADSPBF532__) ||  \
    defined(__ADSPBF533__)) && \
  ((defined(__SILICON_REVISION__) && \
   (__SILICON_REVISION__ == 0xffff || \
     (__SILICON_REVISION__ >= 0x3 && \
      __SILICON_REVISION__ < 0x5))) || \
   defined(__FORCE_LEGACY_WORKAROUNDS__)))


/* 05-00-0204 - "Incorrect data read with write-through cache and
** allocate cache lines on reads only mode.
**
** This problem is cache related with high speed clocks. It apparently does
** not impact BF531 and BF532 because they cannot run at high enough clock
** to cause the anomaly. We build libs for BF532 though so that means we will
** need to do the workaround for BF532 and BF531 also.
**
** Also the 0.3 to 0.4 revision is not an inflexion for libs BF532 and BF561.
** This means a RT check may be required to avoid doing the WA for 0.4.
**
**  Impacted:
**     BF533 - 0.0-0.3 (fixed 0.4)
**     BF534 - 0.0 (fixed 0.1)
**     BF536 - 0.0 (fixed 0.1)
**     BF537 - 0.0 (fixed 0.1)
**     BF538 - 0.0 (fixed 0.1)
**     BF539 - 0.0 (fixed 0.1)
**     BF561 - 0.0-0.3 (fixed 0.4)
*/
#if defined(__ADI_LIB_BUILD__)
#  define __BUILDBF53123 1 /* building one single library for BF531/2/3 */
#else
#  define __BUILDBF53123 0
#endif

#define WA_05000204 \
   ((((__BUILDBF53123==1 && \
       (defined(__ADSPBF531__) || defined(__ADSPBF532__))) || \
      (defined(__ADSPBF533__) || defined(__ADSPBF561__))) && \
     (defined(__SILICON_REVISION__) && \
      (__SILICON_REVISION__ == 0xffff || __SILICON_REVISION__ <= 0x3))) || \
    ((defined(__ADSPBF534__) || defined(__ADSPBF536__) || \
      defined(__ADSPBF537__) || defined(__ADSPBF538__) || \
      defined(__ADSPBF539__)) && \
     (defined(__SILICON_REVISION__) && \
      (__SILICON_REVISION__ == 0xffff || __SILICON_REVISION__ == 0x0))))

#if ((defined(__ADSPBF531__) || defined(__ADSPBF532__) || \
      defined(__ADSPBF533__) || defined(__ADSPBF561__)) && \
     (defined(__SILICON_REVISION__) && \
      (__SILICON_REVISION__ == 0xffff || __SILICON_REVISION__ == 0x3)))
/* check at RT for 0.4 revs when doing 204 workaround */
#  define WA_05000204_CHECK_AVOID_FOR_REV <=3
#elif ((defined(__ADSPBF534__) || defined(__ADSPBF536__) || \
        defined(__ADSPBF537__) || defined(__ADSPBF538__) || \
        defined(__ADSPBF539__)) && \
     (defined(__SILICON_REVISION__) && \
      (__SILICON_REVISION__ == 0xffff || __SILICON_REVISION__ == 0x0)))
/* check at RT for 0.4 revs when doing 204 workaround */
#  define WA_05000204_CHECK_AVOID_FOR_REV <1
#else
/* do not check at RT for 0.4 revs when doing 204 workaround */
#endif

/* 05-00-0258 - "Instruction Cache is corrupted when bit 9 and 12 of
 * the ICPLB Data registers differ"
 *
 * When bit 9 and bit 12 of the ICPLB Data MMR differ, the cache may
 * not update properly.  For example, for a particular cache line,
 * the cache tag may be valid while the contents of that cache line
 * are not present in the cache.
 *
 * Impacted:
 *
 * BF531/2/3 - 0.0-0.4 (fixed 0.5)
 * BF534/6/7/8/9 - 0.0-0.2 (fixed 0.3)
 * BF561 - 0.0-0.4 (fixed 0.5)
 * BF535/AD6532/AD6900 - all revs
 */

#define WA_05000258 \
  defined(__SILICON_REVISION__) && \
    (__SILICON_REVISION__ == 0xffff || \
     !defined(__ADSPLPBLACKFIN__) || \
     ((defined(__ADSPBF531__) ||  \
       defined(__ADSPBF532__) ||  \
       defined(__ADSPBF533__)) && \
      (__SILICON_REVISION__ <= 0x4)) || \
     ((defined(__ADSPBF534__) ||  \
       defined(__ADSPBF536__) ||  \
       defined(__ADSPBF537__) ||  \
       defined(__ADSPBF538__) ||  \
       defined(__ADSPBF539__)) && \
      (__SILICON_REVISION__ <= 0x2)) || \
     ((defined(__ADSPBF561__)) && \
      (__SILICON_REVISION__ <= 0x4)) || \
     ((defined(__ADSPBF561__)) && \
      (__SILICON_REVISION__ < 0x1)))

/* 05-00-0259 - "Non-deterministic ICPLB descriptors delivered to
 * hardware". Whenever ICPLBs are disabled via an MMR write, immediately
 * follow this write with a CSYNC, and locate the MMR write and CSYNC
 * within the same aligned 64 bit word.
 *
 * This problem impacts all revisions of Blackfins.
 */

#define WA_05000259 \
	(defined(__ADSPBLACKFIN__) && defined(__SILICON_REVISION__))


/* 05-00-0261 - "DCPLB_FAULT_ADDR MMR may be corrupted".
 * The DCPLB_FAULT_ADDR MMR may contain the fault address of a
 * aborted memory access which generated both a protection exception
 * and a stall.
 *
 * We work around this by initially ignoring a DCPLB miss exception
 * on the assumption that the faulting address might be invalid.
 * We return without servicing. The exception will be raised
 * again when the faulting instruction is re-executed. The fault
 * address is correct this time round so the miss exception can
 * be serviced as normal. The only complication is we have to
 * ensure that we are about to service the same miss rather than
 * a miss raised within a higher-priority interrupt handler, where
 * the fault address could again be invalid. We therefore record
 * the last seen RETX and only service an exception when RETX and
 * the last seen RETX are equal.
 *
 * This problem impacts:
 * BF531/2/3 - rev 0.0-0.4 (fixed 0.5)
 * BF534/6/7/8/9 - rev 0.0-0.2 (fixed 0.3)
 * BF561 - rev 0.0-0.4 (fixed 0.5)
 *
 */

#define WA_05000261 \
	defined(__SILICON_REVISION__) && \
	 (__SILICON_REVISION__ == 0xffff || \
	  ((defined(__ADSPBF531__) ||  \
	    defined(__ADSPBF532__) ||  \
	    defined(__ADSPBF533__)) && \
	   (__SILICON_REVISION__ <= 0x4)) || \
	  ((defined(__ADSPBF534__) ||  \
	    defined(__ADSPBF536__) ||  \
	    defined(__ADSPBF537__) ||  \
	    defined(__ADSPBF538__) ||  \
	    defined(__ADSPBF539__)) && \
	   (__SILICON_REVISION__ <= 0x2)) || \
	  ((defined(__ADSPBF561__)) && \
	   (__SILICON_REVISION__ <= 0x4)) || \
	  ((defined(__ADSPBF561__)) && \
	   (__SILICON_REVISION__ < 0x1)))

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
 * BF531/2/3 - all revs
 * BF534/6/7/8/9 - all revs
 * BF561/6 - all revs
 */

#define WA_05000283 \
 defined(__ADSPLPBLACKFIN__) && defined(__SILICON_REVISION__)


