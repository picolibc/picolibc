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

/*
** Copyright (C) 2006-2008 Analog Devices Inc., All Rights Reserved.
**
************************************************************************************
**
** This include file contains a list of macro "defines" to enable the programmer
** to use symbolic names for the ADSP-BF514 peripherals.
**
************************************************************************************
** System MMR Register Map
************************************************************************************/

#ifndef _CDEF_BF514_H
#define _CDEF_BF514_H

/* include all Core registers and bit definitions */
#include <defBF514.h>

/* include core specific register pointer definitions */
#include <cdef_LPBlackfin.h>

/* SYSTEM & MMR ADDRESS DEFINITIONS FOR ADSP-BF514 */

/* include cdefBF51x_base.h for the set of #defines that are common to all ADSP-BF51x processors */
#include <cdefBF51x_base.h>

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_19_4:"some macro definitions not MISRA compliant")
#endif /* _MISRA_RULES */

/* The following are the #defines needed by ADSP-BF514 that are not in the common header */

/* SDH Registers (0xFFC03800 - 0xFFC03CFF)*/
#define pSDH_PWR_CTL			((volatile unsigned short *)SDH_PWR_CTL)
#define pSDH_CLK_CTL			((volatile unsigned short *)SDH_CLK_CTL)
#define pSDH_ARGUMENT			((volatile unsigned long  *)SDH_ARGUMENT)
#define pSDH_COMMAND			((volatile unsigned short *)SDH_COMMAND)
#define pSDH_RESP_CMD			((volatile unsigned short *)SDH_RESP_CMD)
#define pSDH_RESPONSE0			((volatile unsigned long  *)SDH_RESPONSE0)
#define pSDH_RESPONSE1			((volatile unsigned long  *)SDH_RESPONSE1)
#define pSDH_RESPONSE2			((volatile unsigned long  *)SDH_RESPONSE2)
#define pSDH_RESPONSE3			((volatile unsigned long  *)SDH_RESPONSE3)
#define pSDH_DATA_TIMER			((volatile unsigned long  *)SDH_DATA_TIMER)
#define pSDH_DATA_LGTH			((volatile unsigned short *)SDH_DATA_LGTH)
#define pSDH_DATA_CTL			((volatile unsigned short *)SDH_DATA_CTL)
#define pSDH_DATA_CNT			((volatile unsigned short *)SDH_DATA_CNT)
#define pSDH_STATUS			((volatile unsigned long  *)SDH_STATUS)
#define pSDH_STATUS_CLR			((volatile unsigned short *)SDH_STATUS_CLR)
#define pSDH_MASK0			((volatile unsigned long  *)SDH_MASK0)
#define pSDH_MASK1			((volatile unsigned long  *)SDH_MASK1)
#define pSDH_FIFO_CNT			((volatile unsigned short *)SDH_FIFO_CNT)
#define pSDH_FIFO				((volatile unsigned long  *)SDH_FIFO)
#define pSDH_E_STATUS			((volatile unsigned short *)SDH_E_STATUS)
#define pSDH_E_MASK			((volatile unsigned short *)SDH_E_MASK)
#define pSDH_CFG				((volatile unsigned short *)SDH_CFG)
#define pSDH_RD_WAIT_EN			((volatile unsigned short *)SDH_RD_WAIT_EN)
#define pSDH_PID0				((volatile unsigned short *)SDH_PID0)
#define pSDH_PID1				((volatile unsigned short *)SDH_PID1)
#define pSDH_PID2				((volatile unsigned short *)SDH_PID2)
#define pSDH_PID3				((volatile unsigned short *)SDH_PID3)
#define pSDH_PID4				((volatile unsigned short *)SDH_PID4)
#define pSDH_PID5				((volatile unsigned short *)SDH_PID5)
#define pSDH_PID6				((volatile unsigned short *)SDH_PID6)
#define pSDH_PID7				((volatile unsigned short *)SDH_PID7)


/* RSI Registers (0xFFC03800 - 0xFFC03CFF)*/
#define pRSI_PWR_CONTROL		((volatile unsigned short *)RSI_PWR_CONTROL)
#define pRSI_CLK_CONTROL		((volatile unsigned short *)RSI_CLK_CONTROL)
#define pRSI_ARGUMENT			((volatile unsigned long  *)RSI_ARGUMENT)
#define pRSI_COMMAND			((volatile unsigned short *)RSI_COMMAND)
#define pRSI_RESP_CMD			((volatile unsigned short *)RSI_RESP_CMD)
#define pRSI_RESPONSE0			((volatile unsigned long  *)RSI_RESPONSE0)
#define pRSI_RESPONSE1			((volatile unsigned long  *)RSI_RESPONSE1)
#define pRSI_RESPONSE2			((volatile unsigned long  *)RSI_RESPONSE2)
#define pRSI_RESPONSE3			((volatile unsigned long  *)RSI_RESPONSE3)
#define pRSI_DATA_TIMER			((volatile unsigned long  *)RSI_DATA_TIMER)
#define pRSI_DATA_LGTH			((volatile unsigned short *)RSI_DATA_LGTH)
#define pRSI_DATA_CONTROL		((volatile unsigned short *)RSI_DATA_CONTROL)
#define pRSI_DATA_CNT			((volatile unsigned short *)RSI_DATA_CNT)
#define pRSI_STATUS			((volatile unsigned long  *)RSI_STATUS)
#define pRSI_STATUSCL			((volatile unsigned short *)RSI_STATUSCL)
#define pRSI_MASK0			((volatile unsigned long  *)RSI_MASK0)
#define pRSI_MASK1			((volatile unsigned long  *)RSI_MASK1)
#define pRSI_FIFO_CNT			((volatile unsigned short *)RSI_FIFO_CNT)
#define pRSI_CEATA_CONTROL		((volatile unsigned short *)RSI_CEATA_CONTROL)
#define pRSI_FIFO				((volatile unsigned long  *)RSI_FIFO)
#define pRSI_ESTAT			((volatile unsigned short *)RSI_ESTAT)
#define pRSI_EMASK			((volatile unsigned short *)RSI_EMASK)
#define pRSI_CONFIG			((volatile unsigned short *)RSI_CONFIG)
#define pRSI_RD_WAIT_EN			((volatile unsigned short *)RSI_RD_WAIT_EN)
#define pRSI_PID0				((volatile unsigned short *)RSI_PID0)
#define pRSI_PID1				((volatile unsigned short *)RSI_PID1)
#define pRSI_PID2				((volatile unsigned short *)RSI_PID2)
#define pRSI_PID3				((volatile unsigned short *)RSI_PID3)

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* _CDEF_BF514_H */
