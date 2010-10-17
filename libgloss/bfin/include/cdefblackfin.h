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
 * cdefblackfin.h
 *
 * (c) Copyright 2002-2005 Analog Devices, Inc.  All rights reserved.
 *
 ************************************************************************/

#ifndef _CDEF_BLACKFIN_H
#define _CDEF_BLACKFIN_H

#if defined(__ADSPLPBLACKFIN__)
#warning cdefblackfin.h should only be included for 535 compatible chips.
#endif
#include <defblackfin.h>

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_19_4:"some macro definitions not MISRA compliant")
#endif /* _MISRA_RULES */

#ifndef _PTR_TO_VOL_VOID_PTR
#ifndef _USE_LEGACY_CDEF_BEHAVIOUR
#define _PTR_TO_VOL_VOID_PTR (void * volatile *)
#else
#define _PTR_TO_VOL_VOID_PTR (volatile void **)
#endif
#endif

/* Cache & SRAM Memory */
#define pSRAM_BASE_ADDRESS (_PTR_TO_VOL_VOID_PTR SRAM_BASE_ADDRESS)
#define pDMEM_CONTROL ((volatile unsigned long *)DMEM_CONTROL)
#define pDCPLB_STATUS ((volatile unsigned long *)DCPLB_STATUS)
#define pDCPLB_FAULT_ADDR (_PTR_TO_VOL_VOID_PTR DCPLB_FAULT_ADDR)
#define pDCPLB_ADDR0 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR0)
#define pDCPLB_ADDR1 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR1)
#define pDCPLB_ADDR2 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR2)
#define pDCPLB_ADDR3 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR3)
#define pDCPLB_ADDR4 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR4)
#define pDCPLB_ADDR5 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR5)
#define pDCPLB_ADDR6 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR6)
#define pDCPLB_ADDR7 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR7)
#define pDCPLB_ADDR8 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR8)
#define pDCPLB_ADDR9 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR9)
#define pDCPLB_ADDR10 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR10)
#define pDCPLB_ADDR11 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR11)
#define pDCPLB_ADDR12 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR12)
#define pDCPLB_ADDR13 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR13)
#define pDCPLB_ADDR14 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR14)
#define pDCPLB_ADDR15 (_PTR_TO_VOL_VOID_PTR DCPLB_ADDR15)
#define pDCPLB_DATA0 ((volatile unsigned long *)DCPLB_DATA0)
#define pDCPLB_DATA1 ((volatile unsigned long *)DCPLB_DATA1)
#define pDCPLB_DATA2 ((volatile unsigned long *)DCPLB_DATA2)
#define pDCPLB_DATA3 ((volatile unsigned long *)DCPLB_DATA3)
#define pDCPLB_DATA4 ((volatile unsigned long *)DCPLB_DATA4)
#define pDCPLB_DATA5 ((volatile unsigned long *)DCPLB_DATA5)
#define pDCPLB_DATA6 ((volatile unsigned long *)DCPLB_DATA6)
#define pDCPLB_DATA7 ((volatile unsigned long *)DCPLB_DATA7)
#define pDCPLB_DATA8 ((volatile unsigned long *)DCPLB_DATA8)
#define pDCPLB_DATA9 ((volatile unsigned long *)DCPLB_DATA9)
#define pDCPLB_DATA10 ((volatile unsigned long *)DCPLB_DATA10)
#define pDCPLB_DATA11 ((volatile unsigned long *)DCPLB_DATA11)
#define pDCPLB_DATA12 ((volatile unsigned long *)DCPLB_DATA12)
#define pDCPLB_DATA13 ((volatile unsigned long *)DCPLB_DATA13)
#define pDCPLB_DATA14 ((volatile unsigned long *)DCPLB_DATA14)
#define pDCPLB_DATA15 ((volatile unsigned long *)DCPLB_DATA15)
#define pDTEST_COMMAND ((volatile unsigned long *)DTEST_COMMAND)
#define pDTEST_DATA0 ((volatile unsigned long *)DTEST_DATA0)
#define pDTEST_DATA1 ((volatile unsigned long *)DTEST_DATA1)
#define pIMEM_CONTROL ((volatile unsigned long *)IMEM_CONTROL)
#define pICPLB_STATUS ((volatile unsigned long *)ICPLB_STATUS)
#define pICPLB_FAULT_ADDR (_PTR_TO_VOL_VOID_PTR ICPLB_FAULT_ADDR)
#define pICPLB_ADDR0 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR0)
#define pICPLB_ADDR1 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR1)
#define pICPLB_ADDR2 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR2)
#define pICPLB_ADDR3 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR3)
#define pICPLB_ADDR4 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR4)
#define pICPLB_ADDR5 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR5)
#define pICPLB_ADDR6 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR6)
#define pICPLB_ADDR7 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR7)
#define pICPLB_ADDR8 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR8)
#define pICPLB_ADDR9 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR9)
#define pICPLB_ADDR10 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR10)
#define pICPLB_ADDR11 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR11)
#define pICPLB_ADDR12 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR12)
#define pICPLB_ADDR13 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR13)
#define pICPLB_ADDR14 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR14)
#define pICPLB_ADDR15 (_PTR_TO_VOL_VOID_PTR ICPLB_ADDR15)
#define pICPLB_DATA0 ((volatile unsigned long *)ICPLB_DATA0)
#define pICPLB_DATA1 ((volatile unsigned long *)ICPLB_DATA1)
#define pICPLB_DATA2 ((volatile unsigned long *)ICPLB_DATA2)
#define pICPLB_DATA3 ((volatile unsigned long *)ICPLB_DATA3)
#define pICPLB_DATA4 ((volatile unsigned long *)ICPLB_DATA4)
#define pICPLB_DATA5 ((volatile unsigned long *)ICPLB_DATA5)
#define pICPLB_DATA6 ((volatile unsigned long *)ICPLB_DATA6)
#define pICPLB_DATA7 ((volatile unsigned long *)ICPLB_DATA7)
#define pICPLB_DATA8 ((volatile unsigned long *)ICPLB_DATA8)
#define pICPLB_DATA9 ((volatile unsigned long *)ICPLB_DATA9)
#define pICPLB_DATA10 ((volatile unsigned long *)ICPLB_DATA10)
#define pICPLB_DATA11 ((volatile unsigned long *)ICPLB_DATA11)
#define pICPLB_DATA12 ((volatile unsigned long *)ICPLB_DATA12)
#define pICPLB_DATA13 ((volatile unsigned long *)ICPLB_DATA13)
#define pICPLB_DATA14 ((volatile unsigned long *)ICPLB_DATA14)
#define pICPLB_DATA15 ((volatile unsigned long *)ICPLB_DATA15)
#define pITEST_COMMAND ((volatile unsigned long *)ITEST_COMMAND)
#define pITEST_DATA0 ((volatile unsigned long *)ITEST_DATA0)
#define pITEST_DATA1 ((volatile unsigned long *)ITEST_DATA1)

/* Event/Interrupt Registers */
#define pEVT0 (_PTR_TO_VOL_VOID_PTR EVT0)
#define pEVT1 (_PTR_TO_VOL_VOID_PTR EVT1)
#define pEVT2 (_PTR_TO_VOL_VOID_PTR EVT2)
#define pEVT3 (_PTR_TO_VOL_VOID_PTR EVT3)
#define pEVT4 (_PTR_TO_VOL_VOID_PTR EVT4)
#define pEVT5 (_PTR_TO_VOL_VOID_PTR EVT5)
#define pEVT6 (_PTR_TO_VOL_VOID_PTR EVT6)
#define pEVT7 (_PTR_TO_VOL_VOID_PTR EVT7)
#define pEVT8 (_PTR_TO_VOL_VOID_PTR EVT8)
#define pEVT9 (_PTR_TO_VOL_VOID_PTR EVT9)
#define pEVT10 (_PTR_TO_VOL_VOID_PTR EVT10)
#define pEVT11 (_PTR_TO_VOL_VOID_PTR EVT11)
#define pEVT12 (_PTR_TO_VOL_VOID_PTR EVT12)
#define pEVT13 (_PTR_TO_VOL_VOID_PTR EVT13)
#define pEVT14 (_PTR_TO_VOL_VOID_PTR EVT14)
#define pEVT15 (_PTR_TO_VOL_VOID_PTR EVT15)
#define pIMASK ((volatile unsigned short *)IMASK)
#define pIPEND ((volatile unsigned short *)IPEND)
#define pILAT ((volatile unsigned short *)ILAT)

/* Core Timer Registers */
#define pTCNTL ((volatile unsigned long *)TCNTL)
#define pTPERIOD ((volatile unsigned long *)TPERIOD)
#define pTSCALE ((volatile unsigned long *)TSCALE)
#define pTCOUNT ((volatile unsigned long *)TCOUNT)

/* Debug/MP/Emulation Registers */
#define pDSPID ((volatile unsigned long *)DSPID)
#define pDBGCTL ((volatile unsigned long *)DBGCTL)
#define pDBGSTAT ((volatile unsigned long *)DBGSTAT)
#define pEMUDAT ((volatile unsigned long *)EMUDAT)

/* Trace Buffer Registers */
#define pTBUFCTL ((volatile unsigned long *)TBUFCTL)
#define pTBUFSTAT ((volatile unsigned long *)TBUFSTAT)
#define pTBUF (_PTR_TO_VOL_VOID_PTR TBUF)

/* Watch Point Control Registers */
#define pWPIACTL ((volatile unsigned long *)WPIACTL)
#define pWPIA0 (_PTR_TO_VOL_VOID_PTR WPIA0)
#define pWPIA1 (_PTR_TO_VOL_VOID_PTR WPIA1)
#define pWPIA2 (_PTR_TO_VOL_VOID_PTR WPIA2)
#define pWPIA3 (_PTR_TO_VOL_VOID_PTR WPIA3)
#define pWPIA4 (_PTR_TO_VOL_VOID_PTR WPIA4)
#define pWPIA5 (_PTR_TO_VOL_VOID_PTR WPIA5)
#define pWPIACNT0 ((volatile unsigned long *)WPIACNT0)
#define pWPIACNT1 ((volatile unsigned long *)WPIACNT1)
#define pWPIACNT2 ((volatile unsigned long *)WPIACNT2)
#define pWPIACNT3 ((volatile unsigned long *)WPIACNT3)
#define pWPIACNT4 ((volatile unsigned long *)WPIACNT4)
#define pWPIACNT5 ((volatile unsigned long *)WPIACNT5)
#define pWPDACTL ((volatile unsigned long *)WPDACTL)
#define pWPDA0 (_PTR_TO_VOL_VOID_PTR WPDA0)
#define pWPDA1 (_PTR_TO_VOL_VOID_PTR WPDA1)
#define pWPDACNT0 ((volatile unsigned long *)WPDACNT0)
#define pWPDACNT1 ((volatile unsigned long *)WPDACNT1)
#define pWPSTAT ((volatile unsigned long *)WPSTAT)

/* Performance Monitor Registers */
#define pPFCTL ((volatile unsigned long *)PFCTL)
#define pPFCNTR0 ((volatile unsigned long *)PFCNTR0)
#define pPFCNTR1 ((volatile unsigned long *)PFCNTR1)

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* _CDEF_BLACKFIN_H */
