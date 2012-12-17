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

/* =========================================================================

     Project      :   ADSP-BF608
     File         :   cdefBF608.h
     Description  :   C register and bitfield definitions

     Date         :   06-07-2012
     Tag          :   BF60X_TOOLS_CCES_1_0_1

     Copyright (c) 2011-2012 Analog Devices, Inc.  All Rights Reserved.
     This software is proprietary and confidential to Analog Devices, Inc. and
     its licensors.

     This file was auto-generated. Do not make local changes to this file.

   ========================================================================= */
#ifndef _CDEF_BF608_H
#define _CDEF_BF608_H

#include <stdint.h>
#include <defBF608.h>

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_5_7:"ADI header will re-use identifiers")
#pragma diag(suppress:misra_rule_6_3:"ADI header allows use of basic types")
#endif /* _MISRA_RULES */




/* =========================================================================
       CNT0
   ========================================================================= */
#define pREG_CNT0_CFG                    ((volatile uint16_t *)REG_CNT0_CFG)                     /* CNT0 Configuration Register */
#define pREG_CNT0_IMSK                   ((volatile uint16_t *)REG_CNT0_IMSK)                    /* CNT0 Interrupt Mask Register */
#define pREG_CNT0_STAT                   ((volatile uint16_t *)REG_CNT0_STAT)                    /* CNT0 Status Register */
#define pREG_CNT0_CMD                    ((volatile uint16_t *)REG_CNT0_CMD)                     /* CNT0 Command Register */
#define pREG_CNT0_DEBNCE                 ((volatile uint16_t *)REG_CNT0_DEBNCE)                  /* CNT0 Debounce Register */
#define pREG_CNT0_CNTR                   ((volatile uint32_t *)REG_CNT0_CNTR)                    /* CNT0 Counter Register */
#define pREG_CNT0_MAX                    ((volatile uint32_t *)REG_CNT0_MAX)                     /* CNT0 Maximum Count Register */
#define pREG_CNT0_MIN                    ((volatile uint32_t *)REG_CNT0_MIN)                     /* CNT0 Minimum Count Register */


/* =========================================================================
       RSI0
   ========================================================================= */
#define pREG_RSI0_CTL                    ((volatile uint16_t *)REG_RSI0_CTL)                     /* RSI0 Control Register */
#define pREG_RSI0_ARG                    ((volatile uint32_t *)REG_RSI0_ARG)                     /* RSI0 Argument Register */
#define pREG_RSI0_CMD                    ((volatile uint16_t *)REG_RSI0_CMD)                     /* RSI0 Command Register */
#define pREG_RSI0_RESP_CMD               ((volatile uint16_t *)REG_RSI0_RESP_CMD)                /* RSI0 Response Command Register */
#define pREG_RSI0_RESP0                  ((volatile uint32_t *)REG_RSI0_RESP0)                   /* RSI0 Response 0 Register */
#define pREG_RSI0_RESP1                  ((volatile uint32_t *)REG_RSI0_RESP1)                   /* RSI0 Response 1 Register */
#define pREG_RSI0_RESP2                  ((volatile uint32_t *)REG_RSI0_RESP2)                   /* RSI0 Response 2 Register */
#define pREG_RSI0_RESP3                  ((volatile uint32_t *)REG_RSI0_RESP3)                   /* RSI0 Response 3 Register */
#define pREG_RSI0_DATA_TMR               ((volatile uint32_t *)REG_RSI0_DATA_TMR)                /* RSI0 Data Timer Register */
#define pREG_RSI0_DATA_LEN               ((volatile uint16_t *)REG_RSI0_DATA_LEN)                /* RSI0 Data Length Register */
#define pREG_RSI0_DATA_CTL               ((volatile uint16_t *)REG_RSI0_DATA_CTL)                /* RSI0 Data Control Register */
#define pREG_RSI0_DATA_CNT               ((volatile uint16_t *)REG_RSI0_DATA_CNT)                /* RSI0 Data Count Register */
#define pREG_RSI0_XFRSTAT                ((volatile uint32_t *)REG_RSI0_XFRSTAT)                 /* RSI0 Status Register */
#define pREG_RSI0_XFRSTAT_CLR            ((volatile uint16_t *)REG_RSI0_XFRSTAT_CLR)             /* RSI0 Status Clear Register */
#define pREG_RSI0_XFR_IMSK0              ((volatile uint32_t *)REG_RSI0_XFR_IMSK0)               /* RSI0 Interrupt 0 Mask Register */
#define pREG_RSI0_XFR_IMSK1              ((volatile uint32_t *)REG_RSI0_XFR_IMSK1)               /* RSI0 Interrupt 1 Mask Register */
#define pREG_RSI0_FIFO_CNT               ((volatile uint16_t *)REG_RSI0_FIFO_CNT)                /* RSI0 FIFO Counter Register */
#define pREG_RSI0_CEATA                  ((volatile uint32_t *)REG_RSI0_CEATA)                   /* RSI0 This register contains bit to dis CCS gen */
#define pREG_RSI0_BOOT_TCNTR             ((volatile uint16_t *)REG_RSI0_BOOT_TCNTR)              /* RSI0 Boot Timing Counter Register */
#define pREG_RSI0_BACK_TOUT              ((volatile uint32_t *)REG_RSI0_BACK_TOUT)               /* RSI0 Boot Acknowledge Timeout Register */
#define pREG_RSI0_SLP_WKUP_TOUT          ((volatile uint32_t *)REG_RSI0_SLP_WKUP_TOUT)           /* RSI0 Sleep Wakeup Timeout Register */
#define pREG_RSI0_BLKSZ                  ((volatile uint16_t *)REG_RSI0_BLKSZ)                   /* RSI0 Block Size Register */
#define pREG_RSI0_FIFO                   ((volatile uint32_t *)REG_RSI0_FIFO)                    /* RSI0 Data FIFO Register */
#define pREG_RSI0_STAT0                  ((volatile uint32_t *)REG_RSI0_STAT0)                   /* RSI0 Exception Status Register */
#define pREG_RSI0_IMSK0                  ((volatile uint32_t *)REG_RSI0_IMSK0)                   /* RSI0 Exception Mask Register */
#define pREG_RSI0_CFG                    ((volatile uint16_t *)REG_RSI0_CFG)                     /* RSI0 Configuration Register */
#define pREG_RSI0_RD_WAIT                ((volatile uint16_t *)REG_RSI0_RD_WAIT)                 /* RSI0 Read Wait Enable Register */
#define pREG_RSI0_PID0                   ((volatile uint32_t *)REG_RSI0_PID0)                    /* RSI0 Peripheral Identification Register */
#define pREG_RSI0_PID1                   ((volatile uint32_t *)REG_RSI0_PID1)                    /* RSI0 Peripheral Identification Register */
#define pREG_RSI0_PID2                   ((volatile uint32_t *)REG_RSI0_PID2)                    /* RSI0 Peripheral Identification Register */
#define pREG_RSI0_PID3                   ((volatile uint32_t *)REG_RSI0_PID3)                    /* RSI0 Peripheral Identification Register */


/* =========================================================================
       CAN0
   ========================================================================= */
#define pREG_CAN0_MC1                    ((volatile uint16_t *)REG_CAN0_MC1)                     /* CAN0 Mailbox Configuration 1 Register */
#define pREG_CAN0_MD1                    ((volatile uint16_t *)REG_CAN0_MD1)                     /* CAN0 Mailbox Direction 1 Register */
#define pREG_CAN0_TRS1                   ((volatile uint16_t *)REG_CAN0_TRS1)                    /* CAN0 Transmission Request Set 1 Register */
#define pREG_CAN0_TRR1                   ((volatile uint16_t *)REG_CAN0_TRR1)                    /* CAN0 Transmission Request Reset 1 Register */
#define pREG_CAN0_TA1                    ((volatile uint16_t *)REG_CAN0_TA1)                     /* CAN0 Transmission Acknowledge 1 Register */
#define pREG_CAN0_AA1                    ((volatile uint16_t *)REG_CAN0_AA1)                     /* CAN0 Abort Acknowledge 1 Register */
#define pREG_CAN0_RMP1                   ((volatile uint16_t *)REG_CAN0_RMP1)                    /* CAN0 Receive Message Pending 1 Register */
#define pREG_CAN0_RML1                   ((volatile uint16_t *)REG_CAN0_RML1)                    /* CAN0 Receive Message Lost 1 Register */
#define pREG_CAN0_MBTIF1                 ((volatile uint16_t *)REG_CAN0_MBTIF1)                  /* CAN0 Mailbox Transmit Interrupt Flag 1 Register */
#define pREG_CAN0_MBRIF1                 ((volatile uint16_t *)REG_CAN0_MBRIF1)                  /* CAN0 Mailbox Receive Interrupt Flag 1 Register */
#define pREG_CAN0_MBIM1                  ((volatile uint16_t *)REG_CAN0_MBIM1)                   /* CAN0 Mailbox Interrupt Mask 1 Register */
#define pREG_CAN0_RFH1                   ((volatile uint16_t *)REG_CAN0_RFH1)                    /* CAN0 Remote Frame Handling 1 Register */
#define pREG_CAN0_OPSS1                  ((volatile uint16_t *)REG_CAN0_OPSS1)                   /* CAN0 Overwrite Protection/Single Shot Transmission 1 Register */
#define pREG_CAN0_MC2                    ((volatile uint16_t *)REG_CAN0_MC2)                     /* CAN0 Mailbox Configuration 2 Register */
#define pREG_CAN0_MD2                    ((volatile uint16_t *)REG_CAN0_MD2)                     /* CAN0 Mailbox Direction 2 Register */
#define pREG_CAN0_TRS2                   ((volatile uint16_t *)REG_CAN0_TRS2)                    /* CAN0 Transmission Request Set 2 Register */
#define pREG_CAN0_TRR2                   ((volatile uint16_t *)REG_CAN0_TRR2)                    /* CAN0 Transmission Request Reset 2 Register */
#define pREG_CAN0_TA2                    ((volatile uint16_t *)REG_CAN0_TA2)                     /* CAN0 Transmission Acknowledge 2 Register */
#define pREG_CAN0_AA2                    ((volatile uint16_t *)REG_CAN0_AA2)                     /* CAN0 Abort Acknowledge 2 Register */
#define pREG_CAN0_RMP2                   ((volatile uint16_t *)REG_CAN0_RMP2)                    /* CAN0 Receive Message Pending 2 Register */
#define pREG_CAN0_RML2                   ((volatile uint16_t *)REG_CAN0_RML2)                    /* CAN0 Receive Message Lost 2 Register */
#define pREG_CAN0_MBTIF2                 ((volatile uint16_t *)REG_CAN0_MBTIF2)                  /* CAN0 Mailbox Transmit Interrupt Flag 2 Register */
#define pREG_CAN0_MBRIF2                 ((volatile uint16_t *)REG_CAN0_MBRIF2)                  /* CAN0 Mailbox Receive Interrupt Flag 2 Register */
#define pREG_CAN0_MBIM2                  ((volatile uint16_t *)REG_CAN0_MBIM2)                   /* CAN0 Mailbox Interrupt Mask 2 Register */
#define pREG_CAN0_RFH2                   ((volatile uint16_t *)REG_CAN0_RFH2)                    /* CAN0 Remote Frame Handling 2 Register */
#define pREG_CAN0_OPSS2                  ((volatile uint16_t *)REG_CAN0_OPSS2)                   /* CAN0 Overwrite Protection/Single Shot Transmission 2 Register */
#define pREG_CAN0_CLK                    ((volatile uint16_t *)REG_CAN0_CLK)                     /* CAN0 Clock Register */
#define pREG_CAN0_TIMING                 ((volatile uint16_t *)REG_CAN0_TIMING)                  /* CAN0 Timing Register */
#define pREG_CAN0_DBG                    ((volatile uint16_t *)REG_CAN0_DBG)                     /* CAN0 Debug Register */
#define pREG_CAN0_STAT                   ((volatile uint16_t *)REG_CAN0_STAT)                    /* CAN0 Status Register */
#define pREG_CAN0_CEC                    ((volatile uint16_t *)REG_CAN0_CEC)                     /* CAN0 Error Counter Register */
#define pREG_CAN0_GIS                    ((volatile uint16_t *)REG_CAN0_GIS)                     /* CAN0 Global CAN Interrupt Status Register */
#define pREG_CAN0_GIM                    ((volatile uint16_t *)REG_CAN0_GIM)                     /* CAN0 Global CAN Interrupt Mask Register */
#define pREG_CAN0_GIF                    ((volatile uint16_t *)REG_CAN0_GIF)                     /* CAN0 Global CAN Interrupt Flag Register */
#define pREG_CAN0_CTL                    ((volatile uint16_t *)REG_CAN0_CTL)                     /* CAN0 CAN Master Control Register */
#define pREG_CAN0_INT                    ((volatile uint16_t *)REG_CAN0_INT)                     /* CAN0 Interrupt Pending Register */
#define pREG_CAN0_MBTD                   ((volatile uint16_t *)REG_CAN0_MBTD)                    /* CAN0 Temporary Mailbox Disable Register */
#define pREG_CAN0_EWR                    ((volatile uint16_t *)REG_CAN0_EWR)                     /* CAN0 Error Counter Warning Level Register */
#define pREG_CAN0_ESR                    ((volatile uint16_t *)REG_CAN0_ESR)                     /* CAN0 Error Status Register */
#define pREG_CAN0_UCCNT                  ((volatile uint16_t *)REG_CAN0_UCCNT)                   /* CAN0 Universal Counter Register */
#define pREG_CAN0_UCRC                   ((volatile uint16_t *)REG_CAN0_UCRC)                    /* CAN0 Universal Counter Reload/Capture Register */
#define pREG_CAN0_UCCNF                  ((volatile uint16_t *)REG_CAN0_UCCNF)                   /* CAN0 Universal Counter Configuration Mode Register */
#define pREG_CAN0_AM00L                  ((volatile uint16_t *)REG_CAN0_AM00L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM01L                  ((volatile uint16_t *)REG_CAN0_AM01L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM02L                  ((volatile uint16_t *)REG_CAN0_AM02L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM03L                  ((volatile uint16_t *)REG_CAN0_AM03L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM04L                  ((volatile uint16_t *)REG_CAN0_AM04L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM05L                  ((volatile uint16_t *)REG_CAN0_AM05L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM06L                  ((volatile uint16_t *)REG_CAN0_AM06L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM07L                  ((volatile uint16_t *)REG_CAN0_AM07L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM08L                  ((volatile uint16_t *)REG_CAN0_AM08L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM09L                  ((volatile uint16_t *)REG_CAN0_AM09L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM10L                  ((volatile uint16_t *)REG_CAN0_AM10L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM11L                  ((volatile uint16_t *)REG_CAN0_AM11L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM12L                  ((volatile uint16_t *)REG_CAN0_AM12L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM13L                  ((volatile uint16_t *)REG_CAN0_AM13L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM14L                  ((volatile uint16_t *)REG_CAN0_AM14L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM15L                  ((volatile uint16_t *)REG_CAN0_AM15L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM16L                  ((volatile uint16_t *)REG_CAN0_AM16L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM17L                  ((volatile uint16_t *)REG_CAN0_AM17L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM18L                  ((volatile uint16_t *)REG_CAN0_AM18L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM19L                  ((volatile uint16_t *)REG_CAN0_AM19L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM20L                  ((volatile uint16_t *)REG_CAN0_AM20L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM21L                  ((volatile uint16_t *)REG_CAN0_AM21L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM22L                  ((volatile uint16_t *)REG_CAN0_AM22L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM23L                  ((volatile uint16_t *)REG_CAN0_AM23L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM24L                  ((volatile uint16_t *)REG_CAN0_AM24L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM25L                  ((volatile uint16_t *)REG_CAN0_AM25L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM26L                  ((volatile uint16_t *)REG_CAN0_AM26L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM27L                  ((volatile uint16_t *)REG_CAN0_AM27L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM28L                  ((volatile uint16_t *)REG_CAN0_AM28L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM29L                  ((volatile uint16_t *)REG_CAN0_AM29L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM30L                  ((volatile uint16_t *)REG_CAN0_AM30L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM31L                  ((volatile uint16_t *)REG_CAN0_AM31L)                   /* CAN0 Acceptance Mask (L) Register */
#define pREG_CAN0_AM00H                  ((volatile uint16_t *)REG_CAN0_AM00H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM01H                  ((volatile uint16_t *)REG_CAN0_AM01H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM02H                  ((volatile uint16_t *)REG_CAN0_AM02H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM03H                  ((volatile uint16_t *)REG_CAN0_AM03H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM04H                  ((volatile uint16_t *)REG_CAN0_AM04H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM05H                  ((volatile uint16_t *)REG_CAN0_AM05H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM06H                  ((volatile uint16_t *)REG_CAN0_AM06H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM07H                  ((volatile uint16_t *)REG_CAN0_AM07H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM08H                  ((volatile uint16_t *)REG_CAN0_AM08H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM09H                  ((volatile uint16_t *)REG_CAN0_AM09H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM10H                  ((volatile uint16_t *)REG_CAN0_AM10H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM11H                  ((volatile uint16_t *)REG_CAN0_AM11H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM12H                  ((volatile uint16_t *)REG_CAN0_AM12H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM13H                  ((volatile uint16_t *)REG_CAN0_AM13H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM14H                  ((volatile uint16_t *)REG_CAN0_AM14H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM15H                  ((volatile uint16_t *)REG_CAN0_AM15H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM16H                  ((volatile uint16_t *)REG_CAN0_AM16H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM17H                  ((volatile uint16_t *)REG_CAN0_AM17H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM18H                  ((volatile uint16_t *)REG_CAN0_AM18H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM19H                  ((volatile uint16_t *)REG_CAN0_AM19H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM20H                  ((volatile uint16_t *)REG_CAN0_AM20H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM21H                  ((volatile uint16_t *)REG_CAN0_AM21H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM22H                  ((volatile uint16_t *)REG_CAN0_AM22H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM23H                  ((volatile uint16_t *)REG_CAN0_AM23H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM24H                  ((volatile uint16_t *)REG_CAN0_AM24H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM25H                  ((volatile uint16_t *)REG_CAN0_AM25H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM26H                  ((volatile uint16_t *)REG_CAN0_AM26H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM27H                  ((volatile uint16_t *)REG_CAN0_AM27H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM28H                  ((volatile uint16_t *)REG_CAN0_AM28H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM29H                  ((volatile uint16_t *)REG_CAN0_AM29H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM30H                  ((volatile uint16_t *)REG_CAN0_AM30H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_AM31H                  ((volatile uint16_t *)REG_CAN0_AM31H)                   /* CAN0 Acceptance Mask (H) Register */
#define pREG_CAN0_MB00_DATA0             ((volatile uint16_t *)REG_CAN0_MB00_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB01_DATA0             ((volatile uint16_t *)REG_CAN0_MB01_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB02_DATA0             ((volatile uint16_t *)REG_CAN0_MB02_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB03_DATA0             ((volatile uint16_t *)REG_CAN0_MB03_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB04_DATA0             ((volatile uint16_t *)REG_CAN0_MB04_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB05_DATA0             ((volatile uint16_t *)REG_CAN0_MB05_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB06_DATA0             ((volatile uint16_t *)REG_CAN0_MB06_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB07_DATA0             ((volatile uint16_t *)REG_CAN0_MB07_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB08_DATA0             ((volatile uint16_t *)REG_CAN0_MB08_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB09_DATA0             ((volatile uint16_t *)REG_CAN0_MB09_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB10_DATA0             ((volatile uint16_t *)REG_CAN0_MB10_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB11_DATA0             ((volatile uint16_t *)REG_CAN0_MB11_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB12_DATA0             ((volatile uint16_t *)REG_CAN0_MB12_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB13_DATA0             ((volatile uint16_t *)REG_CAN0_MB13_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB14_DATA0             ((volatile uint16_t *)REG_CAN0_MB14_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB15_DATA0             ((volatile uint16_t *)REG_CAN0_MB15_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB16_DATA0             ((volatile uint16_t *)REG_CAN0_MB16_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB17_DATA0             ((volatile uint16_t *)REG_CAN0_MB17_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB18_DATA0             ((volatile uint16_t *)REG_CAN0_MB18_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB19_DATA0             ((volatile uint16_t *)REG_CAN0_MB19_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB20_DATA0             ((volatile uint16_t *)REG_CAN0_MB20_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB21_DATA0             ((volatile uint16_t *)REG_CAN0_MB21_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB22_DATA0             ((volatile uint16_t *)REG_CAN0_MB22_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB23_DATA0             ((volatile uint16_t *)REG_CAN0_MB23_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB24_DATA0             ((volatile uint16_t *)REG_CAN0_MB24_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB25_DATA0             ((volatile uint16_t *)REG_CAN0_MB25_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB26_DATA0             ((volatile uint16_t *)REG_CAN0_MB26_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB27_DATA0             ((volatile uint16_t *)REG_CAN0_MB27_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB28_DATA0             ((volatile uint16_t *)REG_CAN0_MB28_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB29_DATA0             ((volatile uint16_t *)REG_CAN0_MB29_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB30_DATA0             ((volatile uint16_t *)REG_CAN0_MB30_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB31_DATA0             ((volatile uint16_t *)REG_CAN0_MB31_DATA0)              /* CAN0 Mailbox Word 0 Register */
#define pREG_CAN0_MB00_DATA1             ((volatile uint16_t *)REG_CAN0_MB00_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB01_DATA1             ((volatile uint16_t *)REG_CAN0_MB01_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB02_DATA1             ((volatile uint16_t *)REG_CAN0_MB02_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB03_DATA1             ((volatile uint16_t *)REG_CAN0_MB03_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB04_DATA1             ((volatile uint16_t *)REG_CAN0_MB04_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB05_DATA1             ((volatile uint16_t *)REG_CAN0_MB05_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB06_DATA1             ((volatile uint16_t *)REG_CAN0_MB06_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB07_DATA1             ((volatile uint16_t *)REG_CAN0_MB07_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB08_DATA1             ((volatile uint16_t *)REG_CAN0_MB08_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB09_DATA1             ((volatile uint16_t *)REG_CAN0_MB09_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB10_DATA1             ((volatile uint16_t *)REG_CAN0_MB10_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB11_DATA1             ((volatile uint16_t *)REG_CAN0_MB11_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB12_DATA1             ((volatile uint16_t *)REG_CAN0_MB12_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB13_DATA1             ((volatile uint16_t *)REG_CAN0_MB13_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB14_DATA1             ((volatile uint16_t *)REG_CAN0_MB14_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB15_DATA1             ((volatile uint16_t *)REG_CAN0_MB15_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB16_DATA1             ((volatile uint16_t *)REG_CAN0_MB16_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB17_DATA1             ((volatile uint16_t *)REG_CAN0_MB17_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB18_DATA1             ((volatile uint16_t *)REG_CAN0_MB18_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB19_DATA1             ((volatile uint16_t *)REG_CAN0_MB19_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB20_DATA1             ((volatile uint16_t *)REG_CAN0_MB20_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB21_DATA1             ((volatile uint16_t *)REG_CAN0_MB21_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB22_DATA1             ((volatile uint16_t *)REG_CAN0_MB22_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB23_DATA1             ((volatile uint16_t *)REG_CAN0_MB23_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB24_DATA1             ((volatile uint16_t *)REG_CAN0_MB24_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB25_DATA1             ((volatile uint16_t *)REG_CAN0_MB25_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB26_DATA1             ((volatile uint16_t *)REG_CAN0_MB26_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB27_DATA1             ((volatile uint16_t *)REG_CAN0_MB27_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB28_DATA1             ((volatile uint16_t *)REG_CAN0_MB28_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB29_DATA1             ((volatile uint16_t *)REG_CAN0_MB29_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB30_DATA1             ((volatile uint16_t *)REG_CAN0_MB30_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB31_DATA1             ((volatile uint16_t *)REG_CAN0_MB31_DATA1)              /* CAN0 Mailbox Word 1 Register */
#define pREG_CAN0_MB00_DATA2             ((volatile uint16_t *)REG_CAN0_MB00_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB01_DATA2             ((volatile uint16_t *)REG_CAN0_MB01_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB02_DATA2             ((volatile uint16_t *)REG_CAN0_MB02_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB03_DATA2             ((volatile uint16_t *)REG_CAN0_MB03_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB04_DATA2             ((volatile uint16_t *)REG_CAN0_MB04_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB05_DATA2             ((volatile uint16_t *)REG_CAN0_MB05_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB06_DATA2             ((volatile uint16_t *)REG_CAN0_MB06_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB07_DATA2             ((volatile uint16_t *)REG_CAN0_MB07_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB08_DATA2             ((volatile uint16_t *)REG_CAN0_MB08_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB09_DATA2             ((volatile uint16_t *)REG_CAN0_MB09_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB10_DATA2             ((volatile uint16_t *)REG_CAN0_MB10_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB11_DATA2             ((volatile uint16_t *)REG_CAN0_MB11_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB12_DATA2             ((volatile uint16_t *)REG_CAN0_MB12_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB13_DATA2             ((volatile uint16_t *)REG_CAN0_MB13_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB14_DATA2             ((volatile uint16_t *)REG_CAN0_MB14_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB15_DATA2             ((volatile uint16_t *)REG_CAN0_MB15_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB16_DATA2             ((volatile uint16_t *)REG_CAN0_MB16_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB17_DATA2             ((volatile uint16_t *)REG_CAN0_MB17_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB18_DATA2             ((volatile uint16_t *)REG_CAN0_MB18_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB19_DATA2             ((volatile uint16_t *)REG_CAN0_MB19_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB20_DATA2             ((volatile uint16_t *)REG_CAN0_MB20_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB21_DATA2             ((volatile uint16_t *)REG_CAN0_MB21_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB22_DATA2             ((volatile uint16_t *)REG_CAN0_MB22_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB23_DATA2             ((volatile uint16_t *)REG_CAN0_MB23_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB24_DATA2             ((volatile uint16_t *)REG_CAN0_MB24_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB25_DATA2             ((volatile uint16_t *)REG_CAN0_MB25_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB26_DATA2             ((volatile uint16_t *)REG_CAN0_MB26_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB27_DATA2             ((volatile uint16_t *)REG_CAN0_MB27_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB28_DATA2             ((volatile uint16_t *)REG_CAN0_MB28_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB29_DATA2             ((volatile uint16_t *)REG_CAN0_MB29_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB30_DATA2             ((volatile uint16_t *)REG_CAN0_MB30_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB31_DATA2             ((volatile uint16_t *)REG_CAN0_MB31_DATA2)              /* CAN0 Mailbox Word 2 Register */
#define pREG_CAN0_MB00_DATA3             ((volatile uint16_t *)REG_CAN0_MB00_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB01_DATA3             ((volatile uint16_t *)REG_CAN0_MB01_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB02_DATA3             ((volatile uint16_t *)REG_CAN0_MB02_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB03_DATA3             ((volatile uint16_t *)REG_CAN0_MB03_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB04_DATA3             ((volatile uint16_t *)REG_CAN0_MB04_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB05_DATA3             ((volatile uint16_t *)REG_CAN0_MB05_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB06_DATA3             ((volatile uint16_t *)REG_CAN0_MB06_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB07_DATA3             ((volatile uint16_t *)REG_CAN0_MB07_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB08_DATA3             ((volatile uint16_t *)REG_CAN0_MB08_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB09_DATA3             ((volatile uint16_t *)REG_CAN0_MB09_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB10_DATA3             ((volatile uint16_t *)REG_CAN0_MB10_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB11_DATA3             ((volatile uint16_t *)REG_CAN0_MB11_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB12_DATA3             ((volatile uint16_t *)REG_CAN0_MB12_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB13_DATA3             ((volatile uint16_t *)REG_CAN0_MB13_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB14_DATA3             ((volatile uint16_t *)REG_CAN0_MB14_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB15_DATA3             ((volatile uint16_t *)REG_CAN0_MB15_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB16_DATA3             ((volatile uint16_t *)REG_CAN0_MB16_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB17_DATA3             ((volatile uint16_t *)REG_CAN0_MB17_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB18_DATA3             ((volatile uint16_t *)REG_CAN0_MB18_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB19_DATA3             ((volatile uint16_t *)REG_CAN0_MB19_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB20_DATA3             ((volatile uint16_t *)REG_CAN0_MB20_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB21_DATA3             ((volatile uint16_t *)REG_CAN0_MB21_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB22_DATA3             ((volatile uint16_t *)REG_CAN0_MB22_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB23_DATA3             ((volatile uint16_t *)REG_CAN0_MB23_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB24_DATA3             ((volatile uint16_t *)REG_CAN0_MB24_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB25_DATA3             ((volatile uint16_t *)REG_CAN0_MB25_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB26_DATA3             ((volatile uint16_t *)REG_CAN0_MB26_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB27_DATA3             ((volatile uint16_t *)REG_CAN0_MB27_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB28_DATA3             ((volatile uint16_t *)REG_CAN0_MB28_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB29_DATA3             ((volatile uint16_t *)REG_CAN0_MB29_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB30_DATA3             ((volatile uint16_t *)REG_CAN0_MB30_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB31_DATA3             ((volatile uint16_t *)REG_CAN0_MB31_DATA3)              /* CAN0 Mailbox Word 3 Register */
#define pREG_CAN0_MB00_LENGTH            ((volatile uint16_t *)REG_CAN0_MB00_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB01_LENGTH            ((volatile uint16_t *)REG_CAN0_MB01_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB02_LENGTH            ((volatile uint16_t *)REG_CAN0_MB02_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB03_LENGTH            ((volatile uint16_t *)REG_CAN0_MB03_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB04_LENGTH            ((volatile uint16_t *)REG_CAN0_MB04_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB05_LENGTH            ((volatile uint16_t *)REG_CAN0_MB05_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB06_LENGTH            ((volatile uint16_t *)REG_CAN0_MB06_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB07_LENGTH            ((volatile uint16_t *)REG_CAN0_MB07_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB08_LENGTH            ((volatile uint16_t *)REG_CAN0_MB08_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB09_LENGTH            ((volatile uint16_t *)REG_CAN0_MB09_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB10_LENGTH            ((volatile uint16_t *)REG_CAN0_MB10_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB11_LENGTH            ((volatile uint16_t *)REG_CAN0_MB11_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB12_LENGTH            ((volatile uint16_t *)REG_CAN0_MB12_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB13_LENGTH            ((volatile uint16_t *)REG_CAN0_MB13_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB14_LENGTH            ((volatile uint16_t *)REG_CAN0_MB14_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB15_LENGTH            ((volatile uint16_t *)REG_CAN0_MB15_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB16_LENGTH            ((volatile uint16_t *)REG_CAN0_MB16_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB17_LENGTH            ((volatile uint16_t *)REG_CAN0_MB17_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB18_LENGTH            ((volatile uint16_t *)REG_CAN0_MB18_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB19_LENGTH            ((volatile uint16_t *)REG_CAN0_MB19_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB20_LENGTH            ((volatile uint16_t *)REG_CAN0_MB20_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB21_LENGTH            ((volatile uint16_t *)REG_CAN0_MB21_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB22_LENGTH            ((volatile uint16_t *)REG_CAN0_MB22_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB23_LENGTH            ((volatile uint16_t *)REG_CAN0_MB23_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB24_LENGTH            ((volatile uint16_t *)REG_CAN0_MB24_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB25_LENGTH            ((volatile uint16_t *)REG_CAN0_MB25_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB26_LENGTH            ((volatile uint16_t *)REG_CAN0_MB26_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB27_LENGTH            ((volatile uint16_t *)REG_CAN0_MB27_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB28_LENGTH            ((volatile uint16_t *)REG_CAN0_MB28_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB29_LENGTH            ((volatile uint16_t *)REG_CAN0_MB29_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB30_LENGTH            ((volatile uint16_t *)REG_CAN0_MB30_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB31_LENGTH            ((volatile uint16_t *)REG_CAN0_MB31_LENGTH)             /* CAN0 Mailbox Length Register */
#define pREG_CAN0_MB00_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB00_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB01_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB01_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB02_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB02_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB03_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB03_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB04_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB04_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB05_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB05_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB06_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB06_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB07_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB07_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB08_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB08_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB09_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB09_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB10_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB10_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB11_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB11_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB12_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB12_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB13_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB13_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB14_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB14_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB15_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB15_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB16_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB16_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB17_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB17_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB18_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB18_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB19_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB19_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB20_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB20_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB21_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB21_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB22_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB22_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB23_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB23_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB24_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB24_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB25_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB25_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB26_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB26_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB27_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB27_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB28_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB28_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB29_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB29_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB30_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB30_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB31_TIMESTAMP         ((volatile uint16_t *)REG_CAN0_MB31_TIMESTAMP)          /* CAN0 Mailbox Timestamp Register */
#define pREG_CAN0_MB00_ID0               ((volatile uint16_t *)REG_CAN0_MB00_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB01_ID0               ((volatile uint16_t *)REG_CAN0_MB01_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB02_ID0               ((volatile uint16_t *)REG_CAN0_MB02_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB03_ID0               ((volatile uint16_t *)REG_CAN0_MB03_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB04_ID0               ((volatile uint16_t *)REG_CAN0_MB04_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB05_ID0               ((volatile uint16_t *)REG_CAN0_MB05_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB06_ID0               ((volatile uint16_t *)REG_CAN0_MB06_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB07_ID0               ((volatile uint16_t *)REG_CAN0_MB07_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB08_ID0               ((volatile uint16_t *)REG_CAN0_MB08_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB09_ID0               ((volatile uint16_t *)REG_CAN0_MB09_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB10_ID0               ((volatile uint16_t *)REG_CAN0_MB10_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB11_ID0               ((volatile uint16_t *)REG_CAN0_MB11_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB12_ID0               ((volatile uint16_t *)REG_CAN0_MB12_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB13_ID0               ((volatile uint16_t *)REG_CAN0_MB13_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB14_ID0               ((volatile uint16_t *)REG_CAN0_MB14_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB15_ID0               ((volatile uint16_t *)REG_CAN0_MB15_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB16_ID0               ((volatile uint16_t *)REG_CAN0_MB16_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB17_ID0               ((volatile uint16_t *)REG_CAN0_MB17_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB18_ID0               ((volatile uint16_t *)REG_CAN0_MB18_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB19_ID0               ((volatile uint16_t *)REG_CAN0_MB19_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB20_ID0               ((volatile uint16_t *)REG_CAN0_MB20_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB21_ID0               ((volatile uint16_t *)REG_CAN0_MB21_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB22_ID0               ((volatile uint16_t *)REG_CAN0_MB22_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB23_ID0               ((volatile uint16_t *)REG_CAN0_MB23_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB24_ID0               ((volatile uint16_t *)REG_CAN0_MB24_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB25_ID0               ((volatile uint16_t *)REG_CAN0_MB25_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB26_ID0               ((volatile uint16_t *)REG_CAN0_MB26_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB27_ID0               ((volatile uint16_t *)REG_CAN0_MB27_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB28_ID0               ((volatile uint16_t *)REG_CAN0_MB28_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB29_ID0               ((volatile uint16_t *)REG_CAN0_MB29_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB30_ID0               ((volatile uint16_t *)REG_CAN0_MB30_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB31_ID0               ((volatile uint16_t *)REG_CAN0_MB31_ID0)                /* CAN0 Mailbox ID 0 Register */
#define pREG_CAN0_MB00_ID1               ((volatile uint16_t *)REG_CAN0_MB00_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB01_ID1               ((volatile uint16_t *)REG_CAN0_MB01_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB02_ID1               ((volatile uint16_t *)REG_CAN0_MB02_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB03_ID1               ((volatile uint16_t *)REG_CAN0_MB03_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB04_ID1               ((volatile uint16_t *)REG_CAN0_MB04_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB05_ID1               ((volatile uint16_t *)REG_CAN0_MB05_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB06_ID1               ((volatile uint16_t *)REG_CAN0_MB06_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB07_ID1               ((volatile uint16_t *)REG_CAN0_MB07_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB08_ID1               ((volatile uint16_t *)REG_CAN0_MB08_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB09_ID1               ((volatile uint16_t *)REG_CAN0_MB09_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB10_ID1               ((volatile uint16_t *)REG_CAN0_MB10_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB11_ID1               ((volatile uint16_t *)REG_CAN0_MB11_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB12_ID1               ((volatile uint16_t *)REG_CAN0_MB12_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB13_ID1               ((volatile uint16_t *)REG_CAN0_MB13_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB14_ID1               ((volatile uint16_t *)REG_CAN0_MB14_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB15_ID1               ((volatile uint16_t *)REG_CAN0_MB15_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB16_ID1               ((volatile uint16_t *)REG_CAN0_MB16_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB17_ID1               ((volatile uint16_t *)REG_CAN0_MB17_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB18_ID1               ((volatile uint16_t *)REG_CAN0_MB18_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB19_ID1               ((volatile uint16_t *)REG_CAN0_MB19_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB20_ID1               ((volatile uint16_t *)REG_CAN0_MB20_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB21_ID1               ((volatile uint16_t *)REG_CAN0_MB21_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB22_ID1               ((volatile uint16_t *)REG_CAN0_MB22_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB23_ID1               ((volatile uint16_t *)REG_CAN0_MB23_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB24_ID1               ((volatile uint16_t *)REG_CAN0_MB24_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB25_ID1               ((volatile uint16_t *)REG_CAN0_MB25_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB26_ID1               ((volatile uint16_t *)REG_CAN0_MB26_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB27_ID1               ((volatile uint16_t *)REG_CAN0_MB27_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB28_ID1               ((volatile uint16_t *)REG_CAN0_MB28_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB29_ID1               ((volatile uint16_t *)REG_CAN0_MB29_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB30_ID1               ((volatile uint16_t *)REG_CAN0_MB30_ID1)                /* CAN0 Mailbox ID 1 Register */
#define pREG_CAN0_MB31_ID1               ((volatile uint16_t *)REG_CAN0_MB31_ID1)                /* CAN0 Mailbox ID 1 Register */


/* =========================================================================
       LP0
   ========================================================================= */
#define pREG_LP0_CTL                     ((volatile uint32_t *)REG_LP0_CTL)                      /* LP0 Control Register */
#define pREG_LP0_STAT                    ((volatile uint32_t *)REG_LP0_STAT)                     /* LP0 Status Register */
#define pREG_LP0_DIV                     ((volatile uint32_t *)REG_LP0_DIV)                      /* LP0 Clock Divider Value */
#define pREG_LP0_TX                      ((volatile uint32_t *)REG_LP0_TX)                       /* LP0 Transmit Buffer */
#define pREG_LP0_RX                      ((volatile uint32_t *)REG_LP0_RX)                       /* LP0 Receive Buffer */
#define pREG_LP0_TXIN_SHDW               ((volatile uint32_t *)REG_LP0_TXIN_SHDW)                /* LP0 Shadow Input Transmit Buffer */
#define pREG_LP0_TXOUT_SHDW              ((volatile uint32_t *)REG_LP0_TXOUT_SHDW)               /* LP0 Shadow Output Transmit Buffer */

/* =========================================================================
       LP1
   ========================================================================= */
#define pREG_LP1_CTL                     ((volatile uint32_t *)REG_LP1_CTL)                      /* LP1 Control Register */
#define pREG_LP1_STAT                    ((volatile uint32_t *)REG_LP1_STAT)                     /* LP1 Status Register */
#define pREG_LP1_DIV                     ((volatile uint32_t *)REG_LP1_DIV)                      /* LP1 Clock Divider Value */
#define pREG_LP1_TX                      ((volatile uint32_t *)REG_LP1_TX)                       /* LP1 Transmit Buffer */
#define pREG_LP1_RX                      ((volatile uint32_t *)REG_LP1_RX)                       /* LP1 Receive Buffer */
#define pREG_LP1_TXIN_SHDW               ((volatile uint32_t *)REG_LP1_TXIN_SHDW)                /* LP1 Shadow Input Transmit Buffer */
#define pREG_LP1_TXOUT_SHDW              ((volatile uint32_t *)REG_LP1_TXOUT_SHDW)               /* LP1 Shadow Output Transmit Buffer */

/* =========================================================================
       LP2
   ========================================================================= */
#define pREG_LP2_CTL                     ((volatile uint32_t *)REG_LP2_CTL)                      /* LP2 Control Register */
#define pREG_LP2_STAT                    ((volatile uint32_t *)REG_LP2_STAT)                     /* LP2 Status Register */
#define pREG_LP2_DIV                     ((volatile uint32_t *)REG_LP2_DIV)                      /* LP2 Clock Divider Value */
#define pREG_LP2_TX                      ((volatile uint32_t *)REG_LP2_TX)                       /* LP2 Transmit Buffer */
#define pREG_LP2_RX                      ((volatile uint32_t *)REG_LP2_RX)                       /* LP2 Receive Buffer */
#define pREG_LP2_TXIN_SHDW               ((volatile uint32_t *)REG_LP2_TXIN_SHDW)                /* LP2 Shadow Input Transmit Buffer */
#define pREG_LP2_TXOUT_SHDW              ((volatile uint32_t *)REG_LP2_TXOUT_SHDW)               /* LP2 Shadow Output Transmit Buffer */

/* =========================================================================
       LP3
   ========================================================================= */
#define pREG_LP3_CTL                     ((volatile uint32_t *)REG_LP3_CTL)                      /* LP3 Control Register */
#define pREG_LP3_STAT                    ((volatile uint32_t *)REG_LP3_STAT)                     /* LP3 Status Register */
#define pREG_LP3_DIV                     ((volatile uint32_t *)REG_LP3_DIV)                      /* LP3 Clock Divider Value */
#define pREG_LP3_TX                      ((volatile uint32_t *)REG_LP3_TX)                       /* LP3 Transmit Buffer */
#define pREG_LP3_RX                      ((volatile uint32_t *)REG_LP3_RX)                       /* LP3 Receive Buffer */
#define pREG_LP3_TXIN_SHDW               ((volatile uint32_t *)REG_LP3_TXIN_SHDW)                /* LP3 Shadow Input Transmit Buffer */
#define pREG_LP3_TXOUT_SHDW              ((volatile uint32_t *)REG_LP3_TXOUT_SHDW)               /* LP3 Shadow Output Transmit Buffer */


/* =========================================================================
       TIMER0
   ========================================================================= */
#define pREG_TIMER0_REVID                ((volatile uint16_t *)REG_TIMER0_REVID)                 /* TIMER0 Revision ID Register */
#define pREG_TIMER0_RUN                  ((volatile uint16_t *)REG_TIMER0_RUN)                   /* TIMER0 Run Register */
#define pREG_TIMER0_RUN_SET              ((volatile uint16_t *)REG_TIMER0_RUN_SET)               /* TIMER0 Run Set Register */
#define pREG_TIMER0_RUN_CLR              ((volatile uint16_t *)REG_TIMER0_RUN_CLR)               /* TIMER0 Run Clear Register */
#define pREG_TIMER0_STOP_CFG             ((volatile uint16_t *)REG_TIMER0_STOP_CFG)              /* TIMER0 Stop Configuration Register */
#define pREG_TIMER0_STOP_CFG_SET         ((volatile uint16_t *)REG_TIMER0_STOP_CFG_SET)          /* TIMER0 Stop Configuration Set Register */
#define pREG_TIMER0_STOP_CFG_CLR         ((volatile uint16_t *)REG_TIMER0_STOP_CFG_CLR)          /* TIMER0 Stop Configuration Clear Register */
#define pREG_TIMER0_DATA_IMSK            ((volatile uint16_t *)REG_TIMER0_DATA_IMSK)             /* TIMER0 Data Interrupt Mask Register */
#define pREG_TIMER0_STAT_IMSK            ((volatile uint16_t *)REG_TIMER0_STAT_IMSK)             /* TIMER0 Status Interrupt Mask Register */
#define pREG_TIMER0_TRG_MSK              ((volatile uint16_t *)REG_TIMER0_TRG_MSK)               /* TIMER0 Trigger Master Mask Register */
#define pREG_TIMER0_TRG_IE               ((volatile uint16_t *)REG_TIMER0_TRG_IE)                /* TIMER0 Trigger Slave Enable Register */
#define pREG_TIMER0_DATA_ILAT            ((volatile uint16_t *)REG_TIMER0_DATA_ILAT)             /* TIMER0 Data Interrupt Latch Register */
#define pREG_TIMER0_STAT_ILAT            ((volatile uint16_t *)REG_TIMER0_STAT_ILAT)             /* TIMER0 Status Interrupt Latch Register */
#define pREG_TIMER0_ERR_TYPE             ((volatile uint32_t *)REG_TIMER0_ERR_TYPE)              /* TIMER0 Error Type Status Register */
#define pREG_TIMER0_BCAST_PER            ((volatile uint32_t *)REG_TIMER0_BCAST_PER)             /* TIMER0 Broadcast Period Register */
#define pREG_TIMER0_BCAST_WID            ((volatile uint32_t *)REG_TIMER0_BCAST_WID)             /* TIMER0 Broadcast Width Register */
#define pREG_TIMER0_BCAST_DLY            ((volatile uint32_t *)REG_TIMER0_BCAST_DLY)             /* TIMER0 Broadcast Delay Register */
#define pREG_TIMER0_TMR0_CFG             ((volatile uint16_t *)REG_TIMER0_TMR0_CFG)              /* TIMER0 Timer n Configuration Register */
#define pREG_TIMER0_TMR1_CFG             ((volatile uint16_t *)REG_TIMER0_TMR1_CFG)              /* TIMER0 Timer n Configuration Register */
#define pREG_TIMER0_TMR2_CFG             ((volatile uint16_t *)REG_TIMER0_TMR2_CFG)              /* TIMER0 Timer n Configuration Register */
#define pREG_TIMER0_TMR3_CFG             ((volatile uint16_t *)REG_TIMER0_TMR3_CFG)              /* TIMER0 Timer n Configuration Register */
#define pREG_TIMER0_TMR4_CFG             ((volatile uint16_t *)REG_TIMER0_TMR4_CFG)              /* TIMER0 Timer n Configuration Register */
#define pREG_TIMER0_TMR5_CFG             ((volatile uint16_t *)REG_TIMER0_TMR5_CFG)              /* TIMER0 Timer n Configuration Register */
#define pREG_TIMER0_TMR6_CFG             ((volatile uint16_t *)REG_TIMER0_TMR6_CFG)              /* TIMER0 Timer n Configuration Register */
#define pREG_TIMER0_TMR7_CFG             ((volatile uint16_t *)REG_TIMER0_TMR7_CFG)              /* TIMER0 Timer n Configuration Register */
#define pREG_TIMER0_TMR0_CNT             ((volatile uint32_t *)REG_TIMER0_TMR0_CNT)              /* TIMER0 Timer n Counter Register */
#define pREG_TIMER0_TMR1_CNT             ((volatile uint32_t *)REG_TIMER0_TMR1_CNT)              /* TIMER0 Timer n Counter Register */
#define pREG_TIMER0_TMR2_CNT             ((volatile uint32_t *)REG_TIMER0_TMR2_CNT)              /* TIMER0 Timer n Counter Register */
#define pREG_TIMER0_TMR3_CNT             ((volatile uint32_t *)REG_TIMER0_TMR3_CNT)              /* TIMER0 Timer n Counter Register */
#define pREG_TIMER0_TMR4_CNT             ((volatile uint32_t *)REG_TIMER0_TMR4_CNT)              /* TIMER0 Timer n Counter Register */
#define pREG_TIMER0_TMR5_CNT             ((volatile uint32_t *)REG_TIMER0_TMR5_CNT)              /* TIMER0 Timer n Counter Register */
#define pREG_TIMER0_TMR6_CNT             ((volatile uint32_t *)REG_TIMER0_TMR6_CNT)              /* TIMER0 Timer n Counter Register */
#define pREG_TIMER0_TMR7_CNT             ((volatile uint32_t *)REG_TIMER0_TMR7_CNT)              /* TIMER0 Timer n Counter Register */
#define pREG_TIMER0_TMR0_PER             ((volatile uint32_t *)REG_TIMER0_TMR0_PER)              /* TIMER0 Timer n Period Register */
#define pREG_TIMER0_TMR1_PER             ((volatile uint32_t *)REG_TIMER0_TMR1_PER)              /* TIMER0 Timer n Period Register */
#define pREG_TIMER0_TMR2_PER             ((volatile uint32_t *)REG_TIMER0_TMR2_PER)              /* TIMER0 Timer n Period Register */
#define pREG_TIMER0_TMR3_PER             ((volatile uint32_t *)REG_TIMER0_TMR3_PER)              /* TIMER0 Timer n Period Register */
#define pREG_TIMER0_TMR4_PER             ((volatile uint32_t *)REG_TIMER0_TMR4_PER)              /* TIMER0 Timer n Period Register */
#define pREG_TIMER0_TMR5_PER             ((volatile uint32_t *)REG_TIMER0_TMR5_PER)              /* TIMER0 Timer n Period Register */
#define pREG_TIMER0_TMR6_PER             ((volatile uint32_t *)REG_TIMER0_TMR6_PER)              /* TIMER0 Timer n Period Register */
#define pREG_TIMER0_TMR7_PER             ((volatile uint32_t *)REG_TIMER0_TMR7_PER)              /* TIMER0 Timer n Period Register */
#define pREG_TIMER0_TMR0_WID             ((volatile uint32_t *)REG_TIMER0_TMR0_WID)              /* TIMER0 Timer n Width Register */
#define pREG_TIMER0_TMR1_WID             ((volatile uint32_t *)REG_TIMER0_TMR1_WID)              /* TIMER0 Timer n Width Register */
#define pREG_TIMER0_TMR2_WID             ((volatile uint32_t *)REG_TIMER0_TMR2_WID)              /* TIMER0 Timer n Width Register */
#define pREG_TIMER0_TMR3_WID             ((volatile uint32_t *)REG_TIMER0_TMR3_WID)              /* TIMER0 Timer n Width Register */
#define pREG_TIMER0_TMR4_WID             ((volatile uint32_t *)REG_TIMER0_TMR4_WID)              /* TIMER0 Timer n Width Register */
#define pREG_TIMER0_TMR5_WID             ((volatile uint32_t *)REG_TIMER0_TMR5_WID)              /* TIMER0 Timer n Width Register */
#define pREG_TIMER0_TMR6_WID             ((volatile uint32_t *)REG_TIMER0_TMR6_WID)              /* TIMER0 Timer n Width Register */
#define pREG_TIMER0_TMR7_WID             ((volatile uint32_t *)REG_TIMER0_TMR7_WID)              /* TIMER0 Timer n Width Register */
#define pREG_TIMER0_TMR0_DLY             ((volatile uint32_t *)REG_TIMER0_TMR0_DLY)              /* TIMER0 Timer n Delay Register */
#define pREG_TIMER0_TMR1_DLY             ((volatile uint32_t *)REG_TIMER0_TMR1_DLY)              /* TIMER0 Timer n Delay Register */
#define pREG_TIMER0_TMR2_DLY             ((volatile uint32_t *)REG_TIMER0_TMR2_DLY)              /* TIMER0 Timer n Delay Register */
#define pREG_TIMER0_TMR3_DLY             ((volatile uint32_t *)REG_TIMER0_TMR3_DLY)              /* TIMER0 Timer n Delay Register */
#define pREG_TIMER0_TMR4_DLY             ((volatile uint32_t *)REG_TIMER0_TMR4_DLY)              /* TIMER0 Timer n Delay Register */
#define pREG_TIMER0_TMR5_DLY             ((volatile uint32_t *)REG_TIMER0_TMR5_DLY)              /* TIMER0 Timer n Delay Register */
#define pREG_TIMER0_TMR6_DLY             ((volatile uint32_t *)REG_TIMER0_TMR6_DLY)              /* TIMER0 Timer n Delay Register */
#define pREG_TIMER0_TMR7_DLY             ((volatile uint32_t *)REG_TIMER0_TMR7_DLY)              /* TIMER0 Timer n Delay Register */


/* =========================================================================
       CRC0
   ========================================================================= */
#define pREG_CRC0_CTL                    ((volatile uint32_t *)REG_CRC0_CTL)                     /* CRC0 Control Register */
#define pREG_CRC0_DCNT                   ((volatile uint32_t *)REG_CRC0_DCNT)                    /* CRC0 Data Word Count Register */
#define pREG_CRC0_DCNTRLD                ((volatile uint32_t *)REG_CRC0_DCNTRLD)                 /* CRC0 Data Word Count Reload Register */
#define pREG_CRC0_COMP                   ((volatile uint32_t *)REG_CRC0_COMP)                    /* CRC0 Data Compare Register */
#define pREG_CRC0_FILLVAL                ((volatile uint32_t *)REG_CRC0_FILLVAL)                 /* CRC0 Fill Value Register */
#define pREG_CRC0_DFIFO                  ((volatile uint32_t *)REG_CRC0_DFIFO)                   /* CRC0 Data FIFO Register */
#define pREG_CRC0_INEN                   ((volatile uint32_t *)REG_CRC0_INEN)                    /* CRC0 Interrupt Enable Register */
#define pREG_CRC0_INEN_SET               ((volatile uint32_t *)REG_CRC0_INEN_SET)                /* CRC0 Interrupt Enable Set Register */
#define pREG_CRC0_INEN_CLR               ((volatile uint32_t *)REG_CRC0_INEN_CLR)                /* CRC0 Interrupt Enable Clear Register */
#define pREG_CRC0_POLY                   ((volatile uint32_t *)REG_CRC0_POLY)                    /* CRC0 Polynomial Register */
#define pREG_CRC0_STAT                   ((volatile uint32_t *)REG_CRC0_STAT)                    /* CRC0 Status Register */
#define pREG_CRC0_DCNTCAP                ((volatile uint32_t *)REG_CRC0_DCNTCAP)                 /* CRC0 Data Count Capture Register */
#define pREG_CRC0_RESULT_FIN             ((volatile uint32_t *)REG_CRC0_RESULT_FIN)              /* CRC0 CRC Final Result Register */
#define pREG_CRC0_RESULT_CUR             ((volatile uint32_t *)REG_CRC0_RESULT_CUR)              /* CRC0 CRC Current Result Register */
#define pREG_CRC0_REVID                  ((volatile uint32_t *)REG_CRC0_REVID)                   /* CRC0 Revision ID Register */

/* =========================================================================
       CRC1
   ========================================================================= */
#define pREG_CRC1_CTL                    ((volatile uint32_t *)REG_CRC1_CTL)                     /* CRC1 Control Register */
#define pREG_CRC1_DCNT                   ((volatile uint32_t *)REG_CRC1_DCNT)                    /* CRC1 Data Word Count Register */
#define pREG_CRC1_DCNTRLD                ((volatile uint32_t *)REG_CRC1_DCNTRLD)                 /* CRC1 Data Word Count Reload Register */
#define pREG_CRC1_COMP                   ((volatile uint32_t *)REG_CRC1_COMP)                    /* CRC1 Data Compare Register */
#define pREG_CRC1_FILLVAL                ((volatile uint32_t *)REG_CRC1_FILLVAL)                 /* CRC1 Fill Value Register */
#define pREG_CRC1_DFIFO                  ((volatile uint32_t *)REG_CRC1_DFIFO)                   /* CRC1 Data FIFO Register */
#define pREG_CRC1_INEN                   ((volatile uint32_t *)REG_CRC1_INEN)                    /* CRC1 Interrupt Enable Register */
#define pREG_CRC1_INEN_SET               ((volatile uint32_t *)REG_CRC1_INEN_SET)                /* CRC1 Interrupt Enable Set Register */
#define pREG_CRC1_INEN_CLR               ((volatile uint32_t *)REG_CRC1_INEN_CLR)                /* CRC1 Interrupt Enable Clear Register */
#define pREG_CRC1_POLY                   ((volatile uint32_t *)REG_CRC1_POLY)                    /* CRC1 Polynomial Register */
#define pREG_CRC1_STAT                   ((volatile uint32_t *)REG_CRC1_STAT)                    /* CRC1 Status Register */
#define pREG_CRC1_DCNTCAP                ((volatile uint32_t *)REG_CRC1_DCNTCAP)                 /* CRC1 Data Count Capture Register */
#define pREG_CRC1_RESULT_FIN             ((volatile uint32_t *)REG_CRC1_RESULT_FIN)              /* CRC1 CRC Final Result Register */
#define pREG_CRC1_RESULT_CUR             ((volatile uint32_t *)REG_CRC1_RESULT_CUR)              /* CRC1 CRC Current Result Register */
#define pREG_CRC1_REVID                  ((volatile uint32_t *)REG_CRC1_REVID)                   /* CRC1 Revision ID Register */


/* =========================================================================
       TWI0
   ========================================================================= */
#define pREG_TWI0_CLKDIV                 ((volatile uint16_t *)REG_TWI0_CLKDIV)                  /* TWI0 SCL Clock Divider Register */
#define pREG_TWI0_CTL                    ((volatile uint16_t *)REG_TWI0_CTL)                     /* TWI0 Control Register */
#define pREG_TWI0_SLVCTL                 ((volatile uint16_t *)REG_TWI0_SLVCTL)                  /* TWI0 Slave Mode Control Register */
#define pREG_TWI0_SLVSTAT                ((volatile uint16_t *)REG_TWI0_SLVSTAT)                 /* TWI0 Slave Mode Status Register */
#define pREG_TWI0_SLVADDR                ((volatile uint16_t *)REG_TWI0_SLVADDR)                 /* TWI0 Slave Mode Address Register */
#define pREG_TWI0_MSTRCTL                ((volatile uint16_t *)REG_TWI0_MSTRCTL)                 /* TWI0 Master Mode Control Registers */
#define pREG_TWI0_MSTRSTAT               ((volatile uint16_t *)REG_TWI0_MSTRSTAT)                /* TWI0 Master Mode Status Register */
#define pREG_TWI0_MSTRADDR               ((volatile uint16_t *)REG_TWI0_MSTRADDR)                /* TWI0 Master Mode Address Register */
#define pREG_TWI0_ISTAT                  ((volatile uint16_t *)REG_TWI0_ISTAT)                   /* TWI0 Interrupt Status Register */
#define pREG_TWI0_IMSK                   ((volatile uint16_t *)REG_TWI0_IMSK)                    /* TWI0 Interrupt Mask Register */
#define pREG_TWI0_FIFOCTL                ((volatile uint16_t *)REG_TWI0_FIFOCTL)                 /* TWI0 FIFO Control Register */
#define pREG_TWI0_FIFOSTAT               ((volatile uint16_t *)REG_TWI0_FIFOSTAT)                /* TWI0 FIFO Status Register */
#define pREG_TWI0_TXDATA8                ((volatile uint16_t *)REG_TWI0_TXDATA8)                 /* TWI0 Tx Data Single-Byte Register */
#define pREG_TWI0_TXDATA16               ((volatile uint16_t *)REG_TWI0_TXDATA16)                /* TWI0 Tx Data Double-Byte Register */
#define pREG_TWI0_RXDATA8                ((volatile uint16_t *)REG_TWI0_RXDATA8)                 /* TWI0 Rx Data Single-Byte Register */
#define pREG_TWI0_RXDATA16               ((volatile uint16_t *)REG_TWI0_RXDATA16)                /* TWI0 Rx Data Double-Byte Register */

/* =========================================================================
       TWI1
   ========================================================================= */
#define pREG_TWI1_CLKDIV                 ((volatile uint16_t *)REG_TWI1_CLKDIV)                  /* TWI1 SCL Clock Divider Register */
#define pREG_TWI1_CTL                    ((volatile uint16_t *)REG_TWI1_CTL)                     /* TWI1 Control Register */
#define pREG_TWI1_SLVCTL                 ((volatile uint16_t *)REG_TWI1_SLVCTL)                  /* TWI1 Slave Mode Control Register */
#define pREG_TWI1_SLVSTAT                ((volatile uint16_t *)REG_TWI1_SLVSTAT)                 /* TWI1 Slave Mode Status Register */
#define pREG_TWI1_SLVADDR                ((volatile uint16_t *)REG_TWI1_SLVADDR)                 /* TWI1 Slave Mode Address Register */
#define pREG_TWI1_MSTRCTL                ((volatile uint16_t *)REG_TWI1_MSTRCTL)                 /* TWI1 Master Mode Control Registers */
#define pREG_TWI1_MSTRSTAT               ((volatile uint16_t *)REG_TWI1_MSTRSTAT)                /* TWI1 Master Mode Status Register */
#define pREG_TWI1_MSTRADDR               ((volatile uint16_t *)REG_TWI1_MSTRADDR)                /* TWI1 Master Mode Address Register */
#define pREG_TWI1_ISTAT                  ((volatile uint16_t *)REG_TWI1_ISTAT)                   /* TWI1 Interrupt Status Register */
#define pREG_TWI1_IMSK                   ((volatile uint16_t *)REG_TWI1_IMSK)                    /* TWI1 Interrupt Mask Register */
#define pREG_TWI1_FIFOCTL                ((volatile uint16_t *)REG_TWI1_FIFOCTL)                 /* TWI1 FIFO Control Register */
#define pREG_TWI1_FIFOSTAT               ((volatile uint16_t *)REG_TWI1_FIFOSTAT)                /* TWI1 FIFO Status Register */
#define pREG_TWI1_TXDATA8                ((volatile uint16_t *)REG_TWI1_TXDATA8)                 /* TWI1 Tx Data Single-Byte Register */
#define pREG_TWI1_TXDATA16               ((volatile uint16_t *)REG_TWI1_TXDATA16)                /* TWI1 Tx Data Double-Byte Register */
#define pREG_TWI1_RXDATA8                ((volatile uint16_t *)REG_TWI1_RXDATA8)                 /* TWI1 Rx Data Single-Byte Register */
#define pREG_TWI1_RXDATA16               ((volatile uint16_t *)REG_TWI1_RXDATA16)                /* TWI1 Rx Data Double-Byte Register */


/* =========================================================================
       UART0
   ========================================================================= */
#define pREG_UART0_REVID                 ((volatile uint32_t *)REG_UART0_REVID)                  /* UART0 Revision ID Register */
#define pREG_UART0_CTL                   ((volatile uint32_t *)REG_UART0_CTL)                    /* UART0 Control Register */
#define pREG_UART0_STAT                  ((volatile uint32_t *)REG_UART0_STAT)                   /* UART0 Status Register */
#define pREG_UART0_SCR                   ((volatile uint32_t *)REG_UART0_SCR)                    /* UART0 Scratch Register */
#define pREG_UART0_CLK                   ((volatile uint32_t *)REG_UART0_CLK)                    /* UART0 Clock Rate Register */
#define pREG_UART0_IMSK                  ((volatile uint32_t *)REG_UART0_IMSK)                   /* UART0 Interrupt Mask Register */
#define pREG_UART0_IMSK_SET              ((volatile uint32_t *)REG_UART0_IMSK_SET)               /* UART0 Interrupt Mask Set Register */
#define pREG_UART0_IMSK_CLR              ((volatile uint32_t *)REG_UART0_IMSK_CLR)               /* UART0 Interrupt Mask Clear Register */
#define pREG_UART0_RBR                   ((volatile uint32_t *)REG_UART0_RBR)                    /* UART0 Receive Buffer Register */
#define pREG_UART0_THR                   ((volatile uint32_t *)REG_UART0_THR)                    /* UART0 Transmit Hold Register */
#define pREG_UART0_TAIP                  ((volatile uint32_t *)REG_UART0_TAIP)                   /* UART0 Transmit Address/Insert Pulse Register */
#define pREG_UART0_TSR                   ((volatile uint32_t *)REG_UART0_TSR)                    /* UART0 Transmit Shift Register */
#define pREG_UART0_RSR                   ((volatile uint32_t *)REG_UART0_RSR)                    /* UART0 Receive Shift Register */
#define pREG_UART0_TXCNT                 ((volatile uint32_t *)REG_UART0_TXCNT)                  /* UART0 Transmit Counter Register */
#define pREG_UART0_RXCNT                 ((volatile uint32_t *)REG_UART0_RXCNT)                  /* UART0 Receive Counter Register */

/* =========================================================================
       UART1
   ========================================================================= */
#define pREG_UART1_REVID                 ((volatile uint32_t *)REG_UART1_REVID)                  /* UART1 Revision ID Register */
#define pREG_UART1_CTL                   ((volatile uint32_t *)REG_UART1_CTL)                    /* UART1 Control Register */
#define pREG_UART1_STAT                  ((volatile uint32_t *)REG_UART1_STAT)                   /* UART1 Status Register */
#define pREG_UART1_SCR                   ((volatile uint32_t *)REG_UART1_SCR)                    /* UART1 Scratch Register */
#define pREG_UART1_CLK                   ((volatile uint32_t *)REG_UART1_CLK)                    /* UART1 Clock Rate Register */
#define pREG_UART1_IMSK                  ((volatile uint32_t *)REG_UART1_IMSK)                   /* UART1 Interrupt Mask Register */
#define pREG_UART1_IMSK_SET              ((volatile uint32_t *)REG_UART1_IMSK_SET)               /* UART1 Interrupt Mask Set Register */
#define pREG_UART1_IMSK_CLR              ((volatile uint32_t *)REG_UART1_IMSK_CLR)               /* UART1 Interrupt Mask Clear Register */
#define pREG_UART1_RBR                   ((volatile uint32_t *)REG_UART1_RBR)                    /* UART1 Receive Buffer Register */
#define pREG_UART1_THR                   ((volatile uint32_t *)REG_UART1_THR)                    /* UART1 Transmit Hold Register */
#define pREG_UART1_TAIP                  ((volatile uint32_t *)REG_UART1_TAIP)                   /* UART1 Transmit Address/Insert Pulse Register */
#define pREG_UART1_TSR                   ((volatile uint32_t *)REG_UART1_TSR)                    /* UART1 Transmit Shift Register */
#define pREG_UART1_RSR                   ((volatile uint32_t *)REG_UART1_RSR)                    /* UART1 Receive Shift Register */
#define pREG_UART1_TXCNT                 ((volatile uint32_t *)REG_UART1_TXCNT)                  /* UART1 Transmit Counter Register */
#define pREG_UART1_RXCNT                 ((volatile uint32_t *)REG_UART1_RXCNT)                  /* UART1 Receive Counter Register */


/* =========================================================================
       PORTA
   ========================================================================= */
#define pREG_PORTA_FER                   ((volatile uint32_t *)REG_PORTA_FER)                    /* PORTA Port x Function Enable Register */
#define pREG_PORTA_FER_SET               ((volatile uint32_t *)REG_PORTA_FER_SET)                /* PORTA Port x Function Enable Set Register */
#define pREG_PORTA_FER_CLR               ((volatile uint32_t *)REG_PORTA_FER_CLR)                /* PORTA Port x Function Enable Clear Register */
#define pREG_PORTA_DATA                  ((volatile uint32_t *)REG_PORTA_DATA)                   /* PORTA Port x GPIO Data Register */
#define pREG_PORTA_DATA_SET              ((volatile uint32_t *)REG_PORTA_DATA_SET)               /* PORTA Port x GPIO Data Set Register */
#define pREG_PORTA_DATA_CLR              ((volatile uint32_t *)REG_PORTA_DATA_CLR)               /* PORTA Port x GPIO Data Clear Register */
#define pREG_PORTA_DIR                   ((volatile uint32_t *)REG_PORTA_DIR)                    /* PORTA Port x GPIO Direction Register */
#define pREG_PORTA_DIR_SET               ((volatile uint32_t *)REG_PORTA_DIR_SET)                /* PORTA Port x GPIO Direction Set Register */
#define pREG_PORTA_DIR_CLR               ((volatile uint32_t *)REG_PORTA_DIR_CLR)                /* PORTA Port x GPIO Direction Clear Register */
#define pREG_PORTA_INEN                  ((volatile uint32_t *)REG_PORTA_INEN)                   /* PORTA Port x GPIO Input Enable Register */
#define pREG_PORTA_INEN_SET              ((volatile uint32_t *)REG_PORTA_INEN_SET)               /* PORTA Port x GPIO Input Enable Set Register */
#define pREG_PORTA_INEN_CLR              ((volatile uint32_t *)REG_PORTA_INEN_CLR)               /* PORTA Port x GPIO Input Enable Clear Register */
#define pREG_PORTA_MUX                   ((volatile uint32_t *)REG_PORTA_MUX)                    /* PORTA Port x Multiplexer Control Register */
#define pREG_PORTA_DATA_TGL              ((volatile uint32_t *)REG_PORTA_DATA_TGL)               /* PORTA Port x GPIO Input Enable Toggle Register */
#define pREG_PORTA_POL                   ((volatile uint32_t *)REG_PORTA_POL)                    /* PORTA Port x GPIO Polarity Invert Register */
#define pREG_PORTA_POL_SET               ((volatile uint32_t *)REG_PORTA_POL_SET)                /* PORTA Port x GPIO Polarity Invert Set Register */
#define pREG_PORTA_POL_CLR               ((volatile uint32_t *)REG_PORTA_POL_CLR)                /* PORTA Port x GPIO Polarity Invert Clear Register */
#define pREG_PORTA_LOCK                  ((volatile uint32_t *)REG_PORTA_LOCK)                   /* PORTA Port x GPIO Lock Register */
#define pREG_PORTA_REVID                 ((volatile uint32_t *)REG_PORTA_REVID)                  /* PORTA Port x GPIO Revision ID */

/* =========================================================================
       PORTB
   ========================================================================= */
#define pREG_PORTB_FER                   ((volatile uint32_t *)REG_PORTB_FER)                    /* PORTB Port x Function Enable Register */
#define pREG_PORTB_FER_SET               ((volatile uint32_t *)REG_PORTB_FER_SET)                /* PORTB Port x Function Enable Set Register */
#define pREG_PORTB_FER_CLR               ((volatile uint32_t *)REG_PORTB_FER_CLR)                /* PORTB Port x Function Enable Clear Register */
#define pREG_PORTB_DATA                  ((volatile uint32_t *)REG_PORTB_DATA)                   /* PORTB Port x GPIO Data Register */
#define pREG_PORTB_DATA_SET              ((volatile uint32_t *)REG_PORTB_DATA_SET)               /* PORTB Port x GPIO Data Set Register */
#define pREG_PORTB_DATA_CLR              ((volatile uint32_t *)REG_PORTB_DATA_CLR)               /* PORTB Port x GPIO Data Clear Register */
#define pREG_PORTB_DIR                   ((volatile uint32_t *)REG_PORTB_DIR)                    /* PORTB Port x GPIO Direction Register */
#define pREG_PORTB_DIR_SET               ((volatile uint32_t *)REG_PORTB_DIR_SET)                /* PORTB Port x GPIO Direction Set Register */
#define pREG_PORTB_DIR_CLR               ((volatile uint32_t *)REG_PORTB_DIR_CLR)                /* PORTB Port x GPIO Direction Clear Register */
#define pREG_PORTB_INEN                  ((volatile uint32_t *)REG_PORTB_INEN)                   /* PORTB Port x GPIO Input Enable Register */
#define pREG_PORTB_INEN_SET              ((volatile uint32_t *)REG_PORTB_INEN_SET)               /* PORTB Port x GPIO Input Enable Set Register */
#define pREG_PORTB_INEN_CLR              ((volatile uint32_t *)REG_PORTB_INEN_CLR)               /* PORTB Port x GPIO Input Enable Clear Register */
#define pREG_PORTB_MUX                   ((volatile uint32_t *)REG_PORTB_MUX)                    /* PORTB Port x Multiplexer Control Register */
#define pREG_PORTB_DATA_TGL              ((volatile uint32_t *)REG_PORTB_DATA_TGL)               /* PORTB Port x GPIO Input Enable Toggle Register */
#define pREG_PORTB_POL                   ((volatile uint32_t *)REG_PORTB_POL)                    /* PORTB Port x GPIO Polarity Invert Register */
#define pREG_PORTB_POL_SET               ((volatile uint32_t *)REG_PORTB_POL_SET)                /* PORTB Port x GPIO Polarity Invert Set Register */
#define pREG_PORTB_POL_CLR               ((volatile uint32_t *)REG_PORTB_POL_CLR)                /* PORTB Port x GPIO Polarity Invert Clear Register */
#define pREG_PORTB_LOCK                  ((volatile uint32_t *)REG_PORTB_LOCK)                   /* PORTB Port x GPIO Lock Register */
#define pREG_PORTB_REVID                 ((volatile uint32_t *)REG_PORTB_REVID)                  /* PORTB Port x GPIO Revision ID */

/* =========================================================================
       PORTC
   ========================================================================= */
#define pREG_PORTC_FER                   ((volatile uint32_t *)REG_PORTC_FER)                    /* PORTC Port x Function Enable Register */
#define pREG_PORTC_FER_SET               ((volatile uint32_t *)REG_PORTC_FER_SET)                /* PORTC Port x Function Enable Set Register */
#define pREG_PORTC_FER_CLR               ((volatile uint32_t *)REG_PORTC_FER_CLR)                /* PORTC Port x Function Enable Clear Register */
#define pREG_PORTC_DATA                  ((volatile uint32_t *)REG_PORTC_DATA)                   /* PORTC Port x GPIO Data Register */
#define pREG_PORTC_DATA_SET              ((volatile uint32_t *)REG_PORTC_DATA_SET)               /* PORTC Port x GPIO Data Set Register */
#define pREG_PORTC_DATA_CLR              ((volatile uint32_t *)REG_PORTC_DATA_CLR)               /* PORTC Port x GPIO Data Clear Register */
#define pREG_PORTC_DIR                   ((volatile uint32_t *)REG_PORTC_DIR)                    /* PORTC Port x GPIO Direction Register */
#define pREG_PORTC_DIR_SET               ((volatile uint32_t *)REG_PORTC_DIR_SET)                /* PORTC Port x GPIO Direction Set Register */
#define pREG_PORTC_DIR_CLR               ((volatile uint32_t *)REG_PORTC_DIR_CLR)                /* PORTC Port x GPIO Direction Clear Register */
#define pREG_PORTC_INEN                  ((volatile uint32_t *)REG_PORTC_INEN)                   /* PORTC Port x GPIO Input Enable Register */
#define pREG_PORTC_INEN_SET              ((volatile uint32_t *)REG_PORTC_INEN_SET)               /* PORTC Port x GPIO Input Enable Set Register */
#define pREG_PORTC_INEN_CLR              ((volatile uint32_t *)REG_PORTC_INEN_CLR)               /* PORTC Port x GPIO Input Enable Clear Register */
#define pREG_PORTC_MUX                   ((volatile uint32_t *)REG_PORTC_MUX)                    /* PORTC Port x Multiplexer Control Register */
#define pREG_PORTC_DATA_TGL              ((volatile uint32_t *)REG_PORTC_DATA_TGL)               /* PORTC Port x GPIO Input Enable Toggle Register */
#define pREG_PORTC_POL                   ((volatile uint32_t *)REG_PORTC_POL)                    /* PORTC Port x GPIO Polarity Invert Register */
#define pREG_PORTC_POL_SET               ((volatile uint32_t *)REG_PORTC_POL_SET)                /* PORTC Port x GPIO Polarity Invert Set Register */
#define pREG_PORTC_POL_CLR               ((volatile uint32_t *)REG_PORTC_POL_CLR)                /* PORTC Port x GPIO Polarity Invert Clear Register */
#define pREG_PORTC_LOCK                  ((volatile uint32_t *)REG_PORTC_LOCK)                   /* PORTC Port x GPIO Lock Register */
#define pREG_PORTC_REVID                 ((volatile uint32_t *)REG_PORTC_REVID)                  /* PORTC Port x GPIO Revision ID */

/* =========================================================================
       PORTD
   ========================================================================= */
#define pREG_PORTD_FER                   ((volatile uint32_t *)REG_PORTD_FER)                    /* PORTD Port x Function Enable Register */
#define pREG_PORTD_FER_SET               ((volatile uint32_t *)REG_PORTD_FER_SET)                /* PORTD Port x Function Enable Set Register */
#define pREG_PORTD_FER_CLR               ((volatile uint32_t *)REG_PORTD_FER_CLR)                /* PORTD Port x Function Enable Clear Register */
#define pREG_PORTD_DATA                  ((volatile uint32_t *)REG_PORTD_DATA)                   /* PORTD Port x GPIO Data Register */
#define pREG_PORTD_DATA_SET              ((volatile uint32_t *)REG_PORTD_DATA_SET)               /* PORTD Port x GPIO Data Set Register */
#define pREG_PORTD_DATA_CLR              ((volatile uint32_t *)REG_PORTD_DATA_CLR)               /* PORTD Port x GPIO Data Clear Register */
#define pREG_PORTD_DIR                   ((volatile uint32_t *)REG_PORTD_DIR)                    /* PORTD Port x GPIO Direction Register */
#define pREG_PORTD_DIR_SET               ((volatile uint32_t *)REG_PORTD_DIR_SET)                /* PORTD Port x GPIO Direction Set Register */
#define pREG_PORTD_DIR_CLR               ((volatile uint32_t *)REG_PORTD_DIR_CLR)                /* PORTD Port x GPIO Direction Clear Register */
#define pREG_PORTD_INEN                  ((volatile uint32_t *)REG_PORTD_INEN)                   /* PORTD Port x GPIO Input Enable Register */
#define pREG_PORTD_INEN_SET              ((volatile uint32_t *)REG_PORTD_INEN_SET)               /* PORTD Port x GPIO Input Enable Set Register */
#define pREG_PORTD_INEN_CLR              ((volatile uint32_t *)REG_PORTD_INEN_CLR)               /* PORTD Port x GPIO Input Enable Clear Register */
#define pREG_PORTD_MUX                   ((volatile uint32_t *)REG_PORTD_MUX)                    /* PORTD Port x Multiplexer Control Register */
#define pREG_PORTD_DATA_TGL              ((volatile uint32_t *)REG_PORTD_DATA_TGL)               /* PORTD Port x GPIO Input Enable Toggle Register */
#define pREG_PORTD_POL                   ((volatile uint32_t *)REG_PORTD_POL)                    /* PORTD Port x GPIO Polarity Invert Register */
#define pREG_PORTD_POL_SET               ((volatile uint32_t *)REG_PORTD_POL_SET)                /* PORTD Port x GPIO Polarity Invert Set Register */
#define pREG_PORTD_POL_CLR               ((volatile uint32_t *)REG_PORTD_POL_CLR)                /* PORTD Port x GPIO Polarity Invert Clear Register */
#define pREG_PORTD_LOCK                  ((volatile uint32_t *)REG_PORTD_LOCK)                   /* PORTD Port x GPIO Lock Register */
#define pREG_PORTD_REVID                 ((volatile uint32_t *)REG_PORTD_REVID)                  /* PORTD Port x GPIO Revision ID */

/* =========================================================================
       PORTE
   ========================================================================= */
#define pREG_PORTE_FER                   ((volatile uint32_t *)REG_PORTE_FER)                    /* PORTE Port x Function Enable Register */
#define pREG_PORTE_FER_SET               ((volatile uint32_t *)REG_PORTE_FER_SET)                /* PORTE Port x Function Enable Set Register */
#define pREG_PORTE_FER_CLR               ((volatile uint32_t *)REG_PORTE_FER_CLR)                /* PORTE Port x Function Enable Clear Register */
#define pREG_PORTE_DATA                  ((volatile uint32_t *)REG_PORTE_DATA)                   /* PORTE Port x GPIO Data Register */
#define pREG_PORTE_DATA_SET              ((volatile uint32_t *)REG_PORTE_DATA_SET)               /* PORTE Port x GPIO Data Set Register */
#define pREG_PORTE_DATA_CLR              ((volatile uint32_t *)REG_PORTE_DATA_CLR)               /* PORTE Port x GPIO Data Clear Register */
#define pREG_PORTE_DIR                   ((volatile uint32_t *)REG_PORTE_DIR)                    /* PORTE Port x GPIO Direction Register */
#define pREG_PORTE_DIR_SET               ((volatile uint32_t *)REG_PORTE_DIR_SET)                /* PORTE Port x GPIO Direction Set Register */
#define pREG_PORTE_DIR_CLR               ((volatile uint32_t *)REG_PORTE_DIR_CLR)                /* PORTE Port x GPIO Direction Clear Register */
#define pREG_PORTE_INEN                  ((volatile uint32_t *)REG_PORTE_INEN)                   /* PORTE Port x GPIO Input Enable Register */
#define pREG_PORTE_INEN_SET              ((volatile uint32_t *)REG_PORTE_INEN_SET)               /* PORTE Port x GPIO Input Enable Set Register */
#define pREG_PORTE_INEN_CLR              ((volatile uint32_t *)REG_PORTE_INEN_CLR)               /* PORTE Port x GPIO Input Enable Clear Register */
#define pREG_PORTE_MUX                   ((volatile uint32_t *)REG_PORTE_MUX)                    /* PORTE Port x Multiplexer Control Register */
#define pREG_PORTE_DATA_TGL              ((volatile uint32_t *)REG_PORTE_DATA_TGL)               /* PORTE Port x GPIO Input Enable Toggle Register */
#define pREG_PORTE_POL                   ((volatile uint32_t *)REG_PORTE_POL)                    /* PORTE Port x GPIO Polarity Invert Register */
#define pREG_PORTE_POL_SET               ((volatile uint32_t *)REG_PORTE_POL_SET)                /* PORTE Port x GPIO Polarity Invert Set Register */
#define pREG_PORTE_POL_CLR               ((volatile uint32_t *)REG_PORTE_POL_CLR)                /* PORTE Port x GPIO Polarity Invert Clear Register */
#define pREG_PORTE_LOCK                  ((volatile uint32_t *)REG_PORTE_LOCK)                   /* PORTE Port x GPIO Lock Register */
#define pREG_PORTE_REVID                 ((volatile uint32_t *)REG_PORTE_REVID)                  /* PORTE Port x GPIO Revision ID */

/* =========================================================================
       PORTF
   ========================================================================= */
#define pREG_PORTF_FER                   ((volatile uint32_t *)REG_PORTF_FER)                    /* PORTF Port x Function Enable Register */
#define pREG_PORTF_FER_SET               ((volatile uint32_t *)REG_PORTF_FER_SET)                /* PORTF Port x Function Enable Set Register */
#define pREG_PORTF_FER_CLR               ((volatile uint32_t *)REG_PORTF_FER_CLR)                /* PORTF Port x Function Enable Clear Register */
#define pREG_PORTF_DATA                  ((volatile uint32_t *)REG_PORTF_DATA)                   /* PORTF Port x GPIO Data Register */
#define pREG_PORTF_DATA_SET              ((volatile uint32_t *)REG_PORTF_DATA_SET)               /* PORTF Port x GPIO Data Set Register */
#define pREG_PORTF_DATA_CLR              ((volatile uint32_t *)REG_PORTF_DATA_CLR)               /* PORTF Port x GPIO Data Clear Register */
#define pREG_PORTF_DIR                   ((volatile uint32_t *)REG_PORTF_DIR)                    /* PORTF Port x GPIO Direction Register */
#define pREG_PORTF_DIR_SET               ((volatile uint32_t *)REG_PORTF_DIR_SET)                /* PORTF Port x GPIO Direction Set Register */
#define pREG_PORTF_DIR_CLR               ((volatile uint32_t *)REG_PORTF_DIR_CLR)                /* PORTF Port x GPIO Direction Clear Register */
#define pREG_PORTF_INEN                  ((volatile uint32_t *)REG_PORTF_INEN)                   /* PORTF Port x GPIO Input Enable Register */
#define pREG_PORTF_INEN_SET              ((volatile uint32_t *)REG_PORTF_INEN_SET)               /* PORTF Port x GPIO Input Enable Set Register */
#define pREG_PORTF_INEN_CLR              ((volatile uint32_t *)REG_PORTF_INEN_CLR)               /* PORTF Port x GPIO Input Enable Clear Register */
#define pREG_PORTF_MUX                   ((volatile uint32_t *)REG_PORTF_MUX)                    /* PORTF Port x Multiplexer Control Register */
#define pREG_PORTF_DATA_TGL              ((volatile uint32_t *)REG_PORTF_DATA_TGL)               /* PORTF Port x GPIO Input Enable Toggle Register */
#define pREG_PORTF_POL                   ((volatile uint32_t *)REG_PORTF_POL)                    /* PORTF Port x GPIO Polarity Invert Register */
#define pREG_PORTF_POL_SET               ((volatile uint32_t *)REG_PORTF_POL_SET)                /* PORTF Port x GPIO Polarity Invert Set Register */
#define pREG_PORTF_POL_CLR               ((volatile uint32_t *)REG_PORTF_POL_CLR)                /* PORTF Port x GPIO Polarity Invert Clear Register */
#define pREG_PORTF_LOCK                  ((volatile uint32_t *)REG_PORTF_LOCK)                   /* PORTF Port x GPIO Lock Register */
#define pREG_PORTF_REVID                 ((volatile uint32_t *)REG_PORTF_REVID)                  /* PORTF Port x GPIO Revision ID */

/* =========================================================================
       PORTG
   ========================================================================= */
#define pREG_PORTG_FER                   ((volatile uint32_t *)REG_PORTG_FER)                    /* PORTG Port x Function Enable Register */
#define pREG_PORTG_FER_SET               ((volatile uint32_t *)REG_PORTG_FER_SET)                /* PORTG Port x Function Enable Set Register */
#define pREG_PORTG_FER_CLR               ((volatile uint32_t *)REG_PORTG_FER_CLR)                /* PORTG Port x Function Enable Clear Register */
#define pREG_PORTG_DATA                  ((volatile uint32_t *)REG_PORTG_DATA)                   /* PORTG Port x GPIO Data Register */
#define pREG_PORTG_DATA_SET              ((volatile uint32_t *)REG_PORTG_DATA_SET)               /* PORTG Port x GPIO Data Set Register */
#define pREG_PORTG_DATA_CLR              ((volatile uint32_t *)REG_PORTG_DATA_CLR)               /* PORTG Port x GPIO Data Clear Register */
#define pREG_PORTG_DIR                   ((volatile uint32_t *)REG_PORTG_DIR)                    /* PORTG Port x GPIO Direction Register */
#define pREG_PORTG_DIR_SET               ((volatile uint32_t *)REG_PORTG_DIR_SET)                /* PORTG Port x GPIO Direction Set Register */
#define pREG_PORTG_DIR_CLR               ((volatile uint32_t *)REG_PORTG_DIR_CLR)                /* PORTG Port x GPIO Direction Clear Register */
#define pREG_PORTG_INEN                  ((volatile uint32_t *)REG_PORTG_INEN)                   /* PORTG Port x GPIO Input Enable Register */
#define pREG_PORTG_INEN_SET              ((volatile uint32_t *)REG_PORTG_INEN_SET)               /* PORTG Port x GPIO Input Enable Set Register */
#define pREG_PORTG_INEN_CLR              ((volatile uint32_t *)REG_PORTG_INEN_CLR)               /* PORTG Port x GPIO Input Enable Clear Register */
#define pREG_PORTG_MUX                   ((volatile uint32_t *)REG_PORTG_MUX)                    /* PORTG Port x Multiplexer Control Register */
#define pREG_PORTG_DATA_TGL              ((volatile uint32_t *)REG_PORTG_DATA_TGL)               /* PORTG Port x GPIO Input Enable Toggle Register */
#define pREG_PORTG_POL                   ((volatile uint32_t *)REG_PORTG_POL)                    /* PORTG Port x GPIO Polarity Invert Register */
#define pREG_PORTG_POL_SET               ((volatile uint32_t *)REG_PORTG_POL_SET)                /* PORTG Port x GPIO Polarity Invert Set Register */
#define pREG_PORTG_POL_CLR               ((volatile uint32_t *)REG_PORTG_POL_CLR)                /* PORTG Port x GPIO Polarity Invert Clear Register */
#define pREG_PORTG_LOCK                  ((volatile uint32_t *)REG_PORTG_LOCK)                   /* PORTG Port x GPIO Lock Register */
#define pREG_PORTG_REVID                 ((volatile uint32_t *)REG_PORTG_REVID)                  /* PORTG Port x GPIO Revision ID */


/* =========================================================================
       PADS0
   ========================================================================= */
#define pREG_PADS0_EMAC_PTP_CLKSEL       ((volatile uint32_t *)REG_PADS0_EMAC_PTP_CLKSEL)        /* PADS0 Clock Selection for EMAC and PTP */
#define pREG_PADS0_TWI_VSEL              ((volatile uint32_t *)REG_PADS0_TWI_VSEL)               /* PADS0 TWI Voltage Selection */
#define pREG_PADS0_PORTS_HYST            ((volatile uint32_t *)REG_PADS0_PORTS_HYST)             /* PADS0 Hysteresis Enable Register */


/* =========================================================================
       PINT0
   ========================================================================= */
#define pREG_PINT0_MSK_SET               ((volatile uint32_t *)REG_PINT0_MSK_SET)                /* PINT0 Pint Mask Set Register */
#define pREG_PINT0_MSK_CLR               ((volatile uint32_t *)REG_PINT0_MSK_CLR)                /* PINT0 Pint Mask Clear Register */
#define pREG_PINT0_REQ                   ((volatile uint32_t *)REG_PINT0_REQ)                    /* PINT0 Pint Request Register */
#define pREG_PINT0_ASSIGN                ((volatile uint32_t *)REG_PINT0_ASSIGN)                 /* PINT0 Pint Assign Register */
#define pREG_PINT0_EDGE_SET              ((volatile uint32_t *)REG_PINT0_EDGE_SET)               /* PINT0 Pint Edge Set Register */
#define pREG_PINT0_EDGE_CLR              ((volatile uint32_t *)REG_PINT0_EDGE_CLR)               /* PINT0 Pint Edge Clear Register */
#define pREG_PINT0_INV_SET               ((volatile uint32_t *)REG_PINT0_INV_SET)                /* PINT0 Pint Invert Set Register */
#define pREG_PINT0_INV_CLR               ((volatile uint32_t *)REG_PINT0_INV_CLR)                /* PINT0 Pint Invert Clear Register */
#define pREG_PINT0_PINSTATE              ((volatile uint32_t *)REG_PINT0_PINSTATE)               /* PINT0 Pint Pinstate Register */
#define pREG_PINT0_LATCH                 ((volatile uint32_t *)REG_PINT0_LATCH)                  /* PINT0 Pint Latch Register */

/* =========================================================================
       PINT1
   ========================================================================= */
#define pREG_PINT1_MSK_SET               ((volatile uint32_t *)REG_PINT1_MSK_SET)                /* PINT1 Pint Mask Set Register */
#define pREG_PINT1_MSK_CLR               ((volatile uint32_t *)REG_PINT1_MSK_CLR)                /* PINT1 Pint Mask Clear Register */
#define pREG_PINT1_REQ                   ((volatile uint32_t *)REG_PINT1_REQ)                    /* PINT1 Pint Request Register */
#define pREG_PINT1_ASSIGN                ((volatile uint32_t *)REG_PINT1_ASSIGN)                 /* PINT1 Pint Assign Register */
#define pREG_PINT1_EDGE_SET              ((volatile uint32_t *)REG_PINT1_EDGE_SET)               /* PINT1 Pint Edge Set Register */
#define pREG_PINT1_EDGE_CLR              ((volatile uint32_t *)REG_PINT1_EDGE_CLR)               /* PINT1 Pint Edge Clear Register */
#define pREG_PINT1_INV_SET               ((volatile uint32_t *)REG_PINT1_INV_SET)                /* PINT1 Pint Invert Set Register */
#define pREG_PINT1_INV_CLR               ((volatile uint32_t *)REG_PINT1_INV_CLR)                /* PINT1 Pint Invert Clear Register */
#define pREG_PINT1_PINSTATE              ((volatile uint32_t *)REG_PINT1_PINSTATE)               /* PINT1 Pint Pinstate Register */
#define pREG_PINT1_LATCH                 ((volatile uint32_t *)REG_PINT1_LATCH)                  /* PINT1 Pint Latch Register */

/* =========================================================================
       PINT2
   ========================================================================= */
#define pREG_PINT2_MSK_SET               ((volatile uint32_t *)REG_PINT2_MSK_SET)                /* PINT2 Pint Mask Set Register */
#define pREG_PINT2_MSK_CLR               ((volatile uint32_t *)REG_PINT2_MSK_CLR)                /* PINT2 Pint Mask Clear Register */
#define pREG_PINT2_REQ                   ((volatile uint32_t *)REG_PINT2_REQ)                    /* PINT2 Pint Request Register */
#define pREG_PINT2_ASSIGN                ((volatile uint32_t *)REG_PINT2_ASSIGN)                 /* PINT2 Pint Assign Register */
#define pREG_PINT2_EDGE_SET              ((volatile uint32_t *)REG_PINT2_EDGE_SET)               /* PINT2 Pint Edge Set Register */
#define pREG_PINT2_EDGE_CLR              ((volatile uint32_t *)REG_PINT2_EDGE_CLR)               /* PINT2 Pint Edge Clear Register */
#define pREG_PINT2_INV_SET               ((volatile uint32_t *)REG_PINT2_INV_SET)                /* PINT2 Pint Invert Set Register */
#define pREG_PINT2_INV_CLR               ((volatile uint32_t *)REG_PINT2_INV_CLR)                /* PINT2 Pint Invert Clear Register */
#define pREG_PINT2_PINSTATE              ((volatile uint32_t *)REG_PINT2_PINSTATE)               /* PINT2 Pint Pinstate Register */
#define pREG_PINT2_LATCH                 ((volatile uint32_t *)REG_PINT2_LATCH)                  /* PINT2 Pint Latch Register */

/* =========================================================================
       PINT3
   ========================================================================= */
#define pREG_PINT3_MSK_SET               ((volatile uint32_t *)REG_PINT3_MSK_SET)                /* PINT3 Pint Mask Set Register */
#define pREG_PINT3_MSK_CLR               ((volatile uint32_t *)REG_PINT3_MSK_CLR)                /* PINT3 Pint Mask Clear Register */
#define pREG_PINT3_REQ                   ((volatile uint32_t *)REG_PINT3_REQ)                    /* PINT3 Pint Request Register */
#define pREG_PINT3_ASSIGN                ((volatile uint32_t *)REG_PINT3_ASSIGN)                 /* PINT3 Pint Assign Register */
#define pREG_PINT3_EDGE_SET              ((volatile uint32_t *)REG_PINT3_EDGE_SET)               /* PINT3 Pint Edge Set Register */
#define pREG_PINT3_EDGE_CLR              ((volatile uint32_t *)REG_PINT3_EDGE_CLR)               /* PINT3 Pint Edge Clear Register */
#define pREG_PINT3_INV_SET               ((volatile uint32_t *)REG_PINT3_INV_SET)                /* PINT3 Pint Invert Set Register */
#define pREG_PINT3_INV_CLR               ((volatile uint32_t *)REG_PINT3_INV_CLR)                /* PINT3 Pint Invert Clear Register */
#define pREG_PINT3_PINSTATE              ((volatile uint32_t *)REG_PINT3_PINSTATE)               /* PINT3 Pint Pinstate Register */
#define pREG_PINT3_LATCH                 ((volatile uint32_t *)REG_PINT3_LATCH)                  /* PINT3 Pint Latch Register */

/* =========================================================================
       PINT4
   ========================================================================= */
#define pREG_PINT4_MSK_SET               ((volatile uint32_t *)REG_PINT4_MSK_SET)                /* PINT4 Pint Mask Set Register */
#define pREG_PINT4_MSK_CLR               ((volatile uint32_t *)REG_PINT4_MSK_CLR)                /* PINT4 Pint Mask Clear Register */
#define pREG_PINT4_REQ                   ((volatile uint32_t *)REG_PINT4_REQ)                    /* PINT4 Pint Request Register */
#define pREG_PINT4_ASSIGN                ((volatile uint32_t *)REG_PINT4_ASSIGN)                 /* PINT4 Pint Assign Register */
#define pREG_PINT4_EDGE_SET              ((volatile uint32_t *)REG_PINT4_EDGE_SET)               /* PINT4 Pint Edge Set Register */
#define pREG_PINT4_EDGE_CLR              ((volatile uint32_t *)REG_PINT4_EDGE_CLR)               /* PINT4 Pint Edge Clear Register */
#define pREG_PINT4_INV_SET               ((volatile uint32_t *)REG_PINT4_INV_SET)                /* PINT4 Pint Invert Set Register */
#define pREG_PINT4_INV_CLR               ((volatile uint32_t *)REG_PINT4_INV_CLR)                /* PINT4 Pint Invert Clear Register */
#define pREG_PINT4_PINSTATE              ((volatile uint32_t *)REG_PINT4_PINSTATE)               /* PINT4 Pint Pinstate Register */
#define pREG_PINT4_LATCH                 ((volatile uint32_t *)REG_PINT4_LATCH)                  /* PINT4 Pint Latch Register */

/* =========================================================================
       PINT5
   ========================================================================= */
#define pREG_PINT5_MSK_SET               ((volatile uint32_t *)REG_PINT5_MSK_SET)                /* PINT5 Pint Mask Set Register */
#define pREG_PINT5_MSK_CLR               ((volatile uint32_t *)REG_PINT5_MSK_CLR)                /* PINT5 Pint Mask Clear Register */
#define pREG_PINT5_REQ                   ((volatile uint32_t *)REG_PINT5_REQ)                    /* PINT5 Pint Request Register */
#define pREG_PINT5_ASSIGN                ((volatile uint32_t *)REG_PINT5_ASSIGN)                 /* PINT5 Pint Assign Register */
#define pREG_PINT5_EDGE_SET              ((volatile uint32_t *)REG_PINT5_EDGE_SET)               /* PINT5 Pint Edge Set Register */
#define pREG_PINT5_EDGE_CLR              ((volatile uint32_t *)REG_PINT5_EDGE_CLR)               /* PINT5 Pint Edge Clear Register */
#define pREG_PINT5_INV_SET               ((volatile uint32_t *)REG_PINT5_INV_SET)                /* PINT5 Pint Invert Set Register */
#define pREG_PINT5_INV_CLR               ((volatile uint32_t *)REG_PINT5_INV_CLR)                /* PINT5 Pint Invert Clear Register */
#define pREG_PINT5_PINSTATE              ((volatile uint32_t *)REG_PINT5_PINSTATE)               /* PINT5 Pint Pinstate Register */
#define pREG_PINT5_LATCH                 ((volatile uint32_t *)REG_PINT5_LATCH)                  /* PINT5 Pint Latch Register */


/* =========================================================================
       SMC0
   ========================================================================= */
#define pREG_SMC0_GCTL                   ((volatile uint32_t *)REG_SMC0_GCTL)                    /* SMC0 Grant Control Register */
#define pREG_SMC0_GSTAT                  ((volatile uint32_t *)REG_SMC0_GSTAT)                   /* SMC0 Grant Status Register */
#define pREG_SMC0_B0CTL                  ((volatile uint32_t *)REG_SMC0_B0CTL)                   /* SMC0 Bank 0 Control Register */
#define pREG_SMC0_B0TIM                  ((volatile uint32_t *)REG_SMC0_B0TIM)                   /* SMC0 Bank 0 Timing Register */
#define pREG_SMC0_B0ETIM                 ((volatile uint32_t *)REG_SMC0_B0ETIM)                  /* SMC0 Bank 0 Extended Timing Register */
#define pREG_SMC0_B1CTL                  ((volatile uint32_t *)REG_SMC0_B1CTL)                   /* SMC0 Bank 1 Control Register */
#define pREG_SMC0_B1TIM                  ((volatile uint32_t *)REG_SMC0_B1TIM)                   /* SMC0 Bank 1 Timing Register */
#define pREG_SMC0_B1ETIM                 ((volatile uint32_t *)REG_SMC0_B1ETIM)                  /* SMC0 Bank 1 Extended Timing Register */
#define pREG_SMC0_B2CTL                  ((volatile uint32_t *)REG_SMC0_B2CTL)                   /* SMC0 Bank 2 Control Register */
#define pREG_SMC0_B2TIM                  ((volatile uint32_t *)REG_SMC0_B2TIM)                   /* SMC0 Bank 2 Timing Register */
#define pREG_SMC0_B2ETIM                 ((volatile uint32_t *)REG_SMC0_B2ETIM)                  /* SMC0 Bank 2 Extended Timing Register */
#define pREG_SMC0_B3CTL                  ((volatile uint32_t *)REG_SMC0_B3CTL)                   /* SMC0 Bank 3 Control Register */
#define pREG_SMC0_B3TIM                  ((volatile uint32_t *)REG_SMC0_B3TIM)                   /* SMC0 Bank 3 Timing Register */
#define pREG_SMC0_B3ETIM                 ((volatile uint32_t *)REG_SMC0_B3ETIM)                  /* SMC0 Bank 3 Extended Timing Register */


/* =========================================================================
       WDOG0
   ========================================================================= */
#define pREG_WDOG0_CTL                   ((volatile uint32_t *)REG_WDOG0_CTL)                    /* WDOG0 Control Register */
#define pREG_WDOG0_CNT                   ((volatile uint32_t *)REG_WDOG0_CNT)                    /* WDOG0 Count Register */
#define pREG_WDOG0_STAT                  ((volatile uint32_t *)REG_WDOG0_STAT)                   /* WDOG0 Watchdog Timer Status Register */

/* =========================================================================
       WDOG1
   ========================================================================= */
#define pREG_WDOG1_CTL                   ((volatile uint32_t *)REG_WDOG1_CTL)                    /* WDOG1 Control Register */
#define pREG_WDOG1_CNT                   ((volatile uint32_t *)REG_WDOG1_CNT)                    /* WDOG1 Count Register */
#define pREG_WDOG1_STAT                  ((volatile uint32_t *)REG_WDOG1_STAT)                   /* WDOG1 Watchdog Timer Status Register */


/* =========================================================================
       EPPI0
   ========================================================================= */
#define pREG_EPPI0_STAT                  ((volatile uint32_t *)REG_EPPI0_STAT)                   /* EPPI0 Status Register */
#define pREG_EPPI0_HCNT                  ((volatile uint32_t *)REG_EPPI0_HCNT)                   /* EPPI0 Horizontal Transfer Count Register */
#define pREG_EPPI0_HDLY                  ((volatile uint32_t *)REG_EPPI0_HDLY)                   /* EPPI0 Horizontal Delay Count Register */
#define pREG_EPPI0_VCNT                  ((volatile uint32_t *)REG_EPPI0_VCNT)                   /* EPPI0 Vertical Transfer Count Register */
#define pREG_EPPI0_VDLY                  ((volatile uint32_t *)REG_EPPI0_VDLY)                   /* EPPI0 Vertical Delay Count Register */
#define pREG_EPPI0_FRAME                 ((volatile uint32_t *)REG_EPPI0_FRAME)                  /* EPPI0 Lines Per Frame Register */
#define pREG_EPPI0_LINE                  ((volatile uint32_t *)REG_EPPI0_LINE)                   /* EPPI0 Samples Per Line Register */
#define pREG_EPPI0_CLKDIV                ((volatile uint32_t *)REG_EPPI0_CLKDIV)                 /* EPPI0 Clock Divide Register */
#define pREG_EPPI0_CTL                   ((volatile uint32_t *)REG_EPPI0_CTL)                    /* EPPI0 Control Register */
#define pREG_EPPI0_FS1_WLHB              ((volatile uint32_t *)REG_EPPI0_FS1_WLHB)               /* EPPI0 FS1 Width Register / EPPI Horizontal Blanking Samples Per Line Register */
#define pREG_EPPI0_FS1_PASPL             ((volatile uint32_t *)REG_EPPI0_FS1_PASPL)              /* EPPI0 FS1 Period Register / EPPI Active Samples Per Line Register */
#define pREG_EPPI0_FS2_WLVB              ((volatile uint32_t *)REG_EPPI0_FS2_WLVB)               /* EPPI0 FS2 Width Register / EPPI Lines Of Vertical Blanking Register */
#define pREG_EPPI0_FS2_PALPF             ((volatile uint32_t *)REG_EPPI0_FS2_PALPF)              /* EPPI0 FS2 Period Register / EPPI Active Lines Per Field Register */
#define pREG_EPPI0_IMSK                  ((volatile uint32_t *)REG_EPPI0_IMSK)                   /* EPPI0 Interrupt Mask Register */
#define pREG_EPPI0_ODDCLIP               ((volatile uint32_t *)REG_EPPI0_ODDCLIP)                /* EPPI0 Clipping Register for ODD (Chroma) Data */
#define pREG_EPPI0_EVENCLIP              ((volatile uint32_t *)REG_EPPI0_EVENCLIP)               /* EPPI0 Clipping Register for EVEN (Luma) Data */
#define pREG_EPPI0_FS1_DLY               ((volatile uint32_t *)REG_EPPI0_FS1_DLY)                /* EPPI0 Frame Sync 1 Delay Value */
#define pREG_EPPI0_FS2_DLY               ((volatile uint32_t *)REG_EPPI0_FS2_DLY)                /* EPPI0 Frame Sync 2 Delay Value */
#define pREG_EPPI0_CTL2                  ((volatile uint32_t *)REG_EPPI0_CTL2)                   /* EPPI0 Control Register 2 */

/* =========================================================================
       EPPI1
   ========================================================================= */
#define pREG_EPPI1_STAT                  ((volatile uint32_t *)REG_EPPI1_STAT)                   /* EPPI1 Status Register */
#define pREG_EPPI1_HCNT                  ((volatile uint32_t *)REG_EPPI1_HCNT)                   /* EPPI1 Horizontal Transfer Count Register */
#define pREG_EPPI1_HDLY                  ((volatile uint32_t *)REG_EPPI1_HDLY)                   /* EPPI1 Horizontal Delay Count Register */
#define pREG_EPPI1_VCNT                  ((volatile uint32_t *)REG_EPPI1_VCNT)                   /* EPPI1 Vertical Transfer Count Register */
#define pREG_EPPI1_VDLY                  ((volatile uint32_t *)REG_EPPI1_VDLY)                   /* EPPI1 Vertical Delay Count Register */
#define pREG_EPPI1_FRAME                 ((volatile uint32_t *)REG_EPPI1_FRAME)                  /* EPPI1 Lines Per Frame Register */
#define pREG_EPPI1_LINE                  ((volatile uint32_t *)REG_EPPI1_LINE)                   /* EPPI1 Samples Per Line Register */
#define pREG_EPPI1_CLKDIV                ((volatile uint32_t *)REG_EPPI1_CLKDIV)                 /* EPPI1 Clock Divide Register */
#define pREG_EPPI1_CTL                   ((volatile uint32_t *)REG_EPPI1_CTL)                    /* EPPI1 Control Register */
#define pREG_EPPI1_FS1_WLHB              ((volatile uint32_t *)REG_EPPI1_FS1_WLHB)               /* EPPI1 FS1 Width Register / EPPI Horizontal Blanking Samples Per Line Register */
#define pREG_EPPI1_FS1_PASPL             ((volatile uint32_t *)REG_EPPI1_FS1_PASPL)              /* EPPI1 FS1 Period Register / EPPI Active Samples Per Line Register */
#define pREG_EPPI1_FS2_WLVB              ((volatile uint32_t *)REG_EPPI1_FS2_WLVB)               /* EPPI1 FS2 Width Register / EPPI Lines Of Vertical Blanking Register */
#define pREG_EPPI1_FS2_PALPF             ((volatile uint32_t *)REG_EPPI1_FS2_PALPF)              /* EPPI1 FS2 Period Register / EPPI Active Lines Per Field Register */
#define pREG_EPPI1_IMSK                  ((volatile uint32_t *)REG_EPPI1_IMSK)                   /* EPPI1 Interrupt Mask Register */
#define pREG_EPPI1_ODDCLIP               ((volatile uint32_t *)REG_EPPI1_ODDCLIP)                /* EPPI1 Clipping Register for ODD (Chroma) Data */
#define pREG_EPPI1_EVENCLIP              ((volatile uint32_t *)REG_EPPI1_EVENCLIP)               /* EPPI1 Clipping Register for EVEN (Luma) Data */
#define pREG_EPPI1_FS1_DLY               ((volatile uint32_t *)REG_EPPI1_FS1_DLY)                /* EPPI1 Frame Sync 1 Delay Value */
#define pREG_EPPI1_FS2_DLY               ((volatile uint32_t *)REG_EPPI1_FS2_DLY)                /* EPPI1 Frame Sync 2 Delay Value */
#define pREG_EPPI1_CTL2                  ((volatile uint32_t *)REG_EPPI1_CTL2)                   /* EPPI1 Control Register 2 */

/* =========================================================================
       EPPI2
   ========================================================================= */
#define pREG_EPPI2_STAT                  ((volatile uint32_t *)REG_EPPI2_STAT)                   /* EPPI2 Status Register */
#define pREG_EPPI2_HCNT                  ((volatile uint32_t *)REG_EPPI2_HCNT)                   /* EPPI2 Horizontal Transfer Count Register */
#define pREG_EPPI2_HDLY                  ((volatile uint32_t *)REG_EPPI2_HDLY)                   /* EPPI2 Horizontal Delay Count Register */
#define pREG_EPPI2_VCNT                  ((volatile uint32_t *)REG_EPPI2_VCNT)                   /* EPPI2 Vertical Transfer Count Register */
#define pREG_EPPI2_VDLY                  ((volatile uint32_t *)REG_EPPI2_VDLY)                   /* EPPI2 Vertical Delay Count Register */
#define pREG_EPPI2_FRAME                 ((volatile uint32_t *)REG_EPPI2_FRAME)                  /* EPPI2 Lines Per Frame Register */
#define pREG_EPPI2_LINE                  ((volatile uint32_t *)REG_EPPI2_LINE)                   /* EPPI2 Samples Per Line Register */
#define pREG_EPPI2_CLKDIV                ((volatile uint32_t *)REG_EPPI2_CLKDIV)                 /* EPPI2 Clock Divide Register */
#define pREG_EPPI2_CTL                   ((volatile uint32_t *)REG_EPPI2_CTL)                    /* EPPI2 Control Register */
#define pREG_EPPI2_FS1_WLHB              ((volatile uint32_t *)REG_EPPI2_FS1_WLHB)               /* EPPI2 FS1 Width Register / EPPI Horizontal Blanking Samples Per Line Register */
#define pREG_EPPI2_FS1_PASPL             ((volatile uint32_t *)REG_EPPI2_FS1_PASPL)              /* EPPI2 FS1 Period Register / EPPI Active Samples Per Line Register */
#define pREG_EPPI2_FS2_WLVB              ((volatile uint32_t *)REG_EPPI2_FS2_WLVB)               /* EPPI2 FS2 Width Register / EPPI Lines Of Vertical Blanking Register */
#define pREG_EPPI2_FS2_PALPF             ((volatile uint32_t *)REG_EPPI2_FS2_PALPF)              /* EPPI2 FS2 Period Register / EPPI Active Lines Per Field Register */
#define pREG_EPPI2_IMSK                  ((volatile uint32_t *)REG_EPPI2_IMSK)                   /* EPPI2 Interrupt Mask Register */
#define pREG_EPPI2_ODDCLIP               ((volatile uint32_t *)REG_EPPI2_ODDCLIP)                /* EPPI2 Clipping Register for ODD (Chroma) Data */
#define pREG_EPPI2_EVENCLIP              ((volatile uint32_t *)REG_EPPI2_EVENCLIP)               /* EPPI2 Clipping Register for EVEN (Luma) Data */
#define pREG_EPPI2_FS1_DLY               ((volatile uint32_t *)REG_EPPI2_FS1_DLY)                /* EPPI2 Frame Sync 1 Delay Value */
#define pREG_EPPI2_FS2_DLY               ((volatile uint32_t *)REG_EPPI2_FS2_DLY)                /* EPPI2 Frame Sync 2 Delay Value */
#define pREG_EPPI2_CTL2                  ((volatile uint32_t *)REG_EPPI2_CTL2)                   /* EPPI2 Control Register 2 */


/* =========================================================================
       PIXC0
   ========================================================================= */
#define pREG_PIXC0_CTL                   ((volatile uint32_t *)REG_PIXC0_CTL)                    /* PIXC0 Control Register */
#define pREG_PIXC0_PPL                   ((volatile uint16_t *)REG_PIXC0_PPL)                    /* PIXC0 Pixels Per Line Register */
#define pREG_PIXC0_LPF                   ((volatile uint16_t *)REG_PIXC0_LPF)                    /* PIXC0 Line Per Frame Register */
#define pREG_PIXC0_HSTART_A              ((volatile uint16_t *)REG_PIXC0_HSTART_A)               /* PIXC0 Overlay A Horizontal Start Register */
#define pREG_PIXC0_HEND_A                ((volatile uint16_t *)REG_PIXC0_HEND_A)                 /* PIXC0 Overlay A Horizontal End Register */
#define pREG_PIXC0_VSTART_A              ((volatile uint16_t *)REG_PIXC0_VSTART_A)               /* PIXC0 Overlay A Vertical Start Register */
#define pREG_PIXC0_VEND_A                ((volatile uint16_t *)REG_PIXC0_VEND_A)                 /* PIXC0 Overlay A Vertical End Register */
#define pREG_PIXC0_TRANSP_A              ((volatile uint16_t *)REG_PIXC0_TRANSP_A)               /* PIXC0 Overlay A Transparency Ratio Register */
#define pREG_PIXC0_HSTART_B              ((volatile uint16_t *)REG_PIXC0_HSTART_B)               /* PIXC0 Overlay B Horizontal Start Register */
#define pREG_PIXC0_HEND_B                ((volatile uint16_t *)REG_PIXC0_HEND_B)                 /* PIXC0 Overlay B Horizontal End Register */
#define pREG_PIXC0_VSTART_B              ((volatile uint16_t *)REG_PIXC0_VSTART_B)               /* PIXC0 Overlay B Vertical Start Register */
#define pREG_PIXC0_VEND_B                ((volatile uint16_t *)REG_PIXC0_VEND_B)                 /* PIXC0 Overlay B Vertical End Register */
#define pREG_PIXC0_TRANSP_B              ((volatile uint16_t *)REG_PIXC0_TRANSP_B)               /* PIXC0 Overlay B Transparency Ratio Register */
#define pREG_PIXC0_IRQSTAT               ((volatile uint16_t *)REG_PIXC0_IRQSTAT)                /* PIXC0 Interrupt Status Register */
#define pREG_PIXC0_CONRY                 ((volatile uint32_t *)REG_PIXC0_CONRY)                  /* PIXC0 RY Conversion Component Register */
#define pREG_PIXC0_CONGU                 ((volatile uint32_t *)REG_PIXC0_CONGU)                  /* PIXC0 GU Conversion Component Register */
#define pREG_PIXC0_CONBV                 ((volatile uint32_t *)REG_PIXC0_CONBV)                  /* PIXC0 BV Conversion Component Register */
#define pREG_PIXC0_CCBIAS                ((volatile uint32_t *)REG_PIXC0_CCBIAS)                 /* PIXC0 Conversion Bias Register */
#define pREG_PIXC0_TC                    ((volatile uint32_t *)REG_PIXC0_TC)                     /* PIXC0 Transparency Color Register */
#define pREG_PIXC0_REVID                 ((volatile uint32_t *)REG_PIXC0_REVID)                  /* PIXC0 Revision Id */


/* =========================================================================
       PVP0
   ========================================================================= */
#define pREG_PVP0_REVID                  ((volatile uint32_t *)REG_PVP0_REVID)                   /* PVP0 Revision ID */
#define pREG_PVP0_CTL                    ((volatile uint32_t *)REG_PVP0_CTL)                     /* PVP0 Control */
#define pREG_PVP0_IMSK0                  ((volatile uint32_t *)REG_PVP0_IMSK0)                   /* PVP0 Interrupt Mask n */
#define pREG_PVP0_IMSK1                  ((volatile uint32_t *)REG_PVP0_IMSK1)                   /* PVP0 Interrupt Mask n */
#define pREG_PVP0_STAT                   ((volatile uint32_t *)REG_PVP0_STAT)                    /* PVP0 Status */
#define pREG_PVP0_ILAT                   ((volatile uint32_t *)REG_PVP0_ILAT)                    /* PVP0 Interrupt Latch Status n */
#define pREG_PVP0_IREQ0                  ((volatile uint32_t *)REG_PVP0_IREQ0)                   /* PVP0 Interrupt Request n */
#define pREG_PVP0_IREQ1                  ((volatile uint32_t *)REG_PVP0_IREQ1)                   /* PVP0 Interrupt Request n */
#define pREG_PVP0_OPF0_CFG               ((volatile uint32_t *)REG_PVP0_OPF0_CFG)                /* PVP0 OPFn (Camera Pipe) Configuration */
#define pREG_PVP0_OPF1_CFG               ((volatile uint32_t *)REG_PVP0_OPF1_CFG)                /* PVP0 OPFn (Camera Pipe) Configuration */
#define pREG_PVP0_OPF2_CFG               ((volatile uint32_t *)REG_PVP0_OPF2_CFG)                /* PVP0 OPFn (Camera Pipe) Configuration */
#define pREG_PVP0_OPF0_CTL               ((volatile uint32_t *)REG_PVP0_OPF0_CTL)                /* PVP0 OPFn (Camera Pipe) Control */
#define pREG_PVP0_OPF1_CTL               ((volatile uint32_t *)REG_PVP0_OPF1_CTL)                /* PVP0 OPFn (Camera Pipe) Control */
#define pREG_PVP0_OPF2_CTL               ((volatile uint32_t *)REG_PVP0_OPF2_CTL)                /* PVP0 OPFn (Camera Pipe) Control */
#define pREG_PVP0_OPF3_CFG               ((volatile uint32_t *)REG_PVP0_OPF3_CFG)                /* PVP0 OPF3 (Memory Pipe) Configuration */
#define pREG_PVP0_OPF3_CTL               ((volatile uint32_t *)REG_PVP0_OPF3_CTL)                /* PVP0 OPF3 (Memory Pipe) Control */
#define pREG_PVP0_PEC_CFG                ((volatile uint32_t *)REG_PVP0_PEC_CFG)                 /* PVP0 PEC Configuration */
#define pREG_PVP0_PEC_CTL                ((volatile uint32_t *)REG_PVP0_PEC_CTL)                 /* PVP0 PEC Control */
#define pREG_PVP0_PEC_D1TH0              ((volatile uint32_t *)REG_PVP0_PEC_D1TH0)               /* PVP0 PEC Lower Hysteresis Threshold */
#define pREG_PVP0_PEC_D1TH1              ((volatile uint32_t *)REG_PVP0_PEC_D1TH1)               /* PVP0 PEC Upper Hysteresis Threshold */
#define pREG_PVP0_PEC_D2TH0              ((volatile uint32_t *)REG_PVP0_PEC_D2TH0)               /* PVP0 PEC Weak Zero Crossing Threshold */
#define pREG_PVP0_PEC_D2TH1              ((volatile uint32_t *)REG_PVP0_PEC_D2TH1)               /* PVP0 PEC Strong Zero Crossing Threshold */
#define pREG_PVP0_IIM0_CFG               ((volatile uint32_t *)REG_PVP0_IIM0_CFG)                /* PVP0 IIMn Configuration */
#define pREG_PVP0_IIM1_CFG               ((volatile uint32_t *)REG_PVP0_IIM1_CFG)                /* PVP0 IIMn Configuration */
#define pREG_PVP0_IIM0_CTL               ((volatile uint32_t *)REG_PVP0_IIM0_CTL)                /* PVP0 IIMn Control */
#define pREG_PVP0_IIM1_CTL               ((volatile uint32_t *)REG_PVP0_IIM1_CTL)                /* PVP0 IIMn Control */
#define pREG_PVP0_IIM0_SCALE             ((volatile uint32_t *)REG_PVP0_IIM0_SCALE)              /* PVP0 IIMn Scaling Values */
#define pREG_PVP0_IIM1_SCALE             ((volatile uint32_t *)REG_PVP0_IIM1_SCALE)              /* PVP0 IIMn Scaling Values */
#define pREG_PVP0_IIM0_SOVF_STAT         ((volatile uint32_t *)REG_PVP0_IIM0_SOVF_STAT)          /* PVP0 IIMn Signed Overflow Status */
#define pREG_PVP0_IIM1_SOVF_STAT         ((volatile uint32_t *)REG_PVP0_IIM1_SOVF_STAT)          /* PVP0 IIMn Signed Overflow Status */
#define pREG_PVP0_IIM0_UOVF_STAT         ((volatile uint32_t *)REG_PVP0_IIM0_UOVF_STAT)          /* PVP0 IIMn Unsigned Overflow Status */
#define pREG_PVP0_IIM1_UOVF_STAT         ((volatile uint32_t *)REG_PVP0_IIM1_UOVF_STAT)          /* PVP0 IIMn Unsigned Overflow Status */
#define pREG_PVP0_ACU_CFG                ((volatile uint32_t *)REG_PVP0_ACU_CFG)                 /* PVP0 ACU Configuration */
#define pREG_PVP0_ACU_CTL                ((volatile uint32_t *)REG_PVP0_ACU_CTL)                 /* PVP0 ACU Control */
#define pREG_PVP0_ACU_OFFSET             ((volatile uint32_t *)REG_PVP0_ACU_OFFSET)              /* PVP0 ACU SUM Constant */
#define pREG_PVP0_ACU_FACTOR             ((volatile uint32_t *)REG_PVP0_ACU_FACTOR)              /* PVP0 ACU PROD Constant */
#define pREG_PVP0_ACU_SHIFT              ((volatile uint32_t *)REG_PVP0_ACU_SHIFT)               /* PVP0 ACU Shift Constant */
#define pREG_PVP0_ACU_MIN                ((volatile uint32_t *)REG_PVP0_ACU_MIN)                 /* PVP0 ACU Lower Sat Threshold Min */
#define pREG_PVP0_ACU_MAX                ((volatile uint32_t *)REG_PVP0_ACU_MAX)                 /* PVP0 ACU Upper Sat Threshold Max */
#define pREG_PVP0_UDS_CFG                ((volatile uint32_t *)REG_PVP0_UDS_CFG)                 /* PVP0 UDS Configuration */
#define pREG_PVP0_UDS_CTL                ((volatile uint32_t *)REG_PVP0_UDS_CTL)                 /* PVP0 UDS Control */
#define pREG_PVP0_UDS_OHCNT              ((volatile uint32_t *)REG_PVP0_UDS_OHCNT)               /* PVP0 UDS Output HCNT */
#define pREG_PVP0_UDS_OVCNT              ((volatile uint32_t *)REG_PVP0_UDS_OVCNT)               /* PVP0 UDS Output VCNT */
#define pREG_PVP0_UDS_HAVG               ((volatile uint32_t *)REG_PVP0_UDS_HAVG)                /* PVP0 UDS HAVG */
#define pREG_PVP0_UDS_VAVG               ((volatile uint32_t *)REG_PVP0_UDS_VAVG)                /* PVP0 UDS VAVG */
#define pREG_PVP0_IPF0_CFG               ((volatile uint32_t *)REG_PVP0_IPF0_CFG)                /* PVP0 IPF0 (Camera Pipe) Configuration */
#define pREG_PVP0_IPF0_PIPECTL           ((volatile uint32_t *)REG_PVP0_IPF0_PIPECTL)            /* PVP0 IPFn (Camera/Memory Pipe) Pipe Control */
#define pREG_PVP0_IPF1_PIPECTL           ((volatile uint32_t *)REG_PVP0_IPF1_PIPECTL)            /* PVP0 IPFn (Camera/Memory Pipe) Pipe Control */
#define pREG_PVP0_IPF0_CTL               ((volatile uint32_t *)REG_PVP0_IPF0_CTL)                /* PVP0 IPFn (Camera/Memory Pipe) Control */
#define pREG_PVP0_IPF1_CTL               ((volatile uint32_t *)REG_PVP0_IPF1_CTL)                /* PVP0 IPFn (Camera/Memory Pipe) Control */
#define pREG_PVP0_IPF0_TAG               ((volatile uint32_t *)REG_PVP0_IPF0_TAG)                /* PVP0 IPFn (Camera/Memory Pipe) TAG Value */
#define pREG_PVP0_IPF1_TAG               ((volatile uint32_t *)REG_PVP0_IPF1_TAG)                /* PVP0 IPFn (Camera/Memory Pipe) TAG Value */
#define pREG_PVP0_IPF0_FCNT              ((volatile uint32_t *)REG_PVP0_IPF0_FCNT)               /* PVP0 IPFn (Camera/Memory Pipe) Frame Count */
#define pREG_PVP0_IPF1_FCNT              ((volatile uint32_t *)REG_PVP0_IPF1_FCNT)               /* PVP0 IPFn (Camera/Memory Pipe) Frame Count */
#define pREG_PVP0_IPF0_HCNT              ((volatile uint32_t *)REG_PVP0_IPF0_HCNT)               /* PVP0 IPFn (Camera/Memory Pipe) Horizontal Count */
#define pREG_PVP0_IPF1_HCNT              ((volatile uint32_t *)REG_PVP0_IPF1_HCNT)               /* PVP0 IPFn (Camera/Memory Pipe) Horizontal Count */
#define pREG_PVP0_IPF0_VCNT              ((volatile uint32_t *)REG_PVP0_IPF0_VCNT)               /* PVP0 IPFn (Camera/Memory Pipe) Vertical Count */
#define pREG_PVP0_IPF1_VCNT              ((volatile uint32_t *)REG_PVP0_IPF1_VCNT)               /* PVP0 IPFn (Camera/Memory Pipe) Vertical Count */
#define pREG_PVP0_IPF0_HPOS              ((volatile uint32_t *)REG_PVP0_IPF0_HPOS)               /* PVP0 IPF0 (Camera Pipe) Horizontal Position */
#define pREG_PVP0_IPF0_VPOS              ((volatile uint32_t *)REG_PVP0_IPF0_VPOS)               /* PVP0 IPF0 (Camera Pipe) Vertical Position */
#define pREG_PVP0_IPF0_TAG_STAT          ((volatile uint32_t *)REG_PVP0_IPF0_TAG_STAT)           /* PVP0 IPFn (Camera/Memory Pipe) TAG Status */
#define pREG_PVP0_IPF1_TAG_STAT          ((volatile uint32_t *)REG_PVP0_IPF1_TAG_STAT)           /* PVP0 IPFn (Camera/Memory Pipe) TAG Status */
#define pREG_PVP0_IPF1_CFG               ((volatile uint32_t *)REG_PVP0_IPF1_CFG)                /* PVP0 IPF1 (Memory Pipe) Configuration */
#define pREG_PVP0_CNV0_CFG               ((volatile uint32_t *)REG_PVP0_CNV0_CFG)                /* PVP0 CNVn Configuration */
#define pREG_PVP0_CNV1_CFG               ((volatile uint32_t *)REG_PVP0_CNV1_CFG)                /* PVP0 CNVn Configuration */
#define pREG_PVP0_CNV2_CFG               ((volatile uint32_t *)REG_PVP0_CNV2_CFG)                /* PVP0 CNVn Configuration */
#define pREG_PVP0_CNV3_CFG               ((volatile uint32_t *)REG_PVP0_CNV3_CFG)                /* PVP0 CNVn Configuration */
#define pREG_PVP0_CNV0_CTL               ((volatile uint32_t *)REG_PVP0_CNV0_CTL)                /* PVP0 CNVn Control */
#define pREG_PVP0_CNV1_CTL               ((volatile uint32_t *)REG_PVP0_CNV1_CTL)                /* PVP0 CNVn Control */
#define pREG_PVP0_CNV2_CTL               ((volatile uint32_t *)REG_PVP0_CNV2_CTL)                /* PVP0 CNVn Control */
#define pREG_PVP0_CNV3_CTL               ((volatile uint32_t *)REG_PVP0_CNV3_CTL)                /* PVP0 CNVn Control */
#define pREG_PVP0_CNV0_C00C01            ((volatile uint32_t *)REG_PVP0_CNV0_C00C01)             /* PVP0 CNVn Coefficients 0,0 and 0,1 */
#define pREG_PVP0_CNV1_C00C01            ((volatile uint32_t *)REG_PVP0_CNV1_C00C01)             /* PVP0 CNVn Coefficients 0,0 and 0,1 */
#define pREG_PVP0_CNV2_C00C01            ((volatile uint32_t *)REG_PVP0_CNV2_C00C01)             /* PVP0 CNVn Coefficients 0,0 and 0,1 */
#define pREG_PVP0_CNV3_C00C01            ((volatile uint32_t *)REG_PVP0_CNV3_C00C01)             /* PVP0 CNVn Coefficients 0,0 and 0,1 */
#define pREG_PVP0_CNV0_C02C03            ((volatile uint32_t *)REG_PVP0_CNV0_C02C03)             /* PVP0 CNVn Coefficients 0,2 and 0,3 */
#define pREG_PVP0_CNV1_C02C03            ((volatile uint32_t *)REG_PVP0_CNV1_C02C03)             /* PVP0 CNVn Coefficients 0,2 and 0,3 */
#define pREG_PVP0_CNV2_C02C03            ((volatile uint32_t *)REG_PVP0_CNV2_C02C03)             /* PVP0 CNVn Coefficients 0,2 and 0,3 */
#define pREG_PVP0_CNV3_C02C03            ((volatile uint32_t *)REG_PVP0_CNV3_C02C03)             /* PVP0 CNVn Coefficients 0,2 and 0,3 */
#define pREG_PVP0_CNV0_C04               ((volatile uint32_t *)REG_PVP0_CNV0_C04)                /* PVP0 CNVn Coefficient 0,4 */
#define pREG_PVP0_CNV1_C04               ((volatile uint32_t *)REG_PVP0_CNV1_C04)                /* PVP0 CNVn Coefficient 0,4 */
#define pREG_PVP0_CNV2_C04               ((volatile uint32_t *)REG_PVP0_CNV2_C04)                /* PVP0 CNVn Coefficient 0,4 */
#define pREG_PVP0_CNV3_C04               ((volatile uint32_t *)REG_PVP0_CNV3_C04)                /* PVP0 CNVn Coefficient 0,4 */
#define pREG_PVP0_CNV0_C10C11            ((volatile uint32_t *)REG_PVP0_CNV0_C10C11)             /* PVP0 CNVn Coefficients 1,0 and 1,1 */
#define pREG_PVP0_CNV1_C10C11            ((volatile uint32_t *)REG_PVP0_CNV1_C10C11)             /* PVP0 CNVn Coefficients 1,0 and 1,1 */
#define pREG_PVP0_CNV2_C10C11            ((volatile uint32_t *)REG_PVP0_CNV2_C10C11)             /* PVP0 CNVn Coefficients 1,0 and 1,1 */
#define pREG_PVP0_CNV3_C10C11            ((volatile uint32_t *)REG_PVP0_CNV3_C10C11)             /* PVP0 CNVn Coefficients 1,0 and 1,1 */
#define pREG_PVP0_CNV0_C12C13            ((volatile uint32_t *)REG_PVP0_CNV0_C12C13)             /* PVP0 CNVn Coefficients 1,2 and 1,3 */
#define pREG_PVP0_CNV1_C12C13            ((volatile uint32_t *)REG_PVP0_CNV1_C12C13)             /* PVP0 CNVn Coefficients 1,2 and 1,3 */
#define pREG_PVP0_CNV2_C12C13            ((volatile uint32_t *)REG_PVP0_CNV2_C12C13)             /* PVP0 CNVn Coefficients 1,2 and 1,3 */
#define pREG_PVP0_CNV3_C12C13            ((volatile uint32_t *)REG_PVP0_CNV3_C12C13)             /* PVP0 CNVn Coefficients 1,2 and 1,3 */
#define pREG_PVP0_CNV0_C14               ((volatile uint32_t *)REG_PVP0_CNV0_C14)                /* PVP0 CNVn Coefficient 1,4 */
#define pREG_PVP0_CNV1_C14               ((volatile uint32_t *)REG_PVP0_CNV1_C14)                /* PVP0 CNVn Coefficient 1,4 */
#define pREG_PVP0_CNV2_C14               ((volatile uint32_t *)REG_PVP0_CNV2_C14)                /* PVP0 CNVn Coefficient 1,4 */
#define pREG_PVP0_CNV3_C14               ((volatile uint32_t *)REG_PVP0_CNV3_C14)                /* PVP0 CNVn Coefficient 1,4 */
#define pREG_PVP0_CNV0_C20C21            ((volatile uint32_t *)REG_PVP0_CNV0_C20C21)             /* PVP0 CNVn Coefficients 2,0 and 2,1 */
#define pREG_PVP0_CNV1_C20C21            ((volatile uint32_t *)REG_PVP0_CNV1_C20C21)             /* PVP0 CNVn Coefficients 2,0 and 2,1 */
#define pREG_PVP0_CNV2_C20C21            ((volatile uint32_t *)REG_PVP0_CNV2_C20C21)             /* PVP0 CNVn Coefficients 2,0 and 2,1 */
#define pREG_PVP0_CNV3_C20C21            ((volatile uint32_t *)REG_PVP0_CNV3_C20C21)             /* PVP0 CNVn Coefficients 2,0 and 2,1 */
#define pREG_PVP0_CNV0_C22C23            ((volatile uint32_t *)REG_PVP0_CNV0_C22C23)             /* PVP0 CNVn Coefficients 2,2 and 2,3 */
#define pREG_PVP0_CNV1_C22C23            ((volatile uint32_t *)REG_PVP0_CNV1_C22C23)             /* PVP0 CNVn Coefficients 2,2 and 2,3 */
#define pREG_PVP0_CNV2_C22C23            ((volatile uint32_t *)REG_PVP0_CNV2_C22C23)             /* PVP0 CNVn Coefficients 2,2 and 2,3 */
#define pREG_PVP0_CNV3_C22C23            ((volatile uint32_t *)REG_PVP0_CNV3_C22C23)             /* PVP0 CNVn Coefficients 2,2 and 2,3 */
#define pREG_PVP0_CNV0_C24               ((volatile uint32_t *)REG_PVP0_CNV0_C24)                /* PVP0 CNVn Coefficient 2,4 */
#define pREG_PVP0_CNV1_C24               ((volatile uint32_t *)REG_PVP0_CNV1_C24)                /* PVP0 CNVn Coefficient 2,4 */
#define pREG_PVP0_CNV2_C24               ((volatile uint32_t *)REG_PVP0_CNV2_C24)                /* PVP0 CNVn Coefficient 2,4 */
#define pREG_PVP0_CNV3_C24               ((volatile uint32_t *)REG_PVP0_CNV3_C24)                /* PVP0 CNVn Coefficient 2,4 */
#define pREG_PVP0_CNV0_C30C31            ((volatile uint32_t *)REG_PVP0_CNV0_C30C31)             /* PVP0 CNVn Coefficients 3,0 and 3,1 */
#define pREG_PVP0_CNV1_C30C31            ((volatile uint32_t *)REG_PVP0_CNV1_C30C31)             /* PVP0 CNVn Coefficients 3,0 and 3,1 */
#define pREG_PVP0_CNV2_C30C31            ((volatile uint32_t *)REG_PVP0_CNV2_C30C31)             /* PVP0 CNVn Coefficients 3,0 and 3,1 */
#define pREG_PVP0_CNV3_C30C31            ((volatile uint32_t *)REG_PVP0_CNV3_C30C31)             /* PVP0 CNVn Coefficients 3,0 and 3,1 */
#define pREG_PVP0_CNV0_C32C33            ((volatile uint32_t *)REG_PVP0_CNV0_C32C33)             /* PVP0 CNVn Coefficients 3,2 and 3,3 */
#define pREG_PVP0_CNV1_C32C33            ((volatile uint32_t *)REG_PVP0_CNV1_C32C33)             /* PVP0 CNVn Coefficients 3,2 and 3,3 */
#define pREG_PVP0_CNV2_C32C33            ((volatile uint32_t *)REG_PVP0_CNV2_C32C33)             /* PVP0 CNVn Coefficients 3,2 and 3,3 */
#define pREG_PVP0_CNV3_C32C33            ((volatile uint32_t *)REG_PVP0_CNV3_C32C33)             /* PVP0 CNVn Coefficients 3,2 and 3,3 */
#define pREG_PVP0_CNV0_C34               ((volatile uint32_t *)REG_PVP0_CNV0_C34)                /* PVP0 CNVn Coefficient 3,4 */
#define pREG_PVP0_CNV1_C34               ((volatile uint32_t *)REG_PVP0_CNV1_C34)                /* PVP0 CNVn Coefficient 3,4 */
#define pREG_PVP0_CNV2_C34               ((volatile uint32_t *)REG_PVP0_CNV2_C34)                /* PVP0 CNVn Coefficient 3,4 */
#define pREG_PVP0_CNV3_C34               ((volatile uint32_t *)REG_PVP0_CNV3_C34)                /* PVP0 CNVn Coefficient 3,4 */
#define pREG_PVP0_CNV0_C40C41            ((volatile uint32_t *)REG_PVP0_CNV0_C40C41)             /* PVP0 CNVn Coefficients 4,0 and 4,1 */
#define pREG_PVP0_CNV1_C40C41            ((volatile uint32_t *)REG_PVP0_CNV1_C40C41)             /* PVP0 CNVn Coefficients 4,0 and 4,1 */
#define pREG_PVP0_CNV2_C40C41            ((volatile uint32_t *)REG_PVP0_CNV2_C40C41)             /* PVP0 CNVn Coefficients 4,0 and 4,1 */
#define pREG_PVP0_CNV3_C40C41            ((volatile uint32_t *)REG_PVP0_CNV3_C40C41)             /* PVP0 CNVn Coefficients 4,0 and 4,1 */
#define pREG_PVP0_CNV0_C42C43            ((volatile uint32_t *)REG_PVP0_CNV0_C42C43)             /* PVP0 CNVn Coefficients 4,2 and 4,3 */
#define pREG_PVP0_CNV1_C42C43            ((volatile uint32_t *)REG_PVP0_CNV1_C42C43)             /* PVP0 CNVn Coefficients 4,2 and 4,3 */
#define pREG_PVP0_CNV2_C42C43            ((volatile uint32_t *)REG_PVP0_CNV2_C42C43)             /* PVP0 CNVn Coefficients 4,2 and 4,3 */
#define pREG_PVP0_CNV3_C42C43            ((volatile uint32_t *)REG_PVP0_CNV3_C42C43)             /* PVP0 CNVn Coefficients 4,2 and 4,3 */
#define pREG_PVP0_CNV0_C44               ((volatile uint32_t *)REG_PVP0_CNV0_C44)                /* PVP0 CNVn Coefficient 4,4 */
#define pREG_PVP0_CNV1_C44               ((volatile uint32_t *)REG_PVP0_CNV1_C44)                /* PVP0 CNVn Coefficient 4,4 */
#define pREG_PVP0_CNV2_C44               ((volatile uint32_t *)REG_PVP0_CNV2_C44)                /* PVP0 CNVn Coefficient 4,4 */
#define pREG_PVP0_CNV3_C44               ((volatile uint32_t *)REG_PVP0_CNV3_C44)                /* PVP0 CNVn Coefficient 4,4 */
#define pREG_PVP0_CNV0_SCALE             ((volatile uint32_t *)REG_PVP0_CNV0_SCALE)              /* PVP0 CNVn Scaling Factor */
#define pREG_PVP0_CNV1_SCALE             ((volatile uint32_t *)REG_PVP0_CNV1_SCALE)              /* PVP0 CNVn Scaling Factor */
#define pREG_PVP0_CNV2_SCALE             ((volatile uint32_t *)REG_PVP0_CNV2_SCALE)              /* PVP0 CNVn Scaling Factor */
#define pREG_PVP0_CNV3_SCALE             ((volatile uint32_t *)REG_PVP0_CNV3_SCALE)              /* PVP0 CNVn Scaling Factor */
#define pREG_PVP0_THC0_CFG               ((volatile uint32_t *)REG_PVP0_THC0_CFG)                /* PVP0 THCn Configuration */
#define pREG_PVP0_THC1_CFG               ((volatile uint32_t *)REG_PVP0_THC1_CFG)                /* PVP0 THCn Configuration */
#define pREG_PVP0_THC0_CTL               ((volatile uint32_t *)REG_PVP0_THC0_CTL)                /* PVP0 THCn Control */
#define pREG_PVP0_THC1_CTL               ((volatile uint32_t *)REG_PVP0_THC1_CTL)                /* PVP0 THCn Control */
#define pREG_PVP0_THC0_HFCNT             ((volatile uint32_t *)REG_PVP0_THC0_HFCNT)              /* PVP0 THCn Histogram Frame Count */
#define pREG_PVP0_THC1_HFCNT             ((volatile uint32_t *)REG_PVP0_THC1_HFCNT)              /* PVP0 THCn Histogram Frame Count */
#define pREG_PVP0_THC0_RMAXREP           ((volatile uint32_t *)REG_PVP0_THC0_RMAXREP)            /* PVP0 THCn Max RLE Reports */
#define pREG_PVP0_THC1_RMAXREP           ((volatile uint32_t *)REG_PVP0_THC1_RMAXREP)            /* PVP0 THCn Max RLE Reports */
#define pREG_PVP0_THC0_CMINVAL           ((volatile  int32_t *)REG_PVP0_THC0_CMINVAL)            /* PVP0 THCn Min Clip Value */
#define pREG_PVP0_THC1_CMINVAL           ((volatile  int32_t *)REG_PVP0_THC1_CMINVAL)            /* PVP0 THCn Min Clip Value */
#define pREG_PVP0_THC0_CMINTH            ((volatile  int32_t *)REG_PVP0_THC0_CMINTH)             /* PVP0 THCn Clip Min Threshold */
#define pREG_PVP0_THC1_CMINTH            ((volatile  int32_t *)REG_PVP0_THC1_CMINTH)             /* PVP0 THCn Clip Min Threshold */
#define pREG_PVP0_THC0_CMAXTH            ((volatile  int32_t *)REG_PVP0_THC0_CMAXTH)             /* PVP0 THCn Clip Max Threshold */
#define pREG_PVP0_THC1_CMAXTH            ((volatile  int32_t *)REG_PVP0_THC1_CMAXTH)             /* PVP0 THCn Clip Max Threshold */
#define pREG_PVP0_THC0_CMAXVAL           ((volatile  int32_t *)REG_PVP0_THC0_CMAXVAL)            /* PVP0 THCn Max Clip Value */
#define pREG_PVP0_THC1_CMAXVAL           ((volatile  int32_t *)REG_PVP0_THC1_CMAXVAL)            /* PVP0 THCn Max Clip Value */
#define pREG_PVP0_THC0_TH0               ((volatile  int32_t *)REG_PVP0_THC0_TH0)                /* PVP0 THCn Threshold Value 0 */
#define pREG_PVP0_THC1_TH0               ((volatile  int32_t *)REG_PVP0_THC1_TH0)                /* PVP0 THCn Threshold Value 0 */
#define pREG_PVP0_THC0_TH1               ((volatile  int32_t *)REG_PVP0_THC0_TH1)                /* PVP0 THCn Threshold Value 1 */
#define pREG_PVP0_THC1_TH1               ((volatile  int32_t *)REG_PVP0_THC1_TH1)                /* PVP0 THCn Threshold Value 1 */
#define pREG_PVP0_THC0_TH2               ((volatile  int32_t *)REG_PVP0_THC0_TH2)                /* PVP0 THCn Threshold Value 2 */
#define pREG_PVP0_THC1_TH2               ((volatile  int32_t *)REG_PVP0_THC1_TH2)                /* PVP0 THCn Threshold Value 2 */
#define pREG_PVP0_THC0_TH3               ((volatile  int32_t *)REG_PVP0_THC0_TH3)                /* PVP0 THCn Threshold Value 3 */
#define pREG_PVP0_THC1_TH3               ((volatile  int32_t *)REG_PVP0_THC1_TH3)                /* PVP0 THCn Threshold Value 3 */
#define pREG_PVP0_THC0_TH4               ((volatile  int32_t *)REG_PVP0_THC0_TH4)                /* PVP0 THCn Threshold Value 4 */
#define pREG_PVP0_THC1_TH4               ((volatile  int32_t *)REG_PVP0_THC1_TH4)                /* PVP0 THCn Threshold Value 4 */
#define pREG_PVP0_THC0_TH5               ((volatile  int32_t *)REG_PVP0_THC0_TH5)                /* PVP0 THCn Threshold Value 5 */
#define pREG_PVP0_THC1_TH5               ((volatile  int32_t *)REG_PVP0_THC1_TH5)                /* PVP0 THCn Threshold Value 5 */
#define pREG_PVP0_THC0_TH6               ((volatile  int32_t *)REG_PVP0_THC0_TH6)                /* PVP0 THCn Threshold Value 6 */
#define pREG_PVP0_THC1_TH6               ((volatile  int32_t *)REG_PVP0_THC1_TH6)                /* PVP0 THCn Threshold Value 6 */
#define pREG_PVP0_THC0_TH7               ((volatile  int32_t *)REG_PVP0_THC0_TH7)                /* PVP0 THCn Threshold Value 7 */
#define pREG_PVP0_THC1_TH7               ((volatile  int32_t *)REG_PVP0_THC1_TH7)                /* PVP0 THCn Threshold Value 7 */
#define pREG_PVP0_THC0_TH8               ((volatile  int32_t *)REG_PVP0_THC0_TH8)                /* PVP0 THCn Threshold Value 8 */
#define pREG_PVP0_THC1_TH8               ((volatile  int32_t *)REG_PVP0_THC1_TH8)                /* PVP0 THCn Threshold Value 8 */
#define pREG_PVP0_THC0_TH9               ((volatile  int32_t *)REG_PVP0_THC0_TH9)                /* PVP0 THCn Threshold Value 9 */
#define pREG_PVP0_THC1_TH9               ((volatile  int32_t *)REG_PVP0_THC1_TH9)                /* PVP0 THCn Threshold Value 9 */
#define pREG_PVP0_THC0_TH10              ((volatile  int32_t *)REG_PVP0_THC0_TH10)               /* PVP0 THCn Threshold Value 10 */
#define pREG_PVP0_THC1_TH10              ((volatile  int32_t *)REG_PVP0_THC1_TH10)               /* PVP0 THCn Threshold Value 10 */
#define pREG_PVP0_THC0_TH11              ((volatile  int32_t *)REG_PVP0_THC0_TH11)               /* PVP0 THCn Threshold Value 11 */
#define pREG_PVP0_THC1_TH11              ((volatile  int32_t *)REG_PVP0_THC1_TH11)               /* PVP0 THCn Threshold Value 11 */
#define pREG_PVP0_THC0_TH12              ((volatile  int32_t *)REG_PVP0_THC0_TH12)               /* PVP0 THCn Threshold Value 12 */
#define pREG_PVP0_THC1_TH12              ((volatile  int32_t *)REG_PVP0_THC1_TH12)               /* PVP0 THCn Threshold Value 12 */
#define pREG_PVP0_THC0_TH13              ((volatile  int32_t *)REG_PVP0_THC0_TH13)               /* PVP0 THCn Threshold Value 13 */
#define pREG_PVP0_THC1_TH13              ((volatile  int32_t *)REG_PVP0_THC1_TH13)               /* PVP0 THCn Threshold Value 13 */
#define pREG_PVP0_THC0_TH14              ((volatile  int32_t *)REG_PVP0_THC0_TH14)               /* PVP0 THCn Threshold Value 14 */
#define pREG_PVP0_THC1_TH14              ((volatile  int32_t *)REG_PVP0_THC1_TH14)               /* PVP0 THCn Threshold Value 14 */
#define pREG_PVP0_THC0_TH15              ((volatile  int32_t *)REG_PVP0_THC0_TH15)               /* PVP0 THCn Threshold Value 15 */
#define pREG_PVP0_THC1_TH15              ((volatile  int32_t *)REG_PVP0_THC1_TH15)               /* PVP0 THCn Threshold Value 15 */
#define pREG_PVP0_THC0_HHPOS             ((volatile uint32_t *)REG_PVP0_THC0_HHPOS)              /* PVP0 THCn Histogram Horzontal Position */
#define pREG_PVP0_THC1_HHPOS             ((volatile uint32_t *)REG_PVP0_THC1_HHPOS)              /* PVP0 THCn Histogram Horzontal Position */
#define pREG_PVP0_THC0_HVPOS             ((volatile uint32_t *)REG_PVP0_THC0_HVPOS)              /* PVP0 THCn Histogram Vertical Position */
#define pREG_PVP0_THC1_HVPOS             ((volatile uint32_t *)REG_PVP0_THC1_HVPOS)              /* PVP0 THCn Histogram Vertical Position */
#define pREG_PVP0_THC0_HHCNT             ((volatile uint32_t *)REG_PVP0_THC0_HHCNT)              /* PVP0 THCn Histogram Horizontal Count */
#define pREG_PVP0_THC1_HHCNT             ((volatile uint32_t *)REG_PVP0_THC1_HHCNT)              /* PVP0 THCn Histogram Horizontal Count */
#define pREG_PVP0_THC0_HVCNT             ((volatile uint32_t *)REG_PVP0_THC0_HVCNT)              /* PVP0 THCn Histogram Vertical Count */
#define pREG_PVP0_THC1_HVCNT             ((volatile uint32_t *)REG_PVP0_THC1_HVCNT)              /* PVP0 THCn Histogram Vertical Count */
#define pREG_PVP0_THC0_RHPOS             ((volatile uint32_t *)REG_PVP0_THC0_RHPOS)              /* PVP0 THCn RLE Horizontal Position */
#define pREG_PVP0_THC1_RHPOS             ((volatile uint32_t *)REG_PVP0_THC1_RHPOS)              /* PVP0 THCn RLE Horizontal Position */
#define pREG_PVP0_THC0_RVPOS             ((volatile uint32_t *)REG_PVP0_THC0_RVPOS)              /* PVP0 THCn RLE Vertical Position */
#define pREG_PVP0_THC1_RVPOS             ((volatile uint32_t *)REG_PVP0_THC1_RVPOS)              /* PVP0 THCn RLE Vertical Position */
#define pREG_PVP0_THC0_RHCNT             ((volatile uint32_t *)REG_PVP0_THC0_RHCNT)              /* PVP0 THCn RLE Horizontal Count */
#define pREG_PVP0_THC1_RHCNT             ((volatile uint32_t *)REG_PVP0_THC1_RHCNT)              /* PVP0 THCn RLE Horizontal Count */
#define pREG_PVP0_THC0_RVCNT             ((volatile uint32_t *)REG_PVP0_THC0_RVCNT)              /* PVP0 THCn RLE Vertical Count */
#define pREG_PVP0_THC1_RVCNT             ((volatile uint32_t *)REG_PVP0_THC1_RVCNT)              /* PVP0 THCn RLE Vertical Count */
#define pREG_PVP0_THC0_HFCNT_STAT        ((volatile uint32_t *)REG_PVP0_THC0_HFCNT_STAT)         /* PVP0 THCn Histogram Frame Count Status */
#define pREG_PVP0_THC1_HFCNT_STAT        ((volatile uint32_t *)REG_PVP0_THC1_HFCNT_STAT)         /* PVP0 THCn Histogram Frame Count Status */
#define pREG_PVP0_THC0_HCNT0_STAT        ((volatile uint32_t *)REG_PVP0_THC0_HCNT0_STAT)         /* PVP0 THCn Histogram Counter Value 0 */
#define pREG_PVP0_THC1_HCNT0_STAT        ((volatile uint32_t *)REG_PVP0_THC1_HCNT0_STAT)         /* PVP0 THCn Histogram Counter Value 0 */
#define pREG_PVP0_THC0_HCNT1_STAT        ((volatile uint32_t *)REG_PVP0_THC0_HCNT1_STAT)         /* PVP0 THCn Histogram Counter Value 1 */
#define pREG_PVP0_THC1_HCNT1_STAT        ((volatile uint32_t *)REG_PVP0_THC1_HCNT1_STAT)         /* PVP0 THCn Histogram Counter Value 1 */
#define pREG_PVP0_THC0_HCNT2_STAT        ((volatile uint32_t *)REG_PVP0_THC0_HCNT2_STAT)         /* PVP0 THCn Histogram Counter Value 2 */
#define pREG_PVP0_THC1_HCNT2_STAT        ((volatile uint32_t *)REG_PVP0_THC1_HCNT2_STAT)         /* PVP0 THCn Histogram Counter Value 2 */
#define pREG_PVP0_THC0_HCNT3_STAT        ((volatile uint32_t *)REG_PVP0_THC0_HCNT3_STAT)         /* PVP0 THCn Histogram Counter Value 3 */
#define pREG_PVP0_THC1_HCNT3_STAT        ((volatile uint32_t *)REG_PVP0_THC1_HCNT3_STAT)         /* PVP0 THCn Histogram Counter Value 3 */
#define pREG_PVP0_THC0_HCNT4_STAT        ((volatile uint32_t *)REG_PVP0_THC0_HCNT4_STAT)         /* PVP0 THCn Histogram Counter Value 4 */
#define pREG_PVP0_THC1_HCNT4_STAT        ((volatile uint32_t *)REG_PVP0_THC1_HCNT4_STAT)         /* PVP0 THCn Histogram Counter Value 4 */
#define pREG_PVP0_THC0_HCNT5_STAT        ((volatile uint32_t *)REG_PVP0_THC0_HCNT5_STAT)         /* PVP0 THCn Histogram Counter Value 5 */
#define pREG_PVP0_THC1_HCNT5_STAT        ((volatile uint32_t *)REG_PVP0_THC1_HCNT5_STAT)         /* PVP0 THCn Histogram Counter Value 5 */
#define pREG_PVP0_THC0_HCNT6_STAT        ((volatile uint32_t *)REG_PVP0_THC0_HCNT6_STAT)         /* PVP0 THCn Histogram Counter Value 6 */
#define pREG_PVP0_THC1_HCNT6_STAT        ((volatile uint32_t *)REG_PVP0_THC1_HCNT6_STAT)         /* PVP0 THCn Histogram Counter Value 6 */
#define pREG_PVP0_THC0_HCNT7_STAT        ((volatile uint32_t *)REG_PVP0_THC0_HCNT7_STAT)         /* PVP0 THCn Histogram Counter Value 7 */
#define pREG_PVP0_THC1_HCNT7_STAT        ((volatile uint32_t *)REG_PVP0_THC1_HCNT7_STAT)         /* PVP0 THCn Histogram Counter Value 7 */
#define pREG_PVP0_THC0_HCNT8_STAT        ((volatile uint32_t *)REG_PVP0_THC0_HCNT8_STAT)         /* PVP0 THCn Histogram Counter Value 8 */
#define pREG_PVP0_THC1_HCNT8_STAT        ((volatile uint32_t *)REG_PVP0_THC1_HCNT8_STAT)         /* PVP0 THCn Histogram Counter Value 8 */
#define pREG_PVP0_THC0_HCNT9_STAT        ((volatile uint32_t *)REG_PVP0_THC0_HCNT9_STAT)         /* PVP0 THCn Histogram Counter Value 9 */
#define pREG_PVP0_THC1_HCNT9_STAT        ((volatile uint32_t *)REG_PVP0_THC1_HCNT9_STAT)         /* PVP0 THCn Histogram Counter Value 9 */
#define pREG_PVP0_THC0_HCNT10_STAT       ((volatile uint32_t *)REG_PVP0_THC0_HCNT10_STAT)        /* PVP0 THCn Histogram Counter Value 10 */
#define pREG_PVP0_THC1_HCNT10_STAT       ((volatile uint32_t *)REG_PVP0_THC1_HCNT10_STAT)        /* PVP0 THCn Histogram Counter Value 10 */
#define pREG_PVP0_THC0_HCNT11_STAT       ((volatile uint32_t *)REG_PVP0_THC0_HCNT11_STAT)        /* PVP0 THCn Histogram Counter Value 11 */
#define pREG_PVP0_THC1_HCNT11_STAT       ((volatile uint32_t *)REG_PVP0_THC1_HCNT11_STAT)        /* PVP0 THCn Histogram Counter Value 11 */
#define pREG_PVP0_THC0_HCNT12_STAT       ((volatile uint32_t *)REG_PVP0_THC0_HCNT12_STAT)        /* PVP0 THCn Histogram Counter Value 12 */
#define pREG_PVP0_THC1_HCNT12_STAT       ((volatile uint32_t *)REG_PVP0_THC1_HCNT12_STAT)        /* PVP0 THCn Histogram Counter Value 12 */
#define pREG_PVP0_THC0_HCNT13_STAT       ((volatile uint32_t *)REG_PVP0_THC0_HCNT13_STAT)        /* PVP0 THCn Histogram Counter Value 13 */
#define pREG_PVP0_THC1_HCNT13_STAT       ((volatile uint32_t *)REG_PVP0_THC1_HCNT13_STAT)        /* PVP0 THCn Histogram Counter Value 13 */
#define pREG_PVP0_THC0_HCNT14_STAT       ((volatile uint32_t *)REG_PVP0_THC0_HCNT14_STAT)        /* PVP0 THCn Histogram Counter Value 14 */
#define pREG_PVP0_THC1_HCNT14_STAT       ((volatile uint32_t *)REG_PVP0_THC1_HCNT14_STAT)        /* PVP0 THCn Histogram Counter Value 14 */
#define pREG_PVP0_THC0_HCNT15_STAT       ((volatile uint32_t *)REG_PVP0_THC0_HCNT15_STAT)        /* PVP0 THCn Histogram Counter Value 15 */
#define pREG_PVP0_THC1_HCNT15_STAT       ((volatile uint32_t *)REG_PVP0_THC1_HCNT15_STAT)        /* PVP0 THCn Histogram Counter Value 15 */
#define pREG_PVP0_THC0_RREP_STAT         ((volatile uint32_t *)REG_PVP0_THC0_RREP_STAT)          /* PVP0 THCn Number of RLE Reports */
#define pREG_PVP0_THC1_RREP_STAT         ((volatile uint32_t *)REG_PVP0_THC1_RREP_STAT)          /* PVP0 THCn Number of RLE Reports */
#define pREG_PVP0_PMA_CFG                ((volatile uint32_t *)REG_PVP0_PMA_CFG)                 /* PVP0 PMA Configuration */


/* =========================================================================
       PWM0
   ========================================================================= */
#define pREG_PWM0_CTL                    ((volatile uint32_t *)REG_PWM0_CTL)                     /* PWM0 Control Register */
#define pREG_PWM0_CHANCFG                ((volatile uint32_t *)REG_PWM0_CHANCFG)                 /* PWM0 Channel Config Register */
#define pREG_PWM0_TRIPCFG                ((volatile uint32_t *)REG_PWM0_TRIPCFG)                 /* PWM0 Trip Config Register */
#define pREG_PWM0_STAT                   ((volatile uint32_t *)REG_PWM0_STAT)                    /* PWM0 Status Register */
#define pREG_PWM0_IMSK                   ((volatile uint32_t *)REG_PWM0_IMSK)                    /* PWM0 Interrupt Mask Register */
#define pREG_PWM0_ILAT                   ((volatile uint32_t *)REG_PWM0_ILAT)                    /* PWM0 Interrupt Latch Register */
#define pREG_PWM0_CHOPCFG                ((volatile uint32_t *)REG_PWM0_CHOPCFG)                 /* PWM0 Chop Configuration Register */
#define pREG_PWM0_DT                     ((volatile uint32_t *)REG_PWM0_DT)                      /* PWM0 Dead Time Register */
#define pREG_PWM0_SYNC_WID               ((volatile uint32_t *)REG_PWM0_SYNC_WID)                /* PWM0 Sync Pulse Width Register */
#define pREG_PWM0_TM0                    ((volatile uint32_t *)REG_PWM0_TM0)                     /* PWM0 Timer 0 Period Register */
#define pREG_PWM0_TM1                    ((volatile uint32_t *)REG_PWM0_TM1)                     /* PWM0 Timer 1 Period Register */
#define pREG_PWM0_TM2                    ((volatile uint32_t *)REG_PWM0_TM2)                     /* PWM0 Timer 2 Period Register */
#define pREG_PWM0_TM3                    ((volatile uint32_t *)REG_PWM0_TM3)                     /* PWM0 Timer 3 Period Register */
#define pREG_PWM0_TM4                    ((volatile uint32_t *)REG_PWM0_TM4)                     /* PWM0 Timer 4 Period Register */
#define pREG_PWM0_DLYA                   ((volatile uint32_t *)REG_PWM0_DLYA)                    /* PWM0 Channel A Delay Register */
#define pREG_PWM0_DLYB                   ((volatile uint32_t *)REG_PWM0_DLYB)                    /* PWM0 Channel B Delay Register */
#define pREG_PWM0_DLYC                   ((volatile uint32_t *)REG_PWM0_DLYC)                    /* PWM0 Channel C Delay Register */
#define pREG_PWM0_DLYD                   ((volatile uint32_t *)REG_PWM0_DLYD)                    /* PWM0 Channel D Delay Register */
#define pREG_PWM0_ACTL                   ((volatile uint32_t *)REG_PWM0_ACTL)                    /* PWM0 Channel A Control Register */
#define pREG_PWM0_AH0                    ((volatile uint32_t *)REG_PWM0_AH0)                     /* PWM0 Channel A-High Duty-0 Register */
#define pREG_PWM0_AH1                    ((volatile uint32_t *)REG_PWM0_AH1)                     /* PWM0 Channel A-High Duty-1 Register */
#define pREG_PWM0_AL0                    ((volatile uint32_t *)REG_PWM0_AL0)                     /* PWM0 Channel A-Low Duty-0 Register */
#define pREG_PWM0_AL1                    ((volatile uint32_t *)REG_PWM0_AL1)                     /* PWM0 Channel A-Low Duty-1 Register */
#define pREG_PWM0_BCTL                   ((volatile uint32_t *)REG_PWM0_BCTL)                    /* PWM0 Channel B Control Register */
#define pREG_PWM0_BH0                    ((volatile uint32_t *)REG_PWM0_BH0)                     /* PWM0 Channel B-High Duty-0 Register */
#define pREG_PWM0_BH1                    ((volatile uint32_t *)REG_PWM0_BH1)                     /* PWM0 Channel B-High Duty-1 Register */
#define pREG_PWM0_BL0                    ((volatile uint32_t *)REG_PWM0_BL0)                     /* PWM0 Channel B-Low Duty-0 Register */
#define pREG_PWM0_BL1                    ((volatile uint32_t *)REG_PWM0_BL1)                     /* PWM0 Channel B-Low Duty-1 Register */
#define pREG_PWM0_CCTL                   ((volatile uint32_t *)REG_PWM0_CCTL)                    /* PWM0 Channel C Control Register */
#define pREG_PWM0_CH0                    ((volatile uint32_t *)REG_PWM0_CH0)                     /* PWM0 Channel C-High Pulse Duty Register 0 */
#define pREG_PWM0_CH1                    ((volatile uint32_t *)REG_PWM0_CH1)                     /* PWM0 Channel C-High Pulse Duty Register 1 */
#define pREG_PWM0_CL0                    ((volatile uint32_t *)REG_PWM0_CL0)                     /* PWM0 Channel C-Low Pulse Duty Register 0 */
#define pREG_PWM0_CL1                    ((volatile uint32_t *)REG_PWM0_CL1)                     /* PWM0 Channel C-Low Duty-1 Register */
#define pREG_PWM0_DCTL                   ((volatile uint32_t *)REG_PWM0_DCTL)                    /* PWM0 Channel D Control Register */
#define pREG_PWM0_DH0                    ((volatile uint32_t *)REG_PWM0_DH0)                     /* PWM0 Channel D-High Duty-0 Register */
#define pREG_PWM0_DH1                    ((volatile uint32_t *)REG_PWM0_DH1)                     /* PWM0 Channel D-High Pulse Duty Register 1 */
#define pREG_PWM0_DL0                    ((volatile uint32_t *)REG_PWM0_DL0)                     /* PWM0 Channel D-Low Pulse Duty Register 0 */
#define pREG_PWM0_DL1                    ((volatile uint32_t *)REG_PWM0_DL1)                     /* PWM0 Channel D-Low Pulse Duty Register 1 */

/* =========================================================================
       PWM1
   ========================================================================= */
#define pREG_PWM1_CTL                    ((volatile uint32_t *)REG_PWM1_CTL)                     /* PWM1 Control Register */
#define pREG_PWM1_CHANCFG                ((volatile uint32_t *)REG_PWM1_CHANCFG)                 /* PWM1 Channel Config Register */
#define pREG_PWM1_TRIPCFG                ((volatile uint32_t *)REG_PWM1_TRIPCFG)                 /* PWM1 Trip Config Register */
#define pREG_PWM1_STAT                   ((volatile uint32_t *)REG_PWM1_STAT)                    /* PWM1 Status Register */
#define pREG_PWM1_IMSK                   ((volatile uint32_t *)REG_PWM1_IMSK)                    /* PWM1 Interrupt Mask Register */
#define pREG_PWM1_ILAT                   ((volatile uint32_t *)REG_PWM1_ILAT)                    /* PWM1 Interrupt Latch Register */
#define pREG_PWM1_CHOPCFG                ((volatile uint32_t *)REG_PWM1_CHOPCFG)                 /* PWM1 Chop Configuration Register */
#define pREG_PWM1_DT                     ((volatile uint32_t *)REG_PWM1_DT)                      /* PWM1 Dead Time Register */
#define pREG_PWM1_SYNC_WID               ((volatile uint32_t *)REG_PWM1_SYNC_WID)                /* PWM1 Sync Pulse Width Register */
#define pREG_PWM1_TM0                    ((volatile uint32_t *)REG_PWM1_TM0)                     /* PWM1 Timer 0 Period Register */
#define pREG_PWM1_TM1                    ((volatile uint32_t *)REG_PWM1_TM1)                     /* PWM1 Timer 1 Period Register */
#define pREG_PWM1_TM2                    ((volatile uint32_t *)REG_PWM1_TM2)                     /* PWM1 Timer 2 Period Register */
#define pREG_PWM1_TM3                    ((volatile uint32_t *)REG_PWM1_TM3)                     /* PWM1 Timer 3 Period Register */
#define pREG_PWM1_TM4                    ((volatile uint32_t *)REG_PWM1_TM4)                     /* PWM1 Timer 4 Period Register */
#define pREG_PWM1_DLYA                   ((volatile uint32_t *)REG_PWM1_DLYA)                    /* PWM1 Channel A Delay Register */
#define pREG_PWM1_DLYB                   ((volatile uint32_t *)REG_PWM1_DLYB)                    /* PWM1 Channel B Delay Register */
#define pREG_PWM1_DLYC                   ((volatile uint32_t *)REG_PWM1_DLYC)                    /* PWM1 Channel C Delay Register */
#define pREG_PWM1_DLYD                   ((volatile uint32_t *)REG_PWM1_DLYD)                    /* PWM1 Channel D Delay Register */
#define pREG_PWM1_ACTL                   ((volatile uint32_t *)REG_PWM1_ACTL)                    /* PWM1 Channel A Control Register */
#define pREG_PWM1_AH0                    ((volatile uint32_t *)REG_PWM1_AH0)                     /* PWM1 Channel A-High Duty-0 Register */
#define pREG_PWM1_AH1                    ((volatile uint32_t *)REG_PWM1_AH1)                     /* PWM1 Channel A-High Duty-1 Register */
#define pREG_PWM1_AL0                    ((volatile uint32_t *)REG_PWM1_AL0)                     /* PWM1 Channel A-Low Duty-0 Register */
#define pREG_PWM1_AL1                    ((volatile uint32_t *)REG_PWM1_AL1)                     /* PWM1 Channel A-Low Duty-1 Register */
#define pREG_PWM1_BCTL                   ((volatile uint32_t *)REG_PWM1_BCTL)                    /* PWM1 Channel B Control Register */
#define pREG_PWM1_BH0                    ((volatile uint32_t *)REG_PWM1_BH0)                     /* PWM1 Channel B-High Duty-0 Register */
#define pREG_PWM1_BH1                    ((volatile uint32_t *)REG_PWM1_BH1)                     /* PWM1 Channel B-High Duty-1 Register */
#define pREG_PWM1_BL0                    ((volatile uint32_t *)REG_PWM1_BL0)                     /* PWM1 Channel B-Low Duty-0 Register */
#define pREG_PWM1_BL1                    ((volatile uint32_t *)REG_PWM1_BL1)                     /* PWM1 Channel B-Low Duty-1 Register */
#define pREG_PWM1_CCTL                   ((volatile uint32_t *)REG_PWM1_CCTL)                    /* PWM1 Channel C Control Register */
#define pREG_PWM1_CH0                    ((volatile uint32_t *)REG_PWM1_CH0)                     /* PWM1 Channel C-High Pulse Duty Register 0 */
#define pREG_PWM1_CH1                    ((volatile uint32_t *)REG_PWM1_CH1)                     /* PWM1 Channel C-High Pulse Duty Register 1 */
#define pREG_PWM1_CL0                    ((volatile uint32_t *)REG_PWM1_CL0)                     /* PWM1 Channel C-Low Pulse Duty Register 0 */
#define pREG_PWM1_CL1                    ((volatile uint32_t *)REG_PWM1_CL1)                     /* PWM1 Channel C-Low Duty-1 Register */
#define pREG_PWM1_DCTL                   ((volatile uint32_t *)REG_PWM1_DCTL)                    /* PWM1 Channel D Control Register */
#define pREG_PWM1_DH0                    ((volatile uint32_t *)REG_PWM1_DH0)                     /* PWM1 Channel D-High Duty-0 Register */
#define pREG_PWM1_DH1                    ((volatile uint32_t *)REG_PWM1_DH1)                     /* PWM1 Channel D-High Pulse Duty Register 1 */
#define pREG_PWM1_DL0                    ((volatile uint32_t *)REG_PWM1_DL0)                     /* PWM1 Channel D-Low Pulse Duty Register 0 */
#define pREG_PWM1_DL1                    ((volatile uint32_t *)REG_PWM1_DL1)                     /* PWM1 Channel D-Low Pulse Duty Register 1 */


/* =========================================================================
       VID0
   ========================================================================= */
#define pREG_VID0_CONN                   ((volatile uint32_t *)REG_VID0_CONN)                    /* VID0 Video Subsystem Connect Register */


/* =========================================================================
       SWU0
   ========================================================================= */
#define pREG_SWU0_GCTL                   ((volatile uint32_t *)REG_SWU0_GCTL)                    /* SWU0 Global Control Register */
#define pREG_SWU0_GSTAT                  ((volatile uint32_t *)REG_SWU0_GSTAT)                   /* SWU0 Global Status Register */
#define pREG_SWU0_CTL0                   ((volatile uint32_t *)REG_SWU0_CTL0)                    /* SWU0 Control Register n */
#define pREG_SWU0_CTL1                   ((volatile uint32_t *)REG_SWU0_CTL1)                    /* SWU0 Control Register n */
#define pREG_SWU0_CTL2                   ((volatile uint32_t *)REG_SWU0_CTL2)                    /* SWU0 Control Register n */
#define pREG_SWU0_CTL3                   ((volatile uint32_t *)REG_SWU0_CTL3)                    /* SWU0 Control Register n */
#define pREG_SWU0_LA0                    ((void * volatile *)REG_SWU0_LA0)                       /* SWU0 Lower Address Register n */
#define pREG_SWU0_LA1                    ((void * volatile *)REG_SWU0_LA1)                       /* SWU0 Lower Address Register n */
#define pREG_SWU0_LA2                    ((void * volatile *)REG_SWU0_LA2)                       /* SWU0 Lower Address Register n */
#define pREG_SWU0_LA3                    ((void * volatile *)REG_SWU0_LA3)                       /* SWU0 Lower Address Register n */
#define pREG_SWU0_UA0                    ((void * volatile *)REG_SWU0_UA0)                       /* SWU0 Upper Address Register n */
#define pREG_SWU0_UA1                    ((void * volatile *)REG_SWU0_UA1)                       /* SWU0 Upper Address Register n */
#define pREG_SWU0_UA2                    ((void * volatile *)REG_SWU0_UA2)                       /* SWU0 Upper Address Register n */
#define pREG_SWU0_UA3                    ((void * volatile *)REG_SWU0_UA3)                       /* SWU0 Upper Address Register n */
#define pREG_SWU0_ID0                    ((volatile uint32_t *)REG_SWU0_ID0)                     /* SWU0 ID Register n */
#define pREG_SWU0_ID1                    ((volatile uint32_t *)REG_SWU0_ID1)                     /* SWU0 ID Register n */
#define pREG_SWU0_ID2                    ((volatile uint32_t *)REG_SWU0_ID2)                     /* SWU0 ID Register n */
#define pREG_SWU0_ID3                    ((volatile uint32_t *)REG_SWU0_ID3)                     /* SWU0 ID Register n */
#define pREG_SWU0_CNT0                   ((volatile uint32_t *)REG_SWU0_CNT0)                    /* SWU0 Count Register n */
#define pREG_SWU0_CNT1                   ((volatile uint32_t *)REG_SWU0_CNT1)                    /* SWU0 Count Register n */
#define pREG_SWU0_CNT2                   ((volatile uint32_t *)REG_SWU0_CNT2)                    /* SWU0 Count Register n */
#define pREG_SWU0_CNT3                   ((volatile uint32_t *)REG_SWU0_CNT3)                    /* SWU0 Count Register n */
#define pREG_SWU0_TARG0                  ((volatile uint32_t *)REG_SWU0_TARG0)                   /* SWU0 Target Register n */
#define pREG_SWU0_TARG1                  ((volatile uint32_t *)REG_SWU0_TARG1)                   /* SWU0 Target Register n */
#define pREG_SWU0_TARG2                  ((volatile uint32_t *)REG_SWU0_TARG2)                   /* SWU0 Target Register n */
#define pREG_SWU0_TARG3                  ((volatile uint32_t *)REG_SWU0_TARG3)                   /* SWU0 Target Register n */
#define pREG_SWU0_HIST0                  ((volatile uint32_t *)REG_SWU0_HIST0)                   /* SWU0 Bandwidth History Register n */
#define pREG_SWU0_HIST1                  ((volatile uint32_t *)REG_SWU0_HIST1)                   /* SWU0 Bandwidth History Register n */
#define pREG_SWU0_HIST2                  ((volatile uint32_t *)REG_SWU0_HIST2)                   /* SWU0 Bandwidth History Register n */
#define pREG_SWU0_HIST3                  ((volatile uint32_t *)REG_SWU0_HIST3)                   /* SWU0 Bandwidth History Register n */
#define pREG_SWU0_CUR0                   ((volatile uint32_t *)REG_SWU0_CUR0)                    /* SWU0 Current Register n */
#define pREG_SWU0_CUR1                   ((volatile uint32_t *)REG_SWU0_CUR1)                    /* SWU0 Current Register n */
#define pREG_SWU0_CUR2                   ((volatile uint32_t *)REG_SWU0_CUR2)                    /* SWU0 Current Register n */
#define pREG_SWU0_CUR3                   ((volatile uint32_t *)REG_SWU0_CUR3)                    /* SWU0 Current Register n */

/* =========================================================================
       SWU1
   ========================================================================= */
#define pREG_SWU1_GCTL                   ((volatile uint32_t *)REG_SWU1_GCTL)                    /* SWU1 Global Control Register */
#define pREG_SWU1_GSTAT                  ((volatile uint32_t *)REG_SWU1_GSTAT)                   /* SWU1 Global Status Register */
#define pREG_SWU1_CTL0                   ((volatile uint32_t *)REG_SWU1_CTL0)                    /* SWU1 Control Register n */
#define pREG_SWU1_CTL1                   ((volatile uint32_t *)REG_SWU1_CTL1)                    /* SWU1 Control Register n */
#define pREG_SWU1_CTL2                   ((volatile uint32_t *)REG_SWU1_CTL2)                    /* SWU1 Control Register n */
#define pREG_SWU1_CTL3                   ((volatile uint32_t *)REG_SWU1_CTL3)                    /* SWU1 Control Register n */
#define pREG_SWU1_LA0                    ((void * volatile *)REG_SWU1_LA0)                       /* SWU1 Lower Address Register n */
#define pREG_SWU1_LA1                    ((void * volatile *)REG_SWU1_LA1)                       /* SWU1 Lower Address Register n */
#define pREG_SWU1_LA2                    ((void * volatile *)REG_SWU1_LA2)                       /* SWU1 Lower Address Register n */
#define pREG_SWU1_LA3                    ((void * volatile *)REG_SWU1_LA3)                       /* SWU1 Lower Address Register n */
#define pREG_SWU1_UA0                    ((void * volatile *)REG_SWU1_UA0)                       /* SWU1 Upper Address Register n */
#define pREG_SWU1_UA1                    ((void * volatile *)REG_SWU1_UA1)                       /* SWU1 Upper Address Register n */
#define pREG_SWU1_UA2                    ((void * volatile *)REG_SWU1_UA2)                       /* SWU1 Upper Address Register n */
#define pREG_SWU1_UA3                    ((void * volatile *)REG_SWU1_UA3)                       /* SWU1 Upper Address Register n */
#define pREG_SWU1_ID0                    ((volatile uint32_t *)REG_SWU1_ID0)                     /* SWU1 ID Register n */
#define pREG_SWU1_ID1                    ((volatile uint32_t *)REG_SWU1_ID1)                     /* SWU1 ID Register n */
#define pREG_SWU1_ID2                    ((volatile uint32_t *)REG_SWU1_ID2)                     /* SWU1 ID Register n */
#define pREG_SWU1_ID3                    ((volatile uint32_t *)REG_SWU1_ID3)                     /* SWU1 ID Register n */
#define pREG_SWU1_CNT0                   ((volatile uint32_t *)REG_SWU1_CNT0)                    /* SWU1 Count Register n */
#define pREG_SWU1_CNT1                   ((volatile uint32_t *)REG_SWU1_CNT1)                    /* SWU1 Count Register n */
#define pREG_SWU1_CNT2                   ((volatile uint32_t *)REG_SWU1_CNT2)                    /* SWU1 Count Register n */
#define pREG_SWU1_CNT3                   ((volatile uint32_t *)REG_SWU1_CNT3)                    /* SWU1 Count Register n */
#define pREG_SWU1_TARG0                  ((volatile uint32_t *)REG_SWU1_TARG0)                   /* SWU1 Target Register n */
#define pREG_SWU1_TARG1                  ((volatile uint32_t *)REG_SWU1_TARG1)                   /* SWU1 Target Register n */
#define pREG_SWU1_TARG2                  ((volatile uint32_t *)REG_SWU1_TARG2)                   /* SWU1 Target Register n */
#define pREG_SWU1_TARG3                  ((volatile uint32_t *)REG_SWU1_TARG3)                   /* SWU1 Target Register n */
#define pREG_SWU1_HIST0                  ((volatile uint32_t *)REG_SWU1_HIST0)                   /* SWU1 Bandwidth History Register n */
#define pREG_SWU1_HIST1                  ((volatile uint32_t *)REG_SWU1_HIST1)                   /* SWU1 Bandwidth History Register n */
#define pREG_SWU1_HIST2                  ((volatile uint32_t *)REG_SWU1_HIST2)                   /* SWU1 Bandwidth History Register n */
#define pREG_SWU1_HIST3                  ((volatile uint32_t *)REG_SWU1_HIST3)                   /* SWU1 Bandwidth History Register n */
#define pREG_SWU1_CUR0                   ((volatile uint32_t *)REG_SWU1_CUR0)                    /* SWU1 Current Register n */
#define pREG_SWU1_CUR1                   ((volatile uint32_t *)REG_SWU1_CUR1)                    /* SWU1 Current Register n */
#define pREG_SWU1_CUR2                   ((volatile uint32_t *)REG_SWU1_CUR2)                    /* SWU1 Current Register n */
#define pREG_SWU1_CUR3                   ((volatile uint32_t *)REG_SWU1_CUR3)                    /* SWU1 Current Register n */

/* =========================================================================
       SWU2
   ========================================================================= */
#define pREG_SWU2_GCTL                   ((volatile uint32_t *)REG_SWU2_GCTL)                    /* SWU2 Global Control Register */
#define pREG_SWU2_GSTAT                  ((volatile uint32_t *)REG_SWU2_GSTAT)                   /* SWU2 Global Status Register */
#define pREG_SWU2_CTL0                   ((volatile uint32_t *)REG_SWU2_CTL0)                    /* SWU2 Control Register n */
#define pREG_SWU2_CTL1                   ((volatile uint32_t *)REG_SWU2_CTL1)                    /* SWU2 Control Register n */
#define pREG_SWU2_CTL2                   ((volatile uint32_t *)REG_SWU2_CTL2)                    /* SWU2 Control Register n */
#define pREG_SWU2_CTL3                   ((volatile uint32_t *)REG_SWU2_CTL3)                    /* SWU2 Control Register n */
#define pREG_SWU2_LA0                    ((void * volatile *)REG_SWU2_LA0)                       /* SWU2 Lower Address Register n */
#define pREG_SWU2_LA1                    ((void * volatile *)REG_SWU2_LA1)                       /* SWU2 Lower Address Register n */
#define pREG_SWU2_LA2                    ((void * volatile *)REG_SWU2_LA2)                       /* SWU2 Lower Address Register n */
#define pREG_SWU2_LA3                    ((void * volatile *)REG_SWU2_LA3)                       /* SWU2 Lower Address Register n */
#define pREG_SWU2_UA0                    ((void * volatile *)REG_SWU2_UA0)                       /* SWU2 Upper Address Register n */
#define pREG_SWU2_UA1                    ((void * volatile *)REG_SWU2_UA1)                       /* SWU2 Upper Address Register n */
#define pREG_SWU2_UA2                    ((void * volatile *)REG_SWU2_UA2)                       /* SWU2 Upper Address Register n */
#define pREG_SWU2_UA3                    ((void * volatile *)REG_SWU2_UA3)                       /* SWU2 Upper Address Register n */
#define pREG_SWU2_ID0                    ((volatile uint32_t *)REG_SWU2_ID0)                     /* SWU2 ID Register n */
#define pREG_SWU2_ID1                    ((volatile uint32_t *)REG_SWU2_ID1)                     /* SWU2 ID Register n */
#define pREG_SWU2_ID2                    ((volatile uint32_t *)REG_SWU2_ID2)                     /* SWU2 ID Register n */
#define pREG_SWU2_ID3                    ((volatile uint32_t *)REG_SWU2_ID3)                     /* SWU2 ID Register n */
#define pREG_SWU2_CNT0                   ((volatile uint32_t *)REG_SWU2_CNT0)                    /* SWU2 Count Register n */
#define pREG_SWU2_CNT1                   ((volatile uint32_t *)REG_SWU2_CNT1)                    /* SWU2 Count Register n */
#define pREG_SWU2_CNT2                   ((volatile uint32_t *)REG_SWU2_CNT2)                    /* SWU2 Count Register n */
#define pREG_SWU2_CNT3                   ((volatile uint32_t *)REG_SWU2_CNT3)                    /* SWU2 Count Register n */
#define pREG_SWU2_TARG0                  ((volatile uint32_t *)REG_SWU2_TARG0)                   /* SWU2 Target Register n */
#define pREG_SWU2_TARG1                  ((volatile uint32_t *)REG_SWU2_TARG1)                   /* SWU2 Target Register n */
#define pREG_SWU2_TARG2                  ((volatile uint32_t *)REG_SWU2_TARG2)                   /* SWU2 Target Register n */
#define pREG_SWU2_TARG3                  ((volatile uint32_t *)REG_SWU2_TARG3)                   /* SWU2 Target Register n */
#define pREG_SWU2_HIST0                  ((volatile uint32_t *)REG_SWU2_HIST0)                   /* SWU2 Bandwidth History Register n */
#define pREG_SWU2_HIST1                  ((volatile uint32_t *)REG_SWU2_HIST1)                   /* SWU2 Bandwidth History Register n */
#define pREG_SWU2_HIST2                  ((volatile uint32_t *)REG_SWU2_HIST2)                   /* SWU2 Bandwidth History Register n */
#define pREG_SWU2_HIST3                  ((volatile uint32_t *)REG_SWU2_HIST3)                   /* SWU2 Bandwidth History Register n */
#define pREG_SWU2_CUR0                   ((volatile uint32_t *)REG_SWU2_CUR0)                    /* SWU2 Current Register n */
#define pREG_SWU2_CUR1                   ((volatile uint32_t *)REG_SWU2_CUR1)                    /* SWU2 Current Register n */
#define pREG_SWU2_CUR2                   ((volatile uint32_t *)REG_SWU2_CUR2)                    /* SWU2 Current Register n */
#define pREG_SWU2_CUR3                   ((volatile uint32_t *)REG_SWU2_CUR3)                    /* SWU2 Current Register n */

/* =========================================================================
       SWU3
   ========================================================================= */
#define pREG_SWU3_GCTL                   ((volatile uint32_t *)REG_SWU3_GCTL)                    /* SWU3 Global Control Register */
#define pREG_SWU3_GSTAT                  ((volatile uint32_t *)REG_SWU3_GSTAT)                   /* SWU3 Global Status Register */
#define pREG_SWU3_CTL0                   ((volatile uint32_t *)REG_SWU3_CTL0)                    /* SWU3 Control Register n */
#define pREG_SWU3_CTL1                   ((volatile uint32_t *)REG_SWU3_CTL1)                    /* SWU3 Control Register n */
#define pREG_SWU3_CTL2                   ((volatile uint32_t *)REG_SWU3_CTL2)                    /* SWU3 Control Register n */
#define pREG_SWU3_CTL3                   ((volatile uint32_t *)REG_SWU3_CTL3)                    /* SWU3 Control Register n */
#define pREG_SWU3_LA0                    ((void * volatile *)REG_SWU3_LA0)                       /* SWU3 Lower Address Register n */
#define pREG_SWU3_LA1                    ((void * volatile *)REG_SWU3_LA1)                       /* SWU3 Lower Address Register n */
#define pREG_SWU3_LA2                    ((void * volatile *)REG_SWU3_LA2)                       /* SWU3 Lower Address Register n */
#define pREG_SWU3_LA3                    ((void * volatile *)REG_SWU3_LA3)                       /* SWU3 Lower Address Register n */
#define pREG_SWU3_UA0                    ((void * volatile *)REG_SWU3_UA0)                       /* SWU3 Upper Address Register n */
#define pREG_SWU3_UA1                    ((void * volatile *)REG_SWU3_UA1)                       /* SWU3 Upper Address Register n */
#define pREG_SWU3_UA2                    ((void * volatile *)REG_SWU3_UA2)                       /* SWU3 Upper Address Register n */
#define pREG_SWU3_UA3                    ((void * volatile *)REG_SWU3_UA3)                       /* SWU3 Upper Address Register n */
#define pREG_SWU3_ID0                    ((volatile uint32_t *)REG_SWU3_ID0)                     /* SWU3 ID Register n */
#define pREG_SWU3_ID1                    ((volatile uint32_t *)REG_SWU3_ID1)                     /* SWU3 ID Register n */
#define pREG_SWU3_ID2                    ((volatile uint32_t *)REG_SWU3_ID2)                     /* SWU3 ID Register n */
#define pREG_SWU3_ID3                    ((volatile uint32_t *)REG_SWU3_ID3)                     /* SWU3 ID Register n */
#define pREG_SWU3_CNT0                   ((volatile uint32_t *)REG_SWU3_CNT0)                    /* SWU3 Count Register n */
#define pREG_SWU3_CNT1                   ((volatile uint32_t *)REG_SWU3_CNT1)                    /* SWU3 Count Register n */
#define pREG_SWU3_CNT2                   ((volatile uint32_t *)REG_SWU3_CNT2)                    /* SWU3 Count Register n */
#define pREG_SWU3_CNT3                   ((volatile uint32_t *)REG_SWU3_CNT3)                    /* SWU3 Count Register n */
#define pREG_SWU3_TARG0                  ((volatile uint32_t *)REG_SWU3_TARG0)                   /* SWU3 Target Register n */
#define pREG_SWU3_TARG1                  ((volatile uint32_t *)REG_SWU3_TARG1)                   /* SWU3 Target Register n */
#define pREG_SWU3_TARG2                  ((volatile uint32_t *)REG_SWU3_TARG2)                   /* SWU3 Target Register n */
#define pREG_SWU3_TARG3                  ((volatile uint32_t *)REG_SWU3_TARG3)                   /* SWU3 Target Register n */
#define pREG_SWU3_HIST0                  ((volatile uint32_t *)REG_SWU3_HIST0)                   /* SWU3 Bandwidth History Register n */
#define pREG_SWU3_HIST1                  ((volatile uint32_t *)REG_SWU3_HIST1)                   /* SWU3 Bandwidth History Register n */
#define pREG_SWU3_HIST2                  ((volatile uint32_t *)REG_SWU3_HIST2)                   /* SWU3 Bandwidth History Register n */
#define pREG_SWU3_HIST3                  ((volatile uint32_t *)REG_SWU3_HIST3)                   /* SWU3 Bandwidth History Register n */
#define pREG_SWU3_CUR0                   ((volatile uint32_t *)REG_SWU3_CUR0)                    /* SWU3 Current Register n */
#define pREG_SWU3_CUR1                   ((volatile uint32_t *)REG_SWU3_CUR1)                    /* SWU3 Current Register n */
#define pREG_SWU3_CUR2                   ((volatile uint32_t *)REG_SWU3_CUR2)                    /* SWU3 Current Register n */
#define pREG_SWU3_CUR3                   ((volatile uint32_t *)REG_SWU3_CUR3)                    /* SWU3 Current Register n */

/* =========================================================================
       SWU4
   ========================================================================= */
#define pREG_SWU4_GCTL                   ((volatile uint32_t *)REG_SWU4_GCTL)                    /* SWU4 Global Control Register */
#define pREG_SWU4_GSTAT                  ((volatile uint32_t *)REG_SWU4_GSTAT)                   /* SWU4 Global Status Register */
#define pREG_SWU4_CTL0                   ((volatile uint32_t *)REG_SWU4_CTL0)                    /* SWU4 Control Register n */
#define pREG_SWU4_CTL1                   ((volatile uint32_t *)REG_SWU4_CTL1)                    /* SWU4 Control Register n */
#define pREG_SWU4_CTL2                   ((volatile uint32_t *)REG_SWU4_CTL2)                    /* SWU4 Control Register n */
#define pREG_SWU4_CTL3                   ((volatile uint32_t *)REG_SWU4_CTL3)                    /* SWU4 Control Register n */
#define pREG_SWU4_LA0                    ((void * volatile *)REG_SWU4_LA0)                       /* SWU4 Lower Address Register n */
#define pREG_SWU4_LA1                    ((void * volatile *)REG_SWU4_LA1)                       /* SWU4 Lower Address Register n */
#define pREG_SWU4_LA2                    ((void * volatile *)REG_SWU4_LA2)                       /* SWU4 Lower Address Register n */
#define pREG_SWU4_LA3                    ((void * volatile *)REG_SWU4_LA3)                       /* SWU4 Lower Address Register n */
#define pREG_SWU4_UA0                    ((void * volatile *)REG_SWU4_UA0)                       /* SWU4 Upper Address Register n */
#define pREG_SWU4_UA1                    ((void * volatile *)REG_SWU4_UA1)                       /* SWU4 Upper Address Register n */
#define pREG_SWU4_UA2                    ((void * volatile *)REG_SWU4_UA2)                       /* SWU4 Upper Address Register n */
#define pREG_SWU4_UA3                    ((void * volatile *)REG_SWU4_UA3)                       /* SWU4 Upper Address Register n */
#define pREG_SWU4_ID0                    ((volatile uint32_t *)REG_SWU4_ID0)                     /* SWU4 ID Register n */
#define pREG_SWU4_ID1                    ((volatile uint32_t *)REG_SWU4_ID1)                     /* SWU4 ID Register n */
#define pREG_SWU4_ID2                    ((volatile uint32_t *)REG_SWU4_ID2)                     /* SWU4 ID Register n */
#define pREG_SWU4_ID3                    ((volatile uint32_t *)REG_SWU4_ID3)                     /* SWU4 ID Register n */
#define pREG_SWU4_CNT0                   ((volatile uint32_t *)REG_SWU4_CNT0)                    /* SWU4 Count Register n */
#define pREG_SWU4_CNT1                   ((volatile uint32_t *)REG_SWU4_CNT1)                    /* SWU4 Count Register n */
#define pREG_SWU4_CNT2                   ((volatile uint32_t *)REG_SWU4_CNT2)                    /* SWU4 Count Register n */
#define pREG_SWU4_CNT3                   ((volatile uint32_t *)REG_SWU4_CNT3)                    /* SWU4 Count Register n */
#define pREG_SWU4_TARG0                  ((volatile uint32_t *)REG_SWU4_TARG0)                   /* SWU4 Target Register n */
#define pREG_SWU4_TARG1                  ((volatile uint32_t *)REG_SWU4_TARG1)                   /* SWU4 Target Register n */
#define pREG_SWU4_TARG2                  ((volatile uint32_t *)REG_SWU4_TARG2)                   /* SWU4 Target Register n */
#define pREG_SWU4_TARG3                  ((volatile uint32_t *)REG_SWU4_TARG3)                   /* SWU4 Target Register n */
#define pREG_SWU4_HIST0                  ((volatile uint32_t *)REG_SWU4_HIST0)                   /* SWU4 Bandwidth History Register n */
#define pREG_SWU4_HIST1                  ((volatile uint32_t *)REG_SWU4_HIST1)                   /* SWU4 Bandwidth History Register n */
#define pREG_SWU4_HIST2                  ((volatile uint32_t *)REG_SWU4_HIST2)                   /* SWU4 Bandwidth History Register n */
#define pREG_SWU4_HIST3                  ((volatile uint32_t *)REG_SWU4_HIST3)                   /* SWU4 Bandwidth History Register n */
#define pREG_SWU4_CUR0                   ((volatile uint32_t *)REG_SWU4_CUR0)                    /* SWU4 Current Register n */
#define pREG_SWU4_CUR1                   ((volatile uint32_t *)REG_SWU4_CUR1)                    /* SWU4 Current Register n */
#define pREG_SWU4_CUR2                   ((volatile uint32_t *)REG_SWU4_CUR2)                    /* SWU4 Current Register n */
#define pREG_SWU4_CUR3                   ((volatile uint32_t *)REG_SWU4_CUR3)                    /* SWU4 Current Register n */

/* =========================================================================
       SWU5
   ========================================================================= */
#define pREG_SWU5_GCTL                   ((volatile uint32_t *)REG_SWU5_GCTL)                    /* SWU5 Global Control Register */
#define pREG_SWU5_GSTAT                  ((volatile uint32_t *)REG_SWU5_GSTAT)                   /* SWU5 Global Status Register */
#define pREG_SWU5_CTL0                   ((volatile uint32_t *)REG_SWU5_CTL0)                    /* SWU5 Control Register n */
#define pREG_SWU5_CTL1                   ((volatile uint32_t *)REG_SWU5_CTL1)                    /* SWU5 Control Register n */
#define pREG_SWU5_CTL2                   ((volatile uint32_t *)REG_SWU5_CTL2)                    /* SWU5 Control Register n */
#define pREG_SWU5_CTL3                   ((volatile uint32_t *)REG_SWU5_CTL3)                    /* SWU5 Control Register n */
#define pREG_SWU5_LA0                    ((void * volatile *)REG_SWU5_LA0)                       /* SWU5 Lower Address Register n */
#define pREG_SWU5_LA1                    ((void * volatile *)REG_SWU5_LA1)                       /* SWU5 Lower Address Register n */
#define pREG_SWU5_LA2                    ((void * volatile *)REG_SWU5_LA2)                       /* SWU5 Lower Address Register n */
#define pREG_SWU5_LA3                    ((void * volatile *)REG_SWU5_LA3)                       /* SWU5 Lower Address Register n */
#define pREG_SWU5_UA0                    ((void * volatile *)REG_SWU5_UA0)                       /* SWU5 Upper Address Register n */
#define pREG_SWU5_UA1                    ((void * volatile *)REG_SWU5_UA1)                       /* SWU5 Upper Address Register n */
#define pREG_SWU5_UA2                    ((void * volatile *)REG_SWU5_UA2)                       /* SWU5 Upper Address Register n */
#define pREG_SWU5_UA3                    ((void * volatile *)REG_SWU5_UA3)                       /* SWU5 Upper Address Register n */
#define pREG_SWU5_ID0                    ((volatile uint32_t *)REG_SWU5_ID0)                     /* SWU5 ID Register n */
#define pREG_SWU5_ID1                    ((volatile uint32_t *)REG_SWU5_ID1)                     /* SWU5 ID Register n */
#define pREG_SWU5_ID2                    ((volatile uint32_t *)REG_SWU5_ID2)                     /* SWU5 ID Register n */
#define pREG_SWU5_ID3                    ((volatile uint32_t *)REG_SWU5_ID3)                     /* SWU5 ID Register n */
#define pREG_SWU5_CNT0                   ((volatile uint32_t *)REG_SWU5_CNT0)                    /* SWU5 Count Register n */
#define pREG_SWU5_CNT1                   ((volatile uint32_t *)REG_SWU5_CNT1)                    /* SWU5 Count Register n */
#define pREG_SWU5_CNT2                   ((volatile uint32_t *)REG_SWU5_CNT2)                    /* SWU5 Count Register n */
#define pREG_SWU5_CNT3                   ((volatile uint32_t *)REG_SWU5_CNT3)                    /* SWU5 Count Register n */
#define pREG_SWU5_TARG0                  ((volatile uint32_t *)REG_SWU5_TARG0)                   /* SWU5 Target Register n */
#define pREG_SWU5_TARG1                  ((volatile uint32_t *)REG_SWU5_TARG1)                   /* SWU5 Target Register n */
#define pREG_SWU5_TARG2                  ((volatile uint32_t *)REG_SWU5_TARG2)                   /* SWU5 Target Register n */
#define pREG_SWU5_TARG3                  ((volatile uint32_t *)REG_SWU5_TARG3)                   /* SWU5 Target Register n */
#define pREG_SWU5_HIST0                  ((volatile uint32_t *)REG_SWU5_HIST0)                   /* SWU5 Bandwidth History Register n */
#define pREG_SWU5_HIST1                  ((volatile uint32_t *)REG_SWU5_HIST1)                   /* SWU5 Bandwidth History Register n */
#define pREG_SWU5_HIST2                  ((volatile uint32_t *)REG_SWU5_HIST2)                   /* SWU5 Bandwidth History Register n */
#define pREG_SWU5_HIST3                  ((volatile uint32_t *)REG_SWU5_HIST3)                   /* SWU5 Bandwidth History Register n */
#define pREG_SWU5_CUR0                   ((volatile uint32_t *)REG_SWU5_CUR0)                    /* SWU5 Current Register n */
#define pREG_SWU5_CUR1                   ((volatile uint32_t *)REG_SWU5_CUR1)                    /* SWU5 Current Register n */
#define pREG_SWU5_CUR2                   ((volatile uint32_t *)REG_SWU5_CUR2)                    /* SWU5 Current Register n */
#define pREG_SWU5_CUR3                   ((volatile uint32_t *)REG_SWU5_CUR3)                    /* SWU5 Current Register n */

/* =========================================================================
       SWU6
   ========================================================================= */
#define pREG_SWU6_GCTL                   ((volatile uint32_t *)REG_SWU6_GCTL)                    /* SWU6 Global Control Register */
#define pREG_SWU6_GSTAT                  ((volatile uint32_t *)REG_SWU6_GSTAT)                   /* SWU6 Global Status Register */
#define pREG_SWU6_CTL0                   ((volatile uint32_t *)REG_SWU6_CTL0)                    /* SWU6 Control Register n */
#define pREG_SWU6_CTL1                   ((volatile uint32_t *)REG_SWU6_CTL1)                    /* SWU6 Control Register n */
#define pREG_SWU6_CTL2                   ((volatile uint32_t *)REG_SWU6_CTL2)                    /* SWU6 Control Register n */
#define pREG_SWU6_CTL3                   ((volatile uint32_t *)REG_SWU6_CTL3)                    /* SWU6 Control Register n */
#define pREG_SWU6_LA0                    ((void * volatile *)REG_SWU6_LA0)                       /* SWU6 Lower Address Register n */
#define pREG_SWU6_LA1                    ((void * volatile *)REG_SWU6_LA1)                       /* SWU6 Lower Address Register n */
#define pREG_SWU6_LA2                    ((void * volatile *)REG_SWU6_LA2)                       /* SWU6 Lower Address Register n */
#define pREG_SWU6_LA3                    ((void * volatile *)REG_SWU6_LA3)                       /* SWU6 Lower Address Register n */
#define pREG_SWU6_UA0                    ((void * volatile *)REG_SWU6_UA0)                       /* SWU6 Upper Address Register n */
#define pREG_SWU6_UA1                    ((void * volatile *)REG_SWU6_UA1)                       /* SWU6 Upper Address Register n */
#define pREG_SWU6_UA2                    ((void * volatile *)REG_SWU6_UA2)                       /* SWU6 Upper Address Register n */
#define pREG_SWU6_UA3                    ((void * volatile *)REG_SWU6_UA3)                       /* SWU6 Upper Address Register n */
#define pREG_SWU6_ID0                    ((volatile uint32_t *)REG_SWU6_ID0)                     /* SWU6 ID Register n */
#define pREG_SWU6_ID1                    ((volatile uint32_t *)REG_SWU6_ID1)                     /* SWU6 ID Register n */
#define pREG_SWU6_ID2                    ((volatile uint32_t *)REG_SWU6_ID2)                     /* SWU6 ID Register n */
#define pREG_SWU6_ID3                    ((volatile uint32_t *)REG_SWU6_ID3)                     /* SWU6 ID Register n */
#define pREG_SWU6_CNT0                   ((volatile uint32_t *)REG_SWU6_CNT0)                    /* SWU6 Count Register n */
#define pREG_SWU6_CNT1                   ((volatile uint32_t *)REG_SWU6_CNT1)                    /* SWU6 Count Register n */
#define pREG_SWU6_CNT2                   ((volatile uint32_t *)REG_SWU6_CNT2)                    /* SWU6 Count Register n */
#define pREG_SWU6_CNT3                   ((volatile uint32_t *)REG_SWU6_CNT3)                    /* SWU6 Count Register n */
#define pREG_SWU6_TARG0                  ((volatile uint32_t *)REG_SWU6_TARG0)                   /* SWU6 Target Register n */
#define pREG_SWU6_TARG1                  ((volatile uint32_t *)REG_SWU6_TARG1)                   /* SWU6 Target Register n */
#define pREG_SWU6_TARG2                  ((volatile uint32_t *)REG_SWU6_TARG2)                   /* SWU6 Target Register n */
#define pREG_SWU6_TARG3                  ((volatile uint32_t *)REG_SWU6_TARG3)                   /* SWU6 Target Register n */
#define pREG_SWU6_HIST0                  ((volatile uint32_t *)REG_SWU6_HIST0)                   /* SWU6 Bandwidth History Register n */
#define pREG_SWU6_HIST1                  ((volatile uint32_t *)REG_SWU6_HIST1)                   /* SWU6 Bandwidth History Register n */
#define pREG_SWU6_HIST2                  ((volatile uint32_t *)REG_SWU6_HIST2)                   /* SWU6 Bandwidth History Register n */
#define pREG_SWU6_HIST3                  ((volatile uint32_t *)REG_SWU6_HIST3)                   /* SWU6 Bandwidth History Register n */
#define pREG_SWU6_CUR0                   ((volatile uint32_t *)REG_SWU6_CUR0)                    /* SWU6 Current Register n */
#define pREG_SWU6_CUR1                   ((volatile uint32_t *)REG_SWU6_CUR1)                    /* SWU6 Current Register n */
#define pREG_SWU6_CUR2                   ((volatile uint32_t *)REG_SWU6_CUR2)                    /* SWU6 Current Register n */
#define pREG_SWU6_CUR3                   ((volatile uint32_t *)REG_SWU6_CUR3)                    /* SWU6 Current Register n */


/* =========================================================================
       SDU0
   ========================================================================= */
#define pREG_SDU0_IDCODE                 ((volatile uint32_t *)REG_SDU0_IDCODE)                  /* SDU0 ID Code Register */
#define pREG_SDU0_CTL                    ((volatile uint32_t *)REG_SDU0_CTL)                     /* SDU0 Control Register */
#define pREG_SDU0_STAT                   ((volatile uint32_t *)REG_SDU0_STAT)                    /* SDU0 Status Register */
#define pREG_SDU0_MACCTL                 ((volatile uint32_t *)REG_SDU0_MACCTL)                  /* SDU0 Memory Access Control Register */
#define pREG_SDU0_MACADDR                ((void * volatile *)REG_SDU0_MACADDR)                   /* SDU0 Memory Access Address Register */
#define pREG_SDU0_MACDATA                ((volatile uint32_t *)REG_SDU0_MACDATA)                 /* SDU0 Memory Access Data Register */
#define pREG_SDU0_DMARD                  ((volatile uint32_t *)REG_SDU0_DMARD)                   /* SDU0 DMA Read Data Register */
#define pREG_SDU0_DMAWD                  ((volatile uint32_t *)REG_SDU0_DMAWD)                   /* SDU0 DMA Write Data Register */
#define pREG_SDU0_MSG                    ((volatile uint32_t *)REG_SDU0_MSG)                     /* SDU0 Message Register */
#define pREG_SDU0_MSG_SET                ((volatile uint32_t *)REG_SDU0_MSG_SET)                 /* SDU0 Message Set Register */
#define pREG_SDU0_MSG_CLR                ((volatile uint32_t *)REG_SDU0_MSG_CLR)                 /* SDU0 Message Clear Register */
#define pREG_SDU0_GHLT                   ((volatile uint32_t *)REG_SDU0_GHLT)                    /* SDU0 Group Halt Register */


/* =========================================================================
       EMAC0
   ========================================================================= */
#define pREG_EMAC0_MACCFG                ((volatile uint32_t *)REG_EMAC0_MACCFG)                 /* EMAC0 MAC Configuration Register */
#define pREG_EMAC0_MACFRMFILT            ((volatile uint32_t *)REG_EMAC0_MACFRMFILT)             /* EMAC0 MAC Rx Frame Filter Register */
#define pREG_EMAC0_HASHTBL_HI            ((volatile uint32_t *)REG_EMAC0_HASHTBL_HI)             /* EMAC0 Hash Table High Register */
#define pREG_EMAC0_HASHTBL_LO            ((volatile uint32_t *)REG_EMAC0_HASHTBL_LO)             /* EMAC0 Hash Table Low Register */
#define pREG_EMAC0_SMI_ADDR              ((volatile uint32_t *)REG_EMAC0_SMI_ADDR)               /* EMAC0 SMI Address Register */
#define pREG_EMAC0_SMI_DATA              ((volatile uint32_t *)REG_EMAC0_SMI_DATA)               /* EMAC0 SMI Data Register */
#define pREG_EMAC0_FLOWCTL               ((volatile uint32_t *)REG_EMAC0_FLOWCTL)                /* EMAC0 FLow Control Register */
#define pREG_EMAC0_VLANTAG               ((volatile uint32_t *)REG_EMAC0_VLANTAG)                /* EMAC0 VLAN Tag Register */
#define pREG_EMAC0_DBG                   ((volatile uint32_t *)REG_EMAC0_DBG)                    /* EMAC0 Debug Register */
#define pREG_EMAC0_ISTAT                 ((volatile uint32_t *)REG_EMAC0_ISTAT)                  /* EMAC0 Interrupt Status Register */
#define pREG_EMAC0_IMSK                  ((volatile uint32_t *)REG_EMAC0_IMSK)                   /* EMAC0 Interrupt Mask Register */
#define pREG_EMAC0_ADDR0_HI              ((volatile uint32_t *)REG_EMAC0_ADDR0_HI)               /* EMAC0 MAC Address 0 High Register */
#define pREG_EMAC0_ADDR0_LO              ((volatile uint32_t *)REG_EMAC0_ADDR0_LO)               /* EMAC0 MAC Address 0 Low Register */
#define pREG_EMAC0_MMC_CTL               ((volatile uint32_t *)REG_EMAC0_MMC_CTL)                /* EMAC0 MMC Control Register */
#define pREG_EMAC0_MMC_RXINT             ((volatile uint32_t *)REG_EMAC0_MMC_RXINT)              /* EMAC0 MMC Rx Interrupt Register */
#define pREG_EMAC0_MMC_TXINT             ((volatile uint32_t *)REG_EMAC0_MMC_TXINT)              /* EMAC0 MMC Tx Interrupt Register */
#define pREG_EMAC0_MMC_RXIMSK            ((volatile uint32_t *)REG_EMAC0_MMC_RXIMSK)             /* EMAC0 MMC Rx Interrupt Mask Register */
#define pREG_EMAC0_MMC_TXIMSK            ((volatile uint32_t *)REG_EMAC0_MMC_TXIMSK)             /* EMAC0 MMC TX Interrupt Mask Register */
#define pREG_EMAC0_TXOCTCNT_GB           ((volatile uint32_t *)REG_EMAC0_TXOCTCNT_GB)            /* EMAC0 Tx OCT Count (Good/Bad) Register */
#define pREG_EMAC0_TXFRMCNT_GB           ((volatile uint32_t *)REG_EMAC0_TXFRMCNT_GB)            /* EMAC0 Tx Frame Count (Good/Bad) Register */
#define pREG_EMAC0_TXBCASTFRM_G          ((volatile uint32_t *)REG_EMAC0_TXBCASTFRM_G)           /* EMAC0 Tx Broadcast Frames (Good) Register */
#define pREG_EMAC0_TXMCASTFRM_G          ((volatile uint32_t *)REG_EMAC0_TXMCASTFRM_G)           /* EMAC0 Tx Multicast Frames (Good) Register */
#define pREG_EMAC0_TX64_GB               ((volatile uint32_t *)REG_EMAC0_TX64_GB)                /* EMAC0 Tx 64-Byte Frames (Good/Bad) Register */
#define pREG_EMAC0_TX65TO127_GB          ((volatile uint32_t *)REG_EMAC0_TX65TO127_GB)           /* EMAC0 Tx 65- to 127-Byte Frames (Good/Bad) Register */
#define pREG_EMAC0_TX128TO255_GB         ((volatile uint32_t *)REG_EMAC0_TX128TO255_GB)          /* EMAC0 Tx 128- to 255-Byte Frames (Good/Bad) Register */
#define pREG_EMAC0_TX256TO511_GB         ((volatile uint32_t *)REG_EMAC0_TX256TO511_GB)          /* EMAC0 Tx 256- to 511-Byte Frames (Good/Bad) Register */
#define pREG_EMAC0_TX512TO1023_GB        ((volatile uint32_t *)REG_EMAC0_TX512TO1023_GB)         /* EMAC0 Tx 512- to 1023-Byte Frames (Good/Bad) Register */
#define pREG_EMAC0_TX1024TOMAX_GB        ((volatile uint32_t *)REG_EMAC0_TX1024TOMAX_GB)         /* EMAC0 Tx 1024- to Max-Byte Frames (Good/Bad) Register */
#define pREG_EMAC0_TXUCASTFRM_GB         ((volatile uint32_t *)REG_EMAC0_TXUCASTFRM_GB)          /* EMAC0 Tx Unicast Frames (Good/Bad) Register */
#define pREG_EMAC0_TXMCASTFRM_GB         ((volatile uint32_t *)REG_EMAC0_TXMCASTFRM_GB)          /* EMAC0 Tx Multicast Frames (Good/Bad) Register */
#define pREG_EMAC0_TXBCASTFRM_GB         ((volatile uint32_t *)REG_EMAC0_TXBCASTFRM_GB)          /* EMAC0 Tx Broadcast Frames (Good/Bad) Register */
#define pREG_EMAC0_TXUNDR_ERR            ((volatile uint32_t *)REG_EMAC0_TXUNDR_ERR)             /* EMAC0 Tx Underflow Error Register */
#define pREG_EMAC0_TXSNGCOL_G            ((volatile uint32_t *)REG_EMAC0_TXSNGCOL_G)             /* EMAC0 Tx Single Collision (Good) Register */
#define pREG_EMAC0_TXMULTCOL_G           ((volatile uint32_t *)REG_EMAC0_TXMULTCOL_G)            /* EMAC0 Tx Multiple Collision (Good) Register */
#define pREG_EMAC0_TXDEFERRED            ((volatile uint32_t *)REG_EMAC0_TXDEFERRED)             /* EMAC0 Tx Deferred Register */
#define pREG_EMAC0_TXLATECOL             ((volatile uint32_t *)REG_EMAC0_TXLATECOL)              /* EMAC0 Tx Late Collision Register */
#define pREG_EMAC0_TXEXCESSCOL           ((volatile uint32_t *)REG_EMAC0_TXEXCESSCOL)            /* EMAC0 Tx Excess Collision Register */
#define pREG_EMAC0_TXCARR_ERR            ((volatile uint32_t *)REG_EMAC0_TXCARR_ERR)             /* EMAC0 Tx Carrier Error Register */
#define pREG_EMAC0_TXOCTCNT_G            ((volatile uint32_t *)REG_EMAC0_TXOCTCNT_G)             /* EMAC0 Tx Octet Count (Good) Register */
#define pREG_EMAC0_TXFRMCNT_G            ((volatile uint32_t *)REG_EMAC0_TXFRMCNT_G)             /* EMAC0 Tx Frame Count (Good) Register */
#define pREG_EMAC0_TXEXCESSDEF           ((volatile uint32_t *)REG_EMAC0_TXEXCESSDEF)            /* EMAC0 Tx Excess Deferral Register */
#define pREG_EMAC0_TXPAUSEFRM            ((volatile uint32_t *)REG_EMAC0_TXPAUSEFRM)             /* EMAC0 Tx Pause Frame Register */
#define pREG_EMAC0_TXVLANFRM_G           ((volatile uint32_t *)REG_EMAC0_TXVLANFRM_G)            /* EMAC0 Tx VLAN Frames (Good) Register */
#define pREG_EMAC0_RXFRMCNT_GB           ((volatile uint32_t *)REG_EMAC0_RXFRMCNT_GB)            /* EMAC0 Rx Frame Count (Good/Bad) Register */
#define pREG_EMAC0_RXOCTCNT_GB           ((volatile uint32_t *)REG_EMAC0_RXOCTCNT_GB)            /* EMAC0 Rx Octet Count (Good/Bad) Register */
#define pREG_EMAC0_RXOCTCNT_G            ((volatile uint32_t *)REG_EMAC0_RXOCTCNT_G)             /* EMAC0 Rx Octet Count (Good) Register */
#define pREG_EMAC0_RXBCASTFRM_G          ((volatile uint32_t *)REG_EMAC0_RXBCASTFRM_G)           /* EMAC0 Rx Broadcast Frames (Good) Register */
#define pREG_EMAC0_RXMCASTFRM_G          ((volatile uint32_t *)REG_EMAC0_RXMCASTFRM_G)           /* EMAC0 Rx Multicast Frames (Good) Register */
#define pREG_EMAC0_RXCRC_ERR             ((volatile uint32_t *)REG_EMAC0_RXCRC_ERR)              /* EMAC0 Rx CRC Error Register */
#define pREG_EMAC0_RXALIGN_ERR           ((volatile uint32_t *)REG_EMAC0_RXALIGN_ERR)            /* EMAC0 Rx alignment Error Register */
#define pREG_EMAC0_RXRUNT_ERR            ((volatile uint32_t *)REG_EMAC0_RXRUNT_ERR)             /* EMAC0 Rx Runt Error Register */
#define pREG_EMAC0_RXJAB_ERR             ((volatile uint32_t *)REG_EMAC0_RXJAB_ERR)              /* EMAC0 Rx Jab Error Register */
#define pREG_EMAC0_RXUSIZE_G             ((volatile uint32_t *)REG_EMAC0_RXUSIZE_G)              /* EMAC0 Rx Undersize (Good) Register */
#define pREG_EMAC0_RXOSIZE_G             ((volatile uint32_t *)REG_EMAC0_RXOSIZE_G)              /* EMAC0 Rx Oversize (Good) Register */
#define pREG_EMAC0_RX64_GB               ((volatile uint32_t *)REG_EMAC0_RX64_GB)                /* EMAC0 Rx 64-Byte Frames (Good/Bad) Register */
#define pREG_EMAC0_RX65TO127_GB          ((volatile uint32_t *)REG_EMAC0_RX65TO127_GB)           /* EMAC0 Rx 65- to 127-Byte Frames (Good/Bad) Register */
#define pREG_EMAC0_RX128TO255_GB         ((volatile uint32_t *)REG_EMAC0_RX128TO255_GB)          /* EMAC0 Rx 128- to 255-Byte Frames (Good/Bad) Register */
#define pREG_EMAC0_RX256TO511_GB         ((volatile uint32_t *)REG_EMAC0_RX256TO511_GB)          /* EMAC0 Rx 256- to 511-Byte Frames (Good/Bad) Register */
#define pREG_EMAC0_RX512TO1023_GB        ((volatile uint32_t *)REG_EMAC0_RX512TO1023_GB)         /* EMAC0 Rx 512- to 1023-Byte Frames (Good/Bad) Register */
#define pREG_EMAC0_RX1024TOMAX_GB        ((volatile uint32_t *)REG_EMAC0_RX1024TOMAX_GB)         /* EMAC0 Rx 1024- to Max-Byte Frames (Good/Bad) Register */
#define pREG_EMAC0_RXUCASTFRM_G          ((volatile uint32_t *)REG_EMAC0_RXUCASTFRM_G)           /* EMAC0 Rx Unicast Frames (Good) Register */
#define pREG_EMAC0_RXLEN_ERR             ((volatile uint32_t *)REG_EMAC0_RXLEN_ERR)              /* EMAC0 Rx Length Error Register */
#define pREG_EMAC0_RXOORTYPE             ((volatile uint32_t *)REG_EMAC0_RXOORTYPE)              /* EMAC0 Rx Out Of Range Type Register */
#define pREG_EMAC0_RXPAUSEFRM            ((volatile uint32_t *)REG_EMAC0_RXPAUSEFRM)             /* EMAC0 Rx Pause Frames Register */
#define pREG_EMAC0_RXFIFO_OVF            ((volatile uint32_t *)REG_EMAC0_RXFIFO_OVF)             /* EMAC0 Rx FIFO Overflow Register */
#define pREG_EMAC0_RXVLANFRM_GB          ((volatile uint32_t *)REG_EMAC0_RXVLANFRM_GB)           /* EMAC0 Rx VLAN Frames (Good/Bad) Register */
#define pREG_EMAC0_RXWDOG_ERR            ((volatile uint32_t *)REG_EMAC0_RXWDOG_ERR)             /* EMAC0 Rx Watch Dog Error Register */
#define pREG_EMAC0_IPC_RXIMSK            ((volatile uint32_t *)REG_EMAC0_IPC_RXIMSK)             /* EMAC0 MMC IPC Rx Interrupt Mask Register */
#define pREG_EMAC0_IPC_RXINT             ((volatile uint32_t *)REG_EMAC0_IPC_RXINT)              /* EMAC0 MMC IPC Rx Interrupt Register */
#define pREG_EMAC0_RXIPV4_GD_FRM         ((volatile uint32_t *)REG_EMAC0_RXIPV4_GD_FRM)          /* EMAC0 Rx IPv4 Datagrams (Good) Register */
#define pREG_EMAC0_RXIPV4_HDR_ERR_FRM    ((volatile uint32_t *)REG_EMAC0_RXIPV4_HDR_ERR_FRM)     /* EMAC0 Rx IPv4 Datagrams Header Errors Register */
#define pREG_EMAC0_RXIPV4_NOPAY_FRM      ((volatile uint32_t *)REG_EMAC0_RXIPV4_NOPAY_FRM)       /* EMAC0 Rx IPv4 Datagrams No Payload Frame Register */
#define pREG_EMAC0_RXIPV4_FRAG_FRM       ((volatile uint32_t *)REG_EMAC0_RXIPV4_FRAG_FRM)        /* EMAC0 Rx IPv4 Datagrams Fragmented Frames Register */
#define pREG_EMAC0_RXIPV4_UDSBL_FRM      ((volatile uint32_t *)REG_EMAC0_RXIPV4_UDSBL_FRM)       /* EMAC0 Rx IPv4 UDP Disabled Frames Register */
#define pREG_EMAC0_RXIPV6_GD_FRM         ((volatile uint32_t *)REG_EMAC0_RXIPV6_GD_FRM)          /* EMAC0 Rx IPv6 Datagrams Good Frames Register */
#define pREG_EMAC0_RXIPV6_HDR_ERR_FRM    ((volatile uint32_t *)REG_EMAC0_RXIPV6_HDR_ERR_FRM)     /* EMAC0 Rx IPv6 Datagrams Header Error Frames Register */
#define pREG_EMAC0_RXIPV6_NOPAY_FRM      ((volatile uint32_t *)REG_EMAC0_RXIPV6_NOPAY_FRM)       /* EMAC0 Rx IPv6 Datagrams No Payload Frames Register */
#define pREG_EMAC0_RXUDP_GD_FRM          ((volatile uint32_t *)REG_EMAC0_RXUDP_GD_FRM)           /* EMAC0 Rx UDP Good Frames Register */
#define pREG_EMAC0_RXUDP_ERR_FRM         ((volatile uint32_t *)REG_EMAC0_RXUDP_ERR_FRM)          /* EMAC0 Rx UDP Error Frames Register */
#define pREG_EMAC0_RXTCP_GD_FRM          ((volatile uint32_t *)REG_EMAC0_RXTCP_GD_FRM)           /* EMAC0 Rx TCP Good Frames Register */
#define pREG_EMAC0_RXTCP_ERR_FRM         ((volatile uint32_t *)REG_EMAC0_RXTCP_ERR_FRM)          /* EMAC0 Rx TCP Error Frames Register */
#define pREG_EMAC0_RXICMP_GD_FRM         ((volatile uint32_t *)REG_EMAC0_RXICMP_GD_FRM)          /* EMAC0 Rx ICMP Good Frames Register */
#define pREG_EMAC0_RXICMP_ERR_FRM        ((volatile uint32_t *)REG_EMAC0_RXICMP_ERR_FRM)         /* EMAC0 Rx ICMP Error Frames Register */
#define pREG_EMAC0_RXIPV4_GD_OCT         ((volatile uint32_t *)REG_EMAC0_RXIPV4_GD_OCT)          /* EMAC0 Rx IPv4 Datagrams Good Octets Register */
#define pREG_EMAC0_RXIPV4_HDR_ERR_OCT    ((volatile uint32_t *)REG_EMAC0_RXIPV4_HDR_ERR_OCT)     /* EMAC0 Rx IPv4 Datagrams Header Errors Register */
#define pREG_EMAC0_RXIPV4_NOPAY_OCT      ((volatile uint32_t *)REG_EMAC0_RXIPV4_NOPAY_OCT)       /* EMAC0 Rx IPv4 Datagrams No Payload Octets Register */
#define pREG_EMAC0_RXIPV4_FRAG_OCT       ((volatile uint32_t *)REG_EMAC0_RXIPV4_FRAG_OCT)        /* EMAC0 Rx IPv4 Datagrams Fragmented Octets Register */
#define pREG_EMAC0_RXIPV4_UDSBL_OCT      ((volatile uint32_t *)REG_EMAC0_RXIPV4_UDSBL_OCT)       /* EMAC0 Rx IPv4 UDP Disabled Octets Register */
#define pREG_EMAC0_RXIPV6_GD_OCT         ((volatile uint32_t *)REG_EMAC0_RXIPV6_GD_OCT)          /* EMAC0 Rx IPv6 Good Octets Register */
#define pREG_EMAC0_RXIPV6_HDR_ERR_OCT    ((volatile uint32_t *)REG_EMAC0_RXIPV6_HDR_ERR_OCT)     /* EMAC0 Rx IPv6 Header Errors Register */
#define pREG_EMAC0_RXIPV6_NOPAY_OCT      ((volatile uint32_t *)REG_EMAC0_RXIPV6_NOPAY_OCT)       /* EMAC0 Rx IPv6 No Payload Octets Register */
#define pREG_EMAC0_RXUDP_GD_OCT          ((volatile uint32_t *)REG_EMAC0_RXUDP_GD_OCT)           /* EMAC0 Rx UDP Good Octets Register */
#define pREG_EMAC0_RXUDP_ERR_OCT         ((volatile uint32_t *)REG_EMAC0_RXUDP_ERR_OCT)          /* EMAC0 Rx UDP Error Octets Register */
#define pREG_EMAC0_RXTCP_GD_OCT          ((volatile uint32_t *)REG_EMAC0_RXTCP_GD_OCT)           /* EMAC0 Rx TCP Good Octets Register */
#define pREG_EMAC0_RXTCP_ERR_OCT         ((volatile uint32_t *)REG_EMAC0_RXTCP_ERR_OCT)          /* EMAC0 Rx TCP Error Octets Register */
#define pREG_EMAC0_RXICMP_GD_OCT         ((volatile uint32_t *)REG_EMAC0_RXICMP_GD_OCT)          /* EMAC0 Rx ICMP Good Octets Register */
#define pREG_EMAC0_RXICMP_ERR_OCT        ((volatile uint32_t *)REG_EMAC0_RXICMP_ERR_OCT)         /* EMAC0 Rx ICMP Error Octets Register */
#define pREG_EMAC0_TM_CTL                ((volatile uint32_t *)REG_EMAC0_TM_CTL)                 /* EMAC0 Time Stamp Control Register */
#define pREG_EMAC0_TM_SUBSEC             ((volatile uint32_t *)REG_EMAC0_TM_SUBSEC)              /* EMAC0 Time Stamp Sub Second Increment Register */
#define pREG_EMAC0_TM_SEC                ((volatile uint32_t *)REG_EMAC0_TM_SEC)                 /* EMAC0 Time Stamp Low Seconds Register */
#define pREG_EMAC0_TM_NSEC               ((volatile uint32_t *)REG_EMAC0_TM_NSEC)                /* EMAC0 Time Stamp Nano Seconds Register */
#define pREG_EMAC0_TM_SECUPDT            ((volatile uint32_t *)REG_EMAC0_TM_SECUPDT)             /* EMAC0 Time Stamp Seconds Update Register */
#define pREG_EMAC0_TM_NSECUPDT           ((volatile uint32_t *)REG_EMAC0_TM_NSECUPDT)            /* EMAC0 Time Stamp Nano Seconds Update Register */
#define pREG_EMAC0_TM_ADDEND             ((volatile uint32_t *)REG_EMAC0_TM_ADDEND)              /* EMAC0 Time Stamp Addend Register */
#define pREG_EMAC0_TM_TGTM               ((volatile uint32_t *)REG_EMAC0_TM_TGTM)                /* EMAC0 Time Stamp Target Time Seconds Register */
#define pREG_EMAC0_TM_NTGTM              ((volatile uint32_t *)REG_EMAC0_TM_NTGTM)               /* EMAC0 Time Stamp Target Time Nano Seconds Register */
#define pREG_EMAC0_TM_HISEC              ((volatile uint32_t *)REG_EMAC0_TM_HISEC)               /* EMAC0 Time Stamp High Second Register */
#define pREG_EMAC0_TM_STMPSTAT           ((volatile uint32_t *)REG_EMAC0_TM_STMPSTAT)            /* EMAC0 Time Stamp Status Register */
#define pREG_EMAC0_TM_PPSCTL             ((volatile uint32_t *)REG_EMAC0_TM_PPSCTL)              /* EMAC0 PPS Control Register */
#define pREG_EMAC0_TM_AUXSTMP_NSEC       ((volatile uint32_t *)REG_EMAC0_TM_AUXSTMP_NSEC)        /* EMAC0 Time Stamp Auxilary TS Nano Seconds Register */
#define pREG_EMAC0_TM_AUXSTMP_SEC        ((volatile uint32_t *)REG_EMAC0_TM_AUXSTMP_SEC)         /* EMAC0 Time Stamp Auxilary TM Seconds Register */
#define pREG_EMAC0_TM_PPSINTVL           ((volatile uint32_t *)REG_EMAC0_TM_PPSINTVL)            /* EMAC0 Time Stamp PPS Interval Register */
#define pREG_EMAC0_TM_PPSWIDTH           ((volatile uint32_t *)REG_EMAC0_TM_PPSWIDTH)            /* EMAC0 PPS Width Register */
#define pREG_EMAC0_DMA_BUSMODE           ((volatile uint32_t *)REG_EMAC0_DMA_BUSMODE)            /* EMAC0 DMA Bus Mode Register */
#define pREG_EMAC0_DMA_TXPOLL            ((volatile uint32_t *)REG_EMAC0_DMA_TXPOLL)             /* EMAC0 DMA Tx Poll Demand Register */
#define pREG_EMAC0_DMA_RXPOLL            ((volatile uint32_t *)REG_EMAC0_DMA_RXPOLL)             /* EMAC0 DMA Rx Poll Demand register */
#define pREG_EMAC0_DMA_RXDSC_ADDR        ((volatile uint32_t *)REG_EMAC0_DMA_RXDSC_ADDR)         /* EMAC0 DMA Rx Descriptor List Address Register */
#define pREG_EMAC0_DMA_TXDSC_ADDR        ((volatile uint32_t *)REG_EMAC0_DMA_TXDSC_ADDR)         /* EMAC0 DMA Tx Descriptor List Address Register */
#define pREG_EMAC0_DMA_STAT              ((volatile uint32_t *)REG_EMAC0_DMA_STAT)               /* EMAC0 DMA Status Register */
#define pREG_EMAC0_DMA_OPMODE            ((volatile uint32_t *)REG_EMAC0_DMA_OPMODE)             /* EMAC0 DMA Operation Mode Register */
#define pREG_EMAC0_DMA_IEN               ((volatile uint32_t *)REG_EMAC0_DMA_IEN)                /* EMAC0 DMA Interrupt Enable Register */
#define pREG_EMAC0_DMA_MISS_FRM          ((volatile uint32_t *)REG_EMAC0_DMA_MISS_FRM)           /* EMAC0 DMA Missed Frame Register */
#define pREG_EMAC0_DMA_RXIWDOG           ((volatile uint32_t *)REG_EMAC0_DMA_RXIWDOG)            /* EMAC0 DMA Rx Interrupt Watch Dog Register */
#define pREG_EMAC0_DMA_BMMODE            ((volatile uint32_t *)REG_EMAC0_DMA_BMMODE)             /* EMAC0 DMA SCB Bus Mode Register */
#define pREG_EMAC0_DMA_BMSTAT            ((volatile uint32_t *)REG_EMAC0_DMA_BMSTAT)             /* EMAC0 DMA SCB Status Register */
#define pREG_EMAC0_DMA_TXDSC_CUR         ((volatile uint32_t *)REG_EMAC0_DMA_TXDSC_CUR)          /* EMAC0 DMA Tx Descriptor Current Register */
#define pREG_EMAC0_DMA_RXDSC_CUR         ((volatile uint32_t *)REG_EMAC0_DMA_RXDSC_CUR)          /* EMAC0 DMA Rx Descriptor Current Register */
#define pREG_EMAC0_DMA_TXBUF_CUR         ((volatile uint32_t *)REG_EMAC0_DMA_TXBUF_CUR)          /* EMAC0 DMA Tx Buffer Current Register */
#define pREG_EMAC0_DMA_RXBUF_CUR         ((volatile uint32_t *)REG_EMAC0_DMA_RXBUF_CUR)          /* EMAC0 DMA Rx Buffer Current Register */

/* =========================================================================
       EMAC1
   ========================================================================= */
#define pREG_EMAC1_MACCFG                ((volatile uint32_t *)REG_EMAC1_MACCFG)                 /* EMAC1 MAC Configuration Register */
#define pREG_EMAC1_MACFRMFILT            ((volatile uint32_t *)REG_EMAC1_MACFRMFILT)             /* EMAC1 MAC Rx Frame Filter Register */
#define pREG_EMAC1_HASHTBL_HI            ((volatile uint32_t *)REG_EMAC1_HASHTBL_HI)             /* EMAC1 Hash Table High Register */
#define pREG_EMAC1_HASHTBL_LO            ((volatile uint32_t *)REG_EMAC1_HASHTBL_LO)             /* EMAC1 Hash Table Low Register */
#define pREG_EMAC1_SMI_ADDR              ((volatile uint32_t *)REG_EMAC1_SMI_ADDR)               /* EMAC1 SMI Address Register */
#define pREG_EMAC1_SMI_DATA              ((volatile uint32_t *)REG_EMAC1_SMI_DATA)               /* EMAC1 SMI Data Register */
#define pREG_EMAC1_FLOWCTL               ((volatile uint32_t *)REG_EMAC1_FLOWCTL)                /* EMAC1 FLow Control Register */
#define pREG_EMAC1_VLANTAG               ((volatile uint32_t *)REG_EMAC1_VLANTAG)                /* EMAC1 VLAN Tag Register */
#define pREG_EMAC1_DBG                   ((volatile uint32_t *)REG_EMAC1_DBG)                    /* EMAC1 Debug Register */
#define pREG_EMAC1_ISTAT                 ((volatile uint32_t *)REG_EMAC1_ISTAT)                  /* EMAC1 Interrupt Status Register */
#define pREG_EMAC1_IMSK                  ((volatile uint32_t *)REG_EMAC1_IMSK)                   /* EMAC1 Interrupt Mask Register */
#define pREG_EMAC1_ADDR0_HI              ((volatile uint32_t *)REG_EMAC1_ADDR0_HI)               /* EMAC1 MAC Address 0 High Register */
#define pREG_EMAC1_ADDR0_LO              ((volatile uint32_t *)REG_EMAC1_ADDR0_LO)               /* EMAC1 MAC Address 0 Low Register */
#define pREG_EMAC1_MMC_CTL               ((volatile uint32_t *)REG_EMAC1_MMC_CTL)                /* EMAC1 MMC Control Register */
#define pREG_EMAC1_MMC_RXINT             ((volatile uint32_t *)REG_EMAC1_MMC_RXINT)              /* EMAC1 MMC Rx Interrupt Register */
#define pREG_EMAC1_MMC_TXINT             ((volatile uint32_t *)REG_EMAC1_MMC_TXINT)              /* EMAC1 MMC Tx Interrupt Register */
#define pREG_EMAC1_MMC_RXIMSK            ((volatile uint32_t *)REG_EMAC1_MMC_RXIMSK)             /* EMAC1 MMC Rx Interrupt Mask Register */
#define pREG_EMAC1_MMC_TXIMSK            ((volatile uint32_t *)REG_EMAC1_MMC_TXIMSK)             /* EMAC1 MMC TX Interrupt Mask Register */
#define pREG_EMAC1_TXOCTCNT_GB           ((volatile uint32_t *)REG_EMAC1_TXOCTCNT_GB)            /* EMAC1 Tx OCT Count (Good/Bad) Register */
#define pREG_EMAC1_TXFRMCNT_GB           ((volatile uint32_t *)REG_EMAC1_TXFRMCNT_GB)            /* EMAC1 Tx Frame Count (Good/Bad) Register */
#define pREG_EMAC1_TXBCASTFRM_G          ((volatile uint32_t *)REG_EMAC1_TXBCASTFRM_G)           /* EMAC1 Tx Broadcast Frames (Good) Register */
#define pREG_EMAC1_TXMCASTFRM_G          ((volatile uint32_t *)REG_EMAC1_TXMCASTFRM_G)           /* EMAC1 Tx Multicast Frames (Good) Register */
#define pREG_EMAC1_TX64_GB               ((volatile uint32_t *)REG_EMAC1_TX64_GB)                /* EMAC1 Tx 64-Byte Frames (Good/Bad) Register */
#define pREG_EMAC1_TX65TO127_GB          ((volatile uint32_t *)REG_EMAC1_TX65TO127_GB)           /* EMAC1 Tx 65- to 127-Byte Frames (Good/Bad) Register */
#define pREG_EMAC1_TX128TO255_GB         ((volatile uint32_t *)REG_EMAC1_TX128TO255_GB)          /* EMAC1 Tx 128- to 255-Byte Frames (Good/Bad) Register */
#define pREG_EMAC1_TX256TO511_GB         ((volatile uint32_t *)REG_EMAC1_TX256TO511_GB)          /* EMAC1 Tx 256- to 511-Byte Frames (Good/Bad) Register */
#define pREG_EMAC1_TX512TO1023_GB        ((volatile uint32_t *)REG_EMAC1_TX512TO1023_GB)         /* EMAC1 Tx 512- to 1023-Byte Frames (Good/Bad) Register */
#define pREG_EMAC1_TX1024TOMAX_GB        ((volatile uint32_t *)REG_EMAC1_TX1024TOMAX_GB)         /* EMAC1 Tx 1024- to Max-Byte Frames (Good/Bad) Register */
#define pREG_EMAC1_TXUCASTFRM_GB         ((volatile uint32_t *)REG_EMAC1_TXUCASTFRM_GB)          /* EMAC1 Tx Unicast Frames (Good/Bad) Register */
#define pREG_EMAC1_TXMCASTFRM_GB         ((volatile uint32_t *)REG_EMAC1_TXMCASTFRM_GB)          /* EMAC1 Tx Multicast Frames (Good/Bad) Register */
#define pREG_EMAC1_TXBCASTFRM_GB         ((volatile uint32_t *)REG_EMAC1_TXBCASTFRM_GB)          /* EMAC1 Tx Broadcast Frames (Good/Bad) Register */
#define pREG_EMAC1_TXUNDR_ERR            ((volatile uint32_t *)REG_EMAC1_TXUNDR_ERR)             /* EMAC1 Tx Underflow Error Register */
#define pREG_EMAC1_TXSNGCOL_G            ((volatile uint32_t *)REG_EMAC1_TXSNGCOL_G)             /* EMAC1 Tx Single Collision (Good) Register */
#define pREG_EMAC1_TXMULTCOL_G           ((volatile uint32_t *)REG_EMAC1_TXMULTCOL_G)            /* EMAC1 Tx Multiple Collision (Good) Register */
#define pREG_EMAC1_TXDEFERRED            ((volatile uint32_t *)REG_EMAC1_TXDEFERRED)             /* EMAC1 Tx Deferred Register */
#define pREG_EMAC1_TXLATECOL             ((volatile uint32_t *)REG_EMAC1_TXLATECOL)              /* EMAC1 Tx Late Collision Register */
#define pREG_EMAC1_TXEXCESSCOL           ((volatile uint32_t *)REG_EMAC1_TXEXCESSCOL)            /* EMAC1 Tx Excess Collision Register */
#define pREG_EMAC1_TXCARR_ERR            ((volatile uint32_t *)REG_EMAC1_TXCARR_ERR)             /* EMAC1 Tx Carrier Error Register */
#define pREG_EMAC1_TXOCTCNT_G            ((volatile uint32_t *)REG_EMAC1_TXOCTCNT_G)             /* EMAC1 Tx Octet Count (Good) Register */
#define pREG_EMAC1_TXFRMCNT_G            ((volatile uint32_t *)REG_EMAC1_TXFRMCNT_G)             /* EMAC1 Tx Frame Count (Good) Register */
#define pREG_EMAC1_TXEXCESSDEF           ((volatile uint32_t *)REG_EMAC1_TXEXCESSDEF)            /* EMAC1 Tx Excess Deferral Register */
#define pREG_EMAC1_TXPAUSEFRM            ((volatile uint32_t *)REG_EMAC1_TXPAUSEFRM)             /* EMAC1 Tx Pause Frame Register */
#define pREG_EMAC1_TXVLANFRM_G           ((volatile uint32_t *)REG_EMAC1_TXVLANFRM_G)            /* EMAC1 Tx VLAN Frames (Good) Register */
#define pREG_EMAC1_RXFRMCNT_GB           ((volatile uint32_t *)REG_EMAC1_RXFRMCNT_GB)            /* EMAC1 Rx Frame Count (Good/Bad) Register */
#define pREG_EMAC1_RXOCTCNT_GB           ((volatile uint32_t *)REG_EMAC1_RXOCTCNT_GB)            /* EMAC1 Rx Octet Count (Good/Bad) Register */
#define pREG_EMAC1_RXOCTCNT_G            ((volatile uint32_t *)REG_EMAC1_RXOCTCNT_G)             /* EMAC1 Rx Octet Count (Good) Register */
#define pREG_EMAC1_RXBCASTFRM_G          ((volatile uint32_t *)REG_EMAC1_RXBCASTFRM_G)           /* EMAC1 Rx Broadcast Frames (Good) Register */
#define pREG_EMAC1_RXMCASTFRM_G          ((volatile uint32_t *)REG_EMAC1_RXMCASTFRM_G)           /* EMAC1 Rx Multicast Frames (Good) Register */
#define pREG_EMAC1_RXCRC_ERR             ((volatile uint32_t *)REG_EMAC1_RXCRC_ERR)              /* EMAC1 Rx CRC Error Register */
#define pREG_EMAC1_RXALIGN_ERR           ((volatile uint32_t *)REG_EMAC1_RXALIGN_ERR)            /* EMAC1 Rx alignment Error Register */
#define pREG_EMAC1_RXRUNT_ERR            ((volatile uint32_t *)REG_EMAC1_RXRUNT_ERR)             /* EMAC1 Rx Runt Error Register */
#define pREG_EMAC1_RXJAB_ERR             ((volatile uint32_t *)REG_EMAC1_RXJAB_ERR)              /* EMAC1 Rx Jab Error Register */
#define pREG_EMAC1_RXUSIZE_G             ((volatile uint32_t *)REG_EMAC1_RXUSIZE_G)              /* EMAC1 Rx Undersize (Good) Register */
#define pREG_EMAC1_RXOSIZE_G             ((volatile uint32_t *)REG_EMAC1_RXOSIZE_G)              /* EMAC1 Rx Oversize (Good) Register */
#define pREG_EMAC1_RX64_GB               ((volatile uint32_t *)REG_EMAC1_RX64_GB)                /* EMAC1 Rx 64-Byte Frames (Good/Bad) Register */
#define pREG_EMAC1_RX65TO127_GB          ((volatile uint32_t *)REG_EMAC1_RX65TO127_GB)           /* EMAC1 Rx 65- to 127-Byte Frames (Good/Bad) Register */
#define pREG_EMAC1_RX128TO255_GB         ((volatile uint32_t *)REG_EMAC1_RX128TO255_GB)          /* EMAC1 Rx 128- to 255-Byte Frames (Good/Bad) Register */
#define pREG_EMAC1_RX256TO511_GB         ((volatile uint32_t *)REG_EMAC1_RX256TO511_GB)          /* EMAC1 Rx 256- to 511-Byte Frames (Good/Bad) Register */
#define pREG_EMAC1_RX512TO1023_GB        ((volatile uint32_t *)REG_EMAC1_RX512TO1023_GB)         /* EMAC1 Rx 512- to 1023-Byte Frames (Good/Bad) Register */
#define pREG_EMAC1_RX1024TOMAX_GB        ((volatile uint32_t *)REG_EMAC1_RX1024TOMAX_GB)         /* EMAC1 Rx 1024- to Max-Byte Frames (Good/Bad) Register */
#define pREG_EMAC1_RXUCASTFRM_G          ((volatile uint32_t *)REG_EMAC1_RXUCASTFRM_G)           /* EMAC1 Rx Unicast Frames (Good) Register */
#define pREG_EMAC1_RXLEN_ERR             ((volatile uint32_t *)REG_EMAC1_RXLEN_ERR)              /* EMAC1 Rx Length Error Register */
#define pREG_EMAC1_RXOORTYPE             ((volatile uint32_t *)REG_EMAC1_RXOORTYPE)              /* EMAC1 Rx Out Of Range Type Register */
#define pREG_EMAC1_RXPAUSEFRM            ((volatile uint32_t *)REG_EMAC1_RXPAUSEFRM)             /* EMAC1 Rx Pause Frames Register */
#define pREG_EMAC1_RXFIFO_OVF            ((volatile uint32_t *)REG_EMAC1_RXFIFO_OVF)             /* EMAC1 Rx FIFO Overflow Register */
#define pREG_EMAC1_RXVLANFRM_GB          ((volatile uint32_t *)REG_EMAC1_RXVLANFRM_GB)           /* EMAC1 Rx VLAN Frames (Good/Bad) Register */
#define pREG_EMAC1_RXWDOG_ERR            ((volatile uint32_t *)REG_EMAC1_RXWDOG_ERR)             /* EMAC1 Rx Watch Dog Error Register */
#define pREG_EMAC1_IPC_RXIMSK            ((volatile uint32_t *)REG_EMAC1_IPC_RXIMSK)             /* EMAC1 MMC IPC Rx Interrupt Mask Register */
#define pREG_EMAC1_IPC_RXINT             ((volatile uint32_t *)REG_EMAC1_IPC_RXINT)              /* EMAC1 MMC IPC Rx Interrupt Register */
#define pREG_EMAC1_RXIPV4_GD_FRM         ((volatile uint32_t *)REG_EMAC1_RXIPV4_GD_FRM)          /* EMAC1 Rx IPv4 Datagrams (Good) Register */
#define pREG_EMAC1_RXIPV4_HDR_ERR_FRM    ((volatile uint32_t *)REG_EMAC1_RXIPV4_HDR_ERR_FRM)     /* EMAC1 Rx IPv4 Datagrams Header Errors Register */
#define pREG_EMAC1_RXIPV4_NOPAY_FRM      ((volatile uint32_t *)REG_EMAC1_RXIPV4_NOPAY_FRM)       /* EMAC1 Rx IPv4 Datagrams No Payload Frame Register */
#define pREG_EMAC1_RXIPV4_FRAG_FRM       ((volatile uint32_t *)REG_EMAC1_RXIPV4_FRAG_FRM)        /* EMAC1 Rx IPv4 Datagrams Fragmented Frames Register */
#define pREG_EMAC1_RXIPV4_UDSBL_FRM      ((volatile uint32_t *)REG_EMAC1_RXIPV4_UDSBL_FRM)       /* EMAC1 Rx IPv4 UDP Disabled Frames Register */
#define pREG_EMAC1_RXIPV6_GD_FRM         ((volatile uint32_t *)REG_EMAC1_RXIPV6_GD_FRM)          /* EMAC1 Rx IPv6 Datagrams Good Frames Register */
#define pREG_EMAC1_RXIPV6_HDR_ERR_FRM    ((volatile uint32_t *)REG_EMAC1_RXIPV6_HDR_ERR_FRM)     /* EMAC1 Rx IPv6 Datagrams Header Error Frames Register */
#define pREG_EMAC1_RXIPV6_NOPAY_FRM      ((volatile uint32_t *)REG_EMAC1_RXIPV6_NOPAY_FRM)       /* EMAC1 Rx IPv6 Datagrams No Payload Frames Register */
#define pREG_EMAC1_RXUDP_GD_FRM          ((volatile uint32_t *)REG_EMAC1_RXUDP_GD_FRM)           /* EMAC1 Rx UDP Good Frames Register */
#define pREG_EMAC1_RXUDP_ERR_FRM         ((volatile uint32_t *)REG_EMAC1_RXUDP_ERR_FRM)          /* EMAC1 Rx UDP Error Frames Register */
#define pREG_EMAC1_RXTCP_GD_FRM          ((volatile uint32_t *)REG_EMAC1_RXTCP_GD_FRM)           /* EMAC1 Rx TCP Good Frames Register */
#define pREG_EMAC1_RXTCP_ERR_FRM         ((volatile uint32_t *)REG_EMAC1_RXTCP_ERR_FRM)          /* EMAC1 Rx TCP Error Frames Register */
#define pREG_EMAC1_RXICMP_GD_FRM         ((volatile uint32_t *)REG_EMAC1_RXICMP_GD_FRM)          /* EMAC1 Rx ICMP Good Frames Register */
#define pREG_EMAC1_RXICMP_ERR_FRM        ((volatile uint32_t *)REG_EMAC1_RXICMP_ERR_FRM)         /* EMAC1 Rx ICMP Error Frames Register */
#define pREG_EMAC1_RXIPV4_GD_OCT         ((volatile uint32_t *)REG_EMAC1_RXIPV4_GD_OCT)          /* EMAC1 Rx IPv4 Datagrams Good Octets Register */
#define pREG_EMAC1_RXIPV4_HDR_ERR_OCT    ((volatile uint32_t *)REG_EMAC1_RXIPV4_HDR_ERR_OCT)     /* EMAC1 Rx IPv4 Datagrams Header Errors Register */
#define pREG_EMAC1_RXIPV4_NOPAY_OCT      ((volatile uint32_t *)REG_EMAC1_RXIPV4_NOPAY_OCT)       /* EMAC1 Rx IPv4 Datagrams No Payload Octets Register */
#define pREG_EMAC1_RXIPV4_FRAG_OCT       ((volatile uint32_t *)REG_EMAC1_RXIPV4_FRAG_OCT)        /* EMAC1 Rx IPv4 Datagrams Fragmented Octets Register */
#define pREG_EMAC1_RXIPV4_UDSBL_OCT      ((volatile uint32_t *)REG_EMAC1_RXIPV4_UDSBL_OCT)       /* EMAC1 Rx IPv4 UDP Disabled Octets Register */
#define pREG_EMAC1_RXIPV6_GD_OCT         ((volatile uint32_t *)REG_EMAC1_RXIPV6_GD_OCT)          /* EMAC1 Rx IPv6 Good Octets Register */
#define pREG_EMAC1_RXIPV6_HDR_ERR_OCT    ((volatile uint32_t *)REG_EMAC1_RXIPV6_HDR_ERR_OCT)     /* EMAC1 Rx IPv6 Header Errors Register */
#define pREG_EMAC1_RXIPV6_NOPAY_OCT      ((volatile uint32_t *)REG_EMAC1_RXIPV6_NOPAY_OCT)       /* EMAC1 Rx IPv6 No Payload Octets Register */
#define pREG_EMAC1_RXUDP_GD_OCT          ((volatile uint32_t *)REG_EMAC1_RXUDP_GD_OCT)           /* EMAC1 Rx UDP Good Octets Register */
#define pREG_EMAC1_RXUDP_ERR_OCT         ((volatile uint32_t *)REG_EMAC1_RXUDP_ERR_OCT)          /* EMAC1 Rx UDP Error Octets Register */
#define pREG_EMAC1_RXTCP_GD_OCT          ((volatile uint32_t *)REG_EMAC1_RXTCP_GD_OCT)           /* EMAC1 Rx TCP Good Octets Register */
#define pREG_EMAC1_RXTCP_ERR_OCT         ((volatile uint32_t *)REG_EMAC1_RXTCP_ERR_OCT)          /* EMAC1 Rx TCP Error Octets Register */
#define pREG_EMAC1_RXICMP_GD_OCT         ((volatile uint32_t *)REG_EMAC1_RXICMP_GD_OCT)          /* EMAC1 Rx ICMP Good Octets Register */
#define pREG_EMAC1_RXICMP_ERR_OCT        ((volatile uint32_t *)REG_EMAC1_RXICMP_ERR_OCT)         /* EMAC1 Rx ICMP Error Octets Register */
#define pREG_EMAC1_TM_CTL                ((volatile uint32_t *)REG_EMAC1_TM_CTL)                 /* EMAC1 Time Stamp Control Register */
#define pREG_EMAC1_TM_SUBSEC             ((volatile uint32_t *)REG_EMAC1_TM_SUBSEC)              /* EMAC1 Time Stamp Sub Second Increment Register */
#define pREG_EMAC1_TM_SEC                ((volatile uint32_t *)REG_EMAC1_TM_SEC)                 /* EMAC1 Time Stamp Low Seconds Register */
#define pREG_EMAC1_TM_NSEC               ((volatile uint32_t *)REG_EMAC1_TM_NSEC)                /* EMAC1 Time Stamp Nano Seconds Register */
#define pREG_EMAC1_TM_SECUPDT            ((volatile uint32_t *)REG_EMAC1_TM_SECUPDT)             /* EMAC1 Time Stamp Seconds Update Register */
#define pREG_EMAC1_TM_NSECUPDT           ((volatile uint32_t *)REG_EMAC1_TM_NSECUPDT)            /* EMAC1 Time Stamp Nano Seconds Update Register */
#define pREG_EMAC1_TM_ADDEND             ((volatile uint32_t *)REG_EMAC1_TM_ADDEND)              /* EMAC1 Time Stamp Addend Register */
#define pREG_EMAC1_TM_TGTM               ((volatile uint32_t *)REG_EMAC1_TM_TGTM)                /* EMAC1 Time Stamp Target Time Seconds Register */
#define pREG_EMAC1_TM_NTGTM              ((volatile uint32_t *)REG_EMAC1_TM_NTGTM)               /* EMAC1 Time Stamp Target Time Nano Seconds Register */
#define pREG_EMAC1_TM_HISEC              ((volatile uint32_t *)REG_EMAC1_TM_HISEC)               /* EMAC1 Time Stamp High Second Register */
#define pREG_EMAC1_TM_STMPSTAT           ((volatile uint32_t *)REG_EMAC1_TM_STMPSTAT)            /* EMAC1 Time Stamp Status Register */
#define pREG_EMAC1_TM_PPSCTL             ((volatile uint32_t *)REG_EMAC1_TM_PPSCTL)              /* EMAC1 PPS Control Register */
#define pREG_EMAC1_TM_AUXSTMP_NSEC       ((volatile uint32_t *)REG_EMAC1_TM_AUXSTMP_NSEC)        /* EMAC1 Time Stamp Auxilary TS Nano Seconds Register */
#define pREG_EMAC1_TM_AUXSTMP_SEC        ((volatile uint32_t *)REG_EMAC1_TM_AUXSTMP_SEC)         /* EMAC1 Time Stamp Auxilary TM Seconds Register */
#define pREG_EMAC1_TM_PPSINTVL           ((volatile uint32_t *)REG_EMAC1_TM_PPSINTVL)            /* EMAC1 Time Stamp PPS Interval Register */
#define pREG_EMAC1_TM_PPSWIDTH           ((volatile uint32_t *)REG_EMAC1_TM_PPSWIDTH)            /* EMAC1 PPS Width Register */
#define pREG_EMAC1_DMA_BUSMODE           ((volatile uint32_t *)REG_EMAC1_DMA_BUSMODE)            /* EMAC1 DMA Bus Mode Register */
#define pREG_EMAC1_DMA_TXPOLL            ((volatile uint32_t *)REG_EMAC1_DMA_TXPOLL)             /* EMAC1 DMA Tx Poll Demand Register */
#define pREG_EMAC1_DMA_RXPOLL            ((volatile uint32_t *)REG_EMAC1_DMA_RXPOLL)             /* EMAC1 DMA Rx Poll Demand register */
#define pREG_EMAC1_DMA_RXDSC_ADDR        ((volatile uint32_t *)REG_EMAC1_DMA_RXDSC_ADDR)         /* EMAC1 DMA Rx Descriptor List Address Register */
#define pREG_EMAC1_DMA_TXDSC_ADDR        ((volatile uint32_t *)REG_EMAC1_DMA_TXDSC_ADDR)         /* EMAC1 DMA Tx Descriptor List Address Register */
#define pREG_EMAC1_DMA_STAT              ((volatile uint32_t *)REG_EMAC1_DMA_STAT)               /* EMAC1 DMA Status Register */
#define pREG_EMAC1_DMA_OPMODE            ((volatile uint32_t *)REG_EMAC1_DMA_OPMODE)             /* EMAC1 DMA Operation Mode Register */
#define pREG_EMAC1_DMA_IEN               ((volatile uint32_t *)REG_EMAC1_DMA_IEN)                /* EMAC1 DMA Interrupt Enable Register */
#define pREG_EMAC1_DMA_MISS_FRM          ((volatile uint32_t *)REG_EMAC1_DMA_MISS_FRM)           /* EMAC1 DMA Missed Frame Register */
#define pREG_EMAC1_DMA_RXIWDOG           ((volatile uint32_t *)REG_EMAC1_DMA_RXIWDOG)            /* EMAC1 DMA Rx Interrupt Watch Dog Register */
#define pREG_EMAC1_DMA_BMMODE            ((volatile uint32_t *)REG_EMAC1_DMA_BMMODE)             /* EMAC1 DMA SCB Bus Mode Register */
#define pREG_EMAC1_DMA_BMSTAT            ((volatile uint32_t *)REG_EMAC1_DMA_BMSTAT)             /* EMAC1 DMA SCB Status Register */
#define pREG_EMAC1_DMA_TXDSC_CUR         ((volatile uint32_t *)REG_EMAC1_DMA_TXDSC_CUR)          /* EMAC1 DMA Tx Descriptor Current Register */
#define pREG_EMAC1_DMA_RXDSC_CUR         ((volatile uint32_t *)REG_EMAC1_DMA_RXDSC_CUR)          /* EMAC1 DMA Rx Descriptor Current Register */
#define pREG_EMAC1_DMA_TXBUF_CUR         ((volatile uint32_t *)REG_EMAC1_DMA_TXBUF_CUR)          /* EMAC1 DMA Tx Buffer Current Register */
#define pREG_EMAC1_DMA_RXBUF_CUR         ((volatile uint32_t *)REG_EMAC1_DMA_RXBUF_CUR)          /* EMAC1 DMA Rx Buffer Current Register */


/* =========================================================================
       SPORT0
   ========================================================================= */
#define pREG_SPORT0_CTL_A                ((volatile uint32_t *)REG_SPORT0_CTL_A)                 /* SPORT0 Half SPORT 'A' Control Register */
#define pREG_SPORT0_DIV_A                ((volatile uint32_t *)REG_SPORT0_DIV_A)                 /* SPORT0 Half SPORT 'A' Divisor Register */
#define pREG_SPORT0_MCTL_A               ((volatile uint32_t *)REG_SPORT0_MCTL_A)                /* SPORT0 Half SPORT 'A' Multi-channel Control Register */
#define pREG_SPORT0_CS0_A                ((volatile uint32_t *)REG_SPORT0_CS0_A)                 /* SPORT0 Half SPORT 'A' Multi-channel 0-31 Select Register */
#define pREG_SPORT0_CS1_A                ((volatile uint32_t *)REG_SPORT0_CS1_A)                 /* SPORT0 Half SPORT 'A' Multi-channel 32-63 Select Register */
#define pREG_SPORT0_CS2_A                ((volatile uint32_t *)REG_SPORT0_CS2_A)                 /* SPORT0 Half SPORT 'A' Multi-channel 64-95 Select Register */
#define pREG_SPORT0_CS3_A                ((volatile uint32_t *)REG_SPORT0_CS3_A)                 /* SPORT0 Half SPORT 'A' Multi-channel 96-127 Select Register */
#define pREG_SPORT0_ERR_A                ((volatile uint32_t *)REG_SPORT0_ERR_A)                 /* SPORT0 Half SPORT 'A' Error Register */
#define pREG_SPORT0_MSTAT_A              ((volatile uint32_t *)REG_SPORT0_MSTAT_A)               /* SPORT0 Half SPORT 'A' Multi-channel Status Register */
#define pREG_SPORT0_CTL2_A               ((volatile uint32_t *)REG_SPORT0_CTL2_A)                /* SPORT0 Half SPORT 'A' Control 2 Register */
#define pREG_SPORT0_TXPRI_A              ((volatile uint32_t *)REG_SPORT0_TXPRI_A)               /* SPORT0 Half SPORT 'A' Tx Buffer (Primary) Register */
#define pREG_SPORT0_RXPRI_A              ((volatile uint32_t *)REG_SPORT0_RXPRI_A)               /* SPORT0 Half SPORT 'A' Rx Buffer (Primary) Register */
#define pREG_SPORT0_TXSEC_A              ((volatile uint32_t *)REG_SPORT0_TXSEC_A)               /* SPORT0 Half SPORT 'A' Tx Buffer (Secondary) Register */
#define pREG_SPORT0_RXSEC_A              ((volatile uint32_t *)REG_SPORT0_RXSEC_A)               /* SPORT0 Half SPORT 'A' Rx Buffer (Secondary) Register */
#define pREG_SPORT0_CTL_B                ((volatile uint32_t *)REG_SPORT0_CTL_B)                 /* SPORT0 Half SPORT 'B' Control Register */
#define pREG_SPORT0_DIV_B                ((volatile uint32_t *)REG_SPORT0_DIV_B)                 /* SPORT0 Half SPORT 'B' Divisor Register */
#define pREG_SPORT0_MCTL_B               ((volatile uint32_t *)REG_SPORT0_MCTL_B)                /* SPORT0 Half SPORT 'B' Multi-channel Control Register */
#define pREG_SPORT0_CS0_B                ((volatile uint32_t *)REG_SPORT0_CS0_B)                 /* SPORT0 Half SPORT 'B' Multi-channel 0-31 Select Register */
#define pREG_SPORT0_CS1_B                ((volatile uint32_t *)REG_SPORT0_CS1_B)                 /* SPORT0 Half SPORT 'B' Multi-channel 32-63 Select Register */
#define pREG_SPORT0_CS2_B                ((volatile uint32_t *)REG_SPORT0_CS2_B)                 /* SPORT0 Half SPORT 'B' Multichannel 64-95 Select Register */
#define pREG_SPORT0_CS3_B                ((volatile uint32_t *)REG_SPORT0_CS3_B)                 /* SPORT0 Half SPORT 'B' Multichannel 96-127 Select Register */
#define pREG_SPORT0_ERR_B                ((volatile uint32_t *)REG_SPORT0_ERR_B)                 /* SPORT0 Half SPORT 'B' Error Register */
#define pREG_SPORT0_MSTAT_B              ((volatile uint32_t *)REG_SPORT0_MSTAT_B)               /* SPORT0 Half SPORT 'B' Multi-channel Status Register */
#define pREG_SPORT0_CTL2_B               ((volatile uint32_t *)REG_SPORT0_CTL2_B)                /* SPORT0 Half SPORT 'B' Control 2 Register */
#define pREG_SPORT0_TXPRI_B              ((volatile uint32_t *)REG_SPORT0_TXPRI_B)               /* SPORT0 Half SPORT 'B' Tx Buffer (Primary) Register */
#define pREG_SPORT0_RXPRI_B              ((volatile uint32_t *)REG_SPORT0_RXPRI_B)               /* SPORT0 Half SPORT 'B' Rx Buffer (Primary) Register */
#define pREG_SPORT0_TXSEC_B              ((volatile uint32_t *)REG_SPORT0_TXSEC_B)               /* SPORT0 Half SPORT 'B' Tx Buffer (Secondary) Register */
#define pREG_SPORT0_RXSEC_B              ((volatile uint32_t *)REG_SPORT0_RXSEC_B)               /* SPORT0 Half SPORT 'B' Rx Buffer (Secondary) Register */

/* =========================================================================
       SPORT1
   ========================================================================= */
#define pREG_SPORT1_CTL_A                ((volatile uint32_t *)REG_SPORT1_CTL_A)                 /* SPORT1 Half SPORT 'A' Control Register */
#define pREG_SPORT1_DIV_A                ((volatile uint32_t *)REG_SPORT1_DIV_A)                 /* SPORT1 Half SPORT 'A' Divisor Register */
#define pREG_SPORT1_MCTL_A               ((volatile uint32_t *)REG_SPORT1_MCTL_A)                /* SPORT1 Half SPORT 'A' Multi-channel Control Register */
#define pREG_SPORT1_CS0_A                ((volatile uint32_t *)REG_SPORT1_CS0_A)                 /* SPORT1 Half SPORT 'A' Multi-channel 0-31 Select Register */
#define pREG_SPORT1_CS1_A                ((volatile uint32_t *)REG_SPORT1_CS1_A)                 /* SPORT1 Half SPORT 'A' Multi-channel 32-63 Select Register */
#define pREG_SPORT1_CS2_A                ((volatile uint32_t *)REG_SPORT1_CS2_A)                 /* SPORT1 Half SPORT 'A' Multi-channel 64-95 Select Register */
#define pREG_SPORT1_CS3_A                ((volatile uint32_t *)REG_SPORT1_CS3_A)                 /* SPORT1 Half SPORT 'A' Multi-channel 96-127 Select Register */
#define pREG_SPORT1_ERR_A                ((volatile uint32_t *)REG_SPORT1_ERR_A)                 /* SPORT1 Half SPORT 'A' Error Register */
#define pREG_SPORT1_MSTAT_A              ((volatile uint32_t *)REG_SPORT1_MSTAT_A)               /* SPORT1 Half SPORT 'A' Multi-channel Status Register */
#define pREG_SPORT1_CTL2_A               ((volatile uint32_t *)REG_SPORT1_CTL2_A)                /* SPORT1 Half SPORT 'A' Control 2 Register */
#define pREG_SPORT1_TXPRI_A              ((volatile uint32_t *)REG_SPORT1_TXPRI_A)               /* SPORT1 Half SPORT 'A' Tx Buffer (Primary) Register */
#define pREG_SPORT1_RXPRI_A              ((volatile uint32_t *)REG_SPORT1_RXPRI_A)               /* SPORT1 Half SPORT 'A' Rx Buffer (Primary) Register */
#define pREG_SPORT1_TXSEC_A              ((volatile uint32_t *)REG_SPORT1_TXSEC_A)               /* SPORT1 Half SPORT 'A' Tx Buffer (Secondary) Register */
#define pREG_SPORT1_RXSEC_A              ((volatile uint32_t *)REG_SPORT1_RXSEC_A)               /* SPORT1 Half SPORT 'A' Rx Buffer (Secondary) Register */
#define pREG_SPORT1_CTL_B                ((volatile uint32_t *)REG_SPORT1_CTL_B)                 /* SPORT1 Half SPORT 'B' Control Register */
#define pREG_SPORT1_DIV_B                ((volatile uint32_t *)REG_SPORT1_DIV_B)                 /* SPORT1 Half SPORT 'B' Divisor Register */
#define pREG_SPORT1_MCTL_B               ((volatile uint32_t *)REG_SPORT1_MCTL_B)                /* SPORT1 Half SPORT 'B' Multi-channel Control Register */
#define pREG_SPORT1_CS0_B                ((volatile uint32_t *)REG_SPORT1_CS0_B)                 /* SPORT1 Half SPORT 'B' Multi-channel 0-31 Select Register */
#define pREG_SPORT1_CS1_B                ((volatile uint32_t *)REG_SPORT1_CS1_B)                 /* SPORT1 Half SPORT 'B' Multi-channel 32-63 Select Register */
#define pREG_SPORT1_CS2_B                ((volatile uint32_t *)REG_SPORT1_CS2_B)                 /* SPORT1 Half SPORT 'B' Multichannel 64-95 Select Register */
#define pREG_SPORT1_CS3_B                ((volatile uint32_t *)REG_SPORT1_CS3_B)                 /* SPORT1 Half SPORT 'B' Multichannel 96-127 Select Register */
#define pREG_SPORT1_ERR_B                ((volatile uint32_t *)REG_SPORT1_ERR_B)                 /* SPORT1 Half SPORT 'B' Error Register */
#define pREG_SPORT1_MSTAT_B              ((volatile uint32_t *)REG_SPORT1_MSTAT_B)               /* SPORT1 Half SPORT 'B' Multi-channel Status Register */
#define pREG_SPORT1_CTL2_B               ((volatile uint32_t *)REG_SPORT1_CTL2_B)                /* SPORT1 Half SPORT 'B' Control 2 Register */
#define pREG_SPORT1_TXPRI_B              ((volatile uint32_t *)REG_SPORT1_TXPRI_B)               /* SPORT1 Half SPORT 'B' Tx Buffer (Primary) Register */
#define pREG_SPORT1_RXPRI_B              ((volatile uint32_t *)REG_SPORT1_RXPRI_B)               /* SPORT1 Half SPORT 'B' Rx Buffer (Primary) Register */
#define pREG_SPORT1_TXSEC_B              ((volatile uint32_t *)REG_SPORT1_TXSEC_B)               /* SPORT1 Half SPORT 'B' Tx Buffer (Secondary) Register */
#define pREG_SPORT1_RXSEC_B              ((volatile uint32_t *)REG_SPORT1_RXSEC_B)               /* SPORT1 Half SPORT 'B' Rx Buffer (Secondary) Register */

/* =========================================================================
       SPORT2
   ========================================================================= */
#define pREG_SPORT2_CTL_A                ((volatile uint32_t *)REG_SPORT2_CTL_A)                 /* SPORT2 Half SPORT 'A' Control Register */
#define pREG_SPORT2_DIV_A                ((volatile uint32_t *)REG_SPORT2_DIV_A)                 /* SPORT2 Half SPORT 'A' Divisor Register */
#define pREG_SPORT2_MCTL_A               ((volatile uint32_t *)REG_SPORT2_MCTL_A)                /* SPORT2 Half SPORT 'A' Multi-channel Control Register */
#define pREG_SPORT2_CS0_A                ((volatile uint32_t *)REG_SPORT2_CS0_A)                 /* SPORT2 Half SPORT 'A' Multi-channel 0-31 Select Register */
#define pREG_SPORT2_CS1_A                ((volatile uint32_t *)REG_SPORT2_CS1_A)                 /* SPORT2 Half SPORT 'A' Multi-channel 32-63 Select Register */
#define pREG_SPORT2_CS2_A                ((volatile uint32_t *)REG_SPORT2_CS2_A)                 /* SPORT2 Half SPORT 'A' Multi-channel 64-95 Select Register */
#define pREG_SPORT2_CS3_A                ((volatile uint32_t *)REG_SPORT2_CS3_A)                 /* SPORT2 Half SPORT 'A' Multi-channel 96-127 Select Register */
#define pREG_SPORT2_ERR_A                ((volatile uint32_t *)REG_SPORT2_ERR_A)                 /* SPORT2 Half SPORT 'A' Error Register */
#define pREG_SPORT2_MSTAT_A              ((volatile uint32_t *)REG_SPORT2_MSTAT_A)               /* SPORT2 Half SPORT 'A' Multi-channel Status Register */
#define pREG_SPORT2_CTL2_A               ((volatile uint32_t *)REG_SPORT2_CTL2_A)                /* SPORT2 Half SPORT 'A' Control 2 Register */
#define pREG_SPORT2_TXPRI_A              ((volatile uint32_t *)REG_SPORT2_TXPRI_A)               /* SPORT2 Half SPORT 'A' Tx Buffer (Primary) Register */
#define pREG_SPORT2_RXPRI_A              ((volatile uint32_t *)REG_SPORT2_RXPRI_A)               /* SPORT2 Half SPORT 'A' Rx Buffer (Primary) Register */
#define pREG_SPORT2_TXSEC_A              ((volatile uint32_t *)REG_SPORT2_TXSEC_A)               /* SPORT2 Half SPORT 'A' Tx Buffer (Secondary) Register */
#define pREG_SPORT2_RXSEC_A              ((volatile uint32_t *)REG_SPORT2_RXSEC_A)               /* SPORT2 Half SPORT 'A' Rx Buffer (Secondary) Register */
#define pREG_SPORT2_CTL_B                ((volatile uint32_t *)REG_SPORT2_CTL_B)                 /* SPORT2 Half SPORT 'B' Control Register */
#define pREG_SPORT2_DIV_B                ((volatile uint32_t *)REG_SPORT2_DIV_B)                 /* SPORT2 Half SPORT 'B' Divisor Register */
#define pREG_SPORT2_MCTL_B               ((volatile uint32_t *)REG_SPORT2_MCTL_B)                /* SPORT2 Half SPORT 'B' Multi-channel Control Register */
#define pREG_SPORT2_CS0_B                ((volatile uint32_t *)REG_SPORT2_CS0_B)                 /* SPORT2 Half SPORT 'B' Multi-channel 0-31 Select Register */
#define pREG_SPORT2_CS1_B                ((volatile uint32_t *)REG_SPORT2_CS1_B)                 /* SPORT2 Half SPORT 'B' Multi-channel 32-63 Select Register */
#define pREG_SPORT2_CS2_B                ((volatile uint32_t *)REG_SPORT2_CS2_B)                 /* SPORT2 Half SPORT 'B' Multichannel 64-95 Select Register */
#define pREG_SPORT2_CS3_B                ((volatile uint32_t *)REG_SPORT2_CS3_B)                 /* SPORT2 Half SPORT 'B' Multichannel 96-127 Select Register */
#define pREG_SPORT2_ERR_B                ((volatile uint32_t *)REG_SPORT2_ERR_B)                 /* SPORT2 Half SPORT 'B' Error Register */
#define pREG_SPORT2_MSTAT_B              ((volatile uint32_t *)REG_SPORT2_MSTAT_B)               /* SPORT2 Half SPORT 'B' Multi-channel Status Register */
#define pREG_SPORT2_CTL2_B               ((volatile uint32_t *)REG_SPORT2_CTL2_B)                /* SPORT2 Half SPORT 'B' Control 2 Register */
#define pREG_SPORT2_TXPRI_B              ((volatile uint32_t *)REG_SPORT2_TXPRI_B)               /* SPORT2 Half SPORT 'B' Tx Buffer (Primary) Register */
#define pREG_SPORT2_RXPRI_B              ((volatile uint32_t *)REG_SPORT2_RXPRI_B)               /* SPORT2 Half SPORT 'B' Rx Buffer (Primary) Register */
#define pREG_SPORT2_TXSEC_B              ((volatile uint32_t *)REG_SPORT2_TXSEC_B)               /* SPORT2 Half SPORT 'B' Tx Buffer (Secondary) Register */
#define pREG_SPORT2_RXSEC_B              ((volatile uint32_t *)REG_SPORT2_RXSEC_B)               /* SPORT2 Half SPORT 'B' Rx Buffer (Secondary) Register */


/* =========================================================================
       SPI0
   ========================================================================= */
#define pREG_SPI0_CTL                    ((volatile uint32_t *)REG_SPI0_CTL)                     /* SPI0 Control Register */
#define pREG_SPI0_RXCTL                  ((volatile uint32_t *)REG_SPI0_RXCTL)                   /* SPI0 Receive Control Register */
#define pREG_SPI0_TXCTL                  ((volatile uint32_t *)REG_SPI0_TXCTL)                   /* SPI0 Transmit Control Register */
#define pREG_SPI0_CLK                    ((volatile uint32_t *)REG_SPI0_CLK)                     /* SPI0 Clock Rate Register */
#define pREG_SPI0_DLY                    ((volatile uint32_t *)REG_SPI0_DLY)                     /* SPI0 Delay Register */
#define pREG_SPI0_SLVSEL                 ((volatile uint32_t *)REG_SPI0_SLVSEL)                  /* SPI0 Slave Select Register */
#define pREG_SPI0_RWC                    ((volatile uint32_t *)REG_SPI0_RWC)                     /* SPI0 Received Word Count Register */
#define pREG_SPI0_RWCR                   ((volatile uint32_t *)REG_SPI0_RWCR)                    /* SPI0 Received Word Count Reload Register */
#define pREG_SPI0_TWC                    ((volatile uint32_t *)REG_SPI0_TWC)                     /* SPI0 Transmitted Word Count Register */
#define pREG_SPI0_TWCR                   ((volatile uint32_t *)REG_SPI0_TWCR)                    /* SPI0 Transmitted Word Count Reload Register */
#define pREG_SPI0_IMSK                   ((volatile uint32_t *)REG_SPI0_IMSK)                    /* SPI0 Interrupt Mask Register */
#define pREG_SPI0_IMSK_CLR               ((volatile uint32_t *)REG_SPI0_IMSK_CLR)                /* SPI0 Interrupt Mask Clear Register */
#define pREG_SPI0_IMSK_SET               ((volatile uint32_t *)REG_SPI0_IMSK_SET)                /* SPI0 Interrupt Mask Set Register */
#define pREG_SPI0_STAT                   ((volatile uint32_t *)REG_SPI0_STAT)                    /* SPI0 Status Register */
#define pREG_SPI0_ILAT                   ((volatile uint32_t *)REG_SPI0_ILAT)                    /* SPI0 Masked Interrupt Condition Register */
#define pREG_SPI0_ILAT_CLR               ((volatile uint32_t *)REG_SPI0_ILAT_CLR)                /* SPI0 Masked Interrupt Clear Register */
#define pREG_SPI0_RFIFO                  ((volatile uint32_t *)REG_SPI0_RFIFO)                   /* SPI0 Receive FIFO Data Register */
#define pREG_SPI0_TFIFO                  ((volatile uint32_t *)REG_SPI0_TFIFO)                   /* SPI0 Transmit FIFO Data Register */

/* =========================================================================
       SPI1
   ========================================================================= */
#define pREG_SPI1_CTL                    ((volatile uint32_t *)REG_SPI1_CTL)                     /* SPI1 Control Register */
#define pREG_SPI1_RXCTL                  ((volatile uint32_t *)REG_SPI1_RXCTL)                   /* SPI1 Receive Control Register */
#define pREG_SPI1_TXCTL                  ((volatile uint32_t *)REG_SPI1_TXCTL)                   /* SPI1 Transmit Control Register */
#define pREG_SPI1_CLK                    ((volatile uint32_t *)REG_SPI1_CLK)                     /* SPI1 Clock Rate Register */
#define pREG_SPI1_DLY                    ((volatile uint32_t *)REG_SPI1_DLY)                     /* SPI1 Delay Register */
#define pREG_SPI1_SLVSEL                 ((volatile uint32_t *)REG_SPI1_SLVSEL)                  /* SPI1 Slave Select Register */
#define pREG_SPI1_RWC                    ((volatile uint32_t *)REG_SPI1_RWC)                     /* SPI1 Received Word Count Register */
#define pREG_SPI1_RWCR                   ((volatile uint32_t *)REG_SPI1_RWCR)                    /* SPI1 Received Word Count Reload Register */
#define pREG_SPI1_TWC                    ((volatile uint32_t *)REG_SPI1_TWC)                     /* SPI1 Transmitted Word Count Register */
#define pREG_SPI1_TWCR                   ((volatile uint32_t *)REG_SPI1_TWCR)                    /* SPI1 Transmitted Word Count Reload Register */
#define pREG_SPI1_IMSK                   ((volatile uint32_t *)REG_SPI1_IMSK)                    /* SPI1 Interrupt Mask Register */
#define pREG_SPI1_IMSK_CLR               ((volatile uint32_t *)REG_SPI1_IMSK_CLR)                /* SPI1 Interrupt Mask Clear Register */
#define pREG_SPI1_IMSK_SET               ((volatile uint32_t *)REG_SPI1_IMSK_SET)                /* SPI1 Interrupt Mask Set Register */
#define pREG_SPI1_STAT                   ((volatile uint32_t *)REG_SPI1_STAT)                    /* SPI1 Status Register */
#define pREG_SPI1_ILAT                   ((volatile uint32_t *)REG_SPI1_ILAT)                    /* SPI1 Masked Interrupt Condition Register */
#define pREG_SPI1_ILAT_CLR               ((volatile uint32_t *)REG_SPI1_ILAT_CLR)                /* SPI1 Masked Interrupt Clear Register */
#define pREG_SPI1_RFIFO                  ((volatile uint32_t *)REG_SPI1_RFIFO)                   /* SPI1 Receive FIFO Data Register */
#define pREG_SPI1_TFIFO                  ((volatile uint32_t *)REG_SPI1_TFIFO)                   /* SPI1 Transmit FIFO Data Register */


/* =========================================================================
       DMA0
   ========================================================================= */
#define pREG_DMA0_DSCPTR_NXT             ((void * volatile *)REG_DMA0_DSCPTR_NXT)                /* DMA0 Pointer to Next Initial Descriptor */
#define pREG_DMA0_ADDRSTART              ((void * volatile *)REG_DMA0_ADDRSTART)                 /* DMA0 Start Address of Current Buffer */
#define pREG_DMA0_CFG                    ((volatile uint32_t *)REG_DMA0_CFG)                     /* DMA0 Configuration Register */
#define pREG_DMA0_XCNT                   ((volatile uint32_t *)REG_DMA0_XCNT)                    /* DMA0 Inner Loop Count Start Value */
#define pREG_DMA0_XMOD                   ((volatile  int32_t *)REG_DMA0_XMOD)                    /* DMA0 Inner Loop Address Increment */
#define pREG_DMA0_YCNT                   ((volatile uint32_t *)REG_DMA0_YCNT)                    /* DMA0 Outer Loop Count Start Value (2D only) */
#define pREG_DMA0_YMOD                   ((volatile  int32_t *)REG_DMA0_YMOD)                    /* DMA0 Outer Loop Address Increment (2D only) */
#define pREG_DMA0_DSCPTR_CUR             ((void * volatile *)REG_DMA0_DSCPTR_CUR)                /* DMA0 Current Descriptor Pointer */
#define pREG_DMA0_DSCPTR_PRV             ((void * volatile *)REG_DMA0_DSCPTR_PRV)                /* DMA0 Previous Initial Descriptor Pointer */
#define pREG_DMA0_ADDR_CUR               ((void * volatile *)REG_DMA0_ADDR_CUR)                  /* DMA0 Current Address */
#define pREG_DMA0_STAT                   ((volatile uint32_t *)REG_DMA0_STAT)                    /* DMA0 Status Register */
#define pREG_DMA0_XCNT_CUR               ((volatile uint32_t *)REG_DMA0_XCNT_CUR)                /* DMA0 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA0_YCNT_CUR               ((volatile uint32_t *)REG_DMA0_YCNT_CUR)                /* DMA0 Current Row Count (2D only) */
#define pREG_DMA0_BWLCNT                 ((volatile uint32_t *)REG_DMA0_BWLCNT)                  /* DMA0 Bandwidth Limit Count */
#define pREG_DMA0_BWLCNT_CUR             ((volatile uint32_t *)REG_DMA0_BWLCNT_CUR)              /* DMA0 Bandwidth Limit Count Current */
#define pREG_DMA0_BWMCNT                 ((volatile uint32_t *)REG_DMA0_BWMCNT)                  /* DMA0 Bandwidth Monitor Count */
#define pREG_DMA0_BWMCNT_CUR             ((volatile uint32_t *)REG_DMA0_BWMCNT_CUR)              /* DMA0 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA1
   ========================================================================= */
#define pREG_DMA1_DSCPTR_NXT             ((void * volatile *)REG_DMA1_DSCPTR_NXT)                /* DMA1 Pointer to Next Initial Descriptor */
#define pREG_DMA1_ADDRSTART              ((void * volatile *)REG_DMA1_ADDRSTART)                 /* DMA1 Start Address of Current Buffer */
#define pREG_DMA1_CFG                    ((volatile uint32_t *)REG_DMA1_CFG)                     /* DMA1 Configuration Register */
#define pREG_DMA1_XCNT                   ((volatile uint32_t *)REG_DMA1_XCNT)                    /* DMA1 Inner Loop Count Start Value */
#define pREG_DMA1_XMOD                   ((volatile  int32_t *)REG_DMA1_XMOD)                    /* DMA1 Inner Loop Address Increment */
#define pREG_DMA1_YCNT                   ((volatile uint32_t *)REG_DMA1_YCNT)                    /* DMA1 Outer Loop Count Start Value (2D only) */
#define pREG_DMA1_YMOD                   ((volatile  int32_t *)REG_DMA1_YMOD)                    /* DMA1 Outer Loop Address Increment (2D only) */
#define pREG_DMA1_DSCPTR_CUR             ((void * volatile *)REG_DMA1_DSCPTR_CUR)                /* DMA1 Current Descriptor Pointer */
#define pREG_DMA1_DSCPTR_PRV             ((void * volatile *)REG_DMA1_DSCPTR_PRV)                /* DMA1 Previous Initial Descriptor Pointer */
#define pREG_DMA1_ADDR_CUR               ((void * volatile *)REG_DMA1_ADDR_CUR)                  /* DMA1 Current Address */
#define pREG_DMA1_STAT                   ((volatile uint32_t *)REG_DMA1_STAT)                    /* DMA1 Status Register */
#define pREG_DMA1_XCNT_CUR               ((volatile uint32_t *)REG_DMA1_XCNT_CUR)                /* DMA1 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA1_YCNT_CUR               ((volatile uint32_t *)REG_DMA1_YCNT_CUR)                /* DMA1 Current Row Count (2D only) */
#define pREG_DMA1_BWLCNT                 ((volatile uint32_t *)REG_DMA1_BWLCNT)                  /* DMA1 Bandwidth Limit Count */
#define pREG_DMA1_BWLCNT_CUR             ((volatile uint32_t *)REG_DMA1_BWLCNT_CUR)              /* DMA1 Bandwidth Limit Count Current */
#define pREG_DMA1_BWMCNT                 ((volatile uint32_t *)REG_DMA1_BWMCNT)                  /* DMA1 Bandwidth Monitor Count */
#define pREG_DMA1_BWMCNT_CUR             ((volatile uint32_t *)REG_DMA1_BWMCNT_CUR)              /* DMA1 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA2
   ========================================================================= */
#define pREG_DMA2_DSCPTR_NXT             ((void * volatile *)REG_DMA2_DSCPTR_NXT)                /* DMA2 Pointer to Next Initial Descriptor */
#define pREG_DMA2_ADDRSTART              ((void * volatile *)REG_DMA2_ADDRSTART)                 /* DMA2 Start Address of Current Buffer */
#define pREG_DMA2_CFG                    ((volatile uint32_t *)REG_DMA2_CFG)                     /* DMA2 Configuration Register */
#define pREG_DMA2_XCNT                   ((volatile uint32_t *)REG_DMA2_XCNT)                    /* DMA2 Inner Loop Count Start Value */
#define pREG_DMA2_XMOD                   ((volatile  int32_t *)REG_DMA2_XMOD)                    /* DMA2 Inner Loop Address Increment */
#define pREG_DMA2_YCNT                   ((volatile uint32_t *)REG_DMA2_YCNT)                    /* DMA2 Outer Loop Count Start Value (2D only) */
#define pREG_DMA2_YMOD                   ((volatile  int32_t *)REG_DMA2_YMOD)                    /* DMA2 Outer Loop Address Increment (2D only) */
#define pREG_DMA2_DSCPTR_CUR             ((void * volatile *)REG_DMA2_DSCPTR_CUR)                /* DMA2 Current Descriptor Pointer */
#define pREG_DMA2_DSCPTR_PRV             ((void * volatile *)REG_DMA2_DSCPTR_PRV)                /* DMA2 Previous Initial Descriptor Pointer */
#define pREG_DMA2_ADDR_CUR               ((void * volatile *)REG_DMA2_ADDR_CUR)                  /* DMA2 Current Address */
#define pREG_DMA2_STAT                   ((volatile uint32_t *)REG_DMA2_STAT)                    /* DMA2 Status Register */
#define pREG_DMA2_XCNT_CUR               ((volatile uint32_t *)REG_DMA2_XCNT_CUR)                /* DMA2 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA2_YCNT_CUR               ((volatile uint32_t *)REG_DMA2_YCNT_CUR)                /* DMA2 Current Row Count (2D only) */
#define pREG_DMA2_BWLCNT                 ((volatile uint32_t *)REG_DMA2_BWLCNT)                  /* DMA2 Bandwidth Limit Count */
#define pREG_DMA2_BWLCNT_CUR             ((volatile uint32_t *)REG_DMA2_BWLCNT_CUR)              /* DMA2 Bandwidth Limit Count Current */
#define pREG_DMA2_BWMCNT                 ((volatile uint32_t *)REG_DMA2_BWMCNT)                  /* DMA2 Bandwidth Monitor Count */
#define pREG_DMA2_BWMCNT_CUR             ((volatile uint32_t *)REG_DMA2_BWMCNT_CUR)              /* DMA2 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA3
   ========================================================================= */
#define pREG_DMA3_DSCPTR_NXT             ((void * volatile *)REG_DMA3_DSCPTR_NXT)                /* DMA3 Pointer to Next Initial Descriptor */
#define pREG_DMA3_ADDRSTART              ((void * volatile *)REG_DMA3_ADDRSTART)                 /* DMA3 Start Address of Current Buffer */
#define pREG_DMA3_CFG                    ((volatile uint32_t *)REG_DMA3_CFG)                     /* DMA3 Configuration Register */
#define pREG_DMA3_XCNT                   ((volatile uint32_t *)REG_DMA3_XCNT)                    /* DMA3 Inner Loop Count Start Value */
#define pREG_DMA3_XMOD                   ((volatile  int32_t *)REG_DMA3_XMOD)                    /* DMA3 Inner Loop Address Increment */
#define pREG_DMA3_YCNT                   ((volatile uint32_t *)REG_DMA3_YCNT)                    /* DMA3 Outer Loop Count Start Value (2D only) */
#define pREG_DMA3_YMOD                   ((volatile  int32_t *)REG_DMA3_YMOD)                    /* DMA3 Outer Loop Address Increment (2D only) */
#define pREG_DMA3_DSCPTR_CUR             ((void * volatile *)REG_DMA3_DSCPTR_CUR)                /* DMA3 Current Descriptor Pointer */
#define pREG_DMA3_DSCPTR_PRV             ((void * volatile *)REG_DMA3_DSCPTR_PRV)                /* DMA3 Previous Initial Descriptor Pointer */
#define pREG_DMA3_ADDR_CUR               ((void * volatile *)REG_DMA3_ADDR_CUR)                  /* DMA3 Current Address */
#define pREG_DMA3_STAT                   ((volatile uint32_t *)REG_DMA3_STAT)                    /* DMA3 Status Register */
#define pREG_DMA3_XCNT_CUR               ((volatile uint32_t *)REG_DMA3_XCNT_CUR)                /* DMA3 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA3_YCNT_CUR               ((volatile uint32_t *)REG_DMA3_YCNT_CUR)                /* DMA3 Current Row Count (2D only) */
#define pREG_DMA3_BWLCNT                 ((volatile uint32_t *)REG_DMA3_BWLCNT)                  /* DMA3 Bandwidth Limit Count */
#define pREG_DMA3_BWLCNT_CUR             ((volatile uint32_t *)REG_DMA3_BWLCNT_CUR)              /* DMA3 Bandwidth Limit Count Current */
#define pREG_DMA3_BWMCNT                 ((volatile uint32_t *)REG_DMA3_BWMCNT)                  /* DMA3 Bandwidth Monitor Count */
#define pREG_DMA3_BWMCNT_CUR             ((volatile uint32_t *)REG_DMA3_BWMCNT_CUR)              /* DMA3 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA4
   ========================================================================= */
#define pREG_DMA4_DSCPTR_NXT             ((void * volatile *)REG_DMA4_DSCPTR_NXT)                /* DMA4 Pointer to Next Initial Descriptor */
#define pREG_DMA4_ADDRSTART              ((void * volatile *)REG_DMA4_ADDRSTART)                 /* DMA4 Start Address of Current Buffer */
#define pREG_DMA4_CFG                    ((volatile uint32_t *)REG_DMA4_CFG)                     /* DMA4 Configuration Register */
#define pREG_DMA4_XCNT                   ((volatile uint32_t *)REG_DMA4_XCNT)                    /* DMA4 Inner Loop Count Start Value */
#define pREG_DMA4_XMOD                   ((volatile  int32_t *)REG_DMA4_XMOD)                    /* DMA4 Inner Loop Address Increment */
#define pREG_DMA4_YCNT                   ((volatile uint32_t *)REG_DMA4_YCNT)                    /* DMA4 Outer Loop Count Start Value (2D only) */
#define pREG_DMA4_YMOD                   ((volatile  int32_t *)REG_DMA4_YMOD)                    /* DMA4 Outer Loop Address Increment (2D only) */
#define pREG_DMA4_DSCPTR_CUR             ((void * volatile *)REG_DMA4_DSCPTR_CUR)                /* DMA4 Current Descriptor Pointer */
#define pREG_DMA4_DSCPTR_PRV             ((void * volatile *)REG_DMA4_DSCPTR_PRV)                /* DMA4 Previous Initial Descriptor Pointer */
#define pREG_DMA4_ADDR_CUR               ((void * volatile *)REG_DMA4_ADDR_CUR)                  /* DMA4 Current Address */
#define pREG_DMA4_STAT                   ((volatile uint32_t *)REG_DMA4_STAT)                    /* DMA4 Status Register */
#define pREG_DMA4_XCNT_CUR               ((volatile uint32_t *)REG_DMA4_XCNT_CUR)                /* DMA4 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA4_YCNT_CUR               ((volatile uint32_t *)REG_DMA4_YCNT_CUR)                /* DMA4 Current Row Count (2D only) */
#define pREG_DMA4_BWLCNT                 ((volatile uint32_t *)REG_DMA4_BWLCNT)                  /* DMA4 Bandwidth Limit Count */
#define pREG_DMA4_BWLCNT_CUR             ((volatile uint32_t *)REG_DMA4_BWLCNT_CUR)              /* DMA4 Bandwidth Limit Count Current */
#define pREG_DMA4_BWMCNT                 ((volatile uint32_t *)REG_DMA4_BWMCNT)                  /* DMA4 Bandwidth Monitor Count */
#define pREG_DMA4_BWMCNT_CUR             ((volatile uint32_t *)REG_DMA4_BWMCNT_CUR)              /* DMA4 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA5
   ========================================================================= */
#define pREG_DMA5_DSCPTR_NXT             ((void * volatile *)REG_DMA5_DSCPTR_NXT)                /* DMA5 Pointer to Next Initial Descriptor */
#define pREG_DMA5_ADDRSTART              ((void * volatile *)REG_DMA5_ADDRSTART)                 /* DMA5 Start Address of Current Buffer */
#define pREG_DMA5_CFG                    ((volatile uint32_t *)REG_DMA5_CFG)                     /* DMA5 Configuration Register */
#define pREG_DMA5_XCNT                   ((volatile uint32_t *)REG_DMA5_XCNT)                    /* DMA5 Inner Loop Count Start Value */
#define pREG_DMA5_XMOD                   ((volatile  int32_t *)REG_DMA5_XMOD)                    /* DMA5 Inner Loop Address Increment */
#define pREG_DMA5_YCNT                   ((volatile uint32_t *)REG_DMA5_YCNT)                    /* DMA5 Outer Loop Count Start Value (2D only) */
#define pREG_DMA5_YMOD                   ((volatile  int32_t *)REG_DMA5_YMOD)                    /* DMA5 Outer Loop Address Increment (2D only) */
#define pREG_DMA5_DSCPTR_CUR             ((void * volatile *)REG_DMA5_DSCPTR_CUR)                /* DMA5 Current Descriptor Pointer */
#define pREG_DMA5_DSCPTR_PRV             ((void * volatile *)REG_DMA5_DSCPTR_PRV)                /* DMA5 Previous Initial Descriptor Pointer */
#define pREG_DMA5_ADDR_CUR               ((void * volatile *)REG_DMA5_ADDR_CUR)                  /* DMA5 Current Address */
#define pREG_DMA5_STAT                   ((volatile uint32_t *)REG_DMA5_STAT)                    /* DMA5 Status Register */
#define pREG_DMA5_XCNT_CUR               ((volatile uint32_t *)REG_DMA5_XCNT_CUR)                /* DMA5 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA5_YCNT_CUR               ((volatile uint32_t *)REG_DMA5_YCNT_CUR)                /* DMA5 Current Row Count (2D only) */
#define pREG_DMA5_BWLCNT                 ((volatile uint32_t *)REG_DMA5_BWLCNT)                  /* DMA5 Bandwidth Limit Count */
#define pREG_DMA5_BWLCNT_CUR             ((volatile uint32_t *)REG_DMA5_BWLCNT_CUR)              /* DMA5 Bandwidth Limit Count Current */
#define pREG_DMA5_BWMCNT                 ((volatile uint32_t *)REG_DMA5_BWMCNT)                  /* DMA5 Bandwidth Monitor Count */
#define pREG_DMA5_BWMCNT_CUR             ((volatile uint32_t *)REG_DMA5_BWMCNT_CUR)              /* DMA5 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA6
   ========================================================================= */
#define pREG_DMA6_DSCPTR_NXT             ((void * volatile *)REG_DMA6_DSCPTR_NXT)                /* DMA6 Pointer to Next Initial Descriptor */
#define pREG_DMA6_ADDRSTART              ((void * volatile *)REG_DMA6_ADDRSTART)                 /* DMA6 Start Address of Current Buffer */
#define pREG_DMA6_CFG                    ((volatile uint32_t *)REG_DMA6_CFG)                     /* DMA6 Configuration Register */
#define pREG_DMA6_XCNT                   ((volatile uint32_t *)REG_DMA6_XCNT)                    /* DMA6 Inner Loop Count Start Value */
#define pREG_DMA6_XMOD                   ((volatile  int32_t *)REG_DMA6_XMOD)                    /* DMA6 Inner Loop Address Increment */
#define pREG_DMA6_YCNT                   ((volatile uint32_t *)REG_DMA6_YCNT)                    /* DMA6 Outer Loop Count Start Value (2D only) */
#define pREG_DMA6_YMOD                   ((volatile  int32_t *)REG_DMA6_YMOD)                    /* DMA6 Outer Loop Address Increment (2D only) */
#define pREG_DMA6_DSCPTR_CUR             ((void * volatile *)REG_DMA6_DSCPTR_CUR)                /* DMA6 Current Descriptor Pointer */
#define pREG_DMA6_DSCPTR_PRV             ((void * volatile *)REG_DMA6_DSCPTR_PRV)                /* DMA6 Previous Initial Descriptor Pointer */
#define pREG_DMA6_ADDR_CUR               ((void * volatile *)REG_DMA6_ADDR_CUR)                  /* DMA6 Current Address */
#define pREG_DMA6_STAT                   ((volatile uint32_t *)REG_DMA6_STAT)                    /* DMA6 Status Register */
#define pREG_DMA6_XCNT_CUR               ((volatile uint32_t *)REG_DMA6_XCNT_CUR)                /* DMA6 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA6_YCNT_CUR               ((volatile uint32_t *)REG_DMA6_YCNT_CUR)                /* DMA6 Current Row Count (2D only) */
#define pREG_DMA6_BWLCNT                 ((volatile uint32_t *)REG_DMA6_BWLCNT)                  /* DMA6 Bandwidth Limit Count */
#define pREG_DMA6_BWLCNT_CUR             ((volatile uint32_t *)REG_DMA6_BWLCNT_CUR)              /* DMA6 Bandwidth Limit Count Current */
#define pREG_DMA6_BWMCNT                 ((volatile uint32_t *)REG_DMA6_BWMCNT)                  /* DMA6 Bandwidth Monitor Count */
#define pREG_DMA6_BWMCNT_CUR             ((volatile uint32_t *)REG_DMA6_BWMCNT_CUR)              /* DMA6 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA7
   ========================================================================= */
#define pREG_DMA7_DSCPTR_NXT             ((void * volatile *)REG_DMA7_DSCPTR_NXT)                /* DMA7 Pointer to Next Initial Descriptor */
#define pREG_DMA7_ADDRSTART              ((void * volatile *)REG_DMA7_ADDRSTART)                 /* DMA7 Start Address of Current Buffer */
#define pREG_DMA7_CFG                    ((volatile uint32_t *)REG_DMA7_CFG)                     /* DMA7 Configuration Register */
#define pREG_DMA7_XCNT                   ((volatile uint32_t *)REG_DMA7_XCNT)                    /* DMA7 Inner Loop Count Start Value */
#define pREG_DMA7_XMOD                   ((volatile  int32_t *)REG_DMA7_XMOD)                    /* DMA7 Inner Loop Address Increment */
#define pREG_DMA7_YCNT                   ((volatile uint32_t *)REG_DMA7_YCNT)                    /* DMA7 Outer Loop Count Start Value (2D only) */
#define pREG_DMA7_YMOD                   ((volatile  int32_t *)REG_DMA7_YMOD)                    /* DMA7 Outer Loop Address Increment (2D only) */
#define pREG_DMA7_DSCPTR_CUR             ((void * volatile *)REG_DMA7_DSCPTR_CUR)                /* DMA7 Current Descriptor Pointer */
#define pREG_DMA7_DSCPTR_PRV             ((void * volatile *)REG_DMA7_DSCPTR_PRV)                /* DMA7 Previous Initial Descriptor Pointer */
#define pREG_DMA7_ADDR_CUR               ((void * volatile *)REG_DMA7_ADDR_CUR)                  /* DMA7 Current Address */
#define pREG_DMA7_STAT                   ((volatile uint32_t *)REG_DMA7_STAT)                    /* DMA7 Status Register */
#define pREG_DMA7_XCNT_CUR               ((volatile uint32_t *)REG_DMA7_XCNT_CUR)                /* DMA7 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA7_YCNT_CUR               ((volatile uint32_t *)REG_DMA7_YCNT_CUR)                /* DMA7 Current Row Count (2D only) */
#define pREG_DMA7_BWLCNT                 ((volatile uint32_t *)REG_DMA7_BWLCNT)                  /* DMA7 Bandwidth Limit Count */
#define pREG_DMA7_BWLCNT_CUR             ((volatile uint32_t *)REG_DMA7_BWLCNT_CUR)              /* DMA7 Bandwidth Limit Count Current */
#define pREG_DMA7_BWMCNT                 ((volatile uint32_t *)REG_DMA7_BWMCNT)                  /* DMA7 Bandwidth Monitor Count */
#define pREG_DMA7_BWMCNT_CUR             ((volatile uint32_t *)REG_DMA7_BWMCNT_CUR)              /* DMA7 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA8
   ========================================================================= */
#define pREG_DMA8_DSCPTR_NXT             ((void * volatile *)REG_DMA8_DSCPTR_NXT)                /* DMA8 Pointer to Next Initial Descriptor */
#define pREG_DMA8_ADDRSTART              ((void * volatile *)REG_DMA8_ADDRSTART)                 /* DMA8 Start Address of Current Buffer */
#define pREG_DMA8_CFG                    ((volatile uint32_t *)REG_DMA8_CFG)                     /* DMA8 Configuration Register */
#define pREG_DMA8_XCNT                   ((volatile uint32_t *)REG_DMA8_XCNT)                    /* DMA8 Inner Loop Count Start Value */
#define pREG_DMA8_XMOD                   ((volatile  int32_t *)REG_DMA8_XMOD)                    /* DMA8 Inner Loop Address Increment */
#define pREG_DMA8_YCNT                   ((volatile uint32_t *)REG_DMA8_YCNT)                    /* DMA8 Outer Loop Count Start Value (2D only) */
#define pREG_DMA8_YMOD                   ((volatile  int32_t *)REG_DMA8_YMOD)                    /* DMA8 Outer Loop Address Increment (2D only) */
#define pREG_DMA8_DSCPTR_CUR             ((void * volatile *)REG_DMA8_DSCPTR_CUR)                /* DMA8 Current Descriptor Pointer */
#define pREG_DMA8_DSCPTR_PRV             ((void * volatile *)REG_DMA8_DSCPTR_PRV)                /* DMA8 Previous Initial Descriptor Pointer */
#define pREG_DMA8_ADDR_CUR               ((void * volatile *)REG_DMA8_ADDR_CUR)                  /* DMA8 Current Address */
#define pREG_DMA8_STAT                   ((volatile uint32_t *)REG_DMA8_STAT)                    /* DMA8 Status Register */
#define pREG_DMA8_XCNT_CUR               ((volatile uint32_t *)REG_DMA8_XCNT_CUR)                /* DMA8 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA8_YCNT_CUR               ((volatile uint32_t *)REG_DMA8_YCNT_CUR)                /* DMA8 Current Row Count (2D only) */
#define pREG_DMA8_BWLCNT                 ((volatile uint32_t *)REG_DMA8_BWLCNT)                  /* DMA8 Bandwidth Limit Count */
#define pREG_DMA8_BWLCNT_CUR             ((volatile uint32_t *)REG_DMA8_BWLCNT_CUR)              /* DMA8 Bandwidth Limit Count Current */
#define pREG_DMA8_BWMCNT                 ((volatile uint32_t *)REG_DMA8_BWMCNT)                  /* DMA8 Bandwidth Monitor Count */
#define pREG_DMA8_BWMCNT_CUR             ((volatile uint32_t *)REG_DMA8_BWMCNT_CUR)              /* DMA8 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA9
   ========================================================================= */
#define pREG_DMA9_DSCPTR_NXT             ((void * volatile *)REG_DMA9_DSCPTR_NXT)                /* DMA9 Pointer to Next Initial Descriptor */
#define pREG_DMA9_ADDRSTART              ((void * volatile *)REG_DMA9_ADDRSTART)                 /* DMA9 Start Address of Current Buffer */
#define pREG_DMA9_CFG                    ((volatile uint32_t *)REG_DMA9_CFG)                     /* DMA9 Configuration Register */
#define pREG_DMA9_XCNT                   ((volatile uint32_t *)REG_DMA9_XCNT)                    /* DMA9 Inner Loop Count Start Value */
#define pREG_DMA9_XMOD                   ((volatile  int32_t *)REG_DMA9_XMOD)                    /* DMA9 Inner Loop Address Increment */
#define pREG_DMA9_YCNT                   ((volatile uint32_t *)REG_DMA9_YCNT)                    /* DMA9 Outer Loop Count Start Value (2D only) */
#define pREG_DMA9_YMOD                   ((volatile  int32_t *)REG_DMA9_YMOD)                    /* DMA9 Outer Loop Address Increment (2D only) */
#define pREG_DMA9_DSCPTR_CUR             ((void * volatile *)REG_DMA9_DSCPTR_CUR)                /* DMA9 Current Descriptor Pointer */
#define pREG_DMA9_DSCPTR_PRV             ((void * volatile *)REG_DMA9_DSCPTR_PRV)                /* DMA9 Previous Initial Descriptor Pointer */
#define pREG_DMA9_ADDR_CUR               ((void * volatile *)REG_DMA9_ADDR_CUR)                  /* DMA9 Current Address */
#define pREG_DMA9_STAT                   ((volatile uint32_t *)REG_DMA9_STAT)                    /* DMA9 Status Register */
#define pREG_DMA9_XCNT_CUR               ((volatile uint32_t *)REG_DMA9_XCNT_CUR)                /* DMA9 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA9_YCNT_CUR               ((volatile uint32_t *)REG_DMA9_YCNT_CUR)                /* DMA9 Current Row Count (2D only) */
#define pREG_DMA9_BWLCNT                 ((volatile uint32_t *)REG_DMA9_BWLCNT)                  /* DMA9 Bandwidth Limit Count */
#define pREG_DMA9_BWLCNT_CUR             ((volatile uint32_t *)REG_DMA9_BWLCNT_CUR)              /* DMA9 Bandwidth Limit Count Current */
#define pREG_DMA9_BWMCNT                 ((volatile uint32_t *)REG_DMA9_BWMCNT)                  /* DMA9 Bandwidth Monitor Count */
#define pREG_DMA9_BWMCNT_CUR             ((volatile uint32_t *)REG_DMA9_BWMCNT_CUR)              /* DMA9 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA10
   ========================================================================= */
#define pREG_DMA10_DSCPTR_NXT            ((void * volatile *)REG_DMA10_DSCPTR_NXT)               /* DMA10 Pointer to Next Initial Descriptor */
#define pREG_DMA10_ADDRSTART             ((void * volatile *)REG_DMA10_ADDRSTART)                /* DMA10 Start Address of Current Buffer */
#define pREG_DMA10_CFG                   ((volatile uint32_t *)REG_DMA10_CFG)                    /* DMA10 Configuration Register */
#define pREG_DMA10_XCNT                  ((volatile uint32_t *)REG_DMA10_XCNT)                   /* DMA10 Inner Loop Count Start Value */
#define pREG_DMA10_XMOD                  ((volatile  int32_t *)REG_DMA10_XMOD)                   /* DMA10 Inner Loop Address Increment */
#define pREG_DMA10_YCNT                  ((volatile uint32_t *)REG_DMA10_YCNT)                   /* DMA10 Outer Loop Count Start Value (2D only) */
#define pREG_DMA10_YMOD                  ((volatile  int32_t *)REG_DMA10_YMOD)                   /* DMA10 Outer Loop Address Increment (2D only) */
#define pREG_DMA10_DSCPTR_CUR            ((void * volatile *)REG_DMA10_DSCPTR_CUR)               /* DMA10 Current Descriptor Pointer */
#define pREG_DMA10_DSCPTR_PRV            ((void * volatile *)REG_DMA10_DSCPTR_PRV)               /* DMA10 Previous Initial Descriptor Pointer */
#define pREG_DMA10_ADDR_CUR              ((void * volatile *)REG_DMA10_ADDR_CUR)                 /* DMA10 Current Address */
#define pREG_DMA10_STAT                  ((volatile uint32_t *)REG_DMA10_STAT)                   /* DMA10 Status Register */
#define pREG_DMA10_XCNT_CUR              ((volatile uint32_t *)REG_DMA10_XCNT_CUR)               /* DMA10 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA10_YCNT_CUR              ((volatile uint32_t *)REG_DMA10_YCNT_CUR)               /* DMA10 Current Row Count (2D only) */
#define pREG_DMA10_BWLCNT                ((volatile uint32_t *)REG_DMA10_BWLCNT)                 /* DMA10 Bandwidth Limit Count */
#define pREG_DMA10_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA10_BWLCNT_CUR)             /* DMA10 Bandwidth Limit Count Current */
#define pREG_DMA10_BWMCNT                ((volatile uint32_t *)REG_DMA10_BWMCNT)                 /* DMA10 Bandwidth Monitor Count */
#define pREG_DMA10_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA10_BWMCNT_CUR)             /* DMA10 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA11
   ========================================================================= */
#define pREG_DMA11_DSCPTR_NXT            ((void * volatile *)REG_DMA11_DSCPTR_NXT)               /* DMA11 Pointer to Next Initial Descriptor */
#define pREG_DMA11_ADDRSTART             ((void * volatile *)REG_DMA11_ADDRSTART)                /* DMA11 Start Address of Current Buffer */
#define pREG_DMA11_CFG                   ((volatile uint32_t *)REG_DMA11_CFG)                    /* DMA11 Configuration Register */
#define pREG_DMA11_XCNT                  ((volatile uint32_t *)REG_DMA11_XCNT)                   /* DMA11 Inner Loop Count Start Value */
#define pREG_DMA11_XMOD                  ((volatile  int32_t *)REG_DMA11_XMOD)                   /* DMA11 Inner Loop Address Increment */
#define pREG_DMA11_YCNT                  ((volatile uint32_t *)REG_DMA11_YCNT)                   /* DMA11 Outer Loop Count Start Value (2D only) */
#define pREG_DMA11_YMOD                  ((volatile  int32_t *)REG_DMA11_YMOD)                   /* DMA11 Outer Loop Address Increment (2D only) */
#define pREG_DMA11_DSCPTR_CUR            ((void * volatile *)REG_DMA11_DSCPTR_CUR)               /* DMA11 Current Descriptor Pointer */
#define pREG_DMA11_DSCPTR_PRV            ((void * volatile *)REG_DMA11_DSCPTR_PRV)               /* DMA11 Previous Initial Descriptor Pointer */
#define pREG_DMA11_ADDR_CUR              ((void * volatile *)REG_DMA11_ADDR_CUR)                 /* DMA11 Current Address */
#define pREG_DMA11_STAT                  ((volatile uint32_t *)REG_DMA11_STAT)                   /* DMA11 Status Register */
#define pREG_DMA11_XCNT_CUR              ((volatile uint32_t *)REG_DMA11_XCNT_CUR)               /* DMA11 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA11_YCNT_CUR              ((volatile uint32_t *)REG_DMA11_YCNT_CUR)               /* DMA11 Current Row Count (2D only) */
#define pREG_DMA11_BWLCNT                ((volatile uint32_t *)REG_DMA11_BWLCNT)                 /* DMA11 Bandwidth Limit Count */
#define pREG_DMA11_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA11_BWLCNT_CUR)             /* DMA11 Bandwidth Limit Count Current */
#define pREG_DMA11_BWMCNT                ((volatile uint32_t *)REG_DMA11_BWMCNT)                 /* DMA11 Bandwidth Monitor Count */
#define pREG_DMA11_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA11_BWMCNT_CUR)             /* DMA11 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA12
   ========================================================================= */
#define pREG_DMA12_DSCPTR_NXT            ((void * volatile *)REG_DMA12_DSCPTR_NXT)               /* DMA12 Pointer to Next Initial Descriptor */
#define pREG_DMA12_ADDRSTART             ((void * volatile *)REG_DMA12_ADDRSTART)                /* DMA12 Start Address of Current Buffer */
#define pREG_DMA12_CFG                   ((volatile uint32_t *)REG_DMA12_CFG)                    /* DMA12 Configuration Register */
#define pREG_DMA12_XCNT                  ((volatile uint32_t *)REG_DMA12_XCNT)                   /* DMA12 Inner Loop Count Start Value */
#define pREG_DMA12_XMOD                  ((volatile  int32_t *)REG_DMA12_XMOD)                   /* DMA12 Inner Loop Address Increment */
#define pREG_DMA12_YCNT                  ((volatile uint32_t *)REG_DMA12_YCNT)                   /* DMA12 Outer Loop Count Start Value (2D only) */
#define pREG_DMA12_YMOD                  ((volatile  int32_t *)REG_DMA12_YMOD)                   /* DMA12 Outer Loop Address Increment (2D only) */
#define pREG_DMA12_DSCPTR_CUR            ((void * volatile *)REG_DMA12_DSCPTR_CUR)               /* DMA12 Current Descriptor Pointer */
#define pREG_DMA12_DSCPTR_PRV            ((void * volatile *)REG_DMA12_DSCPTR_PRV)               /* DMA12 Previous Initial Descriptor Pointer */
#define pREG_DMA12_ADDR_CUR              ((void * volatile *)REG_DMA12_ADDR_CUR)                 /* DMA12 Current Address */
#define pREG_DMA12_STAT                  ((volatile uint32_t *)REG_DMA12_STAT)                   /* DMA12 Status Register */
#define pREG_DMA12_XCNT_CUR              ((volatile uint32_t *)REG_DMA12_XCNT_CUR)               /* DMA12 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA12_YCNT_CUR              ((volatile uint32_t *)REG_DMA12_YCNT_CUR)               /* DMA12 Current Row Count (2D only) */
#define pREG_DMA12_BWLCNT                ((volatile uint32_t *)REG_DMA12_BWLCNT)                 /* DMA12 Bandwidth Limit Count */
#define pREG_DMA12_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA12_BWLCNT_CUR)             /* DMA12 Bandwidth Limit Count Current */
#define pREG_DMA12_BWMCNT                ((volatile uint32_t *)REG_DMA12_BWMCNT)                 /* DMA12 Bandwidth Monitor Count */
#define pREG_DMA12_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA12_BWMCNT_CUR)             /* DMA12 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA13
   ========================================================================= */
#define pREG_DMA13_DSCPTR_NXT            ((void * volatile *)REG_DMA13_DSCPTR_NXT)               /* DMA13 Pointer to Next Initial Descriptor */
#define pREG_DMA13_ADDRSTART             ((void * volatile *)REG_DMA13_ADDRSTART)                /* DMA13 Start Address of Current Buffer */
#define pREG_DMA13_CFG                   ((volatile uint32_t *)REG_DMA13_CFG)                    /* DMA13 Configuration Register */
#define pREG_DMA13_XCNT                  ((volatile uint32_t *)REG_DMA13_XCNT)                   /* DMA13 Inner Loop Count Start Value */
#define pREG_DMA13_XMOD                  ((volatile  int32_t *)REG_DMA13_XMOD)                   /* DMA13 Inner Loop Address Increment */
#define pREG_DMA13_YCNT                  ((volatile uint32_t *)REG_DMA13_YCNT)                   /* DMA13 Outer Loop Count Start Value (2D only) */
#define pREG_DMA13_YMOD                  ((volatile  int32_t *)REG_DMA13_YMOD)                   /* DMA13 Outer Loop Address Increment (2D only) */
#define pREG_DMA13_DSCPTR_CUR            ((void * volatile *)REG_DMA13_DSCPTR_CUR)               /* DMA13 Current Descriptor Pointer */
#define pREG_DMA13_DSCPTR_PRV            ((void * volatile *)REG_DMA13_DSCPTR_PRV)               /* DMA13 Previous Initial Descriptor Pointer */
#define pREG_DMA13_ADDR_CUR              ((void * volatile *)REG_DMA13_ADDR_CUR)                 /* DMA13 Current Address */
#define pREG_DMA13_STAT                  ((volatile uint32_t *)REG_DMA13_STAT)                   /* DMA13 Status Register */
#define pREG_DMA13_XCNT_CUR              ((volatile uint32_t *)REG_DMA13_XCNT_CUR)               /* DMA13 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA13_YCNT_CUR              ((volatile uint32_t *)REG_DMA13_YCNT_CUR)               /* DMA13 Current Row Count (2D only) */
#define pREG_DMA13_BWLCNT                ((volatile uint32_t *)REG_DMA13_BWLCNT)                 /* DMA13 Bandwidth Limit Count */
#define pREG_DMA13_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA13_BWLCNT_CUR)             /* DMA13 Bandwidth Limit Count Current */
#define pREG_DMA13_BWMCNT                ((volatile uint32_t *)REG_DMA13_BWMCNT)                 /* DMA13 Bandwidth Monitor Count */
#define pREG_DMA13_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA13_BWMCNT_CUR)             /* DMA13 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA14
   ========================================================================= */
#define pREG_DMA14_DSCPTR_NXT            ((void * volatile *)REG_DMA14_DSCPTR_NXT)               /* DMA14 Pointer to Next Initial Descriptor */
#define pREG_DMA14_ADDRSTART             ((void * volatile *)REG_DMA14_ADDRSTART)                /* DMA14 Start Address of Current Buffer */
#define pREG_DMA14_CFG                   ((volatile uint32_t *)REG_DMA14_CFG)                    /* DMA14 Configuration Register */
#define pREG_DMA14_XCNT                  ((volatile uint32_t *)REG_DMA14_XCNT)                   /* DMA14 Inner Loop Count Start Value */
#define pREG_DMA14_XMOD                  ((volatile  int32_t *)REG_DMA14_XMOD)                   /* DMA14 Inner Loop Address Increment */
#define pREG_DMA14_YCNT                  ((volatile uint32_t *)REG_DMA14_YCNT)                   /* DMA14 Outer Loop Count Start Value (2D only) */
#define pREG_DMA14_YMOD                  ((volatile  int32_t *)REG_DMA14_YMOD)                   /* DMA14 Outer Loop Address Increment (2D only) */
#define pREG_DMA14_DSCPTR_CUR            ((void * volatile *)REG_DMA14_DSCPTR_CUR)               /* DMA14 Current Descriptor Pointer */
#define pREG_DMA14_DSCPTR_PRV            ((void * volatile *)REG_DMA14_DSCPTR_PRV)               /* DMA14 Previous Initial Descriptor Pointer */
#define pREG_DMA14_ADDR_CUR              ((void * volatile *)REG_DMA14_ADDR_CUR)                 /* DMA14 Current Address */
#define pREG_DMA14_STAT                  ((volatile uint32_t *)REG_DMA14_STAT)                   /* DMA14 Status Register */
#define pREG_DMA14_XCNT_CUR              ((volatile uint32_t *)REG_DMA14_XCNT_CUR)               /* DMA14 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA14_YCNT_CUR              ((volatile uint32_t *)REG_DMA14_YCNT_CUR)               /* DMA14 Current Row Count (2D only) */
#define pREG_DMA14_BWLCNT                ((volatile uint32_t *)REG_DMA14_BWLCNT)                 /* DMA14 Bandwidth Limit Count */
#define pREG_DMA14_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA14_BWLCNT_CUR)             /* DMA14 Bandwidth Limit Count Current */
#define pREG_DMA14_BWMCNT                ((volatile uint32_t *)REG_DMA14_BWMCNT)                 /* DMA14 Bandwidth Monitor Count */
#define pREG_DMA14_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA14_BWMCNT_CUR)             /* DMA14 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA15
   ========================================================================= */
#define pREG_DMA15_DSCPTR_NXT            ((void * volatile *)REG_DMA15_DSCPTR_NXT)               /* DMA15 Pointer to Next Initial Descriptor */
#define pREG_DMA15_ADDRSTART             ((void * volatile *)REG_DMA15_ADDRSTART)                /* DMA15 Start Address of Current Buffer */
#define pREG_DMA15_CFG                   ((volatile uint32_t *)REG_DMA15_CFG)                    /* DMA15 Configuration Register */
#define pREG_DMA15_XCNT                  ((volatile uint32_t *)REG_DMA15_XCNT)                   /* DMA15 Inner Loop Count Start Value */
#define pREG_DMA15_XMOD                  ((volatile  int32_t *)REG_DMA15_XMOD)                   /* DMA15 Inner Loop Address Increment */
#define pREG_DMA15_YCNT                  ((volatile uint32_t *)REG_DMA15_YCNT)                   /* DMA15 Outer Loop Count Start Value (2D only) */
#define pREG_DMA15_YMOD                  ((volatile  int32_t *)REG_DMA15_YMOD)                   /* DMA15 Outer Loop Address Increment (2D only) */
#define pREG_DMA15_DSCPTR_CUR            ((void * volatile *)REG_DMA15_DSCPTR_CUR)               /* DMA15 Current Descriptor Pointer */
#define pREG_DMA15_DSCPTR_PRV            ((void * volatile *)REG_DMA15_DSCPTR_PRV)               /* DMA15 Previous Initial Descriptor Pointer */
#define pREG_DMA15_ADDR_CUR              ((void * volatile *)REG_DMA15_ADDR_CUR)                 /* DMA15 Current Address */
#define pREG_DMA15_STAT                  ((volatile uint32_t *)REG_DMA15_STAT)                   /* DMA15 Status Register */
#define pREG_DMA15_XCNT_CUR              ((volatile uint32_t *)REG_DMA15_XCNT_CUR)               /* DMA15 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA15_YCNT_CUR              ((volatile uint32_t *)REG_DMA15_YCNT_CUR)               /* DMA15 Current Row Count (2D only) */
#define pREG_DMA15_BWLCNT                ((volatile uint32_t *)REG_DMA15_BWLCNT)                 /* DMA15 Bandwidth Limit Count */
#define pREG_DMA15_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA15_BWLCNT_CUR)             /* DMA15 Bandwidth Limit Count Current */
#define pREG_DMA15_BWMCNT                ((volatile uint32_t *)REG_DMA15_BWMCNT)                 /* DMA15 Bandwidth Monitor Count */
#define pREG_DMA15_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA15_BWMCNT_CUR)             /* DMA15 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA16
   ========================================================================= */
#define pREG_DMA16_DSCPTR_NXT            ((void * volatile *)REG_DMA16_DSCPTR_NXT)               /* DMA16 Pointer to Next Initial Descriptor */
#define pREG_DMA16_ADDRSTART             ((void * volatile *)REG_DMA16_ADDRSTART)                /* DMA16 Start Address of Current Buffer */
#define pREG_DMA16_CFG                   ((volatile uint32_t *)REG_DMA16_CFG)                    /* DMA16 Configuration Register */
#define pREG_DMA16_XCNT                  ((volatile uint32_t *)REG_DMA16_XCNT)                   /* DMA16 Inner Loop Count Start Value */
#define pREG_DMA16_XMOD                  ((volatile  int32_t *)REG_DMA16_XMOD)                   /* DMA16 Inner Loop Address Increment */
#define pREG_DMA16_YCNT                  ((volatile uint32_t *)REG_DMA16_YCNT)                   /* DMA16 Outer Loop Count Start Value (2D only) */
#define pREG_DMA16_YMOD                  ((volatile  int32_t *)REG_DMA16_YMOD)                   /* DMA16 Outer Loop Address Increment (2D only) */
#define pREG_DMA16_DSCPTR_CUR            ((void * volatile *)REG_DMA16_DSCPTR_CUR)               /* DMA16 Current Descriptor Pointer */
#define pREG_DMA16_DSCPTR_PRV            ((void * volatile *)REG_DMA16_DSCPTR_PRV)               /* DMA16 Previous Initial Descriptor Pointer */
#define pREG_DMA16_ADDR_CUR              ((void * volatile *)REG_DMA16_ADDR_CUR)                 /* DMA16 Current Address */
#define pREG_DMA16_STAT                  ((volatile uint32_t *)REG_DMA16_STAT)                   /* DMA16 Status Register */
#define pREG_DMA16_XCNT_CUR              ((volatile uint32_t *)REG_DMA16_XCNT_CUR)               /* DMA16 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA16_YCNT_CUR              ((volatile uint32_t *)REG_DMA16_YCNT_CUR)               /* DMA16 Current Row Count (2D only) */
#define pREG_DMA16_BWLCNT                ((volatile uint32_t *)REG_DMA16_BWLCNT)                 /* DMA16 Bandwidth Limit Count */
#define pREG_DMA16_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA16_BWLCNT_CUR)             /* DMA16 Bandwidth Limit Count Current */
#define pREG_DMA16_BWMCNT                ((volatile uint32_t *)REG_DMA16_BWMCNT)                 /* DMA16 Bandwidth Monitor Count */
#define pREG_DMA16_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA16_BWMCNT_CUR)             /* DMA16 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA17
   ========================================================================= */
#define pREG_DMA17_DSCPTR_NXT            ((void * volatile *)REG_DMA17_DSCPTR_NXT)               /* DMA17 Pointer to Next Initial Descriptor */
#define pREG_DMA17_ADDRSTART             ((void * volatile *)REG_DMA17_ADDRSTART)                /* DMA17 Start Address of Current Buffer */
#define pREG_DMA17_CFG                   ((volatile uint32_t *)REG_DMA17_CFG)                    /* DMA17 Configuration Register */
#define pREG_DMA17_XCNT                  ((volatile uint32_t *)REG_DMA17_XCNT)                   /* DMA17 Inner Loop Count Start Value */
#define pREG_DMA17_XMOD                  ((volatile  int32_t *)REG_DMA17_XMOD)                   /* DMA17 Inner Loop Address Increment */
#define pREG_DMA17_YCNT                  ((volatile uint32_t *)REG_DMA17_YCNT)                   /* DMA17 Outer Loop Count Start Value (2D only) */
#define pREG_DMA17_YMOD                  ((volatile  int32_t *)REG_DMA17_YMOD)                   /* DMA17 Outer Loop Address Increment (2D only) */
#define pREG_DMA17_DSCPTR_CUR            ((void * volatile *)REG_DMA17_DSCPTR_CUR)               /* DMA17 Current Descriptor Pointer */
#define pREG_DMA17_DSCPTR_PRV            ((void * volatile *)REG_DMA17_DSCPTR_PRV)               /* DMA17 Previous Initial Descriptor Pointer */
#define pREG_DMA17_ADDR_CUR              ((void * volatile *)REG_DMA17_ADDR_CUR)                 /* DMA17 Current Address */
#define pREG_DMA17_STAT                  ((volatile uint32_t *)REG_DMA17_STAT)                   /* DMA17 Status Register */
#define pREG_DMA17_XCNT_CUR              ((volatile uint32_t *)REG_DMA17_XCNT_CUR)               /* DMA17 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA17_YCNT_CUR              ((volatile uint32_t *)REG_DMA17_YCNT_CUR)               /* DMA17 Current Row Count (2D only) */
#define pREG_DMA17_BWLCNT                ((volatile uint32_t *)REG_DMA17_BWLCNT)                 /* DMA17 Bandwidth Limit Count */
#define pREG_DMA17_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA17_BWLCNT_CUR)             /* DMA17 Bandwidth Limit Count Current */
#define pREG_DMA17_BWMCNT                ((volatile uint32_t *)REG_DMA17_BWMCNT)                 /* DMA17 Bandwidth Monitor Count */
#define pREG_DMA17_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA17_BWMCNT_CUR)             /* DMA17 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA18
   ========================================================================= */
#define pREG_DMA18_DSCPTR_NXT            ((void * volatile *)REG_DMA18_DSCPTR_NXT)               /* DMA18 Pointer to Next Initial Descriptor */
#define pREG_DMA18_ADDRSTART             ((void * volatile *)REG_DMA18_ADDRSTART)                /* DMA18 Start Address of Current Buffer */
#define pREG_DMA18_CFG                   ((volatile uint32_t *)REG_DMA18_CFG)                    /* DMA18 Configuration Register */
#define pREG_DMA18_XCNT                  ((volatile uint32_t *)REG_DMA18_XCNT)                   /* DMA18 Inner Loop Count Start Value */
#define pREG_DMA18_XMOD                  ((volatile  int32_t *)REG_DMA18_XMOD)                   /* DMA18 Inner Loop Address Increment */
#define pREG_DMA18_YCNT                  ((volatile uint32_t *)REG_DMA18_YCNT)                   /* DMA18 Outer Loop Count Start Value (2D only) */
#define pREG_DMA18_YMOD                  ((volatile  int32_t *)REG_DMA18_YMOD)                   /* DMA18 Outer Loop Address Increment (2D only) */
#define pREG_DMA18_DSCPTR_CUR            ((void * volatile *)REG_DMA18_DSCPTR_CUR)               /* DMA18 Current Descriptor Pointer */
#define pREG_DMA18_DSCPTR_PRV            ((void * volatile *)REG_DMA18_DSCPTR_PRV)               /* DMA18 Previous Initial Descriptor Pointer */
#define pREG_DMA18_ADDR_CUR              ((void * volatile *)REG_DMA18_ADDR_CUR)                 /* DMA18 Current Address */
#define pREG_DMA18_STAT                  ((volatile uint32_t *)REG_DMA18_STAT)                   /* DMA18 Status Register */
#define pREG_DMA18_XCNT_CUR              ((volatile uint32_t *)REG_DMA18_XCNT_CUR)               /* DMA18 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA18_YCNT_CUR              ((volatile uint32_t *)REG_DMA18_YCNT_CUR)               /* DMA18 Current Row Count (2D only) */
#define pREG_DMA18_BWLCNT                ((volatile uint32_t *)REG_DMA18_BWLCNT)                 /* DMA18 Bandwidth Limit Count */
#define pREG_DMA18_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA18_BWLCNT_CUR)             /* DMA18 Bandwidth Limit Count Current */
#define pREG_DMA18_BWMCNT                ((volatile uint32_t *)REG_DMA18_BWMCNT)                 /* DMA18 Bandwidth Monitor Count */
#define pREG_DMA18_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA18_BWMCNT_CUR)             /* DMA18 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA19
   ========================================================================= */
#define pREG_DMA19_DSCPTR_NXT            ((void * volatile *)REG_DMA19_DSCPTR_NXT)               /* DMA19 Pointer to Next Initial Descriptor */
#define pREG_DMA19_ADDRSTART             ((void * volatile *)REG_DMA19_ADDRSTART)                /* DMA19 Start Address of Current Buffer */
#define pREG_DMA19_CFG                   ((volatile uint32_t *)REG_DMA19_CFG)                    /* DMA19 Configuration Register */
#define pREG_DMA19_XCNT                  ((volatile uint32_t *)REG_DMA19_XCNT)                   /* DMA19 Inner Loop Count Start Value */
#define pREG_DMA19_XMOD                  ((volatile  int32_t *)REG_DMA19_XMOD)                   /* DMA19 Inner Loop Address Increment */
#define pREG_DMA19_YCNT                  ((volatile uint32_t *)REG_DMA19_YCNT)                   /* DMA19 Outer Loop Count Start Value (2D only) */
#define pREG_DMA19_YMOD                  ((volatile  int32_t *)REG_DMA19_YMOD)                   /* DMA19 Outer Loop Address Increment (2D only) */
#define pREG_DMA19_DSCPTR_CUR            ((void * volatile *)REG_DMA19_DSCPTR_CUR)               /* DMA19 Current Descriptor Pointer */
#define pREG_DMA19_DSCPTR_PRV            ((void * volatile *)REG_DMA19_DSCPTR_PRV)               /* DMA19 Previous Initial Descriptor Pointer */
#define pREG_DMA19_ADDR_CUR              ((void * volatile *)REG_DMA19_ADDR_CUR)                 /* DMA19 Current Address */
#define pREG_DMA19_STAT                  ((volatile uint32_t *)REG_DMA19_STAT)                   /* DMA19 Status Register */
#define pREG_DMA19_XCNT_CUR              ((volatile uint32_t *)REG_DMA19_XCNT_CUR)               /* DMA19 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA19_YCNT_CUR              ((volatile uint32_t *)REG_DMA19_YCNT_CUR)               /* DMA19 Current Row Count (2D only) */
#define pREG_DMA19_BWLCNT                ((volatile uint32_t *)REG_DMA19_BWLCNT)                 /* DMA19 Bandwidth Limit Count */
#define pREG_DMA19_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA19_BWLCNT_CUR)             /* DMA19 Bandwidth Limit Count Current */
#define pREG_DMA19_BWMCNT                ((volatile uint32_t *)REG_DMA19_BWMCNT)                 /* DMA19 Bandwidth Monitor Count */
#define pREG_DMA19_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA19_BWMCNT_CUR)             /* DMA19 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA20
   ========================================================================= */
#define pREG_DMA20_DSCPTR_NXT            ((void * volatile *)REG_DMA20_DSCPTR_NXT)               /* DMA20 Pointer to Next Initial Descriptor */
#define pREG_DMA20_ADDRSTART             ((void * volatile *)REG_DMA20_ADDRSTART)                /* DMA20 Start Address of Current Buffer */
#define pREG_DMA20_CFG                   ((volatile uint32_t *)REG_DMA20_CFG)                    /* DMA20 Configuration Register */
#define pREG_DMA20_XCNT                  ((volatile uint32_t *)REG_DMA20_XCNT)                   /* DMA20 Inner Loop Count Start Value */
#define pREG_DMA20_XMOD                  ((volatile  int32_t *)REG_DMA20_XMOD)                   /* DMA20 Inner Loop Address Increment */
#define pREG_DMA20_YCNT                  ((volatile uint32_t *)REG_DMA20_YCNT)                   /* DMA20 Outer Loop Count Start Value (2D only) */
#define pREG_DMA20_YMOD                  ((volatile  int32_t *)REG_DMA20_YMOD)                   /* DMA20 Outer Loop Address Increment (2D only) */
#define pREG_DMA20_DSCPTR_CUR            ((void * volatile *)REG_DMA20_DSCPTR_CUR)               /* DMA20 Current Descriptor Pointer */
#define pREG_DMA20_DSCPTR_PRV            ((void * volatile *)REG_DMA20_DSCPTR_PRV)               /* DMA20 Previous Initial Descriptor Pointer */
#define pREG_DMA20_ADDR_CUR              ((void * volatile *)REG_DMA20_ADDR_CUR)                 /* DMA20 Current Address */
#define pREG_DMA20_STAT                  ((volatile uint32_t *)REG_DMA20_STAT)                   /* DMA20 Status Register */
#define pREG_DMA20_XCNT_CUR              ((volatile uint32_t *)REG_DMA20_XCNT_CUR)               /* DMA20 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA20_YCNT_CUR              ((volatile uint32_t *)REG_DMA20_YCNT_CUR)               /* DMA20 Current Row Count (2D only) */
#define pREG_DMA20_BWLCNT                ((volatile uint32_t *)REG_DMA20_BWLCNT)                 /* DMA20 Bandwidth Limit Count */
#define pREG_DMA20_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA20_BWLCNT_CUR)             /* DMA20 Bandwidth Limit Count Current */
#define pREG_DMA20_BWMCNT                ((volatile uint32_t *)REG_DMA20_BWMCNT)                 /* DMA20 Bandwidth Monitor Count */
#define pREG_DMA20_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA20_BWMCNT_CUR)             /* DMA20 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA21
   ========================================================================= */
#define pREG_DMA21_DSCPTR_NXT            ((void * volatile *)REG_DMA21_DSCPTR_NXT)               /* DMA21 Pointer to Next Initial Descriptor */
#define pREG_DMA21_ADDRSTART             ((void * volatile *)REG_DMA21_ADDRSTART)                /* DMA21 Start Address of Current Buffer */
#define pREG_DMA21_CFG                   ((volatile uint32_t *)REG_DMA21_CFG)                    /* DMA21 Configuration Register */
#define pREG_DMA21_XCNT                  ((volatile uint32_t *)REG_DMA21_XCNT)                   /* DMA21 Inner Loop Count Start Value */
#define pREG_DMA21_XMOD                  ((volatile  int32_t *)REG_DMA21_XMOD)                   /* DMA21 Inner Loop Address Increment */
#define pREG_DMA21_YCNT                  ((volatile uint32_t *)REG_DMA21_YCNT)                   /* DMA21 Outer Loop Count Start Value (2D only) */
#define pREG_DMA21_YMOD                  ((volatile  int32_t *)REG_DMA21_YMOD)                   /* DMA21 Outer Loop Address Increment (2D only) */
#define pREG_DMA21_DSCPTR_CUR            ((void * volatile *)REG_DMA21_DSCPTR_CUR)               /* DMA21 Current Descriptor Pointer */
#define pREG_DMA21_DSCPTR_PRV            ((void * volatile *)REG_DMA21_DSCPTR_PRV)               /* DMA21 Previous Initial Descriptor Pointer */
#define pREG_DMA21_ADDR_CUR              ((void * volatile *)REG_DMA21_ADDR_CUR)                 /* DMA21 Current Address */
#define pREG_DMA21_STAT                  ((volatile uint32_t *)REG_DMA21_STAT)                   /* DMA21 Status Register */
#define pREG_DMA21_XCNT_CUR              ((volatile uint32_t *)REG_DMA21_XCNT_CUR)               /* DMA21 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA21_YCNT_CUR              ((volatile uint32_t *)REG_DMA21_YCNT_CUR)               /* DMA21 Current Row Count (2D only) */
#define pREG_DMA21_BWLCNT                ((volatile uint32_t *)REG_DMA21_BWLCNT)                 /* DMA21 Bandwidth Limit Count */
#define pREG_DMA21_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA21_BWLCNT_CUR)             /* DMA21 Bandwidth Limit Count Current */
#define pREG_DMA21_BWMCNT                ((volatile uint32_t *)REG_DMA21_BWMCNT)                 /* DMA21 Bandwidth Monitor Count */
#define pREG_DMA21_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA21_BWMCNT_CUR)             /* DMA21 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA22
   ========================================================================= */
#define pREG_DMA22_DSCPTR_NXT            ((void * volatile *)REG_DMA22_DSCPTR_NXT)               /* DMA22 Pointer to Next Initial Descriptor */
#define pREG_DMA22_ADDRSTART             ((void * volatile *)REG_DMA22_ADDRSTART)                /* DMA22 Start Address of Current Buffer */
#define pREG_DMA22_CFG                   ((volatile uint32_t *)REG_DMA22_CFG)                    /* DMA22 Configuration Register */
#define pREG_DMA22_XCNT                  ((volatile uint32_t *)REG_DMA22_XCNT)                   /* DMA22 Inner Loop Count Start Value */
#define pREG_DMA22_XMOD                  ((volatile  int32_t *)REG_DMA22_XMOD)                   /* DMA22 Inner Loop Address Increment */
#define pREG_DMA22_YCNT                  ((volatile uint32_t *)REG_DMA22_YCNT)                   /* DMA22 Outer Loop Count Start Value (2D only) */
#define pREG_DMA22_YMOD                  ((volatile  int32_t *)REG_DMA22_YMOD)                   /* DMA22 Outer Loop Address Increment (2D only) */
#define pREG_DMA22_DSCPTR_CUR            ((void * volatile *)REG_DMA22_DSCPTR_CUR)               /* DMA22 Current Descriptor Pointer */
#define pREG_DMA22_DSCPTR_PRV            ((void * volatile *)REG_DMA22_DSCPTR_PRV)               /* DMA22 Previous Initial Descriptor Pointer */
#define pREG_DMA22_ADDR_CUR              ((void * volatile *)REG_DMA22_ADDR_CUR)                 /* DMA22 Current Address */
#define pREG_DMA22_STAT                  ((volatile uint32_t *)REG_DMA22_STAT)                   /* DMA22 Status Register */
#define pREG_DMA22_XCNT_CUR              ((volatile uint32_t *)REG_DMA22_XCNT_CUR)               /* DMA22 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA22_YCNT_CUR              ((volatile uint32_t *)REG_DMA22_YCNT_CUR)               /* DMA22 Current Row Count (2D only) */
#define pREG_DMA22_BWLCNT                ((volatile uint32_t *)REG_DMA22_BWLCNT)                 /* DMA22 Bandwidth Limit Count */
#define pREG_DMA22_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA22_BWLCNT_CUR)             /* DMA22 Bandwidth Limit Count Current */
#define pREG_DMA22_BWMCNT                ((volatile uint32_t *)REG_DMA22_BWMCNT)                 /* DMA22 Bandwidth Monitor Count */
#define pREG_DMA22_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA22_BWMCNT_CUR)             /* DMA22 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA23
   ========================================================================= */
#define pREG_DMA23_DSCPTR_NXT            ((void * volatile *)REG_DMA23_DSCPTR_NXT)               /* DMA23 Pointer to Next Initial Descriptor */
#define pREG_DMA23_ADDRSTART             ((void * volatile *)REG_DMA23_ADDRSTART)                /* DMA23 Start Address of Current Buffer */
#define pREG_DMA23_CFG                   ((volatile uint32_t *)REG_DMA23_CFG)                    /* DMA23 Configuration Register */
#define pREG_DMA23_XCNT                  ((volatile uint32_t *)REG_DMA23_XCNT)                   /* DMA23 Inner Loop Count Start Value */
#define pREG_DMA23_XMOD                  ((volatile  int32_t *)REG_DMA23_XMOD)                   /* DMA23 Inner Loop Address Increment */
#define pREG_DMA23_YCNT                  ((volatile uint32_t *)REG_DMA23_YCNT)                   /* DMA23 Outer Loop Count Start Value (2D only) */
#define pREG_DMA23_YMOD                  ((volatile  int32_t *)REG_DMA23_YMOD)                   /* DMA23 Outer Loop Address Increment (2D only) */
#define pREG_DMA23_DSCPTR_CUR            ((void * volatile *)REG_DMA23_DSCPTR_CUR)               /* DMA23 Current Descriptor Pointer */
#define pREG_DMA23_DSCPTR_PRV            ((void * volatile *)REG_DMA23_DSCPTR_PRV)               /* DMA23 Previous Initial Descriptor Pointer */
#define pREG_DMA23_ADDR_CUR              ((void * volatile *)REG_DMA23_ADDR_CUR)                 /* DMA23 Current Address */
#define pREG_DMA23_STAT                  ((volatile uint32_t *)REG_DMA23_STAT)                   /* DMA23 Status Register */
#define pREG_DMA23_XCNT_CUR              ((volatile uint32_t *)REG_DMA23_XCNT_CUR)               /* DMA23 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA23_YCNT_CUR              ((volatile uint32_t *)REG_DMA23_YCNT_CUR)               /* DMA23 Current Row Count (2D only) */
#define pREG_DMA23_BWLCNT                ((volatile uint32_t *)REG_DMA23_BWLCNT)                 /* DMA23 Bandwidth Limit Count */
#define pREG_DMA23_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA23_BWLCNT_CUR)             /* DMA23 Bandwidth Limit Count Current */
#define pREG_DMA23_BWMCNT                ((volatile uint32_t *)REG_DMA23_BWMCNT)                 /* DMA23 Bandwidth Monitor Count */
#define pREG_DMA23_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA23_BWMCNT_CUR)             /* DMA23 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA24
   ========================================================================= */
#define pREG_DMA24_DSCPTR_NXT            ((void * volatile *)REG_DMA24_DSCPTR_NXT)               /* DMA24 Pointer to Next Initial Descriptor */
#define pREG_DMA24_ADDRSTART             ((void * volatile *)REG_DMA24_ADDRSTART)                /* DMA24 Start Address of Current Buffer */
#define pREG_DMA24_CFG                   ((volatile uint32_t *)REG_DMA24_CFG)                    /* DMA24 Configuration Register */
#define pREG_DMA24_XCNT                  ((volatile uint32_t *)REG_DMA24_XCNT)                   /* DMA24 Inner Loop Count Start Value */
#define pREG_DMA24_XMOD                  ((volatile  int32_t *)REG_DMA24_XMOD)                   /* DMA24 Inner Loop Address Increment */
#define pREG_DMA24_YCNT                  ((volatile uint32_t *)REG_DMA24_YCNT)                   /* DMA24 Outer Loop Count Start Value (2D only) */
#define pREG_DMA24_YMOD                  ((volatile  int32_t *)REG_DMA24_YMOD)                   /* DMA24 Outer Loop Address Increment (2D only) */
#define pREG_DMA24_DSCPTR_CUR            ((void * volatile *)REG_DMA24_DSCPTR_CUR)               /* DMA24 Current Descriptor Pointer */
#define pREG_DMA24_DSCPTR_PRV            ((void * volatile *)REG_DMA24_DSCPTR_PRV)               /* DMA24 Previous Initial Descriptor Pointer */
#define pREG_DMA24_ADDR_CUR              ((void * volatile *)REG_DMA24_ADDR_CUR)                 /* DMA24 Current Address */
#define pREG_DMA24_STAT                  ((volatile uint32_t *)REG_DMA24_STAT)                   /* DMA24 Status Register */
#define pREG_DMA24_XCNT_CUR              ((volatile uint32_t *)REG_DMA24_XCNT_CUR)               /* DMA24 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA24_YCNT_CUR              ((volatile uint32_t *)REG_DMA24_YCNT_CUR)               /* DMA24 Current Row Count (2D only) */
#define pREG_DMA24_BWLCNT                ((volatile uint32_t *)REG_DMA24_BWLCNT)                 /* DMA24 Bandwidth Limit Count */
#define pREG_DMA24_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA24_BWLCNT_CUR)             /* DMA24 Bandwidth Limit Count Current */
#define pREG_DMA24_BWMCNT                ((volatile uint32_t *)REG_DMA24_BWMCNT)                 /* DMA24 Bandwidth Monitor Count */
#define pREG_DMA24_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA24_BWMCNT_CUR)             /* DMA24 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA25
   ========================================================================= */
#define pREG_DMA25_DSCPTR_NXT            ((void * volatile *)REG_DMA25_DSCPTR_NXT)               /* DMA25 Pointer to Next Initial Descriptor */
#define pREG_DMA25_ADDRSTART             ((void * volatile *)REG_DMA25_ADDRSTART)                /* DMA25 Start Address of Current Buffer */
#define pREG_DMA25_CFG                   ((volatile uint32_t *)REG_DMA25_CFG)                    /* DMA25 Configuration Register */
#define pREG_DMA25_XCNT                  ((volatile uint32_t *)REG_DMA25_XCNT)                   /* DMA25 Inner Loop Count Start Value */
#define pREG_DMA25_XMOD                  ((volatile  int32_t *)REG_DMA25_XMOD)                   /* DMA25 Inner Loop Address Increment */
#define pREG_DMA25_YCNT                  ((volatile uint32_t *)REG_DMA25_YCNT)                   /* DMA25 Outer Loop Count Start Value (2D only) */
#define pREG_DMA25_YMOD                  ((volatile  int32_t *)REG_DMA25_YMOD)                   /* DMA25 Outer Loop Address Increment (2D only) */
#define pREG_DMA25_DSCPTR_CUR            ((void * volatile *)REG_DMA25_DSCPTR_CUR)               /* DMA25 Current Descriptor Pointer */
#define pREG_DMA25_DSCPTR_PRV            ((void * volatile *)REG_DMA25_DSCPTR_PRV)               /* DMA25 Previous Initial Descriptor Pointer */
#define pREG_DMA25_ADDR_CUR              ((void * volatile *)REG_DMA25_ADDR_CUR)                 /* DMA25 Current Address */
#define pREG_DMA25_STAT                  ((volatile uint32_t *)REG_DMA25_STAT)                   /* DMA25 Status Register */
#define pREG_DMA25_XCNT_CUR              ((volatile uint32_t *)REG_DMA25_XCNT_CUR)               /* DMA25 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA25_YCNT_CUR              ((volatile uint32_t *)REG_DMA25_YCNT_CUR)               /* DMA25 Current Row Count (2D only) */
#define pREG_DMA25_BWLCNT                ((volatile uint32_t *)REG_DMA25_BWLCNT)                 /* DMA25 Bandwidth Limit Count */
#define pREG_DMA25_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA25_BWLCNT_CUR)             /* DMA25 Bandwidth Limit Count Current */
#define pREG_DMA25_BWMCNT                ((volatile uint32_t *)REG_DMA25_BWMCNT)                 /* DMA25 Bandwidth Monitor Count */
#define pREG_DMA25_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA25_BWMCNT_CUR)             /* DMA25 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA26
   ========================================================================= */
#define pREG_DMA26_DSCPTR_NXT            ((void * volatile *)REG_DMA26_DSCPTR_NXT)               /* DMA26 Pointer to Next Initial Descriptor */
#define pREG_DMA26_ADDRSTART             ((void * volatile *)REG_DMA26_ADDRSTART)                /* DMA26 Start Address of Current Buffer */
#define pREG_DMA26_CFG                   ((volatile uint32_t *)REG_DMA26_CFG)                    /* DMA26 Configuration Register */
#define pREG_DMA26_XCNT                  ((volatile uint32_t *)REG_DMA26_XCNT)                   /* DMA26 Inner Loop Count Start Value */
#define pREG_DMA26_XMOD                  ((volatile  int32_t *)REG_DMA26_XMOD)                   /* DMA26 Inner Loop Address Increment */
#define pREG_DMA26_YCNT                  ((volatile uint32_t *)REG_DMA26_YCNT)                   /* DMA26 Outer Loop Count Start Value (2D only) */
#define pREG_DMA26_YMOD                  ((volatile  int32_t *)REG_DMA26_YMOD)                   /* DMA26 Outer Loop Address Increment (2D only) */
#define pREG_DMA26_DSCPTR_CUR            ((void * volatile *)REG_DMA26_DSCPTR_CUR)               /* DMA26 Current Descriptor Pointer */
#define pREG_DMA26_DSCPTR_PRV            ((void * volatile *)REG_DMA26_DSCPTR_PRV)               /* DMA26 Previous Initial Descriptor Pointer */
#define pREG_DMA26_ADDR_CUR              ((void * volatile *)REG_DMA26_ADDR_CUR)                 /* DMA26 Current Address */
#define pREG_DMA26_STAT                  ((volatile uint32_t *)REG_DMA26_STAT)                   /* DMA26 Status Register */
#define pREG_DMA26_XCNT_CUR              ((volatile uint32_t *)REG_DMA26_XCNT_CUR)               /* DMA26 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA26_YCNT_CUR              ((volatile uint32_t *)REG_DMA26_YCNT_CUR)               /* DMA26 Current Row Count (2D only) */
#define pREG_DMA26_BWLCNT                ((volatile uint32_t *)REG_DMA26_BWLCNT)                 /* DMA26 Bandwidth Limit Count */
#define pREG_DMA26_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA26_BWLCNT_CUR)             /* DMA26 Bandwidth Limit Count Current */
#define pREG_DMA26_BWMCNT                ((volatile uint32_t *)REG_DMA26_BWMCNT)                 /* DMA26 Bandwidth Monitor Count */
#define pREG_DMA26_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA26_BWMCNT_CUR)             /* DMA26 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA27
   ========================================================================= */
#define pREG_DMA27_DSCPTR_NXT            ((void * volatile *)REG_DMA27_DSCPTR_NXT)               /* DMA27 Pointer to Next Initial Descriptor */
#define pREG_DMA27_ADDRSTART             ((void * volatile *)REG_DMA27_ADDRSTART)                /* DMA27 Start Address of Current Buffer */
#define pREG_DMA27_CFG                   ((volatile uint32_t *)REG_DMA27_CFG)                    /* DMA27 Configuration Register */
#define pREG_DMA27_XCNT                  ((volatile uint32_t *)REG_DMA27_XCNT)                   /* DMA27 Inner Loop Count Start Value */
#define pREG_DMA27_XMOD                  ((volatile  int32_t *)REG_DMA27_XMOD)                   /* DMA27 Inner Loop Address Increment */
#define pREG_DMA27_YCNT                  ((volatile uint32_t *)REG_DMA27_YCNT)                   /* DMA27 Outer Loop Count Start Value (2D only) */
#define pREG_DMA27_YMOD                  ((volatile  int32_t *)REG_DMA27_YMOD)                   /* DMA27 Outer Loop Address Increment (2D only) */
#define pREG_DMA27_DSCPTR_CUR            ((void * volatile *)REG_DMA27_DSCPTR_CUR)               /* DMA27 Current Descriptor Pointer */
#define pREG_DMA27_DSCPTR_PRV            ((void * volatile *)REG_DMA27_DSCPTR_PRV)               /* DMA27 Previous Initial Descriptor Pointer */
#define pREG_DMA27_ADDR_CUR              ((void * volatile *)REG_DMA27_ADDR_CUR)                 /* DMA27 Current Address */
#define pREG_DMA27_STAT                  ((volatile uint32_t *)REG_DMA27_STAT)                   /* DMA27 Status Register */
#define pREG_DMA27_XCNT_CUR              ((volatile uint32_t *)REG_DMA27_XCNT_CUR)               /* DMA27 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA27_YCNT_CUR              ((volatile uint32_t *)REG_DMA27_YCNT_CUR)               /* DMA27 Current Row Count (2D only) */
#define pREG_DMA27_BWLCNT                ((volatile uint32_t *)REG_DMA27_BWLCNT)                 /* DMA27 Bandwidth Limit Count */
#define pREG_DMA27_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA27_BWLCNT_CUR)             /* DMA27 Bandwidth Limit Count Current */
#define pREG_DMA27_BWMCNT                ((volatile uint32_t *)REG_DMA27_BWMCNT)                 /* DMA27 Bandwidth Monitor Count */
#define pREG_DMA27_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA27_BWMCNT_CUR)             /* DMA27 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA28
   ========================================================================= */
#define pREG_DMA28_DSCPTR_NXT            ((void * volatile *)REG_DMA28_DSCPTR_NXT)               /* DMA28 Pointer to Next Initial Descriptor */
#define pREG_DMA28_ADDRSTART             ((void * volatile *)REG_DMA28_ADDRSTART)                /* DMA28 Start Address of Current Buffer */
#define pREG_DMA28_CFG                   ((volatile uint32_t *)REG_DMA28_CFG)                    /* DMA28 Configuration Register */
#define pREG_DMA28_XCNT                  ((volatile uint32_t *)REG_DMA28_XCNT)                   /* DMA28 Inner Loop Count Start Value */
#define pREG_DMA28_XMOD                  ((volatile  int32_t *)REG_DMA28_XMOD)                   /* DMA28 Inner Loop Address Increment */
#define pREG_DMA28_YCNT                  ((volatile uint32_t *)REG_DMA28_YCNT)                   /* DMA28 Outer Loop Count Start Value (2D only) */
#define pREG_DMA28_YMOD                  ((volatile  int32_t *)REG_DMA28_YMOD)                   /* DMA28 Outer Loop Address Increment (2D only) */
#define pREG_DMA28_DSCPTR_CUR            ((void * volatile *)REG_DMA28_DSCPTR_CUR)               /* DMA28 Current Descriptor Pointer */
#define pREG_DMA28_DSCPTR_PRV            ((void * volatile *)REG_DMA28_DSCPTR_PRV)               /* DMA28 Previous Initial Descriptor Pointer */
#define pREG_DMA28_ADDR_CUR              ((void * volatile *)REG_DMA28_ADDR_CUR)                 /* DMA28 Current Address */
#define pREG_DMA28_STAT                  ((volatile uint32_t *)REG_DMA28_STAT)                   /* DMA28 Status Register */
#define pREG_DMA28_XCNT_CUR              ((volatile uint32_t *)REG_DMA28_XCNT_CUR)               /* DMA28 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA28_YCNT_CUR              ((volatile uint32_t *)REG_DMA28_YCNT_CUR)               /* DMA28 Current Row Count (2D only) */
#define pREG_DMA28_BWLCNT                ((volatile uint32_t *)REG_DMA28_BWLCNT)                 /* DMA28 Bandwidth Limit Count */
#define pREG_DMA28_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA28_BWLCNT_CUR)             /* DMA28 Bandwidth Limit Count Current */
#define pREG_DMA28_BWMCNT                ((volatile uint32_t *)REG_DMA28_BWMCNT)                 /* DMA28 Bandwidth Monitor Count */
#define pREG_DMA28_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA28_BWMCNT_CUR)             /* DMA28 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA29
   ========================================================================= */
#define pREG_DMA29_DSCPTR_NXT            ((void * volatile *)REG_DMA29_DSCPTR_NXT)               /* DMA29 Pointer to Next Initial Descriptor */
#define pREG_DMA29_ADDRSTART             ((void * volatile *)REG_DMA29_ADDRSTART)                /* DMA29 Start Address of Current Buffer */
#define pREG_DMA29_CFG                   ((volatile uint32_t *)REG_DMA29_CFG)                    /* DMA29 Configuration Register */
#define pREG_DMA29_XCNT                  ((volatile uint32_t *)REG_DMA29_XCNT)                   /* DMA29 Inner Loop Count Start Value */
#define pREG_DMA29_XMOD                  ((volatile  int32_t *)REG_DMA29_XMOD)                   /* DMA29 Inner Loop Address Increment */
#define pREG_DMA29_YCNT                  ((volatile uint32_t *)REG_DMA29_YCNT)                   /* DMA29 Outer Loop Count Start Value (2D only) */
#define pREG_DMA29_YMOD                  ((volatile  int32_t *)REG_DMA29_YMOD)                   /* DMA29 Outer Loop Address Increment (2D only) */
#define pREG_DMA29_DSCPTR_CUR            ((void * volatile *)REG_DMA29_DSCPTR_CUR)               /* DMA29 Current Descriptor Pointer */
#define pREG_DMA29_DSCPTR_PRV            ((void * volatile *)REG_DMA29_DSCPTR_PRV)               /* DMA29 Previous Initial Descriptor Pointer */
#define pREG_DMA29_ADDR_CUR              ((void * volatile *)REG_DMA29_ADDR_CUR)                 /* DMA29 Current Address */
#define pREG_DMA29_STAT                  ((volatile uint32_t *)REG_DMA29_STAT)                   /* DMA29 Status Register */
#define pREG_DMA29_XCNT_CUR              ((volatile uint32_t *)REG_DMA29_XCNT_CUR)               /* DMA29 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA29_YCNT_CUR              ((volatile uint32_t *)REG_DMA29_YCNT_CUR)               /* DMA29 Current Row Count (2D only) */
#define pREG_DMA29_BWLCNT                ((volatile uint32_t *)REG_DMA29_BWLCNT)                 /* DMA29 Bandwidth Limit Count */
#define pREG_DMA29_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA29_BWLCNT_CUR)             /* DMA29 Bandwidth Limit Count Current */
#define pREG_DMA29_BWMCNT                ((volatile uint32_t *)REG_DMA29_BWMCNT)                 /* DMA29 Bandwidth Monitor Count */
#define pREG_DMA29_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA29_BWMCNT_CUR)             /* DMA29 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA30
   ========================================================================= */
#define pREG_DMA30_DSCPTR_NXT            ((void * volatile *)REG_DMA30_DSCPTR_NXT)               /* DMA30 Pointer to Next Initial Descriptor */
#define pREG_DMA30_ADDRSTART             ((void * volatile *)REG_DMA30_ADDRSTART)                /* DMA30 Start Address of Current Buffer */
#define pREG_DMA30_CFG                   ((volatile uint32_t *)REG_DMA30_CFG)                    /* DMA30 Configuration Register */
#define pREG_DMA30_XCNT                  ((volatile uint32_t *)REG_DMA30_XCNT)                   /* DMA30 Inner Loop Count Start Value */
#define pREG_DMA30_XMOD                  ((volatile  int32_t *)REG_DMA30_XMOD)                   /* DMA30 Inner Loop Address Increment */
#define pREG_DMA30_YCNT                  ((volatile uint32_t *)REG_DMA30_YCNT)                   /* DMA30 Outer Loop Count Start Value (2D only) */
#define pREG_DMA30_YMOD                  ((volatile  int32_t *)REG_DMA30_YMOD)                   /* DMA30 Outer Loop Address Increment (2D only) */
#define pREG_DMA30_DSCPTR_CUR            ((void * volatile *)REG_DMA30_DSCPTR_CUR)               /* DMA30 Current Descriptor Pointer */
#define pREG_DMA30_DSCPTR_PRV            ((void * volatile *)REG_DMA30_DSCPTR_PRV)               /* DMA30 Previous Initial Descriptor Pointer */
#define pREG_DMA30_ADDR_CUR              ((void * volatile *)REG_DMA30_ADDR_CUR)                 /* DMA30 Current Address */
#define pREG_DMA30_STAT                  ((volatile uint32_t *)REG_DMA30_STAT)                   /* DMA30 Status Register */
#define pREG_DMA30_XCNT_CUR              ((volatile uint32_t *)REG_DMA30_XCNT_CUR)               /* DMA30 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA30_YCNT_CUR              ((volatile uint32_t *)REG_DMA30_YCNT_CUR)               /* DMA30 Current Row Count (2D only) */
#define pREG_DMA30_BWLCNT                ((volatile uint32_t *)REG_DMA30_BWLCNT)                 /* DMA30 Bandwidth Limit Count */
#define pREG_DMA30_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA30_BWLCNT_CUR)             /* DMA30 Bandwidth Limit Count Current */
#define pREG_DMA30_BWMCNT                ((volatile uint32_t *)REG_DMA30_BWMCNT)                 /* DMA30 Bandwidth Monitor Count */
#define pREG_DMA30_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA30_BWMCNT_CUR)             /* DMA30 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA31
   ========================================================================= */
#define pREG_DMA31_DSCPTR_NXT            ((void * volatile *)REG_DMA31_DSCPTR_NXT)               /* DMA31 Pointer to Next Initial Descriptor */
#define pREG_DMA31_ADDRSTART             ((void * volatile *)REG_DMA31_ADDRSTART)                /* DMA31 Start Address of Current Buffer */
#define pREG_DMA31_CFG                   ((volatile uint32_t *)REG_DMA31_CFG)                    /* DMA31 Configuration Register */
#define pREG_DMA31_XCNT                  ((volatile uint32_t *)REG_DMA31_XCNT)                   /* DMA31 Inner Loop Count Start Value */
#define pREG_DMA31_XMOD                  ((volatile  int32_t *)REG_DMA31_XMOD)                   /* DMA31 Inner Loop Address Increment */
#define pREG_DMA31_YCNT                  ((volatile uint32_t *)REG_DMA31_YCNT)                   /* DMA31 Outer Loop Count Start Value (2D only) */
#define pREG_DMA31_YMOD                  ((volatile  int32_t *)REG_DMA31_YMOD)                   /* DMA31 Outer Loop Address Increment (2D only) */
#define pREG_DMA31_DSCPTR_CUR            ((void * volatile *)REG_DMA31_DSCPTR_CUR)               /* DMA31 Current Descriptor Pointer */
#define pREG_DMA31_DSCPTR_PRV            ((void * volatile *)REG_DMA31_DSCPTR_PRV)               /* DMA31 Previous Initial Descriptor Pointer */
#define pREG_DMA31_ADDR_CUR              ((void * volatile *)REG_DMA31_ADDR_CUR)                 /* DMA31 Current Address */
#define pREG_DMA31_STAT                  ((volatile uint32_t *)REG_DMA31_STAT)                   /* DMA31 Status Register */
#define pREG_DMA31_XCNT_CUR              ((volatile uint32_t *)REG_DMA31_XCNT_CUR)               /* DMA31 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA31_YCNT_CUR              ((volatile uint32_t *)REG_DMA31_YCNT_CUR)               /* DMA31 Current Row Count (2D only) */
#define pREG_DMA31_BWLCNT                ((volatile uint32_t *)REG_DMA31_BWLCNT)                 /* DMA31 Bandwidth Limit Count */
#define pREG_DMA31_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA31_BWLCNT_CUR)             /* DMA31 Bandwidth Limit Count Current */
#define pREG_DMA31_BWMCNT                ((volatile uint32_t *)REG_DMA31_BWMCNT)                 /* DMA31 Bandwidth Monitor Count */
#define pREG_DMA31_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA31_BWMCNT_CUR)             /* DMA31 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA32
   ========================================================================= */
#define pREG_DMA32_DSCPTR_NXT            ((void * volatile *)REG_DMA32_DSCPTR_NXT)               /* DMA32 Pointer to Next Initial Descriptor */
#define pREG_DMA32_ADDRSTART             ((void * volatile *)REG_DMA32_ADDRSTART)                /* DMA32 Start Address of Current Buffer */
#define pREG_DMA32_CFG                   ((volatile uint32_t *)REG_DMA32_CFG)                    /* DMA32 Configuration Register */
#define pREG_DMA32_XCNT                  ((volatile uint32_t *)REG_DMA32_XCNT)                   /* DMA32 Inner Loop Count Start Value */
#define pREG_DMA32_XMOD                  ((volatile  int32_t *)REG_DMA32_XMOD)                   /* DMA32 Inner Loop Address Increment */
#define pREG_DMA32_YCNT                  ((volatile uint32_t *)REG_DMA32_YCNT)                   /* DMA32 Outer Loop Count Start Value (2D only) */
#define pREG_DMA32_YMOD                  ((volatile  int32_t *)REG_DMA32_YMOD)                   /* DMA32 Outer Loop Address Increment (2D only) */
#define pREG_DMA32_DSCPTR_CUR            ((void * volatile *)REG_DMA32_DSCPTR_CUR)               /* DMA32 Current Descriptor Pointer */
#define pREG_DMA32_DSCPTR_PRV            ((void * volatile *)REG_DMA32_DSCPTR_PRV)               /* DMA32 Previous Initial Descriptor Pointer */
#define pREG_DMA32_ADDR_CUR              ((void * volatile *)REG_DMA32_ADDR_CUR)                 /* DMA32 Current Address */
#define pREG_DMA32_STAT                  ((volatile uint32_t *)REG_DMA32_STAT)                   /* DMA32 Status Register */
#define pREG_DMA32_XCNT_CUR              ((volatile uint32_t *)REG_DMA32_XCNT_CUR)               /* DMA32 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA32_YCNT_CUR              ((volatile uint32_t *)REG_DMA32_YCNT_CUR)               /* DMA32 Current Row Count (2D only) */
#define pREG_DMA32_BWLCNT                ((volatile uint32_t *)REG_DMA32_BWLCNT)                 /* DMA32 Bandwidth Limit Count */
#define pREG_DMA32_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA32_BWLCNT_CUR)             /* DMA32 Bandwidth Limit Count Current */
#define pREG_DMA32_BWMCNT                ((volatile uint32_t *)REG_DMA32_BWMCNT)                 /* DMA32 Bandwidth Monitor Count */
#define pREG_DMA32_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA32_BWMCNT_CUR)             /* DMA32 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA33
   ========================================================================= */
#define pREG_DMA33_DSCPTR_NXT            ((void * volatile *)REG_DMA33_DSCPTR_NXT)               /* DMA33 Pointer to Next Initial Descriptor */
#define pREG_DMA33_ADDRSTART             ((void * volatile *)REG_DMA33_ADDRSTART)                /* DMA33 Start Address of Current Buffer */
#define pREG_DMA33_CFG                   ((volatile uint32_t *)REG_DMA33_CFG)                    /* DMA33 Configuration Register */
#define pREG_DMA33_XCNT                  ((volatile uint32_t *)REG_DMA33_XCNT)                   /* DMA33 Inner Loop Count Start Value */
#define pREG_DMA33_XMOD                  ((volatile  int32_t *)REG_DMA33_XMOD)                   /* DMA33 Inner Loop Address Increment */
#define pREG_DMA33_YCNT                  ((volatile uint32_t *)REG_DMA33_YCNT)                   /* DMA33 Outer Loop Count Start Value (2D only) */
#define pREG_DMA33_YMOD                  ((volatile  int32_t *)REG_DMA33_YMOD)                   /* DMA33 Outer Loop Address Increment (2D only) */
#define pREG_DMA33_DSCPTR_CUR            ((void * volatile *)REG_DMA33_DSCPTR_CUR)               /* DMA33 Current Descriptor Pointer */
#define pREG_DMA33_DSCPTR_PRV            ((void * volatile *)REG_DMA33_DSCPTR_PRV)               /* DMA33 Previous Initial Descriptor Pointer */
#define pREG_DMA33_ADDR_CUR              ((void * volatile *)REG_DMA33_ADDR_CUR)                 /* DMA33 Current Address */
#define pREG_DMA33_STAT                  ((volatile uint32_t *)REG_DMA33_STAT)                   /* DMA33 Status Register */
#define pREG_DMA33_XCNT_CUR              ((volatile uint32_t *)REG_DMA33_XCNT_CUR)               /* DMA33 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA33_YCNT_CUR              ((volatile uint32_t *)REG_DMA33_YCNT_CUR)               /* DMA33 Current Row Count (2D only) */
#define pREG_DMA33_BWLCNT                ((volatile uint32_t *)REG_DMA33_BWLCNT)                 /* DMA33 Bandwidth Limit Count */
#define pREG_DMA33_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA33_BWLCNT_CUR)             /* DMA33 Bandwidth Limit Count Current */
#define pREG_DMA33_BWMCNT                ((volatile uint32_t *)REG_DMA33_BWMCNT)                 /* DMA33 Bandwidth Monitor Count */
#define pREG_DMA33_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA33_BWMCNT_CUR)             /* DMA33 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA34
   ========================================================================= */
#define pREG_DMA34_DSCPTR_NXT            ((void * volatile *)REG_DMA34_DSCPTR_NXT)               /* DMA34 Pointer to Next Initial Descriptor */
#define pREG_DMA34_ADDRSTART             ((void * volatile *)REG_DMA34_ADDRSTART)                /* DMA34 Start Address of Current Buffer */
#define pREG_DMA34_CFG                   ((volatile uint32_t *)REG_DMA34_CFG)                    /* DMA34 Configuration Register */
#define pREG_DMA34_XCNT                  ((volatile uint32_t *)REG_DMA34_XCNT)                   /* DMA34 Inner Loop Count Start Value */
#define pREG_DMA34_XMOD                  ((volatile  int32_t *)REG_DMA34_XMOD)                   /* DMA34 Inner Loop Address Increment */
#define pREG_DMA34_YCNT                  ((volatile uint32_t *)REG_DMA34_YCNT)                   /* DMA34 Outer Loop Count Start Value (2D only) */
#define pREG_DMA34_YMOD                  ((volatile  int32_t *)REG_DMA34_YMOD)                   /* DMA34 Outer Loop Address Increment (2D only) */
#define pREG_DMA34_DSCPTR_CUR            ((void * volatile *)REG_DMA34_DSCPTR_CUR)               /* DMA34 Current Descriptor Pointer */
#define pREG_DMA34_DSCPTR_PRV            ((void * volatile *)REG_DMA34_DSCPTR_PRV)               /* DMA34 Previous Initial Descriptor Pointer */
#define pREG_DMA34_ADDR_CUR              ((void * volatile *)REG_DMA34_ADDR_CUR)                 /* DMA34 Current Address */
#define pREG_DMA34_STAT                  ((volatile uint32_t *)REG_DMA34_STAT)                   /* DMA34 Status Register */
#define pREG_DMA34_XCNT_CUR              ((volatile uint32_t *)REG_DMA34_XCNT_CUR)               /* DMA34 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA34_YCNT_CUR              ((volatile uint32_t *)REG_DMA34_YCNT_CUR)               /* DMA34 Current Row Count (2D only) */
#define pREG_DMA34_BWLCNT                ((volatile uint32_t *)REG_DMA34_BWLCNT)                 /* DMA34 Bandwidth Limit Count */
#define pREG_DMA34_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA34_BWLCNT_CUR)             /* DMA34 Bandwidth Limit Count Current */
#define pREG_DMA34_BWMCNT                ((volatile uint32_t *)REG_DMA34_BWMCNT)                 /* DMA34 Bandwidth Monitor Count */
#define pREG_DMA34_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA34_BWMCNT_CUR)             /* DMA34 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA35
   ========================================================================= */
#define pREG_DMA35_DSCPTR_NXT            ((void * volatile *)REG_DMA35_DSCPTR_NXT)               /* DMA35 Pointer to Next Initial Descriptor */
#define pREG_DMA35_ADDRSTART             ((void * volatile *)REG_DMA35_ADDRSTART)                /* DMA35 Start Address of Current Buffer */
#define pREG_DMA35_CFG                   ((volatile uint32_t *)REG_DMA35_CFG)                    /* DMA35 Configuration Register */
#define pREG_DMA35_XCNT                  ((volatile uint32_t *)REG_DMA35_XCNT)                   /* DMA35 Inner Loop Count Start Value */
#define pREG_DMA35_XMOD                  ((volatile  int32_t *)REG_DMA35_XMOD)                   /* DMA35 Inner Loop Address Increment */
#define pREG_DMA35_YCNT                  ((volatile uint32_t *)REG_DMA35_YCNT)                   /* DMA35 Outer Loop Count Start Value (2D only) */
#define pREG_DMA35_YMOD                  ((volatile  int32_t *)REG_DMA35_YMOD)                   /* DMA35 Outer Loop Address Increment (2D only) */
#define pREG_DMA35_DSCPTR_CUR            ((void * volatile *)REG_DMA35_DSCPTR_CUR)               /* DMA35 Current Descriptor Pointer */
#define pREG_DMA35_DSCPTR_PRV            ((void * volatile *)REG_DMA35_DSCPTR_PRV)               /* DMA35 Previous Initial Descriptor Pointer */
#define pREG_DMA35_ADDR_CUR              ((void * volatile *)REG_DMA35_ADDR_CUR)                 /* DMA35 Current Address */
#define pREG_DMA35_STAT                  ((volatile uint32_t *)REG_DMA35_STAT)                   /* DMA35 Status Register */
#define pREG_DMA35_XCNT_CUR              ((volatile uint32_t *)REG_DMA35_XCNT_CUR)               /* DMA35 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA35_YCNT_CUR              ((volatile uint32_t *)REG_DMA35_YCNT_CUR)               /* DMA35 Current Row Count (2D only) */
#define pREG_DMA35_BWLCNT                ((volatile uint32_t *)REG_DMA35_BWLCNT)                 /* DMA35 Bandwidth Limit Count */
#define pREG_DMA35_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA35_BWLCNT_CUR)             /* DMA35 Bandwidth Limit Count Current */
#define pREG_DMA35_BWMCNT                ((volatile uint32_t *)REG_DMA35_BWMCNT)                 /* DMA35 Bandwidth Monitor Count */
#define pREG_DMA35_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA35_BWMCNT_CUR)             /* DMA35 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA36
   ========================================================================= */
#define pREG_DMA36_DSCPTR_NXT            ((void * volatile *)REG_DMA36_DSCPTR_NXT)               /* DMA36 Pointer to Next Initial Descriptor */
#define pREG_DMA36_ADDRSTART             ((void * volatile *)REG_DMA36_ADDRSTART)                /* DMA36 Start Address of Current Buffer */
#define pREG_DMA36_CFG                   ((volatile uint32_t *)REG_DMA36_CFG)                    /* DMA36 Configuration Register */
#define pREG_DMA36_XCNT                  ((volatile uint32_t *)REG_DMA36_XCNT)                   /* DMA36 Inner Loop Count Start Value */
#define pREG_DMA36_XMOD                  ((volatile  int32_t *)REG_DMA36_XMOD)                   /* DMA36 Inner Loop Address Increment */
#define pREG_DMA36_YCNT                  ((volatile uint32_t *)REG_DMA36_YCNT)                   /* DMA36 Outer Loop Count Start Value (2D only) */
#define pREG_DMA36_YMOD                  ((volatile  int32_t *)REG_DMA36_YMOD)                   /* DMA36 Outer Loop Address Increment (2D only) */
#define pREG_DMA36_DSCPTR_CUR            ((void * volatile *)REG_DMA36_DSCPTR_CUR)               /* DMA36 Current Descriptor Pointer */
#define pREG_DMA36_DSCPTR_PRV            ((void * volatile *)REG_DMA36_DSCPTR_PRV)               /* DMA36 Previous Initial Descriptor Pointer */
#define pREG_DMA36_ADDR_CUR              ((void * volatile *)REG_DMA36_ADDR_CUR)                 /* DMA36 Current Address */
#define pREG_DMA36_STAT                  ((volatile uint32_t *)REG_DMA36_STAT)                   /* DMA36 Status Register */
#define pREG_DMA36_XCNT_CUR              ((volatile uint32_t *)REG_DMA36_XCNT_CUR)               /* DMA36 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA36_YCNT_CUR              ((volatile uint32_t *)REG_DMA36_YCNT_CUR)               /* DMA36 Current Row Count (2D only) */
#define pREG_DMA36_BWLCNT                ((volatile uint32_t *)REG_DMA36_BWLCNT)                 /* DMA36 Bandwidth Limit Count */
#define pREG_DMA36_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA36_BWLCNT_CUR)             /* DMA36 Bandwidth Limit Count Current */
#define pREG_DMA36_BWMCNT                ((volatile uint32_t *)REG_DMA36_BWMCNT)                 /* DMA36 Bandwidth Monitor Count */
#define pREG_DMA36_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA36_BWMCNT_CUR)             /* DMA36 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA37
   ========================================================================= */
#define pREG_DMA37_DSCPTR_NXT            ((void * volatile *)REG_DMA37_DSCPTR_NXT)               /* DMA37 Pointer to Next Initial Descriptor */
#define pREG_DMA37_ADDRSTART             ((void * volatile *)REG_DMA37_ADDRSTART)                /* DMA37 Start Address of Current Buffer */
#define pREG_DMA37_CFG                   ((volatile uint32_t *)REG_DMA37_CFG)                    /* DMA37 Configuration Register */
#define pREG_DMA37_XCNT                  ((volatile uint32_t *)REG_DMA37_XCNT)                   /* DMA37 Inner Loop Count Start Value */
#define pREG_DMA37_XMOD                  ((volatile  int32_t *)REG_DMA37_XMOD)                   /* DMA37 Inner Loop Address Increment */
#define pREG_DMA37_YCNT                  ((volatile uint32_t *)REG_DMA37_YCNT)                   /* DMA37 Outer Loop Count Start Value (2D only) */
#define pREG_DMA37_YMOD                  ((volatile  int32_t *)REG_DMA37_YMOD)                   /* DMA37 Outer Loop Address Increment (2D only) */
#define pREG_DMA37_DSCPTR_CUR            ((void * volatile *)REG_DMA37_DSCPTR_CUR)               /* DMA37 Current Descriptor Pointer */
#define pREG_DMA37_DSCPTR_PRV            ((void * volatile *)REG_DMA37_DSCPTR_PRV)               /* DMA37 Previous Initial Descriptor Pointer */
#define pREG_DMA37_ADDR_CUR              ((void * volatile *)REG_DMA37_ADDR_CUR)                 /* DMA37 Current Address */
#define pREG_DMA37_STAT                  ((volatile uint32_t *)REG_DMA37_STAT)                   /* DMA37 Status Register */
#define pREG_DMA37_XCNT_CUR              ((volatile uint32_t *)REG_DMA37_XCNT_CUR)               /* DMA37 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA37_YCNT_CUR              ((volatile uint32_t *)REG_DMA37_YCNT_CUR)               /* DMA37 Current Row Count (2D only) */
#define pREG_DMA37_BWLCNT                ((volatile uint32_t *)REG_DMA37_BWLCNT)                 /* DMA37 Bandwidth Limit Count */
#define pREG_DMA37_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA37_BWLCNT_CUR)             /* DMA37 Bandwidth Limit Count Current */
#define pREG_DMA37_BWMCNT                ((volatile uint32_t *)REG_DMA37_BWMCNT)                 /* DMA37 Bandwidth Monitor Count */
#define pREG_DMA37_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA37_BWMCNT_CUR)             /* DMA37 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA38
   ========================================================================= */
#define pREG_DMA38_DSCPTR_NXT            ((void * volatile *)REG_DMA38_DSCPTR_NXT)               /* DMA38 Pointer to Next Initial Descriptor */
#define pREG_DMA38_ADDRSTART             ((void * volatile *)REG_DMA38_ADDRSTART)                /* DMA38 Start Address of Current Buffer */
#define pREG_DMA38_CFG                   ((volatile uint32_t *)REG_DMA38_CFG)                    /* DMA38 Configuration Register */
#define pREG_DMA38_XCNT                  ((volatile uint32_t *)REG_DMA38_XCNT)                   /* DMA38 Inner Loop Count Start Value */
#define pREG_DMA38_XMOD                  ((volatile  int32_t *)REG_DMA38_XMOD)                   /* DMA38 Inner Loop Address Increment */
#define pREG_DMA38_YCNT                  ((volatile uint32_t *)REG_DMA38_YCNT)                   /* DMA38 Outer Loop Count Start Value (2D only) */
#define pREG_DMA38_YMOD                  ((volatile  int32_t *)REG_DMA38_YMOD)                   /* DMA38 Outer Loop Address Increment (2D only) */
#define pREG_DMA38_DSCPTR_CUR            ((void * volatile *)REG_DMA38_DSCPTR_CUR)               /* DMA38 Current Descriptor Pointer */
#define pREG_DMA38_DSCPTR_PRV            ((void * volatile *)REG_DMA38_DSCPTR_PRV)               /* DMA38 Previous Initial Descriptor Pointer */
#define pREG_DMA38_ADDR_CUR              ((void * volatile *)REG_DMA38_ADDR_CUR)                 /* DMA38 Current Address */
#define pREG_DMA38_STAT                  ((volatile uint32_t *)REG_DMA38_STAT)                   /* DMA38 Status Register */
#define pREG_DMA38_XCNT_CUR              ((volatile uint32_t *)REG_DMA38_XCNT_CUR)               /* DMA38 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA38_YCNT_CUR              ((volatile uint32_t *)REG_DMA38_YCNT_CUR)               /* DMA38 Current Row Count (2D only) */
#define pREG_DMA38_BWLCNT                ((volatile uint32_t *)REG_DMA38_BWLCNT)                 /* DMA38 Bandwidth Limit Count */
#define pREG_DMA38_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA38_BWLCNT_CUR)             /* DMA38 Bandwidth Limit Count Current */
#define pREG_DMA38_BWMCNT                ((volatile uint32_t *)REG_DMA38_BWMCNT)                 /* DMA38 Bandwidth Monitor Count */
#define pREG_DMA38_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA38_BWMCNT_CUR)             /* DMA38 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA39
   ========================================================================= */
#define pREG_DMA39_DSCPTR_NXT            ((void * volatile *)REG_DMA39_DSCPTR_NXT)               /* DMA39 Pointer to Next Initial Descriptor */
#define pREG_DMA39_ADDRSTART             ((void * volatile *)REG_DMA39_ADDRSTART)                /* DMA39 Start Address of Current Buffer */
#define pREG_DMA39_CFG                   ((volatile uint32_t *)REG_DMA39_CFG)                    /* DMA39 Configuration Register */
#define pREG_DMA39_XCNT                  ((volatile uint32_t *)REG_DMA39_XCNT)                   /* DMA39 Inner Loop Count Start Value */
#define pREG_DMA39_XMOD                  ((volatile  int32_t *)REG_DMA39_XMOD)                   /* DMA39 Inner Loop Address Increment */
#define pREG_DMA39_YCNT                  ((volatile uint32_t *)REG_DMA39_YCNT)                   /* DMA39 Outer Loop Count Start Value (2D only) */
#define pREG_DMA39_YMOD                  ((volatile  int32_t *)REG_DMA39_YMOD)                   /* DMA39 Outer Loop Address Increment (2D only) */
#define pREG_DMA39_DSCPTR_CUR            ((void * volatile *)REG_DMA39_DSCPTR_CUR)               /* DMA39 Current Descriptor Pointer */
#define pREG_DMA39_DSCPTR_PRV            ((void * volatile *)REG_DMA39_DSCPTR_PRV)               /* DMA39 Previous Initial Descriptor Pointer */
#define pREG_DMA39_ADDR_CUR              ((void * volatile *)REG_DMA39_ADDR_CUR)                 /* DMA39 Current Address */
#define pREG_DMA39_STAT                  ((volatile uint32_t *)REG_DMA39_STAT)                   /* DMA39 Status Register */
#define pREG_DMA39_XCNT_CUR              ((volatile uint32_t *)REG_DMA39_XCNT_CUR)               /* DMA39 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA39_YCNT_CUR              ((volatile uint32_t *)REG_DMA39_YCNT_CUR)               /* DMA39 Current Row Count (2D only) */
#define pREG_DMA39_BWLCNT                ((volatile uint32_t *)REG_DMA39_BWLCNT)                 /* DMA39 Bandwidth Limit Count */
#define pREG_DMA39_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA39_BWLCNT_CUR)             /* DMA39 Bandwidth Limit Count Current */
#define pREG_DMA39_BWMCNT                ((volatile uint32_t *)REG_DMA39_BWMCNT)                 /* DMA39 Bandwidth Monitor Count */
#define pREG_DMA39_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA39_BWMCNT_CUR)             /* DMA39 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA40
   ========================================================================= */
#define pREG_DMA40_DSCPTR_NXT            ((void * volatile *)REG_DMA40_DSCPTR_NXT)               /* DMA40 Pointer to Next Initial Descriptor */
#define pREG_DMA40_ADDRSTART             ((void * volatile *)REG_DMA40_ADDRSTART)                /* DMA40 Start Address of Current Buffer */
#define pREG_DMA40_CFG                   ((volatile uint32_t *)REG_DMA40_CFG)                    /* DMA40 Configuration Register */
#define pREG_DMA40_XCNT                  ((volatile uint32_t *)REG_DMA40_XCNT)                   /* DMA40 Inner Loop Count Start Value */
#define pREG_DMA40_XMOD                  ((volatile  int32_t *)REG_DMA40_XMOD)                   /* DMA40 Inner Loop Address Increment */
#define pREG_DMA40_YCNT                  ((volatile uint32_t *)REG_DMA40_YCNT)                   /* DMA40 Outer Loop Count Start Value (2D only) */
#define pREG_DMA40_YMOD                  ((volatile  int32_t *)REG_DMA40_YMOD)                   /* DMA40 Outer Loop Address Increment (2D only) */
#define pREG_DMA40_DSCPTR_CUR            ((void * volatile *)REG_DMA40_DSCPTR_CUR)               /* DMA40 Current Descriptor Pointer */
#define pREG_DMA40_DSCPTR_PRV            ((void * volatile *)REG_DMA40_DSCPTR_PRV)               /* DMA40 Previous Initial Descriptor Pointer */
#define pREG_DMA40_ADDR_CUR              ((void * volatile *)REG_DMA40_ADDR_CUR)                 /* DMA40 Current Address */
#define pREG_DMA40_STAT                  ((volatile uint32_t *)REG_DMA40_STAT)                   /* DMA40 Status Register */
#define pREG_DMA40_XCNT_CUR              ((volatile uint32_t *)REG_DMA40_XCNT_CUR)               /* DMA40 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA40_YCNT_CUR              ((volatile uint32_t *)REG_DMA40_YCNT_CUR)               /* DMA40 Current Row Count (2D only) */
#define pREG_DMA40_BWLCNT                ((volatile uint32_t *)REG_DMA40_BWLCNT)                 /* DMA40 Bandwidth Limit Count */
#define pREG_DMA40_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA40_BWLCNT_CUR)             /* DMA40 Bandwidth Limit Count Current */
#define pREG_DMA40_BWMCNT                ((volatile uint32_t *)REG_DMA40_BWMCNT)                 /* DMA40 Bandwidth Monitor Count */
#define pREG_DMA40_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA40_BWMCNT_CUR)             /* DMA40 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA41
   ========================================================================= */
#define pREG_DMA41_DSCPTR_NXT            ((void * volatile *)REG_DMA41_DSCPTR_NXT)               /* DMA41 Pointer to Next Initial Descriptor */
#define pREG_DMA41_ADDRSTART             ((void * volatile *)REG_DMA41_ADDRSTART)                /* DMA41 Start Address of Current Buffer */
#define pREG_DMA41_CFG                   ((volatile uint32_t *)REG_DMA41_CFG)                    /* DMA41 Configuration Register */
#define pREG_DMA41_XCNT                  ((volatile uint32_t *)REG_DMA41_XCNT)                   /* DMA41 Inner Loop Count Start Value */
#define pREG_DMA41_XMOD                  ((volatile  int32_t *)REG_DMA41_XMOD)                   /* DMA41 Inner Loop Address Increment */
#define pREG_DMA41_YCNT                  ((volatile uint32_t *)REG_DMA41_YCNT)                   /* DMA41 Outer Loop Count Start Value (2D only) */
#define pREG_DMA41_YMOD                  ((volatile  int32_t *)REG_DMA41_YMOD)                   /* DMA41 Outer Loop Address Increment (2D only) */
#define pREG_DMA41_DSCPTR_CUR            ((void * volatile *)REG_DMA41_DSCPTR_CUR)               /* DMA41 Current Descriptor Pointer */
#define pREG_DMA41_DSCPTR_PRV            ((void * volatile *)REG_DMA41_DSCPTR_PRV)               /* DMA41 Previous Initial Descriptor Pointer */
#define pREG_DMA41_ADDR_CUR              ((void * volatile *)REG_DMA41_ADDR_CUR)                 /* DMA41 Current Address */
#define pREG_DMA41_STAT                  ((volatile uint32_t *)REG_DMA41_STAT)                   /* DMA41 Status Register */
#define pREG_DMA41_XCNT_CUR              ((volatile uint32_t *)REG_DMA41_XCNT_CUR)               /* DMA41 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA41_YCNT_CUR              ((volatile uint32_t *)REG_DMA41_YCNT_CUR)               /* DMA41 Current Row Count (2D only) */
#define pREG_DMA41_BWLCNT                ((volatile uint32_t *)REG_DMA41_BWLCNT)                 /* DMA41 Bandwidth Limit Count */
#define pREG_DMA41_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA41_BWLCNT_CUR)             /* DMA41 Bandwidth Limit Count Current */
#define pREG_DMA41_BWMCNT                ((volatile uint32_t *)REG_DMA41_BWMCNT)                 /* DMA41 Bandwidth Monitor Count */
#define pREG_DMA41_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA41_BWMCNT_CUR)             /* DMA41 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA42
   ========================================================================= */
#define pREG_DMA42_DSCPTR_NXT            ((void * volatile *)REG_DMA42_DSCPTR_NXT)               /* DMA42 Pointer to Next Initial Descriptor */
#define pREG_DMA42_ADDRSTART             ((void * volatile *)REG_DMA42_ADDRSTART)                /* DMA42 Start Address of Current Buffer */
#define pREG_DMA42_CFG                   ((volatile uint32_t *)REG_DMA42_CFG)                    /* DMA42 Configuration Register */
#define pREG_DMA42_XCNT                  ((volatile uint32_t *)REG_DMA42_XCNT)                   /* DMA42 Inner Loop Count Start Value */
#define pREG_DMA42_XMOD                  ((volatile  int32_t *)REG_DMA42_XMOD)                   /* DMA42 Inner Loop Address Increment */
#define pREG_DMA42_YCNT                  ((volatile uint32_t *)REG_DMA42_YCNT)                   /* DMA42 Outer Loop Count Start Value (2D only) */
#define pREG_DMA42_YMOD                  ((volatile  int32_t *)REG_DMA42_YMOD)                   /* DMA42 Outer Loop Address Increment (2D only) */
#define pREG_DMA42_DSCPTR_CUR            ((void * volatile *)REG_DMA42_DSCPTR_CUR)               /* DMA42 Current Descriptor Pointer */
#define pREG_DMA42_DSCPTR_PRV            ((void * volatile *)REG_DMA42_DSCPTR_PRV)               /* DMA42 Previous Initial Descriptor Pointer */
#define pREG_DMA42_ADDR_CUR              ((void * volatile *)REG_DMA42_ADDR_CUR)                 /* DMA42 Current Address */
#define pREG_DMA42_STAT                  ((volatile uint32_t *)REG_DMA42_STAT)                   /* DMA42 Status Register */
#define pREG_DMA42_XCNT_CUR              ((volatile uint32_t *)REG_DMA42_XCNT_CUR)               /* DMA42 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA42_YCNT_CUR              ((volatile uint32_t *)REG_DMA42_YCNT_CUR)               /* DMA42 Current Row Count (2D only) */
#define pREG_DMA42_BWLCNT                ((volatile uint32_t *)REG_DMA42_BWLCNT)                 /* DMA42 Bandwidth Limit Count */
#define pREG_DMA42_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA42_BWLCNT_CUR)             /* DMA42 Bandwidth Limit Count Current */
#define pREG_DMA42_BWMCNT                ((volatile uint32_t *)REG_DMA42_BWMCNT)                 /* DMA42 Bandwidth Monitor Count */
#define pREG_DMA42_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA42_BWMCNT_CUR)             /* DMA42 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA43
   ========================================================================= */
#define pREG_DMA43_DSCPTR_NXT            ((void * volatile *)REG_DMA43_DSCPTR_NXT)               /* DMA43 Pointer to Next Initial Descriptor */
#define pREG_DMA43_ADDRSTART             ((void * volatile *)REG_DMA43_ADDRSTART)                /* DMA43 Start Address of Current Buffer */
#define pREG_DMA43_CFG                   ((volatile uint32_t *)REG_DMA43_CFG)                    /* DMA43 Configuration Register */
#define pREG_DMA43_XCNT                  ((volatile uint32_t *)REG_DMA43_XCNT)                   /* DMA43 Inner Loop Count Start Value */
#define pREG_DMA43_XMOD                  ((volatile  int32_t *)REG_DMA43_XMOD)                   /* DMA43 Inner Loop Address Increment */
#define pREG_DMA43_YCNT                  ((volatile uint32_t *)REG_DMA43_YCNT)                   /* DMA43 Outer Loop Count Start Value (2D only) */
#define pREG_DMA43_YMOD                  ((volatile  int32_t *)REG_DMA43_YMOD)                   /* DMA43 Outer Loop Address Increment (2D only) */
#define pREG_DMA43_DSCPTR_CUR            ((void * volatile *)REG_DMA43_DSCPTR_CUR)               /* DMA43 Current Descriptor Pointer */
#define pREG_DMA43_DSCPTR_PRV            ((void * volatile *)REG_DMA43_DSCPTR_PRV)               /* DMA43 Previous Initial Descriptor Pointer */
#define pREG_DMA43_ADDR_CUR              ((void * volatile *)REG_DMA43_ADDR_CUR)                 /* DMA43 Current Address */
#define pREG_DMA43_STAT                  ((volatile uint32_t *)REG_DMA43_STAT)                   /* DMA43 Status Register */
#define pREG_DMA43_XCNT_CUR              ((volatile uint32_t *)REG_DMA43_XCNT_CUR)               /* DMA43 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA43_YCNT_CUR              ((volatile uint32_t *)REG_DMA43_YCNT_CUR)               /* DMA43 Current Row Count (2D only) */
#define pREG_DMA43_BWLCNT                ((volatile uint32_t *)REG_DMA43_BWLCNT)                 /* DMA43 Bandwidth Limit Count */
#define pREG_DMA43_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA43_BWLCNT_CUR)             /* DMA43 Bandwidth Limit Count Current */
#define pREG_DMA43_BWMCNT                ((volatile uint32_t *)REG_DMA43_BWMCNT)                 /* DMA43 Bandwidth Monitor Count */
#define pREG_DMA43_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA43_BWMCNT_CUR)             /* DMA43 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA44
   ========================================================================= */
#define pREG_DMA44_DSCPTR_NXT            ((void * volatile *)REG_DMA44_DSCPTR_NXT)               /* DMA44 Pointer to Next Initial Descriptor */
#define pREG_DMA44_ADDRSTART             ((void * volatile *)REG_DMA44_ADDRSTART)                /* DMA44 Start Address of Current Buffer */
#define pREG_DMA44_CFG                   ((volatile uint32_t *)REG_DMA44_CFG)                    /* DMA44 Configuration Register */
#define pREG_DMA44_XCNT                  ((volatile uint32_t *)REG_DMA44_XCNT)                   /* DMA44 Inner Loop Count Start Value */
#define pREG_DMA44_XMOD                  ((volatile  int32_t *)REG_DMA44_XMOD)                   /* DMA44 Inner Loop Address Increment */
#define pREG_DMA44_YCNT                  ((volatile uint32_t *)REG_DMA44_YCNT)                   /* DMA44 Outer Loop Count Start Value (2D only) */
#define pREG_DMA44_YMOD                  ((volatile  int32_t *)REG_DMA44_YMOD)                   /* DMA44 Outer Loop Address Increment (2D only) */
#define pREG_DMA44_DSCPTR_CUR            ((void * volatile *)REG_DMA44_DSCPTR_CUR)               /* DMA44 Current Descriptor Pointer */
#define pREG_DMA44_DSCPTR_PRV            ((void * volatile *)REG_DMA44_DSCPTR_PRV)               /* DMA44 Previous Initial Descriptor Pointer */
#define pREG_DMA44_ADDR_CUR              ((void * volatile *)REG_DMA44_ADDR_CUR)                 /* DMA44 Current Address */
#define pREG_DMA44_STAT                  ((volatile uint32_t *)REG_DMA44_STAT)                   /* DMA44 Status Register */
#define pREG_DMA44_XCNT_CUR              ((volatile uint32_t *)REG_DMA44_XCNT_CUR)               /* DMA44 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA44_YCNT_CUR              ((volatile uint32_t *)REG_DMA44_YCNT_CUR)               /* DMA44 Current Row Count (2D only) */
#define pREG_DMA44_BWLCNT                ((volatile uint32_t *)REG_DMA44_BWLCNT)                 /* DMA44 Bandwidth Limit Count */
#define pREG_DMA44_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA44_BWLCNT_CUR)             /* DMA44 Bandwidth Limit Count Current */
#define pREG_DMA44_BWMCNT                ((volatile uint32_t *)REG_DMA44_BWMCNT)                 /* DMA44 Bandwidth Monitor Count */
#define pREG_DMA44_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA44_BWMCNT_CUR)             /* DMA44 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA45
   ========================================================================= */
#define pREG_DMA45_DSCPTR_NXT            ((void * volatile *)REG_DMA45_DSCPTR_NXT)               /* DMA45 Pointer to Next Initial Descriptor */
#define pREG_DMA45_ADDRSTART             ((void * volatile *)REG_DMA45_ADDRSTART)                /* DMA45 Start Address of Current Buffer */
#define pREG_DMA45_CFG                   ((volatile uint32_t *)REG_DMA45_CFG)                    /* DMA45 Configuration Register */
#define pREG_DMA45_XCNT                  ((volatile uint32_t *)REG_DMA45_XCNT)                   /* DMA45 Inner Loop Count Start Value */
#define pREG_DMA45_XMOD                  ((volatile  int32_t *)REG_DMA45_XMOD)                   /* DMA45 Inner Loop Address Increment */
#define pREG_DMA45_YCNT                  ((volatile uint32_t *)REG_DMA45_YCNT)                   /* DMA45 Outer Loop Count Start Value (2D only) */
#define pREG_DMA45_YMOD                  ((volatile  int32_t *)REG_DMA45_YMOD)                   /* DMA45 Outer Loop Address Increment (2D only) */
#define pREG_DMA45_DSCPTR_CUR            ((void * volatile *)REG_DMA45_DSCPTR_CUR)               /* DMA45 Current Descriptor Pointer */
#define pREG_DMA45_DSCPTR_PRV            ((void * volatile *)REG_DMA45_DSCPTR_PRV)               /* DMA45 Previous Initial Descriptor Pointer */
#define pREG_DMA45_ADDR_CUR              ((void * volatile *)REG_DMA45_ADDR_CUR)                 /* DMA45 Current Address */
#define pREG_DMA45_STAT                  ((volatile uint32_t *)REG_DMA45_STAT)                   /* DMA45 Status Register */
#define pREG_DMA45_XCNT_CUR              ((volatile uint32_t *)REG_DMA45_XCNT_CUR)               /* DMA45 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA45_YCNT_CUR              ((volatile uint32_t *)REG_DMA45_YCNT_CUR)               /* DMA45 Current Row Count (2D only) */
#define pREG_DMA45_BWLCNT                ((volatile uint32_t *)REG_DMA45_BWLCNT)                 /* DMA45 Bandwidth Limit Count */
#define pREG_DMA45_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA45_BWLCNT_CUR)             /* DMA45 Bandwidth Limit Count Current */
#define pREG_DMA45_BWMCNT                ((volatile uint32_t *)REG_DMA45_BWMCNT)                 /* DMA45 Bandwidth Monitor Count */
#define pREG_DMA45_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA45_BWMCNT_CUR)             /* DMA45 Bandwidth Monitor Count Current */

/* =========================================================================
       DMA46
   ========================================================================= */
#define pREG_DMA46_DSCPTR_NXT            ((void * volatile *)REG_DMA46_DSCPTR_NXT)               /* DMA46 Pointer to Next Initial Descriptor */
#define pREG_DMA46_ADDRSTART             ((void * volatile *)REG_DMA46_ADDRSTART)                /* DMA46 Start Address of Current Buffer */
#define pREG_DMA46_CFG                   ((volatile uint32_t *)REG_DMA46_CFG)                    /* DMA46 Configuration Register */
#define pREG_DMA46_XCNT                  ((volatile uint32_t *)REG_DMA46_XCNT)                   /* DMA46 Inner Loop Count Start Value */
#define pREG_DMA46_XMOD                  ((volatile  int32_t *)REG_DMA46_XMOD)                   /* DMA46 Inner Loop Address Increment */
#define pREG_DMA46_YCNT                  ((volatile uint32_t *)REG_DMA46_YCNT)                   /* DMA46 Outer Loop Count Start Value (2D only) */
#define pREG_DMA46_YMOD                  ((volatile  int32_t *)REG_DMA46_YMOD)                   /* DMA46 Outer Loop Address Increment (2D only) */
#define pREG_DMA46_DSCPTR_CUR            ((void * volatile *)REG_DMA46_DSCPTR_CUR)               /* DMA46 Current Descriptor Pointer */
#define pREG_DMA46_DSCPTR_PRV            ((void * volatile *)REG_DMA46_DSCPTR_PRV)               /* DMA46 Previous Initial Descriptor Pointer */
#define pREG_DMA46_ADDR_CUR              ((void * volatile *)REG_DMA46_ADDR_CUR)                 /* DMA46 Current Address */
#define pREG_DMA46_STAT                  ((volatile uint32_t *)REG_DMA46_STAT)                   /* DMA46 Status Register */
#define pREG_DMA46_XCNT_CUR              ((volatile uint32_t *)REG_DMA46_XCNT_CUR)               /* DMA46 Current Count(1D) or intra-row XCNT (2D) */
#define pREG_DMA46_YCNT_CUR              ((volatile uint32_t *)REG_DMA46_YCNT_CUR)               /* DMA46 Current Row Count (2D only) */
#define pREG_DMA46_BWLCNT                ((volatile uint32_t *)REG_DMA46_BWLCNT)                 /* DMA46 Bandwidth Limit Count */
#define pREG_DMA46_BWLCNT_CUR            ((volatile uint32_t *)REG_DMA46_BWLCNT_CUR)             /* DMA46 Bandwidth Limit Count Current */
#define pREG_DMA46_BWMCNT                ((volatile uint32_t *)REG_DMA46_BWMCNT)                 /* DMA46 Bandwidth Monitor Count */
#define pREG_DMA46_BWMCNT_CUR            ((volatile uint32_t *)REG_DMA46_BWMCNT_CUR)             /* DMA46 Bandwidth Monitor Count Current */


/* =========================================================================
       ACM0
   ========================================================================= */
#define pREG_ACM0_CTL                    ((volatile uint32_t *)REG_ACM0_CTL)                     /* ACM0 ACM Control Register */
#define pREG_ACM0_TC0                    ((volatile uint32_t *)REG_ACM0_TC0)                     /* ACM0 ACM Timing Configuration 0 Register */
#define pREG_ACM0_TC1                    ((volatile uint32_t *)REG_ACM0_TC1)                     /* ACM0 ACM Timing Configuration 1 Register */
#define pREG_ACM0_STAT                   ((volatile uint32_t *)REG_ACM0_STAT)                    /* ACM0 ACM Status Register */
#define pREG_ACM0_EVSTAT                 ((volatile uint32_t *)REG_ACM0_EVSTAT)                  /* ACM0 ACM Event Status Register */
#define pREG_ACM0_EVMSK                  ((volatile uint32_t *)REG_ACM0_EVMSK)                   /* ACM0 ACM Completed Event Interrupt Mask Register */
#define pREG_ACM0_MEVSTAT                ((volatile uint32_t *)REG_ACM0_MEVSTAT)                 /* ACM0 ACM Missed Event Status Register */
#define pREG_ACM0_MEVMSK                 ((volatile uint32_t *)REG_ACM0_MEVMSK)                  /* ACM0 ACM Missed Event Interrupt Mask Register */
#define pREG_ACM0_EVCTL0                 ((volatile uint32_t *)REG_ACM0_EVCTL0)                  /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL1                 ((volatile uint32_t *)REG_ACM0_EVCTL1)                  /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL2                 ((volatile uint32_t *)REG_ACM0_EVCTL2)                  /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL3                 ((volatile uint32_t *)REG_ACM0_EVCTL3)                  /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL4                 ((volatile uint32_t *)REG_ACM0_EVCTL4)                  /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL5                 ((volatile uint32_t *)REG_ACM0_EVCTL5)                  /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL6                 ((volatile uint32_t *)REG_ACM0_EVCTL6)                  /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL7                 ((volatile uint32_t *)REG_ACM0_EVCTL7)                  /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL8                 ((volatile uint32_t *)REG_ACM0_EVCTL8)                  /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL9                 ((volatile uint32_t *)REG_ACM0_EVCTL9)                  /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL10                ((volatile uint32_t *)REG_ACM0_EVCTL10)                 /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL11                ((volatile uint32_t *)REG_ACM0_EVCTL11)                 /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL12                ((volatile uint32_t *)REG_ACM0_EVCTL12)                 /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL13                ((volatile uint32_t *)REG_ACM0_EVCTL13)                 /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL14                ((volatile uint32_t *)REG_ACM0_EVCTL14)                 /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVCTL15                ((volatile uint32_t *)REG_ACM0_EVCTL15)                 /* ACM0 ACM Eventn Control Register */
#define pREG_ACM0_EVTIME0                ((volatile uint32_t *)REG_ACM0_EVTIME0)                 /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME1                ((volatile uint32_t *)REG_ACM0_EVTIME1)                 /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME2                ((volatile uint32_t *)REG_ACM0_EVTIME2)                 /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME3                ((volatile uint32_t *)REG_ACM0_EVTIME3)                 /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME4                ((volatile uint32_t *)REG_ACM0_EVTIME4)                 /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME5                ((volatile uint32_t *)REG_ACM0_EVTIME5)                 /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME6                ((volatile uint32_t *)REG_ACM0_EVTIME6)                 /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME7                ((volatile uint32_t *)REG_ACM0_EVTIME7)                 /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME8                ((volatile uint32_t *)REG_ACM0_EVTIME8)                 /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME9                ((volatile uint32_t *)REG_ACM0_EVTIME9)                 /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME10               ((volatile uint32_t *)REG_ACM0_EVTIME10)                /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME11               ((volatile uint32_t *)REG_ACM0_EVTIME11)                /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME12               ((volatile uint32_t *)REG_ACM0_EVTIME12)                /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME13               ((volatile uint32_t *)REG_ACM0_EVTIME13)                /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME14               ((volatile uint32_t *)REG_ACM0_EVTIME14)                /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVTIME15               ((volatile uint32_t *)REG_ACM0_EVTIME15)                /* ACM0 ACM Eventn Time Register */
#define pREG_ACM0_EVORD0                 ((volatile uint32_t *)REG_ACM0_EVORD0)                  /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD1                 ((volatile uint32_t *)REG_ACM0_EVORD1)                  /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD2                 ((volatile uint32_t *)REG_ACM0_EVORD2)                  /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD3                 ((volatile uint32_t *)REG_ACM0_EVORD3)                  /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD4                 ((volatile uint32_t *)REG_ACM0_EVORD4)                  /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD5                 ((volatile uint32_t *)REG_ACM0_EVORD5)                  /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD6                 ((volatile uint32_t *)REG_ACM0_EVORD6)                  /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD7                 ((volatile uint32_t *)REG_ACM0_EVORD7)                  /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD8                 ((volatile uint32_t *)REG_ACM0_EVORD8)                  /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD9                 ((volatile uint32_t *)REG_ACM0_EVORD9)                  /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD10                ((volatile uint32_t *)REG_ACM0_EVORD10)                 /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD11                ((volatile uint32_t *)REG_ACM0_EVORD11)                 /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD12                ((volatile uint32_t *)REG_ACM0_EVORD12)                 /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD13                ((volatile uint32_t *)REG_ACM0_EVORD13)                 /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD14                ((volatile uint32_t *)REG_ACM0_EVORD14)                 /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_EVORD15                ((volatile uint32_t *)REG_ACM0_EVORD15)                 /* ACM0 ACM Eventn Order Register */
#define pREG_ACM0_TMR0                   ((volatile uint32_t *)REG_ACM0_TMR0)                    /* ACM0 ACM Timer 0 Register */
#define pREG_ACM0_TMR1                   ((volatile uint32_t *)REG_ACM0_TMR1)                    /* ACM0 ACM Timer 1 Register */


/* =========================================================================
       DMC0
   ========================================================================= */
#define pREG_DMC0_CTL                    ((volatile uint32_t *)REG_DMC0_CTL)                     /* DMC0 Control Register */
#define pREG_DMC0_STAT                   ((volatile uint32_t *)REG_DMC0_STAT)                    /* DMC0 Status Register */
#define pREG_DMC0_EFFCTL                 ((volatile uint32_t *)REG_DMC0_EFFCTL)                  /* DMC0 Efficiency Control Register */
#define pREG_DMC0_PRIO                   ((volatile uint32_t *)REG_DMC0_PRIO)                    /* DMC0 Priority ID Register */
#define pREG_DMC0_PRIOMSK                ((volatile uint32_t *)REG_DMC0_PRIOMSK)                 /* DMC0 Priority ID Mask Register */
#define pREG_DMC0_CFG                    ((volatile uint32_t *)REG_DMC0_CFG)                     /* DMC0 Configuration Register */
#define pREG_DMC0_TR0                    ((volatile uint32_t *)REG_DMC0_TR0)                     /* DMC0 Timing 0 Register */
#define pREG_DMC0_TR1                    ((volatile uint32_t *)REG_DMC0_TR1)                     /* DMC0 Timing 1 Register */
#define pREG_DMC0_TR2                    ((volatile uint32_t *)REG_DMC0_TR2)                     /* DMC0 Timing 2 Register */
#define pREG_DMC0_MSK                    ((volatile uint32_t *)REG_DMC0_MSK)                     /* DMC0 Mask (Mode Register Shadow) Register */
#define pREG_DMC0_MR                     ((volatile uint32_t *)REG_DMC0_MR)                      /* DMC0 Shadow MR Register */
#define pREG_DMC0_EMR1                   ((volatile uint32_t *)REG_DMC0_EMR1)                    /* DMC0 Shadow EMR1 Register */
#define pREG_DMC0_EMR2                   ((volatile uint32_t *)REG_DMC0_EMR2)                    /* DMC0 Shadow EMR2 Register */
#define pREG_DMC0_EMR3                   ((volatile uint32_t *)REG_DMC0_EMR3)                    /* DMC0 Shadow EMR3 Register */
#define pREG_DMC0_DLLCTL                 ((volatile uint32_t *)REG_DMC0_DLLCTL)                  /* DMC0 DLL Control Register */
#define pREG_DMC0_PHY_CTL0               ((volatile uint32_t *)REG_DMC0_PHY_CTL0)                /* DMC0 PHY Control 0 Register */
#define pREG_DMC0_PHY_CTL1               ((volatile uint32_t *)REG_DMC0_PHY_CTL1)                /* DMC0 PHY Control 1 Register */
#define pREG_DMC0_PHY_CTL2               ((volatile uint32_t *)REG_DMC0_PHY_CTL2)                /* DMC0 PHY Control 2 Register */
#define pREG_DMC0_PHY_CTL3               ((volatile uint32_t *)REG_DMC0_PHY_CTL3)                /* DMC0 PHY Control 3 Register */
#define pREG_DMC0_PADCTL                 ((volatile uint32_t *)REG_DMC0_PADCTL)                  /* DMC0 PAD Control Register */


/* =========================================================================
       SCB0
   ========================================================================= */
#define pREG_SCB0_ARBR0                  ((volatile uint32_t *)REG_SCB0_ARBR0)                   /* SCB0 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB0_ARBR1                  ((volatile uint32_t *)REG_SCB0_ARBR1)                   /* SCB0 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB0_ARBR2                  ((volatile uint32_t *)REG_SCB0_ARBR2)                   /* SCB0 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB0_ARBR3                  ((volatile uint32_t *)REG_SCB0_ARBR3)                   /* SCB0 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB0_ARBR4                  ((volatile uint32_t *)REG_SCB0_ARBR4)                   /* SCB0 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB0_ARBR5                  ((volatile uint32_t *)REG_SCB0_ARBR5)                   /* SCB0 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB0_ARBW0                  ((volatile uint32_t *)REG_SCB0_ARBW0)                   /* SCB0 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB0_ARBW1                  ((volatile uint32_t *)REG_SCB0_ARBW1)                   /* SCB0 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB0_ARBW2                  ((volatile uint32_t *)REG_SCB0_ARBW2)                   /* SCB0 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB0_ARBW3                  ((volatile uint32_t *)REG_SCB0_ARBW3)                   /* SCB0 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB0_ARBW4                  ((volatile uint32_t *)REG_SCB0_ARBW4)                   /* SCB0 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB0_ARBW5                  ((volatile uint32_t *)REG_SCB0_ARBW5)                   /* SCB0 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB0_SLAVES                 ((volatile uint32_t *)REG_SCB0_SLAVES)                  /* SCB0 Slave Interfaces Number Register */
#define pREG_SCB0_MASTERS                ((volatile uint32_t *)REG_SCB0_MASTERS)                 /* SCB0 Master Interfaces Number Register */

/* =========================================================================
       SCB1
   ========================================================================= */
#define pREG_SCB1_ARBR0                  ((volatile uint32_t *)REG_SCB1_ARBR0)                   /* SCB1 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB1_ARBW0                  ((volatile uint32_t *)REG_SCB1_ARBW0)                   /* SCB1 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB1_SLAVES                 ((volatile uint32_t *)REG_SCB1_SLAVES)                  /* SCB1 Slave Interfaces Number Register */
#define pREG_SCB1_MASTERS                ((volatile uint32_t *)REG_SCB1_MASTERS)                 /* SCB1 Master Interfaces Number Register */

/* =========================================================================
       SCB2
   ========================================================================= */
#define pREG_SCB2_ARBR0                  ((volatile uint32_t *)REG_SCB2_ARBR0)                   /* SCB2 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB2_ARBW0                  ((volatile uint32_t *)REG_SCB2_ARBW0)                   /* SCB2 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB2_SLAVES                 ((volatile uint32_t *)REG_SCB2_SLAVES)                  /* SCB2 Slave Interfaces Number Register */
#define pREG_SCB2_MASTERS                ((volatile uint32_t *)REG_SCB2_MASTERS)                 /* SCB2 Master Interfaces Number Register */

/* =========================================================================
       SCB3
   ========================================================================= */
#define pREG_SCB3_ARBR0                  ((volatile uint32_t *)REG_SCB3_ARBR0)                   /* SCB3 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB3_ARBW0                  ((volatile uint32_t *)REG_SCB3_ARBW0)                   /* SCB3 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB3_SLAVES                 ((volatile uint32_t *)REG_SCB3_SLAVES)                  /* SCB3 Slave Interfaces Number Register */
#define pREG_SCB3_MASTERS                ((volatile uint32_t *)REG_SCB3_MASTERS)                 /* SCB3 Master Interfaces Number Register */

/* =========================================================================
       SCB4
   ========================================================================= */
#define pREG_SCB4_ARBR0                  ((volatile uint32_t *)REG_SCB4_ARBR0)                   /* SCB4 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB4_ARBW0                  ((volatile uint32_t *)REG_SCB4_ARBW0)                   /* SCB4 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB4_SLAVES                 ((volatile uint32_t *)REG_SCB4_SLAVES)                  /* SCB4 Slave Interfaces Number Register */
#define pREG_SCB4_MASTERS                ((volatile uint32_t *)REG_SCB4_MASTERS)                 /* SCB4 Master Interfaces Number Register */

/* =========================================================================
       SCB5
   ========================================================================= */
#define pREG_SCB5_ARBR0                  ((volatile uint32_t *)REG_SCB5_ARBR0)                   /* SCB5 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB5_ARBW0                  ((volatile uint32_t *)REG_SCB5_ARBW0)                   /* SCB5 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB5_SLAVES                 ((volatile uint32_t *)REG_SCB5_SLAVES)                  /* SCB5 Slave Interfaces Number Register */
#define pREG_SCB5_MASTERS                ((volatile uint32_t *)REG_SCB5_MASTERS)                 /* SCB5 Master Interfaces Number Register */

/* =========================================================================
       SCB6
   ========================================================================= */
#define pREG_SCB6_ARBR0                  ((volatile uint32_t *)REG_SCB6_ARBR0)                   /* SCB6 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB6_ARBW0                  ((volatile uint32_t *)REG_SCB6_ARBW0)                   /* SCB6 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB6_SLAVES                 ((volatile uint32_t *)REG_SCB6_SLAVES)                  /* SCB6 Slave Interfaces Number Register */
#define pREG_SCB6_MASTERS                ((volatile uint32_t *)REG_SCB6_MASTERS)                 /* SCB6 Master Interfaces Number Register */

/* =========================================================================
       SCB7
   ========================================================================= */
#define pREG_SCB7_ARBR0                  ((volatile uint32_t *)REG_SCB7_ARBR0)                   /* SCB7 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB7_ARBW0                  ((volatile uint32_t *)REG_SCB7_ARBW0)                   /* SCB7 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB7_SLAVES                 ((volatile uint32_t *)REG_SCB7_SLAVES)                  /* SCB7 Slave Interfaces Number Register */
#define pREG_SCB7_MASTERS                ((volatile uint32_t *)REG_SCB7_MASTERS)                 /* SCB7 Master Interfaces Number Register */

/* =========================================================================
       SCB8
   ========================================================================= */
#define pREG_SCB8_ARBR0                  ((volatile uint32_t *)REG_SCB8_ARBR0)                   /* SCB8 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB8_ARBW0                  ((volatile uint32_t *)REG_SCB8_ARBW0)                   /* SCB8 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB8_SLAVES                 ((volatile uint32_t *)REG_SCB8_SLAVES)                  /* SCB8 Slave Interfaces Number Register */
#define pREG_SCB8_MASTERS                ((volatile uint32_t *)REG_SCB8_MASTERS)                 /* SCB8 Master Interfaces Number Register */

/* =========================================================================
       SCB9
   ========================================================================= */
#define pREG_SCB9_ARBR0                  ((volatile uint32_t *)REG_SCB9_ARBR0)                   /* SCB9 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB9_ARBW0                  ((volatile uint32_t *)REG_SCB9_ARBW0)                   /* SCB9 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB9_SLAVES                 ((volatile uint32_t *)REG_SCB9_SLAVES)                  /* SCB9 Slave Interfaces Number Register */
#define pREG_SCB9_MASTERS                ((volatile uint32_t *)REG_SCB9_MASTERS)                 /* SCB9 Master Interfaces Number Register */

/* =========================================================================
       SCB10
   ========================================================================= */
#define pREG_SCB10_ARBR0                 ((volatile uint32_t *)REG_SCB10_ARBR0)                  /* SCB10 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB10_ARBR1                 ((volatile uint32_t *)REG_SCB10_ARBR1)                  /* SCB10 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB10_ARBR2                 ((volatile uint32_t *)REG_SCB10_ARBR2)                  /* SCB10 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB10_ARBW0                 ((volatile uint32_t *)REG_SCB10_ARBW0)                  /* SCB10 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB10_ARBW1                 ((volatile uint32_t *)REG_SCB10_ARBW1)                  /* SCB10 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB10_ARBW2                 ((volatile uint32_t *)REG_SCB10_ARBW2)                  /* SCB10 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB10_SLAVES                ((volatile uint32_t *)REG_SCB10_SLAVES)                 /* SCB10 Slave Interfaces Number Register */
#define pREG_SCB10_MASTERS               ((volatile uint32_t *)REG_SCB10_MASTERS)                /* SCB10 Master Interfaces Number Register */

/* =========================================================================
       SCB11
   ========================================================================= */
#define pREG_SCB11_ARBR0                 ((volatile uint32_t *)REG_SCB11_ARBR0)                  /* SCB11 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB11_ARBR1                 ((volatile uint32_t *)REG_SCB11_ARBR1)                  /* SCB11 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB11_ARBR2                 ((volatile uint32_t *)REG_SCB11_ARBR2)                  /* SCB11 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB11_ARBR3                 ((volatile uint32_t *)REG_SCB11_ARBR3)                  /* SCB11 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB11_ARBR4                 ((volatile uint32_t *)REG_SCB11_ARBR4)                  /* SCB11 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB11_ARBR5                 ((volatile uint32_t *)REG_SCB11_ARBR5)                  /* SCB11 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB11_ARBR6                 ((volatile uint32_t *)REG_SCB11_ARBR6)                  /* SCB11 Arbitration Read Channel Master Interface n Register */
#define pREG_SCB11_ARBW0                 ((volatile uint32_t *)REG_SCB11_ARBW0)                  /* SCB11 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB11_ARBW1                 ((volatile uint32_t *)REG_SCB11_ARBW1)                  /* SCB11 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB11_ARBW2                 ((volatile uint32_t *)REG_SCB11_ARBW2)                  /* SCB11 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB11_ARBW3                 ((volatile uint32_t *)REG_SCB11_ARBW3)                  /* SCB11 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB11_ARBW4                 ((volatile uint32_t *)REG_SCB11_ARBW4)                  /* SCB11 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB11_ARBW5                 ((volatile uint32_t *)REG_SCB11_ARBW5)                  /* SCB11 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB11_ARBW6                 ((volatile uint32_t *)REG_SCB11_ARBW6)                  /* SCB11 Arbitration Write Channel Master Interface n Register */
#define pREG_SCB11_SLAVES                ((volatile uint32_t *)REG_SCB11_SLAVES)                 /* SCB11 Slave Interfaces Number Register */
#define pREG_SCB11_MASTERS               ((volatile uint32_t *)REG_SCB11_MASTERS)                /* SCB11 Master Interfaces Number Register */


/* =========================================================================
       L2CTL0
   ========================================================================= */
#define pREG_L2CTL0_CTL                  ((volatile uint32_t *)REG_L2CTL0_CTL)                   /* L2CTL0 Control Register */
#define pREG_L2CTL0_ACTL_C0              ((volatile uint32_t *)REG_L2CTL0_ACTL_C0)               /* L2CTL0 Access Control Core 0 Register */
#define pREG_L2CTL0_ACTL_C1              ((volatile uint32_t *)REG_L2CTL0_ACTL_C1)               /* L2CTL0 Access Control Core 1 Register */
#define pREG_L2CTL0_ACTL_SYS             ((volatile uint32_t *)REG_L2CTL0_ACTL_SYS)              /* L2CTL0 Access Control System Register */
#define pREG_L2CTL0_STAT                 ((volatile uint32_t *)REG_L2CTL0_STAT)                  /* L2CTL0 Status Register */
#define pREG_L2CTL0_RPCR                 ((volatile uint32_t *)REG_L2CTL0_RPCR)                  /* L2CTL0 Read Priority Count Register */
#define pREG_L2CTL0_WPCR                 ((volatile uint32_t *)REG_L2CTL0_WPCR)                  /* L2CTL0 Write Priority Count Register */
#define pREG_L2CTL0_RFA                  ((void * volatile *)REG_L2CTL0_RFA)                     /* L2CTL0 Refresh Address Register */
#define pREG_L2CTL0_ERRADDR0             ((void * volatile *)REG_L2CTL0_ERRADDR0)                /* L2CTL0 ECC Error Address 0 Register */
#define pREG_L2CTL0_ERRADDR1             ((void * volatile *)REG_L2CTL0_ERRADDR1)                /* L2CTL0 ECC Error Address 1 Register */
#define pREG_L2CTL0_ERRADDR2             ((void * volatile *)REG_L2CTL0_ERRADDR2)                /* L2CTL0 ECC Error Address 2 Register */
#define pREG_L2CTL0_ERRADDR3             ((void * volatile *)REG_L2CTL0_ERRADDR3)                /* L2CTL0 ECC Error Address 3 Register */
#define pREG_L2CTL0_ERRADDR4             ((void * volatile *)REG_L2CTL0_ERRADDR4)                /* L2CTL0 ECC Error Address 4 Register */
#define pREG_L2CTL0_ERRADDR5             ((void * volatile *)REG_L2CTL0_ERRADDR5)                /* L2CTL0 ECC Error Address 5 Register */
#define pREG_L2CTL0_ERRADDR6             ((void * volatile *)REG_L2CTL0_ERRADDR6)                /* L2CTL0 ECC Error Address 6 Register */
#define pREG_L2CTL0_ERRADDR7             ((void * volatile *)REG_L2CTL0_ERRADDR7)                /* L2CTL0 ECC Error Address 7 Register */
#define pREG_L2CTL0_ET0                  ((volatile uint32_t *)REG_L2CTL0_ET0)                   /* L2CTL0 Error Type 0 Register */
#define pREG_L2CTL0_EADDR0               ((void * volatile *)REG_L2CTL0_EADDR0)                  /* L2CTL0 Error Type 0 Address Register */
#define pREG_L2CTL0_ET1                  ((volatile uint32_t *)REG_L2CTL0_ET1)                   /* L2CTL0 Error Type 1 Register */
#define pREG_L2CTL0_EADDR1               ((void * volatile *)REG_L2CTL0_EADDR1)                  /* L2CTL0 Error Type 1 Address Register */


/* =========================================================================
       SEC0
   ========================================================================= */

/*    SEC Core Interface (SCI) Registers    */
#define pREG_SEC0_CCTL0                  ((volatile uint32_t *)REG_SEC0_CCTL0)                   /* SEC0 SCI Control Register n */
#define pREG_SEC0_CCTL1                  ((volatile uint32_t *)REG_SEC0_CCTL1)                   /* SEC0 SCI Control Register n */
#define pREG_SEC0_CSTAT0                 ((volatile uint32_t *)REG_SEC0_CSTAT0)                  /* SEC0 SCI Status Register n */
#define pREG_SEC0_CSTAT1                 ((volatile uint32_t *)REG_SEC0_CSTAT1)                  /* SEC0 SCI Status Register n */
#define pREG_SEC0_CPND0                  ((volatile uint32_t *)REG_SEC0_CPND0)                   /* SEC0 Core Pending Register n */
#define pREG_SEC0_CPND1                  ((volatile uint32_t *)REG_SEC0_CPND1)                   /* SEC0 Core Pending Register n */
#define pREG_SEC0_CACT0                  ((volatile uint32_t *)REG_SEC0_CACT0)                   /* SEC0 SCI Active Register n */
#define pREG_SEC0_CACT1                  ((volatile uint32_t *)REG_SEC0_CACT1)                   /* SEC0 SCI Active Register n */
#define pREG_SEC0_CPMSK0                 ((volatile uint32_t *)REG_SEC0_CPMSK0)                  /* SEC0 SCI Priority Mask Register n */
#define pREG_SEC0_CPMSK1                 ((volatile uint32_t *)REG_SEC0_CPMSK1)                  /* SEC0 SCI Priority Mask Register n */
#define pREG_SEC0_CGMSK0                 ((volatile uint32_t *)REG_SEC0_CGMSK0)                  /* SEC0 SCI Group Mask Register n */
#define pREG_SEC0_CGMSK1                 ((volatile uint32_t *)REG_SEC0_CGMSK1)                  /* SEC0 SCI Group Mask Register n */
#define pREG_SEC0_CPLVL0                 ((volatile uint32_t *)REG_SEC0_CPLVL0)                  /* SEC0 SCI Priority Level Register n */
#define pREG_SEC0_CPLVL1                 ((volatile uint32_t *)REG_SEC0_CPLVL1)                  /* SEC0 SCI Priority Level Register n */
#define pREG_SEC0_CSID0                  ((volatile uint32_t *)REG_SEC0_CSID0)                   /* SEC0 SCI Source ID Register n */
#define pREG_SEC0_CSID1                  ((volatile uint32_t *)REG_SEC0_CSID1)                   /* SEC0 SCI Source ID Register n */

/*    SEC Fault Management Interface (SFI) Registers    */
#define pREG_SEC0_FCTL                   ((volatile uint32_t *)REG_SEC0_FCTL)                    /* SEC0 Fault Control Register */
#define pREG_SEC0_FSTAT                  ((volatile uint32_t *)REG_SEC0_FSTAT)                   /* SEC0 Fault Status Register */
#define pREG_SEC0_FSID                   ((volatile uint32_t *)REG_SEC0_FSID)                    /* SEC0 Fault Source ID Register */
#define pREG_SEC0_FEND                   ((volatile uint32_t *)REG_SEC0_FEND)                    /* SEC0 Fault End Register */
#define pREG_SEC0_FDLY                   ((volatile uint32_t *)REG_SEC0_FDLY)                    /* SEC0 Fault Delay Register */
#define pREG_SEC0_FDLY_CUR               ((volatile uint32_t *)REG_SEC0_FDLY_CUR)                /* SEC0 Fault Delay Current Register */
#define pREG_SEC0_FSRDLY                 ((volatile uint32_t *)REG_SEC0_FSRDLY)                  /* SEC0 Fault System Reset Delay Register */
#define pREG_SEC0_FSRDLY_CUR             ((volatile uint32_t *)REG_SEC0_FSRDLY_CUR)              /* SEC0 Fault System Reset Delay Current Register */
#define pREG_SEC0_FCOPP                  ((volatile uint32_t *)REG_SEC0_FCOPP)                   /* SEC0 Fault COP Period Register */
#define pREG_SEC0_FCOPP_CUR              ((volatile uint32_t *)REG_SEC0_FCOPP_CUR)               /* SEC0 Fault COP Period Current Register */

/*    SEC Global Registers    */
#define pREG_SEC0_GCTL                   ((volatile uint32_t *)REG_SEC0_GCTL)                    /* SEC0 Global Control Register */
#define pREG_SEC0_GSTAT                  ((volatile uint32_t *)REG_SEC0_GSTAT)                   /* SEC0 Global Status Register */
#define pREG_SEC0_RAISE                  ((volatile uint32_t *)REG_SEC0_RAISE)                   /* SEC0 Global Raise Register */
#define pREG_SEC0_END                    ((volatile uint32_t *)REG_SEC0_END)                     /* SEC0 Global End Register */

/*    SEC Source Interface (SSI) Registers    */
#define pREG_SEC0_SCTL0                  ((volatile uint32_t *)REG_SEC0_SCTL0)                   /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL1                  ((volatile uint32_t *)REG_SEC0_SCTL1)                   /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL2                  ((volatile uint32_t *)REG_SEC0_SCTL2)                   /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL3                  ((volatile uint32_t *)REG_SEC0_SCTL3)                   /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL4                  ((volatile uint32_t *)REG_SEC0_SCTL4)                   /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL5                  ((volatile uint32_t *)REG_SEC0_SCTL5)                   /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL6                  ((volatile uint32_t *)REG_SEC0_SCTL6)                   /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL7                  ((volatile uint32_t *)REG_SEC0_SCTL7)                   /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL8                  ((volatile uint32_t *)REG_SEC0_SCTL8)                   /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL9                  ((volatile uint32_t *)REG_SEC0_SCTL9)                   /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL10                 ((volatile uint32_t *)REG_SEC0_SCTL10)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL11                 ((volatile uint32_t *)REG_SEC0_SCTL11)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL12                 ((volatile uint32_t *)REG_SEC0_SCTL12)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL13                 ((volatile uint32_t *)REG_SEC0_SCTL13)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL14                 ((volatile uint32_t *)REG_SEC0_SCTL14)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL15                 ((volatile uint32_t *)REG_SEC0_SCTL15)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL16                 ((volatile uint32_t *)REG_SEC0_SCTL16)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL17                 ((volatile uint32_t *)REG_SEC0_SCTL17)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL18                 ((volatile uint32_t *)REG_SEC0_SCTL18)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL19                 ((volatile uint32_t *)REG_SEC0_SCTL19)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL20                 ((volatile uint32_t *)REG_SEC0_SCTL20)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL21                 ((volatile uint32_t *)REG_SEC0_SCTL21)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL22                 ((volatile uint32_t *)REG_SEC0_SCTL22)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL23                 ((volatile uint32_t *)REG_SEC0_SCTL23)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL24                 ((volatile uint32_t *)REG_SEC0_SCTL24)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL25                 ((volatile uint32_t *)REG_SEC0_SCTL25)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL26                 ((volatile uint32_t *)REG_SEC0_SCTL26)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL27                 ((volatile uint32_t *)REG_SEC0_SCTL27)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL28                 ((volatile uint32_t *)REG_SEC0_SCTL28)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL29                 ((volatile uint32_t *)REG_SEC0_SCTL29)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL30                 ((volatile uint32_t *)REG_SEC0_SCTL30)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL31                 ((volatile uint32_t *)REG_SEC0_SCTL31)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL32                 ((volatile uint32_t *)REG_SEC0_SCTL32)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL33                 ((volatile uint32_t *)REG_SEC0_SCTL33)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL34                 ((volatile uint32_t *)REG_SEC0_SCTL34)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL35                 ((volatile uint32_t *)REG_SEC0_SCTL35)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL36                 ((volatile uint32_t *)REG_SEC0_SCTL36)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL37                 ((volatile uint32_t *)REG_SEC0_SCTL37)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL38                 ((volatile uint32_t *)REG_SEC0_SCTL38)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL39                 ((volatile uint32_t *)REG_SEC0_SCTL39)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL40                 ((volatile uint32_t *)REG_SEC0_SCTL40)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL41                 ((volatile uint32_t *)REG_SEC0_SCTL41)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL42                 ((volatile uint32_t *)REG_SEC0_SCTL42)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL43                 ((volatile uint32_t *)REG_SEC0_SCTL43)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL44                 ((volatile uint32_t *)REG_SEC0_SCTL44)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL45                 ((volatile uint32_t *)REG_SEC0_SCTL45)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL46                 ((volatile uint32_t *)REG_SEC0_SCTL46)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL47                 ((volatile uint32_t *)REG_SEC0_SCTL47)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL48                 ((volatile uint32_t *)REG_SEC0_SCTL48)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL49                 ((volatile uint32_t *)REG_SEC0_SCTL49)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL50                 ((volatile uint32_t *)REG_SEC0_SCTL50)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL51                 ((volatile uint32_t *)REG_SEC0_SCTL51)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL52                 ((volatile uint32_t *)REG_SEC0_SCTL52)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL53                 ((volatile uint32_t *)REG_SEC0_SCTL53)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL54                 ((volatile uint32_t *)REG_SEC0_SCTL54)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL55                 ((volatile uint32_t *)REG_SEC0_SCTL55)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL56                 ((volatile uint32_t *)REG_SEC0_SCTL56)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL57                 ((volatile uint32_t *)REG_SEC0_SCTL57)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL58                 ((volatile uint32_t *)REG_SEC0_SCTL58)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL59                 ((volatile uint32_t *)REG_SEC0_SCTL59)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL60                 ((volatile uint32_t *)REG_SEC0_SCTL60)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL61                 ((volatile uint32_t *)REG_SEC0_SCTL61)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL62                 ((volatile uint32_t *)REG_SEC0_SCTL62)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL63                 ((volatile uint32_t *)REG_SEC0_SCTL63)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL64                 ((volatile uint32_t *)REG_SEC0_SCTL64)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL65                 ((volatile uint32_t *)REG_SEC0_SCTL65)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL66                 ((volatile uint32_t *)REG_SEC0_SCTL66)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL67                 ((volatile uint32_t *)REG_SEC0_SCTL67)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL68                 ((volatile uint32_t *)REG_SEC0_SCTL68)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL69                 ((volatile uint32_t *)REG_SEC0_SCTL69)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL70                 ((volatile uint32_t *)REG_SEC0_SCTL70)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL71                 ((volatile uint32_t *)REG_SEC0_SCTL71)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL72                 ((volatile uint32_t *)REG_SEC0_SCTL72)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL73                 ((volatile uint32_t *)REG_SEC0_SCTL73)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL74                 ((volatile uint32_t *)REG_SEC0_SCTL74)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL75                 ((volatile uint32_t *)REG_SEC0_SCTL75)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL76                 ((volatile uint32_t *)REG_SEC0_SCTL76)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL77                 ((volatile uint32_t *)REG_SEC0_SCTL77)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL78                 ((volatile uint32_t *)REG_SEC0_SCTL78)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL79                 ((volatile uint32_t *)REG_SEC0_SCTL79)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL80                 ((volatile uint32_t *)REG_SEC0_SCTL80)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL81                 ((volatile uint32_t *)REG_SEC0_SCTL81)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL82                 ((volatile uint32_t *)REG_SEC0_SCTL82)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL83                 ((volatile uint32_t *)REG_SEC0_SCTL83)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL84                 ((volatile uint32_t *)REG_SEC0_SCTL84)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL85                 ((volatile uint32_t *)REG_SEC0_SCTL85)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL86                 ((volatile uint32_t *)REG_SEC0_SCTL86)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL87                 ((volatile uint32_t *)REG_SEC0_SCTL87)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL88                 ((volatile uint32_t *)REG_SEC0_SCTL88)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL89                 ((volatile uint32_t *)REG_SEC0_SCTL89)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL90                 ((volatile uint32_t *)REG_SEC0_SCTL90)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL91                 ((volatile uint32_t *)REG_SEC0_SCTL91)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL92                 ((volatile uint32_t *)REG_SEC0_SCTL92)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL93                 ((volatile uint32_t *)REG_SEC0_SCTL93)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL94                 ((volatile uint32_t *)REG_SEC0_SCTL94)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL95                 ((volatile uint32_t *)REG_SEC0_SCTL95)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL96                 ((volatile uint32_t *)REG_SEC0_SCTL96)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL97                 ((volatile uint32_t *)REG_SEC0_SCTL97)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL98                 ((volatile uint32_t *)REG_SEC0_SCTL98)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL99                 ((volatile uint32_t *)REG_SEC0_SCTL99)                  /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL100                ((volatile uint32_t *)REG_SEC0_SCTL100)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL101                ((volatile uint32_t *)REG_SEC0_SCTL101)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL102                ((volatile uint32_t *)REG_SEC0_SCTL102)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL103                ((volatile uint32_t *)REG_SEC0_SCTL103)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL104                ((volatile uint32_t *)REG_SEC0_SCTL104)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL105                ((volatile uint32_t *)REG_SEC0_SCTL105)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL106                ((volatile uint32_t *)REG_SEC0_SCTL106)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL107                ((volatile uint32_t *)REG_SEC0_SCTL107)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL108                ((volatile uint32_t *)REG_SEC0_SCTL108)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL109                ((volatile uint32_t *)REG_SEC0_SCTL109)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL110                ((volatile uint32_t *)REG_SEC0_SCTL110)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL111                ((volatile uint32_t *)REG_SEC0_SCTL111)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL112                ((volatile uint32_t *)REG_SEC0_SCTL112)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL113                ((volatile uint32_t *)REG_SEC0_SCTL113)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL114                ((volatile uint32_t *)REG_SEC0_SCTL114)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL115                ((volatile uint32_t *)REG_SEC0_SCTL115)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL116                ((volatile uint32_t *)REG_SEC0_SCTL116)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL117                ((volatile uint32_t *)REG_SEC0_SCTL117)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL118                ((volatile uint32_t *)REG_SEC0_SCTL118)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL119                ((volatile uint32_t *)REG_SEC0_SCTL119)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL120                ((volatile uint32_t *)REG_SEC0_SCTL120)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL121                ((volatile uint32_t *)REG_SEC0_SCTL121)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL122                ((volatile uint32_t *)REG_SEC0_SCTL122)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL123                ((volatile uint32_t *)REG_SEC0_SCTL123)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL124                ((volatile uint32_t *)REG_SEC0_SCTL124)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL125                ((volatile uint32_t *)REG_SEC0_SCTL125)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL126                ((volatile uint32_t *)REG_SEC0_SCTL126)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL127                ((volatile uint32_t *)REG_SEC0_SCTL127)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL128                ((volatile uint32_t *)REG_SEC0_SCTL128)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL129                ((volatile uint32_t *)REG_SEC0_SCTL129)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL130                ((volatile uint32_t *)REG_SEC0_SCTL130)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL131                ((volatile uint32_t *)REG_SEC0_SCTL131)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL132                ((volatile uint32_t *)REG_SEC0_SCTL132)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL133                ((volatile uint32_t *)REG_SEC0_SCTL133)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL134                ((volatile uint32_t *)REG_SEC0_SCTL134)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL135                ((volatile uint32_t *)REG_SEC0_SCTL135)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL136                ((volatile uint32_t *)REG_SEC0_SCTL136)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL137                ((volatile uint32_t *)REG_SEC0_SCTL137)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL138                ((volatile uint32_t *)REG_SEC0_SCTL138)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SCTL139                ((volatile uint32_t *)REG_SEC0_SCTL139)                 /* SEC0 Source Control Register n */
#define pREG_SEC0_SSTAT0                 ((volatile uint32_t *)REG_SEC0_SSTAT0)                  /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT1                 ((volatile uint32_t *)REG_SEC0_SSTAT1)                  /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT2                 ((volatile uint32_t *)REG_SEC0_SSTAT2)                  /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT3                 ((volatile uint32_t *)REG_SEC0_SSTAT3)                  /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT4                 ((volatile uint32_t *)REG_SEC0_SSTAT4)                  /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT5                 ((volatile uint32_t *)REG_SEC0_SSTAT5)                  /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT6                 ((volatile uint32_t *)REG_SEC0_SSTAT6)                  /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT7                 ((volatile uint32_t *)REG_SEC0_SSTAT7)                  /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT8                 ((volatile uint32_t *)REG_SEC0_SSTAT8)                  /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT9                 ((volatile uint32_t *)REG_SEC0_SSTAT9)                  /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT10                ((volatile uint32_t *)REG_SEC0_SSTAT10)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT11                ((volatile uint32_t *)REG_SEC0_SSTAT11)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT12                ((volatile uint32_t *)REG_SEC0_SSTAT12)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT13                ((volatile uint32_t *)REG_SEC0_SSTAT13)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT14                ((volatile uint32_t *)REG_SEC0_SSTAT14)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT15                ((volatile uint32_t *)REG_SEC0_SSTAT15)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT16                ((volatile uint32_t *)REG_SEC0_SSTAT16)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT17                ((volatile uint32_t *)REG_SEC0_SSTAT17)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT18                ((volatile uint32_t *)REG_SEC0_SSTAT18)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT19                ((volatile uint32_t *)REG_SEC0_SSTAT19)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT20                ((volatile uint32_t *)REG_SEC0_SSTAT20)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT21                ((volatile uint32_t *)REG_SEC0_SSTAT21)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT22                ((volatile uint32_t *)REG_SEC0_SSTAT22)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT23                ((volatile uint32_t *)REG_SEC0_SSTAT23)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT24                ((volatile uint32_t *)REG_SEC0_SSTAT24)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT25                ((volatile uint32_t *)REG_SEC0_SSTAT25)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT26                ((volatile uint32_t *)REG_SEC0_SSTAT26)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT27                ((volatile uint32_t *)REG_SEC0_SSTAT27)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT28                ((volatile uint32_t *)REG_SEC0_SSTAT28)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT29                ((volatile uint32_t *)REG_SEC0_SSTAT29)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT30                ((volatile uint32_t *)REG_SEC0_SSTAT30)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT31                ((volatile uint32_t *)REG_SEC0_SSTAT31)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT32                ((volatile uint32_t *)REG_SEC0_SSTAT32)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT33                ((volatile uint32_t *)REG_SEC0_SSTAT33)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT34                ((volatile uint32_t *)REG_SEC0_SSTAT34)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT35                ((volatile uint32_t *)REG_SEC0_SSTAT35)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT36                ((volatile uint32_t *)REG_SEC0_SSTAT36)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT37                ((volatile uint32_t *)REG_SEC0_SSTAT37)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT38                ((volatile uint32_t *)REG_SEC0_SSTAT38)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT39                ((volatile uint32_t *)REG_SEC0_SSTAT39)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT40                ((volatile uint32_t *)REG_SEC0_SSTAT40)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT41                ((volatile uint32_t *)REG_SEC0_SSTAT41)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT42                ((volatile uint32_t *)REG_SEC0_SSTAT42)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT43                ((volatile uint32_t *)REG_SEC0_SSTAT43)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT44                ((volatile uint32_t *)REG_SEC0_SSTAT44)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT45                ((volatile uint32_t *)REG_SEC0_SSTAT45)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT46                ((volatile uint32_t *)REG_SEC0_SSTAT46)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT47                ((volatile uint32_t *)REG_SEC0_SSTAT47)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT48                ((volatile uint32_t *)REG_SEC0_SSTAT48)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT49                ((volatile uint32_t *)REG_SEC0_SSTAT49)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT50                ((volatile uint32_t *)REG_SEC0_SSTAT50)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT51                ((volatile uint32_t *)REG_SEC0_SSTAT51)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT52                ((volatile uint32_t *)REG_SEC0_SSTAT52)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT53                ((volatile uint32_t *)REG_SEC0_SSTAT53)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT54                ((volatile uint32_t *)REG_SEC0_SSTAT54)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT55                ((volatile uint32_t *)REG_SEC0_SSTAT55)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT56                ((volatile uint32_t *)REG_SEC0_SSTAT56)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT57                ((volatile uint32_t *)REG_SEC0_SSTAT57)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT58                ((volatile uint32_t *)REG_SEC0_SSTAT58)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT59                ((volatile uint32_t *)REG_SEC0_SSTAT59)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT60                ((volatile uint32_t *)REG_SEC0_SSTAT60)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT61                ((volatile uint32_t *)REG_SEC0_SSTAT61)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT62                ((volatile uint32_t *)REG_SEC0_SSTAT62)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT63                ((volatile uint32_t *)REG_SEC0_SSTAT63)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT64                ((volatile uint32_t *)REG_SEC0_SSTAT64)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT65                ((volatile uint32_t *)REG_SEC0_SSTAT65)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT66                ((volatile uint32_t *)REG_SEC0_SSTAT66)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT67                ((volatile uint32_t *)REG_SEC0_SSTAT67)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT68                ((volatile uint32_t *)REG_SEC0_SSTAT68)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT69                ((volatile uint32_t *)REG_SEC0_SSTAT69)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT70                ((volatile uint32_t *)REG_SEC0_SSTAT70)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT71                ((volatile uint32_t *)REG_SEC0_SSTAT71)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT72                ((volatile uint32_t *)REG_SEC0_SSTAT72)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT73                ((volatile uint32_t *)REG_SEC0_SSTAT73)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT74                ((volatile uint32_t *)REG_SEC0_SSTAT74)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT75                ((volatile uint32_t *)REG_SEC0_SSTAT75)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT76                ((volatile uint32_t *)REG_SEC0_SSTAT76)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT77                ((volatile uint32_t *)REG_SEC0_SSTAT77)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT78                ((volatile uint32_t *)REG_SEC0_SSTAT78)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT79                ((volatile uint32_t *)REG_SEC0_SSTAT79)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT80                ((volatile uint32_t *)REG_SEC0_SSTAT80)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT81                ((volatile uint32_t *)REG_SEC0_SSTAT81)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT82                ((volatile uint32_t *)REG_SEC0_SSTAT82)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT83                ((volatile uint32_t *)REG_SEC0_SSTAT83)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT84                ((volatile uint32_t *)REG_SEC0_SSTAT84)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT85                ((volatile uint32_t *)REG_SEC0_SSTAT85)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT86                ((volatile uint32_t *)REG_SEC0_SSTAT86)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT87                ((volatile uint32_t *)REG_SEC0_SSTAT87)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT88                ((volatile uint32_t *)REG_SEC0_SSTAT88)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT89                ((volatile uint32_t *)REG_SEC0_SSTAT89)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT90                ((volatile uint32_t *)REG_SEC0_SSTAT90)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT91                ((volatile uint32_t *)REG_SEC0_SSTAT91)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT92                ((volatile uint32_t *)REG_SEC0_SSTAT92)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT93                ((volatile uint32_t *)REG_SEC0_SSTAT93)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT94                ((volatile uint32_t *)REG_SEC0_SSTAT94)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT95                ((volatile uint32_t *)REG_SEC0_SSTAT95)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT96                ((volatile uint32_t *)REG_SEC0_SSTAT96)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT97                ((volatile uint32_t *)REG_SEC0_SSTAT97)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT98                ((volatile uint32_t *)REG_SEC0_SSTAT98)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT99                ((volatile uint32_t *)REG_SEC0_SSTAT99)                 /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT100               ((volatile uint32_t *)REG_SEC0_SSTAT100)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT101               ((volatile uint32_t *)REG_SEC0_SSTAT101)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT102               ((volatile uint32_t *)REG_SEC0_SSTAT102)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT103               ((volatile uint32_t *)REG_SEC0_SSTAT103)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT104               ((volatile uint32_t *)REG_SEC0_SSTAT104)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT105               ((volatile uint32_t *)REG_SEC0_SSTAT105)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT106               ((volatile uint32_t *)REG_SEC0_SSTAT106)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT107               ((volatile uint32_t *)REG_SEC0_SSTAT107)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT108               ((volatile uint32_t *)REG_SEC0_SSTAT108)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT109               ((volatile uint32_t *)REG_SEC0_SSTAT109)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT110               ((volatile uint32_t *)REG_SEC0_SSTAT110)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT111               ((volatile uint32_t *)REG_SEC0_SSTAT111)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT112               ((volatile uint32_t *)REG_SEC0_SSTAT112)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT113               ((volatile uint32_t *)REG_SEC0_SSTAT113)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT114               ((volatile uint32_t *)REG_SEC0_SSTAT114)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT115               ((volatile uint32_t *)REG_SEC0_SSTAT115)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT116               ((volatile uint32_t *)REG_SEC0_SSTAT116)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT117               ((volatile uint32_t *)REG_SEC0_SSTAT117)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT118               ((volatile uint32_t *)REG_SEC0_SSTAT118)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT119               ((volatile uint32_t *)REG_SEC0_SSTAT119)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT120               ((volatile uint32_t *)REG_SEC0_SSTAT120)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT121               ((volatile uint32_t *)REG_SEC0_SSTAT121)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT122               ((volatile uint32_t *)REG_SEC0_SSTAT122)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT123               ((volatile uint32_t *)REG_SEC0_SSTAT123)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT124               ((volatile uint32_t *)REG_SEC0_SSTAT124)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT125               ((volatile uint32_t *)REG_SEC0_SSTAT125)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT126               ((volatile uint32_t *)REG_SEC0_SSTAT126)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT127               ((volatile uint32_t *)REG_SEC0_SSTAT127)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT128               ((volatile uint32_t *)REG_SEC0_SSTAT128)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT129               ((volatile uint32_t *)REG_SEC0_SSTAT129)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT130               ((volatile uint32_t *)REG_SEC0_SSTAT130)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT131               ((volatile uint32_t *)REG_SEC0_SSTAT131)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT132               ((volatile uint32_t *)REG_SEC0_SSTAT132)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT133               ((volatile uint32_t *)REG_SEC0_SSTAT133)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT134               ((volatile uint32_t *)REG_SEC0_SSTAT134)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT135               ((volatile uint32_t *)REG_SEC0_SSTAT135)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT136               ((volatile uint32_t *)REG_SEC0_SSTAT136)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT137               ((volatile uint32_t *)REG_SEC0_SSTAT137)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT138               ((volatile uint32_t *)REG_SEC0_SSTAT138)                /* SEC0 Source Status Register n */
#define pREG_SEC0_SSTAT139               ((volatile uint32_t *)REG_SEC0_SSTAT139)                /* SEC0 Source Status Register n */


/* =========================================================================
       TRU0
   ========================================================================= */
#define pREG_TRU0_SSR0                   ((volatile uint32_t *)REG_TRU0_SSR0)                    /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR1                   ((volatile uint32_t *)REG_TRU0_SSR1)                    /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR2                   ((volatile uint32_t *)REG_TRU0_SSR2)                    /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR3                   ((volatile uint32_t *)REG_TRU0_SSR3)                    /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR4                   ((volatile uint32_t *)REG_TRU0_SSR4)                    /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR5                   ((volatile uint32_t *)REG_TRU0_SSR5)                    /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR6                   ((volatile uint32_t *)REG_TRU0_SSR6)                    /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR7                   ((volatile uint32_t *)REG_TRU0_SSR7)                    /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR8                   ((volatile uint32_t *)REG_TRU0_SSR8)                    /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR9                   ((volatile uint32_t *)REG_TRU0_SSR9)                    /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR10                  ((volatile uint32_t *)REG_TRU0_SSR10)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR11                  ((volatile uint32_t *)REG_TRU0_SSR11)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR12                  ((volatile uint32_t *)REG_TRU0_SSR12)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR13                  ((volatile uint32_t *)REG_TRU0_SSR13)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR14                  ((volatile uint32_t *)REG_TRU0_SSR14)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR15                  ((volatile uint32_t *)REG_TRU0_SSR15)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR16                  ((volatile uint32_t *)REG_TRU0_SSR16)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR17                  ((volatile uint32_t *)REG_TRU0_SSR17)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR18                  ((volatile uint32_t *)REG_TRU0_SSR18)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR19                  ((volatile uint32_t *)REG_TRU0_SSR19)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR20                  ((volatile uint32_t *)REG_TRU0_SSR20)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR21                  ((volatile uint32_t *)REG_TRU0_SSR21)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR22                  ((volatile uint32_t *)REG_TRU0_SSR22)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR23                  ((volatile uint32_t *)REG_TRU0_SSR23)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR24                  ((volatile uint32_t *)REG_TRU0_SSR24)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR25                  ((volatile uint32_t *)REG_TRU0_SSR25)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR26                  ((volatile uint32_t *)REG_TRU0_SSR26)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR27                  ((volatile uint32_t *)REG_TRU0_SSR27)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR28                  ((volatile uint32_t *)REG_TRU0_SSR28)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR29                  ((volatile uint32_t *)REG_TRU0_SSR29)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR30                  ((volatile uint32_t *)REG_TRU0_SSR30)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR31                  ((volatile uint32_t *)REG_TRU0_SSR31)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR32                  ((volatile uint32_t *)REG_TRU0_SSR32)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR33                  ((volatile uint32_t *)REG_TRU0_SSR33)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR34                  ((volatile uint32_t *)REG_TRU0_SSR34)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR35                  ((volatile uint32_t *)REG_TRU0_SSR35)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR36                  ((volatile uint32_t *)REG_TRU0_SSR36)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR37                  ((volatile uint32_t *)REG_TRU0_SSR37)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR38                  ((volatile uint32_t *)REG_TRU0_SSR38)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR39                  ((volatile uint32_t *)REG_TRU0_SSR39)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR40                  ((volatile uint32_t *)REG_TRU0_SSR40)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR41                  ((volatile uint32_t *)REG_TRU0_SSR41)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR42                  ((volatile uint32_t *)REG_TRU0_SSR42)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR43                  ((volatile uint32_t *)REG_TRU0_SSR43)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR44                  ((volatile uint32_t *)REG_TRU0_SSR44)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR45                  ((volatile uint32_t *)REG_TRU0_SSR45)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR46                  ((volatile uint32_t *)REG_TRU0_SSR46)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR47                  ((volatile uint32_t *)REG_TRU0_SSR47)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR48                  ((volatile uint32_t *)REG_TRU0_SSR48)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR49                  ((volatile uint32_t *)REG_TRU0_SSR49)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR50                  ((volatile uint32_t *)REG_TRU0_SSR50)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR51                  ((volatile uint32_t *)REG_TRU0_SSR51)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR52                  ((volatile uint32_t *)REG_TRU0_SSR52)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR53                  ((volatile uint32_t *)REG_TRU0_SSR53)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR54                  ((volatile uint32_t *)REG_TRU0_SSR54)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR55                  ((volatile uint32_t *)REG_TRU0_SSR55)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR56                  ((volatile uint32_t *)REG_TRU0_SSR56)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR57                  ((volatile uint32_t *)REG_TRU0_SSR57)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR58                  ((volatile uint32_t *)REG_TRU0_SSR58)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR59                  ((volatile uint32_t *)REG_TRU0_SSR59)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR60                  ((volatile uint32_t *)REG_TRU0_SSR60)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR61                  ((volatile uint32_t *)REG_TRU0_SSR61)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR62                  ((volatile uint32_t *)REG_TRU0_SSR62)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR63                  ((volatile uint32_t *)REG_TRU0_SSR63)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR64                  ((volatile uint32_t *)REG_TRU0_SSR64)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR65                  ((volatile uint32_t *)REG_TRU0_SSR65)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR66                  ((volatile uint32_t *)REG_TRU0_SSR66)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR67                  ((volatile uint32_t *)REG_TRU0_SSR67)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR68                  ((volatile uint32_t *)REG_TRU0_SSR68)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR69                  ((volatile uint32_t *)REG_TRU0_SSR69)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR70                  ((volatile uint32_t *)REG_TRU0_SSR70)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR71                  ((volatile uint32_t *)REG_TRU0_SSR71)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR72                  ((volatile uint32_t *)REG_TRU0_SSR72)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR73                  ((volatile uint32_t *)REG_TRU0_SSR73)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR74                  ((volatile uint32_t *)REG_TRU0_SSR74)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR75                  ((volatile uint32_t *)REG_TRU0_SSR75)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR76                  ((volatile uint32_t *)REG_TRU0_SSR76)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR77                  ((volatile uint32_t *)REG_TRU0_SSR77)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR78                  ((volatile uint32_t *)REG_TRU0_SSR78)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR79                  ((volatile uint32_t *)REG_TRU0_SSR79)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR80                  ((volatile uint32_t *)REG_TRU0_SSR80)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR81                  ((volatile uint32_t *)REG_TRU0_SSR81)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR82                  ((volatile uint32_t *)REG_TRU0_SSR82)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR83                  ((volatile uint32_t *)REG_TRU0_SSR83)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR84                  ((volatile uint32_t *)REG_TRU0_SSR84)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR85                  ((volatile uint32_t *)REG_TRU0_SSR85)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_SSR86                  ((volatile uint32_t *)REG_TRU0_SSR86)                   /* TRU0 Slave Select Register */
#define pREG_TRU0_MTR                    ((volatile uint32_t *)REG_TRU0_MTR)                     /* TRU0 Master Trigger Register */
#define pREG_TRU0_ERRADDR                ((volatile uint32_t *)REG_TRU0_ERRADDR)                 /* TRU0 Error Address Register */
#define pREG_TRU0_STAT                   ((volatile uint32_t *)REG_TRU0_STAT)                    /* TRU0 Status Information Register */
#define pREG_TRU0_REVID                  ((volatile uint32_t *)REG_TRU0_REVID)                   /* TRU0 Revision ID Register */
#define pREG_TRU0_GCTL                   ((volatile uint32_t *)REG_TRU0_GCTL)                    /* TRU0 Global Control Register */


/* =========================================================================
       RCU0
   ========================================================================= */
#define pREG_RCU0_CTL                    ((volatile uint32_t *)REG_RCU0_CTL)                     /* RCU0 Control Register */
#define pREG_RCU0_STAT                   ((volatile uint32_t *)REG_RCU0_STAT)                    /* RCU0 Status Register */
#define pREG_RCU0_CRCTL                  ((volatile uint32_t *)REG_RCU0_CRCTL)                   /* RCU0 Core Reset Control Register */
#define pREG_RCU0_CRSTAT                 ((volatile uint32_t *)REG_RCU0_CRSTAT)                  /* RCU0 Core Reset Status Register */
#define pREG_RCU0_SIDIS                  ((volatile uint32_t *)REG_RCU0_SIDIS)                   /* RCU0 System Interface Disable Register */
#define pREG_RCU0_SISTAT                 ((volatile uint32_t *)REG_RCU0_SISTAT)                  /* RCU0 System Interface Status Register */
#define pREG_RCU0_SVECT_LCK              ((volatile uint32_t *)REG_RCU0_SVECT_LCK)               /* RCU0 SVECT Lock Register */
#define pREG_RCU0_BCODE                  ((volatile uint32_t *)REG_RCU0_BCODE)                   /* RCU0 Boot Code Register */
#define pREG_RCU0_SVECT0                 ((void * volatile *)REG_RCU0_SVECT0)                    /* RCU0 Software Vector Register n */
#define pREG_RCU0_SVECT1                 ((void * volatile *)REG_RCU0_SVECT1)                    /* RCU0 Software Vector Register n */


/* =========================================================================
       SPU0
   ========================================================================= */
#define pREG_SPU0_CTL                    ((volatile uint32_t *)REG_SPU0_CTL)                     /* SPU0 Control Register */
#define pREG_SPU0_STAT                   ((volatile uint32_t *)REG_SPU0_STAT)                    /* SPU0 Status Register */
#define pREG_SPU0_WP0                    ((volatile uint32_t *)REG_SPU0_WP0)                     /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP1                    ((volatile uint32_t *)REG_SPU0_WP1)                     /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP2                    ((volatile uint32_t *)REG_SPU0_WP2)                     /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP3                    ((volatile uint32_t *)REG_SPU0_WP3)                     /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP4                    ((volatile uint32_t *)REG_SPU0_WP4)                     /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP5                    ((volatile uint32_t *)REG_SPU0_WP5)                     /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP6                    ((volatile uint32_t *)REG_SPU0_WP6)                     /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP7                    ((volatile uint32_t *)REG_SPU0_WP7)                     /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP8                    ((volatile uint32_t *)REG_SPU0_WP8)                     /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP9                    ((volatile uint32_t *)REG_SPU0_WP9)                     /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP10                   ((volatile uint32_t *)REG_SPU0_WP10)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP11                   ((volatile uint32_t *)REG_SPU0_WP11)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP12                   ((volatile uint32_t *)REG_SPU0_WP12)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP13                   ((volatile uint32_t *)REG_SPU0_WP13)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP14                   ((volatile uint32_t *)REG_SPU0_WP14)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP15                   ((volatile uint32_t *)REG_SPU0_WP15)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP16                   ((volatile uint32_t *)REG_SPU0_WP16)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP17                   ((volatile uint32_t *)REG_SPU0_WP17)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP18                   ((volatile uint32_t *)REG_SPU0_WP18)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP19                   ((volatile uint32_t *)REG_SPU0_WP19)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP20                   ((volatile uint32_t *)REG_SPU0_WP20)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP21                   ((volatile uint32_t *)REG_SPU0_WP21)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP22                   ((volatile uint32_t *)REG_SPU0_WP22)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP23                   ((volatile uint32_t *)REG_SPU0_WP23)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP24                   ((volatile uint32_t *)REG_SPU0_WP24)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP25                   ((volatile uint32_t *)REG_SPU0_WP25)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP26                   ((volatile uint32_t *)REG_SPU0_WP26)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP27                   ((volatile uint32_t *)REG_SPU0_WP27)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP28                   ((volatile uint32_t *)REG_SPU0_WP28)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP29                   ((volatile uint32_t *)REG_SPU0_WP29)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP30                   ((volatile uint32_t *)REG_SPU0_WP30)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP31                   ((volatile uint32_t *)REG_SPU0_WP31)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP32                   ((volatile uint32_t *)REG_SPU0_WP32)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP33                   ((volatile uint32_t *)REG_SPU0_WP33)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP34                   ((volatile uint32_t *)REG_SPU0_WP34)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP35                   ((volatile uint32_t *)REG_SPU0_WP35)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP36                   ((volatile uint32_t *)REG_SPU0_WP36)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP37                   ((volatile uint32_t *)REG_SPU0_WP37)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP38                   ((volatile uint32_t *)REG_SPU0_WP38)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP39                   ((volatile uint32_t *)REG_SPU0_WP39)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP40                   ((volatile uint32_t *)REG_SPU0_WP40)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP41                   ((volatile uint32_t *)REG_SPU0_WP41)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP42                   ((volatile uint32_t *)REG_SPU0_WP42)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP43                   ((volatile uint32_t *)REG_SPU0_WP43)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP44                   ((volatile uint32_t *)REG_SPU0_WP44)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP45                   ((volatile uint32_t *)REG_SPU0_WP45)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP46                   ((volatile uint32_t *)REG_SPU0_WP46)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP47                   ((volatile uint32_t *)REG_SPU0_WP47)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP48                   ((volatile uint32_t *)REG_SPU0_WP48)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP49                   ((volatile uint32_t *)REG_SPU0_WP49)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP50                   ((volatile uint32_t *)REG_SPU0_WP50)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP51                   ((volatile uint32_t *)REG_SPU0_WP51)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP52                   ((volatile uint32_t *)REG_SPU0_WP52)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP53                   ((volatile uint32_t *)REG_SPU0_WP53)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP54                   ((volatile uint32_t *)REG_SPU0_WP54)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP55                   ((volatile uint32_t *)REG_SPU0_WP55)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP56                   ((volatile uint32_t *)REG_SPU0_WP56)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP57                   ((volatile uint32_t *)REG_SPU0_WP57)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP58                   ((volatile uint32_t *)REG_SPU0_WP58)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP59                   ((volatile uint32_t *)REG_SPU0_WP59)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP60                   ((volatile uint32_t *)REG_SPU0_WP60)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP61                   ((volatile uint32_t *)REG_SPU0_WP61)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP62                   ((volatile uint32_t *)REG_SPU0_WP62)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP63                   ((volatile uint32_t *)REG_SPU0_WP63)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP64                   ((volatile uint32_t *)REG_SPU0_WP64)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP65                   ((volatile uint32_t *)REG_SPU0_WP65)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP66                   ((volatile uint32_t *)REG_SPU0_WP66)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP67                   ((volatile uint32_t *)REG_SPU0_WP67)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP68                   ((volatile uint32_t *)REG_SPU0_WP68)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP69                   ((volatile uint32_t *)REG_SPU0_WP69)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP70                   ((volatile uint32_t *)REG_SPU0_WP70)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP71                   ((volatile uint32_t *)REG_SPU0_WP71)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP72                   ((volatile uint32_t *)REG_SPU0_WP72)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP73                   ((volatile uint32_t *)REG_SPU0_WP73)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP74                   ((volatile uint32_t *)REG_SPU0_WP74)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP75                   ((volatile uint32_t *)REG_SPU0_WP75)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP76                   ((volatile uint32_t *)REG_SPU0_WP76)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP77                   ((volatile uint32_t *)REG_SPU0_WP77)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP78                   ((volatile uint32_t *)REG_SPU0_WP78)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP79                   ((volatile uint32_t *)REG_SPU0_WP79)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP80                   ((volatile uint32_t *)REG_SPU0_WP80)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP81                   ((volatile uint32_t *)REG_SPU0_WP81)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP82                   ((volatile uint32_t *)REG_SPU0_WP82)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP83                   ((volatile uint32_t *)REG_SPU0_WP83)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP84                   ((volatile uint32_t *)REG_SPU0_WP84)                    /* SPU0 Write Protect Register n */
#define pREG_SPU0_WP85                   ((volatile uint32_t *)REG_SPU0_WP85)                    /* SPU0 Write Protect Register n */


/* =========================================================================
       CGU0
   ========================================================================= */
#define pREG_CGU0_CTL                    ((volatile uint32_t *)REG_CGU0_CTL)                     /* CGU0 Control Register */
#define pREG_CGU0_STAT                   ((volatile uint32_t *)REG_CGU0_STAT)                    /* CGU0 Status Register */
#define pREG_CGU0_DIV                    ((volatile uint32_t *)REG_CGU0_DIV)                     /* CGU0 Divisor Register */
#define pREG_CGU0_CLKOUTSEL              ((volatile uint32_t *)REG_CGU0_CLKOUTSEL)               /* CGU0 CLKOUT Select Register */


/* =========================================================================
       DPM0
   ========================================================================= */
#define pREG_DPM0_CTL                    ((volatile uint32_t *)REG_DPM0_CTL)                     /* DPM0 Control Register */
#define pREG_DPM0_STAT                   ((volatile uint32_t *)REG_DPM0_STAT)                    /* DPM0 Status Register */
#define pREG_DPM0_CCBF_DIS               ((volatile uint32_t *)REG_DPM0_CCBF_DIS)                /* DPM0 Core Clock Buffer Disable Register */
#define pREG_DPM0_CCBF_EN                ((volatile uint32_t *)REG_DPM0_CCBF_EN)                 /* DPM0 Core Clock Buffer Enable Register */
#define pREG_DPM0_CCBF_STAT              ((volatile uint32_t *)REG_DPM0_CCBF_STAT)               /* DPM0 Core Clock Buffer Status Register */
#define pREG_DPM0_CCBF_STAT_STKY         ((volatile uint32_t *)REG_DPM0_CCBF_STAT_STKY)          /* DPM0 Core Clock Buffer Status Sticky Register */
#define pREG_DPM0_SCBF_DIS               ((volatile uint32_t *)REG_DPM0_SCBF_DIS)                /* DPM0 System Clock Buffer Disable Register */
#define pREG_DPM0_WAKE_EN                ((volatile uint32_t *)REG_DPM0_WAKE_EN)                 /* DPM0 Wakeup Enable Register */
#define pREG_DPM0_WAKE_POL               ((volatile uint32_t *)REG_DPM0_WAKE_POL)                /* DPM0 Wakeup Polarity Register */
#define pREG_DPM0_WAKE_STAT              ((volatile uint32_t *)REG_DPM0_WAKE_STAT)               /* DPM0 Wakeup Status Register */
#define pREG_DPM0_HIB_DIS                ((volatile uint32_t *)REG_DPM0_HIB_DIS)                 /* DPM0 Hibernate Disable Register */
#define pREG_DPM0_PGCNTR                 ((volatile uint32_t *)REG_DPM0_PGCNTR)                  /* DPM0 Power Good Counter Register */
#define pREG_DPM0_RESTORE0               ((volatile uint32_t *)REG_DPM0_RESTORE0)                /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE1               ((volatile uint32_t *)REG_DPM0_RESTORE1)                /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE2               ((volatile uint32_t *)REG_DPM0_RESTORE2)                /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE3               ((volatile uint32_t *)REG_DPM0_RESTORE3)                /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE4               ((volatile uint32_t *)REG_DPM0_RESTORE4)                /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE5               ((volatile uint32_t *)REG_DPM0_RESTORE5)                /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE6               ((volatile uint32_t *)REG_DPM0_RESTORE6)                /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE7               ((volatile uint32_t *)REG_DPM0_RESTORE7)                /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE8               ((volatile uint32_t *)REG_DPM0_RESTORE8)                /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE9               ((volatile uint32_t *)REG_DPM0_RESTORE9)                /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE10              ((volatile uint32_t *)REG_DPM0_RESTORE10)               /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE11              ((volatile uint32_t *)REG_DPM0_RESTORE11)               /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE12              ((volatile uint32_t *)REG_DPM0_RESTORE12)               /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE13              ((volatile uint32_t *)REG_DPM0_RESTORE13)               /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE14              ((volatile uint32_t *)REG_DPM0_RESTORE14)               /* DPM0 Restore n Register */
#define pREG_DPM0_RESTORE15              ((volatile uint32_t *)REG_DPM0_RESTORE15)               /* DPM0 Restore n Register */


/* =========================================================================
       EFS0
   ========================================================================= */
#define pREG_EFS0_CTL                    ((volatile uint32_t *)REG_EFS0_CTL)                     /* EFS0 Control Register */
#define pREG_EFS0_DAT0                   ((volatile uint32_t *)REG_EFS0_DAT0)                    /* EFS0 Data Register 0 */
#define pREG_EFS0_DAT1                   ((volatile uint32_t *)REG_EFS0_DAT1)                    /* EFS0 Data Register 1 */
#define pREG_EFS0_DAT2                   ((volatile uint32_t *)REG_EFS0_DAT2)                    /* EFS0 Data Register 2 */
#define pREG_EFS0_DAT3                   ((volatile uint32_t *)REG_EFS0_DAT3)                    /* EFS0 Data Register 3 */
#define pREG_EFS0_DAT4                   ((volatile uint32_t *)REG_EFS0_DAT4)                    /* EFS0 Data Register 4 */
#define pREG_EFS0_DAT5                   ((volatile uint32_t *)REG_EFS0_DAT5)                    /* EFS0 Data Register 5 */
#define pREG_EFS0_DAT6                   ((volatile uint32_t *)REG_EFS0_DAT6)                    /* EFS0 Data Register 6 */
#define pREG_EFS0_DAT7                   ((volatile uint32_t *)REG_EFS0_DAT7)                    /* EFS0 Data Register 7 */


/* =========================================================================
       USB0
   ========================================================================= */
#define pREG_USB0_FADDR                  ((volatile  uint8_t *)REG_USB0_FADDR)                   /* USB0 Function Address Register */
#define pREG_USB0_POWER                  ((volatile  uint8_t *)REG_USB0_POWER)                   /* USB0 Power and Device Control Register */
#define pREG_USB0_INTRTX                 ((volatile uint16_t *)REG_USB0_INTRTX)                  /* USB0 Transmit Interrupt Register */
#define pREG_USB0_INTRRX                 ((volatile uint16_t *)REG_USB0_INTRRX)                  /* USB0 Receive Interrupt Register */
#define pREG_USB0_INTRTXE                ((volatile uint16_t *)REG_USB0_INTRTXE)                 /* USB0 Transmit Interrupt Enable Register */
#define pREG_USB0_INTRRXE                ((volatile uint16_t *)REG_USB0_INTRRXE)                 /* USB0 Receive Interrupt Enable Register */
#define pREG_USB0_IRQ                    ((volatile  uint8_t *)REG_USB0_IRQ)                     /* USB0 Common Interrupts Register */
#define pREG_USB0_IEN                    ((volatile  uint8_t *)REG_USB0_IEN)                     /* USB0 Common Interrupts Enable Register */
#define pREG_USB0_FRAME                  ((volatile uint16_t *)REG_USB0_FRAME)                   /* USB0 Frame Number Register */
#define pREG_USB0_INDEX                  ((volatile  uint8_t *)REG_USB0_INDEX)                   /* USB0 Index Register */
#define pREG_USB0_TESTMODE               ((volatile  uint8_t *)REG_USB0_TESTMODE)                /* USB0 Testmode Register */
#define pREG_USB0_EPI_TXMAXP0            ((volatile uint16_t *)REG_USB0_EPI_TXMAXP0)             /* USB0 EPn Transmit Maximum Packet Length Register */
#define pREG_USB0_EPI_TXCSR_P0           ((volatile uint16_t *)REG_USB0_EPI_TXCSR_P0)            /* USB0 EPn Transmit Configuration and Status (Peripheral) Register */
#define pREG_USB0_EPI_TXCSR_H0           ((volatile uint16_t *)REG_USB0_EPI_TXCSR_H0)            /* USB0 EPn Transmit Configuration and Status (Host) Register */
#define pREG_USB0_EP0I_CSR0_P            ((volatile uint16_t *)REG_USB0_EP0I_CSR0_P)             /* USB0 EP0 Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP0I_CSR0_H            ((volatile uint16_t *)REG_USB0_EP0I_CSR0_H)             /* USB0 EP0 Configuration and Status (Host) Register */
#define pREG_USB0_EPI_RXMAXP0            ((volatile uint16_t *)REG_USB0_EPI_RXMAXP0)             /* USB0 EPn Receive Maximum Packet Length Register */
#define pREG_USB0_EPI_RXCSR_H0           ((volatile uint16_t *)REG_USB0_EPI_RXCSR_H0)            /* USB0 EPn Receive Configuration and Status (Host) Register */
#define pREG_USB0_EPI_RXCSR_P0           ((volatile uint16_t *)REG_USB0_EPI_RXCSR_P0)            /* USB0 EPn Receive Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP0I_CNT0              ((volatile uint16_t *)REG_USB0_EP0I_CNT0)               /* USB0 EP0 Number of Received Bytes Register */
#define pREG_USB0_EPI_RXCNT0             ((volatile uint16_t *)REG_USB0_EPI_RXCNT0)              /* USB0 EPn Number of Bytes Received Register */
#define pREG_USB0_EPI_TXTYPE0            ((volatile  uint8_t *)REG_USB0_EPI_TXTYPE0)             /* USB0 EPn Transmit Type Register */
#define pREG_USB0_EP0I_TYPE0             ((volatile  uint8_t *)REG_USB0_EP0I_TYPE0)              /* USB0 EP0 Connection Type Register */
#define pREG_USB0_EPI_TXINTERVAL0        ((volatile  uint8_t *)REG_USB0_EPI_TXINTERVAL0)         /* USB0 EPn Transmit Polling Interval Register */
#define pREG_USB0_EP0I_NAKLIMIT0         ((volatile  uint8_t *)REG_USB0_EP0I_NAKLIMIT0)          /* USB0 EP0 NAK Limit Register */
#define pREG_USB0_EPI_RXTYPE0            ((volatile  uint8_t *)REG_USB0_EPI_RXTYPE0)             /* USB0 EPn Receive Type Register */
#define pREG_USB0_EPI_RXINTERVAL0        ((volatile  uint8_t *)REG_USB0_EPI_RXINTERVAL0)         /* USB0 EPn Receive Polling Interval Register */
#define pREG_USB0_EP0I_CFGDATA0          ((volatile  uint8_t *)REG_USB0_EP0I_CFGDATA0)           /* USB0 EP0 Configuration Information Register */
#define pREG_USB0_FIFOB0                 ((volatile  uint8_t *)REG_USB0_FIFOB0)                  /* USB0 FIFO Byte (8-Bit) Register */
#define pREG_USB0_FIFOB1                 ((volatile  uint8_t *)REG_USB0_FIFOB1)                  /* USB0 FIFO Byte (8-Bit) Register */
#define pREG_USB0_FIFOB2                 ((volatile  uint8_t *)REG_USB0_FIFOB2)                  /* USB0 FIFO Byte (8-Bit) Register */
#define pREG_USB0_FIFOB3                 ((volatile  uint8_t *)REG_USB0_FIFOB3)                  /* USB0 FIFO Byte (8-Bit) Register */
#define pREG_USB0_FIFOB4                 ((volatile  uint8_t *)REG_USB0_FIFOB4)                  /* USB0 FIFO Byte (8-Bit) Register */
#define pREG_USB0_FIFOB5                 ((volatile  uint8_t *)REG_USB0_FIFOB5)                  /* USB0 FIFO Byte (8-Bit) Register */
#define pREG_USB0_FIFOB6                 ((volatile  uint8_t *)REG_USB0_FIFOB6)                  /* USB0 FIFO Byte (8-Bit) Register */
#define pREG_USB0_FIFOB7                 ((volatile  uint8_t *)REG_USB0_FIFOB7)                  /* USB0 FIFO Byte (8-Bit) Register */
#define pREG_USB0_FIFOB8                 ((volatile  uint8_t *)REG_USB0_FIFOB8)                  /* USB0 FIFO Byte (8-Bit) Register */
#define pREG_USB0_FIFOB9                 ((volatile  uint8_t *)REG_USB0_FIFOB9)                  /* USB0 FIFO Byte (8-Bit) Register */
#define pREG_USB0_FIFOB10                ((volatile  uint8_t *)REG_USB0_FIFOB10)                 /* USB0 FIFO Byte (8-Bit) Register */
#define pREG_USB0_FIFOB11                ((volatile  uint8_t *)REG_USB0_FIFOB11)                 /* USB0 FIFO Byte (8-Bit) Register */
#define pREG_USB0_FIFOH0                 ((volatile uint16_t *)REG_USB0_FIFOH0)                  /* USB0 FIFO Half-Word (16-Bit) Register */
#define pREG_USB0_FIFOH1                 ((volatile uint16_t *)REG_USB0_FIFOH1)                  /* USB0 FIFO Half-Word (16-Bit) Register */
#define pREG_USB0_FIFOH2                 ((volatile uint16_t *)REG_USB0_FIFOH2)                  /* USB0 FIFO Half-Word (16-Bit) Register */
#define pREG_USB0_FIFOH3                 ((volatile uint16_t *)REG_USB0_FIFOH3)                  /* USB0 FIFO Half-Word (16-Bit) Register */
#define pREG_USB0_FIFOH4                 ((volatile uint16_t *)REG_USB0_FIFOH4)                  /* USB0 FIFO Half-Word (16-Bit) Register */
#define pREG_USB0_FIFOH5                 ((volatile uint16_t *)REG_USB0_FIFOH5)                  /* USB0 FIFO Half-Word (16-Bit) Register */
#define pREG_USB0_FIFOH6                 ((volatile uint16_t *)REG_USB0_FIFOH6)                  /* USB0 FIFO Half-Word (16-Bit) Register */
#define pREG_USB0_FIFOH7                 ((volatile uint16_t *)REG_USB0_FIFOH7)                  /* USB0 FIFO Half-Word (16-Bit) Register */
#define pREG_USB0_FIFOH8                 ((volatile uint16_t *)REG_USB0_FIFOH8)                  /* USB0 FIFO Half-Word (16-Bit) Register */
#define pREG_USB0_FIFOH9                 ((volatile uint16_t *)REG_USB0_FIFOH9)                  /* USB0 FIFO Half-Word (16-Bit) Register */
#define pREG_USB0_FIFOH10                ((volatile uint16_t *)REG_USB0_FIFOH10)                 /* USB0 FIFO Half-Word (16-Bit) Register */
#define pREG_USB0_FIFOH11                ((volatile uint16_t *)REG_USB0_FIFOH11)                 /* USB0 FIFO Half-Word (16-Bit) Register */
#define pREG_USB0_FIFO0                  ((volatile uint32_t *)REG_USB0_FIFO0)                   /* USB0 FIFO Word (32-Bit) Register */
#define pREG_USB0_FIFO1                  ((volatile uint32_t *)REG_USB0_FIFO1)                   /* USB0 FIFO Word (32-Bit) Register */
#define pREG_USB0_FIFO2                  ((volatile uint32_t *)REG_USB0_FIFO2)                   /* USB0 FIFO Word (32-Bit) Register */
#define pREG_USB0_FIFO3                  ((volatile uint32_t *)REG_USB0_FIFO3)                   /* USB0 FIFO Word (32-Bit) Register */
#define pREG_USB0_FIFO4                  ((volatile uint32_t *)REG_USB0_FIFO4)                   /* USB0 FIFO Word (32-Bit) Register */
#define pREG_USB0_FIFO5                  ((volatile uint32_t *)REG_USB0_FIFO5)                   /* USB0 FIFO Word (32-Bit) Register */
#define pREG_USB0_FIFO6                  ((volatile uint32_t *)REG_USB0_FIFO6)                   /* USB0 FIFO Word (32-Bit) Register */
#define pREG_USB0_FIFO7                  ((volatile uint32_t *)REG_USB0_FIFO7)                   /* USB0 FIFO Word (32-Bit) Register */
#define pREG_USB0_FIFO8                  ((volatile uint32_t *)REG_USB0_FIFO8)                   /* USB0 FIFO Word (32-Bit) Register */
#define pREG_USB0_FIFO9                  ((volatile uint32_t *)REG_USB0_FIFO9)                   /* USB0 FIFO Word (32-Bit) Register */
#define pREG_USB0_FIFO10                 ((volatile uint32_t *)REG_USB0_FIFO10)                  /* USB0 FIFO Word (32-Bit) Register */
#define pREG_USB0_FIFO11                 ((volatile uint32_t *)REG_USB0_FIFO11)                  /* USB0 FIFO Word (32-Bit) Register */
#define pREG_USB0_DEV_CTL                ((volatile  uint8_t *)REG_USB0_DEV_CTL)                 /* USB0 Device Control Register */
#define pREG_USB0_TXFIFOSZ               ((volatile  uint8_t *)REG_USB0_TXFIFOSZ)                /* USB0 Transmit FIFO Size Register */
#define pREG_USB0_RXFIFOSZ               ((volatile  uint8_t *)REG_USB0_RXFIFOSZ)                /* USB0 Receive FIFO Size Register */
#define pREG_USB0_TXFIFOADDR             ((volatile uint16_t *)REG_USB0_TXFIFOADDR)              /* USB0 Transmit FIFO Address Register */
#define pREG_USB0_RXFIFOADDR             ((volatile uint16_t *)REG_USB0_RXFIFOADDR)              /* USB0 Receive FIFO Address Register */
#define pREG_USB0_EPINFO                 ((volatile  uint8_t *)REG_USB0_EPINFO)                  /* USB0 Endpoint Information Register */
#define pREG_USB0_RAMINFO                ((volatile  uint8_t *)REG_USB0_RAMINFO)                 /* USB0 RAM Information Register */
#define pREG_USB0_LINKINFO               ((volatile  uint8_t *)REG_USB0_LINKINFO)                /* USB0 Link Information Register */
#define pREG_USB0_VPLEN                  ((volatile  uint8_t *)REG_USB0_VPLEN)                   /* USB0 VBUS Pulse Length Register */
#define pREG_USB0_HS_EOF1                ((volatile  uint8_t *)REG_USB0_HS_EOF1)                 /* USB0 High-Speed EOF 1 Register */
#define pREG_USB0_FS_EOF1                ((volatile  uint8_t *)REG_USB0_FS_EOF1)                 /* USB0 Full-Speed EOF 1 Register */
#define pREG_USB0_LS_EOF1                ((volatile  uint8_t *)REG_USB0_LS_EOF1)                 /* USB0 Low-Speed EOF 1 Register */
#define pREG_USB0_SOFT_RST               ((volatile  uint8_t *)REG_USB0_SOFT_RST)                /* USB0 Software Reset Register */
#define pREG_USB0_MP0_TXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP0_TXFUNCADDR)          /* USB0 MPn Transmit Function Address Register */
#define pREG_USB0_MP1_TXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP1_TXFUNCADDR)          /* USB0 MPn Transmit Function Address Register */
#define pREG_USB0_MP2_TXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP2_TXFUNCADDR)          /* USB0 MPn Transmit Function Address Register */
#define pREG_USB0_MP3_TXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP3_TXFUNCADDR)          /* USB0 MPn Transmit Function Address Register */
#define pREG_USB0_MP4_TXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP4_TXFUNCADDR)          /* USB0 MPn Transmit Function Address Register */
#define pREG_USB0_MP5_TXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP5_TXFUNCADDR)          /* USB0 MPn Transmit Function Address Register */
#define pREG_USB0_MP6_TXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP6_TXFUNCADDR)          /* USB0 MPn Transmit Function Address Register */
#define pREG_USB0_MP7_TXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP7_TXFUNCADDR)          /* USB0 MPn Transmit Function Address Register */
#define pREG_USB0_MP8_TXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP8_TXFUNCADDR)          /* USB0 MPn Transmit Function Address Register */
#define pREG_USB0_MP9_TXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP9_TXFUNCADDR)          /* USB0 MPn Transmit Function Address Register */
#define pREG_USB0_MP10_TXFUNCADDR        ((volatile  uint8_t *)REG_USB0_MP10_TXFUNCADDR)         /* USB0 MPn Transmit Function Address Register */
#define pREG_USB0_MP11_TXFUNCADDR        ((volatile  uint8_t *)REG_USB0_MP11_TXFUNCADDR)         /* USB0 MPn Transmit Function Address Register */
#define pREG_USB0_MP0_TXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP0_TXHUBADDR)           /* USB0 MPn Transmit Hub Address Register */
#define pREG_USB0_MP1_TXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP1_TXHUBADDR)           /* USB0 MPn Transmit Hub Address Register */
#define pREG_USB0_MP2_TXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP2_TXHUBADDR)           /* USB0 MPn Transmit Hub Address Register */
#define pREG_USB0_MP3_TXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP3_TXHUBADDR)           /* USB0 MPn Transmit Hub Address Register */
#define pREG_USB0_MP4_TXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP4_TXHUBADDR)           /* USB0 MPn Transmit Hub Address Register */
#define pREG_USB0_MP5_TXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP5_TXHUBADDR)           /* USB0 MPn Transmit Hub Address Register */
#define pREG_USB0_MP6_TXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP6_TXHUBADDR)           /* USB0 MPn Transmit Hub Address Register */
#define pREG_USB0_MP7_TXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP7_TXHUBADDR)           /* USB0 MPn Transmit Hub Address Register */
#define pREG_USB0_MP8_TXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP8_TXHUBADDR)           /* USB0 MPn Transmit Hub Address Register */
#define pREG_USB0_MP9_TXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP9_TXHUBADDR)           /* USB0 MPn Transmit Hub Address Register */
#define pREG_USB0_MP10_TXHUBADDR         ((volatile  uint8_t *)REG_USB0_MP10_TXHUBADDR)          /* USB0 MPn Transmit Hub Address Register */
#define pREG_USB0_MP11_TXHUBADDR         ((volatile  uint8_t *)REG_USB0_MP11_TXHUBADDR)          /* USB0 MPn Transmit Hub Address Register */
#define pREG_USB0_MP0_TXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP0_TXHUBPORT)           /* USB0 MPn Transmit Hub Port Register */
#define pREG_USB0_MP1_TXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP1_TXHUBPORT)           /* USB0 MPn Transmit Hub Port Register */
#define pREG_USB0_MP2_TXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP2_TXHUBPORT)           /* USB0 MPn Transmit Hub Port Register */
#define pREG_USB0_MP3_TXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP3_TXHUBPORT)           /* USB0 MPn Transmit Hub Port Register */
#define pREG_USB0_MP4_TXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP4_TXHUBPORT)           /* USB0 MPn Transmit Hub Port Register */
#define pREG_USB0_MP5_TXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP5_TXHUBPORT)           /* USB0 MPn Transmit Hub Port Register */
#define pREG_USB0_MP6_TXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP6_TXHUBPORT)           /* USB0 MPn Transmit Hub Port Register */
#define pREG_USB0_MP7_TXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP7_TXHUBPORT)           /* USB0 MPn Transmit Hub Port Register */
#define pREG_USB0_MP8_TXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP8_TXHUBPORT)           /* USB0 MPn Transmit Hub Port Register */
#define pREG_USB0_MP9_TXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP9_TXHUBPORT)           /* USB0 MPn Transmit Hub Port Register */
#define pREG_USB0_MP10_TXHUBPORT         ((volatile  uint8_t *)REG_USB0_MP10_TXHUBPORT)          /* USB0 MPn Transmit Hub Port Register */
#define pREG_USB0_MP11_TXHUBPORT         ((volatile  uint8_t *)REG_USB0_MP11_TXHUBPORT)          /* USB0 MPn Transmit Hub Port Register */
#define pREG_USB0_MP0_RXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP0_RXFUNCADDR)          /* USB0 MPn Receive Function Address Register */
#define pREG_USB0_MP1_RXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP1_RXFUNCADDR)          /* USB0 MPn Receive Function Address Register */
#define pREG_USB0_MP2_RXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP2_RXFUNCADDR)          /* USB0 MPn Receive Function Address Register */
#define pREG_USB0_MP3_RXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP3_RXFUNCADDR)          /* USB0 MPn Receive Function Address Register */
#define pREG_USB0_MP4_RXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP4_RXFUNCADDR)          /* USB0 MPn Receive Function Address Register */
#define pREG_USB0_MP5_RXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP5_RXFUNCADDR)          /* USB0 MPn Receive Function Address Register */
#define pREG_USB0_MP6_RXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP6_RXFUNCADDR)          /* USB0 MPn Receive Function Address Register */
#define pREG_USB0_MP7_RXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP7_RXFUNCADDR)          /* USB0 MPn Receive Function Address Register */
#define pREG_USB0_MP8_RXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP8_RXFUNCADDR)          /* USB0 MPn Receive Function Address Register */
#define pREG_USB0_MP9_RXFUNCADDR         ((volatile  uint8_t *)REG_USB0_MP9_RXFUNCADDR)          /* USB0 MPn Receive Function Address Register */
#define pREG_USB0_MP10_RXFUNCADDR        ((volatile  uint8_t *)REG_USB0_MP10_RXFUNCADDR)         /* USB0 MPn Receive Function Address Register */
#define pREG_USB0_MP11_RXFUNCADDR        ((volatile  uint8_t *)REG_USB0_MP11_RXFUNCADDR)         /* USB0 MPn Receive Function Address Register */
#define pREG_USB0_MP0_RXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP0_RXHUBADDR)           /* USB0 MPn Receive Hub Address Register */
#define pREG_USB0_MP1_RXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP1_RXHUBADDR)           /* USB0 MPn Receive Hub Address Register */
#define pREG_USB0_MP2_RXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP2_RXHUBADDR)           /* USB0 MPn Receive Hub Address Register */
#define pREG_USB0_MP3_RXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP3_RXHUBADDR)           /* USB0 MPn Receive Hub Address Register */
#define pREG_USB0_MP4_RXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP4_RXHUBADDR)           /* USB0 MPn Receive Hub Address Register */
#define pREG_USB0_MP5_RXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP5_RXHUBADDR)           /* USB0 MPn Receive Hub Address Register */
#define pREG_USB0_MP6_RXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP6_RXHUBADDR)           /* USB0 MPn Receive Hub Address Register */
#define pREG_USB0_MP7_RXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP7_RXHUBADDR)           /* USB0 MPn Receive Hub Address Register */
#define pREG_USB0_MP8_RXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP8_RXHUBADDR)           /* USB0 MPn Receive Hub Address Register */
#define pREG_USB0_MP9_RXHUBADDR          ((volatile  uint8_t *)REG_USB0_MP9_RXHUBADDR)           /* USB0 MPn Receive Hub Address Register */
#define pREG_USB0_MP10_RXHUBADDR         ((volatile  uint8_t *)REG_USB0_MP10_RXHUBADDR)          /* USB0 MPn Receive Hub Address Register */
#define pREG_USB0_MP11_RXHUBADDR         ((volatile  uint8_t *)REG_USB0_MP11_RXHUBADDR)          /* USB0 MPn Receive Hub Address Register */
#define pREG_USB0_MP0_RXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP0_RXHUBPORT)           /* USB0 MPn Receive Hub Port Register */
#define pREG_USB0_MP1_RXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP1_RXHUBPORT)           /* USB0 MPn Receive Hub Port Register */
#define pREG_USB0_MP2_RXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP2_RXHUBPORT)           /* USB0 MPn Receive Hub Port Register */
#define pREG_USB0_MP3_RXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP3_RXHUBPORT)           /* USB0 MPn Receive Hub Port Register */
#define pREG_USB0_MP4_RXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP4_RXHUBPORT)           /* USB0 MPn Receive Hub Port Register */
#define pREG_USB0_MP5_RXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP5_RXHUBPORT)           /* USB0 MPn Receive Hub Port Register */
#define pREG_USB0_MP6_RXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP6_RXHUBPORT)           /* USB0 MPn Receive Hub Port Register */
#define pREG_USB0_MP7_RXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP7_RXHUBPORT)           /* USB0 MPn Receive Hub Port Register */
#define pREG_USB0_MP8_RXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP8_RXHUBPORT)           /* USB0 MPn Receive Hub Port Register */
#define pREG_USB0_MP9_RXHUBPORT          ((volatile  uint8_t *)REG_USB0_MP9_RXHUBPORT)           /* USB0 MPn Receive Hub Port Register */
#define pREG_USB0_MP10_RXHUBPORT         ((volatile  uint8_t *)REG_USB0_MP10_RXHUBPORT)          /* USB0 MPn Receive Hub Port Register */
#define pREG_USB0_MP11_RXHUBPORT         ((volatile  uint8_t *)REG_USB0_MP11_RXHUBPORT)          /* USB0 MPn Receive Hub Port Register */
#define pREG_USB0_EP0_TXMAXP             ((volatile uint16_t *)REG_USB0_EP0_TXMAXP)              /* USB0 EPn Transmit Maximum Packet Length Register */
#define pREG_USB0_EP1_TXMAXP             ((volatile uint16_t *)REG_USB0_EP1_TXMAXP)              /* USB0 EPn Transmit Maximum Packet Length Register */
#define pREG_USB0_EP2_TXMAXP             ((volatile uint16_t *)REG_USB0_EP2_TXMAXP)              /* USB0 EPn Transmit Maximum Packet Length Register */
#define pREG_USB0_EP3_TXMAXP             ((volatile uint16_t *)REG_USB0_EP3_TXMAXP)              /* USB0 EPn Transmit Maximum Packet Length Register */
#define pREG_USB0_EP4_TXMAXP             ((volatile uint16_t *)REG_USB0_EP4_TXMAXP)              /* USB0 EPn Transmit Maximum Packet Length Register */
#define pREG_USB0_EP5_TXMAXP             ((volatile uint16_t *)REG_USB0_EP5_TXMAXP)              /* USB0 EPn Transmit Maximum Packet Length Register */
#define pREG_USB0_EP6_TXMAXP             ((volatile uint16_t *)REG_USB0_EP6_TXMAXP)              /* USB0 EPn Transmit Maximum Packet Length Register */
#define pREG_USB0_EP7_TXMAXP             ((volatile uint16_t *)REG_USB0_EP7_TXMAXP)              /* USB0 EPn Transmit Maximum Packet Length Register */
#define pREG_USB0_EP8_TXMAXP             ((volatile uint16_t *)REG_USB0_EP8_TXMAXP)              /* USB0 EPn Transmit Maximum Packet Length Register */
#define pREG_USB0_EP9_TXMAXP             ((volatile uint16_t *)REG_USB0_EP9_TXMAXP)              /* USB0 EPn Transmit Maximum Packet Length Register */
#define pREG_USB0_EP10_TXMAXP            ((volatile uint16_t *)REG_USB0_EP10_TXMAXP)             /* USB0 EPn Transmit Maximum Packet Length Register */
#define pREG_USB0_EP11_TXMAXP            ((volatile uint16_t *)REG_USB0_EP11_TXMAXP)             /* USB0 EPn Transmit Maximum Packet Length Register */
#define pREG_USB0_EP0_CSR0_H             ((volatile uint16_t *)REG_USB0_EP0_CSR0_H)              /* USB0 EP0 Configuration and Status (Host) Register */
#define pREG_USB0_EP0_TXCSR_H            ((volatile uint16_t *)REG_USB0_EP0_TXCSR_H)             /* USB0 EPn Transmit Configuration and Status (Host) Register */
#define pREG_USB0_EP1_TXCSR_H            ((volatile uint16_t *)REG_USB0_EP1_TXCSR_H)             /* USB0 EPn Transmit Configuration and Status (Host) Register */
#define pREG_USB0_EP2_TXCSR_H            ((volatile uint16_t *)REG_USB0_EP2_TXCSR_H)             /* USB0 EPn Transmit Configuration and Status (Host) Register */
#define pREG_USB0_EP3_TXCSR_H            ((volatile uint16_t *)REG_USB0_EP3_TXCSR_H)             /* USB0 EPn Transmit Configuration and Status (Host) Register */
#define pREG_USB0_EP4_TXCSR_H            ((volatile uint16_t *)REG_USB0_EP4_TXCSR_H)             /* USB0 EPn Transmit Configuration and Status (Host) Register */
#define pREG_USB0_EP5_TXCSR_H            ((volatile uint16_t *)REG_USB0_EP5_TXCSR_H)             /* USB0 EPn Transmit Configuration and Status (Host) Register */
#define pREG_USB0_EP6_TXCSR_H            ((volatile uint16_t *)REG_USB0_EP6_TXCSR_H)             /* USB0 EPn Transmit Configuration and Status (Host) Register */
#define pREG_USB0_EP7_TXCSR_H            ((volatile uint16_t *)REG_USB0_EP7_TXCSR_H)             /* USB0 EPn Transmit Configuration and Status (Host) Register */
#define pREG_USB0_EP8_TXCSR_H            ((volatile uint16_t *)REG_USB0_EP8_TXCSR_H)             /* USB0 EPn Transmit Configuration and Status (Host) Register */
#define pREG_USB0_EP9_TXCSR_H            ((volatile uint16_t *)REG_USB0_EP9_TXCSR_H)             /* USB0 EPn Transmit Configuration and Status (Host) Register */
#define pREG_USB0_EP10_TXCSR_H           ((volatile uint16_t *)REG_USB0_EP10_TXCSR_H)            /* USB0 EPn Transmit Configuration and Status (Host) Register */
#define pREG_USB0_EP11_TXCSR_H           ((volatile uint16_t *)REG_USB0_EP11_TXCSR_H)            /* USB0 EPn Transmit Configuration and Status (Host) Register */
#define pREG_USB0_EP0_CSR0_P             ((volatile uint16_t *)REG_USB0_EP0_CSR0_P)              /* USB0 EP0 Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP0_TXCSR_P            ((volatile uint16_t *)REG_USB0_EP0_TXCSR_P)             /* USB0 EPn Transmit Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP1_TXCSR_P            ((volatile uint16_t *)REG_USB0_EP1_TXCSR_P)             /* USB0 EPn Transmit Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP2_TXCSR_P            ((volatile uint16_t *)REG_USB0_EP2_TXCSR_P)             /* USB0 EPn Transmit Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP3_TXCSR_P            ((volatile uint16_t *)REG_USB0_EP3_TXCSR_P)             /* USB0 EPn Transmit Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP4_TXCSR_P            ((volatile uint16_t *)REG_USB0_EP4_TXCSR_P)             /* USB0 EPn Transmit Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP5_TXCSR_P            ((volatile uint16_t *)REG_USB0_EP5_TXCSR_P)             /* USB0 EPn Transmit Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP6_TXCSR_P            ((volatile uint16_t *)REG_USB0_EP6_TXCSR_P)             /* USB0 EPn Transmit Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP7_TXCSR_P            ((volatile uint16_t *)REG_USB0_EP7_TXCSR_P)             /* USB0 EPn Transmit Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP8_TXCSR_P            ((volatile uint16_t *)REG_USB0_EP8_TXCSR_P)             /* USB0 EPn Transmit Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP9_TXCSR_P            ((volatile uint16_t *)REG_USB0_EP9_TXCSR_P)             /* USB0 EPn Transmit Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP10_TXCSR_P           ((volatile uint16_t *)REG_USB0_EP10_TXCSR_P)            /* USB0 EPn Transmit Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP11_TXCSR_P           ((volatile uint16_t *)REG_USB0_EP11_TXCSR_P)            /* USB0 EPn Transmit Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP0_RXMAXP             ((volatile uint16_t *)REG_USB0_EP0_RXMAXP)              /* USB0 EPn Receive Maximum Packet Length Register */
#define pREG_USB0_EP1_RXMAXP             ((volatile uint16_t *)REG_USB0_EP1_RXMAXP)              /* USB0 EPn Receive Maximum Packet Length Register */
#define pREG_USB0_EP2_RXMAXP             ((volatile uint16_t *)REG_USB0_EP2_RXMAXP)              /* USB0 EPn Receive Maximum Packet Length Register */
#define pREG_USB0_EP3_RXMAXP             ((volatile uint16_t *)REG_USB0_EP3_RXMAXP)              /* USB0 EPn Receive Maximum Packet Length Register */
#define pREG_USB0_EP4_RXMAXP             ((volatile uint16_t *)REG_USB0_EP4_RXMAXP)              /* USB0 EPn Receive Maximum Packet Length Register */
#define pREG_USB0_EP5_RXMAXP             ((volatile uint16_t *)REG_USB0_EP5_RXMAXP)              /* USB0 EPn Receive Maximum Packet Length Register */
#define pREG_USB0_EP6_RXMAXP             ((volatile uint16_t *)REG_USB0_EP6_RXMAXP)              /* USB0 EPn Receive Maximum Packet Length Register */
#define pREG_USB0_EP7_RXMAXP             ((volatile uint16_t *)REG_USB0_EP7_RXMAXP)              /* USB0 EPn Receive Maximum Packet Length Register */
#define pREG_USB0_EP8_RXMAXP             ((volatile uint16_t *)REG_USB0_EP8_RXMAXP)              /* USB0 EPn Receive Maximum Packet Length Register */
#define pREG_USB0_EP9_RXMAXP             ((volatile uint16_t *)REG_USB0_EP9_RXMAXP)              /* USB0 EPn Receive Maximum Packet Length Register */
#define pREG_USB0_EP10_RXMAXP            ((volatile uint16_t *)REG_USB0_EP10_RXMAXP)             /* USB0 EPn Receive Maximum Packet Length Register */
#define pREG_USB0_EP11_RXMAXP            ((volatile uint16_t *)REG_USB0_EP11_RXMAXP)             /* USB0 EPn Receive Maximum Packet Length Register */
#define pREG_USB0_EP0_RXCSR_H            ((volatile uint16_t *)REG_USB0_EP0_RXCSR_H)             /* USB0 EPn Receive Configuration and Status (Host) Register */
#define pREG_USB0_EP1_RXCSR_H            ((volatile uint16_t *)REG_USB0_EP1_RXCSR_H)             /* USB0 EPn Receive Configuration and Status (Host) Register */
#define pREG_USB0_EP2_RXCSR_H            ((volatile uint16_t *)REG_USB0_EP2_RXCSR_H)             /* USB0 EPn Receive Configuration and Status (Host) Register */
#define pREG_USB0_EP3_RXCSR_H            ((volatile uint16_t *)REG_USB0_EP3_RXCSR_H)             /* USB0 EPn Receive Configuration and Status (Host) Register */
#define pREG_USB0_EP4_RXCSR_H            ((volatile uint16_t *)REG_USB0_EP4_RXCSR_H)             /* USB0 EPn Receive Configuration and Status (Host) Register */
#define pREG_USB0_EP5_RXCSR_H            ((volatile uint16_t *)REG_USB0_EP5_RXCSR_H)             /* USB0 EPn Receive Configuration and Status (Host) Register */
#define pREG_USB0_EP6_RXCSR_H            ((volatile uint16_t *)REG_USB0_EP6_RXCSR_H)             /* USB0 EPn Receive Configuration and Status (Host) Register */
#define pREG_USB0_EP7_RXCSR_H            ((volatile uint16_t *)REG_USB0_EP7_RXCSR_H)             /* USB0 EPn Receive Configuration and Status (Host) Register */
#define pREG_USB0_EP8_RXCSR_H            ((volatile uint16_t *)REG_USB0_EP8_RXCSR_H)             /* USB0 EPn Receive Configuration and Status (Host) Register */
#define pREG_USB0_EP9_RXCSR_H            ((volatile uint16_t *)REG_USB0_EP9_RXCSR_H)             /* USB0 EPn Receive Configuration and Status (Host) Register */
#define pREG_USB0_EP10_RXCSR_H           ((volatile uint16_t *)REG_USB0_EP10_RXCSR_H)            /* USB0 EPn Receive Configuration and Status (Host) Register */
#define pREG_USB0_EP11_RXCSR_H           ((volatile uint16_t *)REG_USB0_EP11_RXCSR_H)            /* USB0 EPn Receive Configuration and Status (Host) Register */
#define pREG_USB0_EP0_RXCSR_P            ((volatile uint16_t *)REG_USB0_EP0_RXCSR_P)             /* USB0 EPn Receive Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP1_RXCSR_P            ((volatile uint16_t *)REG_USB0_EP1_RXCSR_P)             /* USB0 EPn Receive Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP2_RXCSR_P            ((volatile uint16_t *)REG_USB0_EP2_RXCSR_P)             /* USB0 EPn Receive Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP3_RXCSR_P            ((volatile uint16_t *)REG_USB0_EP3_RXCSR_P)             /* USB0 EPn Receive Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP4_RXCSR_P            ((volatile uint16_t *)REG_USB0_EP4_RXCSR_P)             /* USB0 EPn Receive Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP5_RXCSR_P            ((volatile uint16_t *)REG_USB0_EP5_RXCSR_P)             /* USB0 EPn Receive Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP6_RXCSR_P            ((volatile uint16_t *)REG_USB0_EP6_RXCSR_P)             /* USB0 EPn Receive Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP7_RXCSR_P            ((volatile uint16_t *)REG_USB0_EP7_RXCSR_P)             /* USB0 EPn Receive Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP8_RXCSR_P            ((volatile uint16_t *)REG_USB0_EP8_RXCSR_P)             /* USB0 EPn Receive Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP9_RXCSR_P            ((volatile uint16_t *)REG_USB0_EP9_RXCSR_P)             /* USB0 EPn Receive Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP10_RXCSR_P           ((volatile uint16_t *)REG_USB0_EP10_RXCSR_P)            /* USB0 EPn Receive Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP11_RXCSR_P           ((volatile uint16_t *)REG_USB0_EP11_RXCSR_P)            /* USB0 EPn Receive Configuration and Status (Peripheral) Register */
#define pREG_USB0_EP0_CNT0               ((volatile uint16_t *)REG_USB0_EP0_CNT0)                /* USB0 EP0 Number of Received Bytes Register */
#define pREG_USB0_EP0_RXCNT              ((volatile uint16_t *)REG_USB0_EP0_RXCNT)               /* USB0 EPn Number of Bytes Received Register */
#define pREG_USB0_EP1_RXCNT              ((volatile uint16_t *)REG_USB0_EP1_RXCNT)               /* USB0 EPn Number of Bytes Received Register */
#define pREG_USB0_EP2_RXCNT              ((volatile uint16_t *)REG_USB0_EP2_RXCNT)               /* USB0 EPn Number of Bytes Received Register */
#define pREG_USB0_EP3_RXCNT              ((volatile uint16_t *)REG_USB0_EP3_RXCNT)               /* USB0 EPn Number of Bytes Received Register */
#define pREG_USB0_EP4_RXCNT              ((volatile uint16_t *)REG_USB0_EP4_RXCNT)               /* USB0 EPn Number of Bytes Received Register */
#define pREG_USB0_EP5_RXCNT              ((volatile uint16_t *)REG_USB0_EP5_RXCNT)               /* USB0 EPn Number of Bytes Received Register */
#define pREG_USB0_EP6_RXCNT              ((volatile uint16_t *)REG_USB0_EP6_RXCNT)               /* USB0 EPn Number of Bytes Received Register */
#define pREG_USB0_EP7_RXCNT              ((volatile uint16_t *)REG_USB0_EP7_RXCNT)               /* USB0 EPn Number of Bytes Received Register */
#define pREG_USB0_EP8_RXCNT              ((volatile uint16_t *)REG_USB0_EP8_RXCNT)               /* USB0 EPn Number of Bytes Received Register */
#define pREG_USB0_EP9_RXCNT              ((volatile uint16_t *)REG_USB0_EP9_RXCNT)               /* USB0 EPn Number of Bytes Received Register */
#define pREG_USB0_EP10_RXCNT             ((volatile uint16_t *)REG_USB0_EP10_RXCNT)              /* USB0 EPn Number of Bytes Received Register */
#define pREG_USB0_EP11_RXCNT             ((volatile uint16_t *)REG_USB0_EP11_RXCNT)              /* USB0 EPn Number of Bytes Received Register */
#define pREG_USB0_EP0_TYPE0              ((volatile  uint8_t *)REG_USB0_EP0_TYPE0)               /* USB0 EP0 Connection Type Register */
#define pREG_USB0_EP0_TXTYPE             ((volatile  uint8_t *)REG_USB0_EP0_TXTYPE)              /* USB0 EPn Transmit Type Register */
#define pREG_USB0_EP1_TXTYPE             ((volatile  uint8_t *)REG_USB0_EP1_TXTYPE)              /* USB0 EPn Transmit Type Register */
#define pREG_USB0_EP2_TXTYPE             ((volatile  uint8_t *)REG_USB0_EP2_TXTYPE)              /* USB0 EPn Transmit Type Register */
#define pREG_USB0_EP3_TXTYPE             ((volatile  uint8_t *)REG_USB0_EP3_TXTYPE)              /* USB0 EPn Transmit Type Register */
#define pREG_USB0_EP4_TXTYPE             ((volatile  uint8_t *)REG_USB0_EP4_TXTYPE)              /* USB0 EPn Transmit Type Register */
#define pREG_USB0_EP5_TXTYPE             ((volatile  uint8_t *)REG_USB0_EP5_TXTYPE)              /* USB0 EPn Transmit Type Register */
#define pREG_USB0_EP6_TXTYPE             ((volatile  uint8_t *)REG_USB0_EP6_TXTYPE)              /* USB0 EPn Transmit Type Register */
#define pREG_USB0_EP7_TXTYPE             ((volatile  uint8_t *)REG_USB0_EP7_TXTYPE)              /* USB0 EPn Transmit Type Register */
#define pREG_USB0_EP8_TXTYPE             ((volatile  uint8_t *)REG_USB0_EP8_TXTYPE)              /* USB0 EPn Transmit Type Register */
#define pREG_USB0_EP9_TXTYPE             ((volatile  uint8_t *)REG_USB0_EP9_TXTYPE)              /* USB0 EPn Transmit Type Register */
#define pREG_USB0_EP10_TXTYPE            ((volatile  uint8_t *)REG_USB0_EP10_TXTYPE)             /* USB0 EPn Transmit Type Register */
#define pREG_USB0_EP11_TXTYPE            ((volatile  uint8_t *)REG_USB0_EP11_TXTYPE)             /* USB0 EPn Transmit Type Register */
#define pREG_USB0_EP0_NAKLIMIT0          ((volatile  uint8_t *)REG_USB0_EP0_NAKLIMIT0)           /* USB0 EP0 NAK Limit Register */
#define pREG_USB0_EP0_TXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP0_TXINTERVAL)          /* USB0 EPn Transmit Polling Interval Register */
#define pREG_USB0_EP1_TXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP1_TXINTERVAL)          /* USB0 EPn Transmit Polling Interval Register */
#define pREG_USB0_EP2_TXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP2_TXINTERVAL)          /* USB0 EPn Transmit Polling Interval Register */
#define pREG_USB0_EP3_TXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP3_TXINTERVAL)          /* USB0 EPn Transmit Polling Interval Register */
#define pREG_USB0_EP4_TXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP4_TXINTERVAL)          /* USB0 EPn Transmit Polling Interval Register */
#define pREG_USB0_EP5_TXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP5_TXINTERVAL)          /* USB0 EPn Transmit Polling Interval Register */
#define pREG_USB0_EP6_TXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP6_TXINTERVAL)          /* USB0 EPn Transmit Polling Interval Register */
#define pREG_USB0_EP7_TXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP7_TXINTERVAL)          /* USB0 EPn Transmit Polling Interval Register */
#define pREG_USB0_EP8_TXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP8_TXINTERVAL)          /* USB0 EPn Transmit Polling Interval Register */
#define pREG_USB0_EP9_TXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP9_TXINTERVAL)          /* USB0 EPn Transmit Polling Interval Register */
#define pREG_USB0_EP10_TXINTERVAL        ((volatile  uint8_t *)REG_USB0_EP10_TXINTERVAL)         /* USB0 EPn Transmit Polling Interval Register */
#define pREG_USB0_EP11_TXINTERVAL        ((volatile  uint8_t *)REG_USB0_EP11_TXINTERVAL)         /* USB0 EPn Transmit Polling Interval Register */
#define pREG_USB0_EP0_RXTYPE             ((volatile  uint8_t *)REG_USB0_EP0_RXTYPE)              /* USB0 EPn Receive Type Register */
#define pREG_USB0_EP1_RXTYPE             ((volatile  uint8_t *)REG_USB0_EP1_RXTYPE)              /* USB0 EPn Receive Type Register */
#define pREG_USB0_EP2_RXTYPE             ((volatile  uint8_t *)REG_USB0_EP2_RXTYPE)              /* USB0 EPn Receive Type Register */
#define pREG_USB0_EP3_RXTYPE             ((volatile  uint8_t *)REG_USB0_EP3_RXTYPE)              /* USB0 EPn Receive Type Register */
#define pREG_USB0_EP4_RXTYPE             ((volatile  uint8_t *)REG_USB0_EP4_RXTYPE)              /* USB0 EPn Receive Type Register */
#define pREG_USB0_EP5_RXTYPE             ((volatile  uint8_t *)REG_USB0_EP5_RXTYPE)              /* USB0 EPn Receive Type Register */
#define pREG_USB0_EP6_RXTYPE             ((volatile  uint8_t *)REG_USB0_EP6_RXTYPE)              /* USB0 EPn Receive Type Register */
#define pREG_USB0_EP7_RXTYPE             ((volatile  uint8_t *)REG_USB0_EP7_RXTYPE)              /* USB0 EPn Receive Type Register */
#define pREG_USB0_EP8_RXTYPE             ((volatile  uint8_t *)REG_USB0_EP8_RXTYPE)              /* USB0 EPn Receive Type Register */
#define pREG_USB0_EP9_RXTYPE             ((volatile  uint8_t *)REG_USB0_EP9_RXTYPE)              /* USB0 EPn Receive Type Register */
#define pREG_USB0_EP10_RXTYPE            ((volatile  uint8_t *)REG_USB0_EP10_RXTYPE)             /* USB0 EPn Receive Type Register */
#define pREG_USB0_EP11_RXTYPE            ((volatile  uint8_t *)REG_USB0_EP11_RXTYPE)             /* USB0 EPn Receive Type Register */
#define pREG_USB0_EP0_RXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP0_RXINTERVAL)          /* USB0 EPn Receive Polling Interval Register */
#define pREG_USB0_EP1_RXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP1_RXINTERVAL)          /* USB0 EPn Receive Polling Interval Register */
#define pREG_USB0_EP2_RXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP2_RXINTERVAL)          /* USB0 EPn Receive Polling Interval Register */
#define pREG_USB0_EP3_RXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP3_RXINTERVAL)          /* USB0 EPn Receive Polling Interval Register */
#define pREG_USB0_EP4_RXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP4_RXINTERVAL)          /* USB0 EPn Receive Polling Interval Register */
#define pREG_USB0_EP5_RXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP5_RXINTERVAL)          /* USB0 EPn Receive Polling Interval Register */
#define pREG_USB0_EP6_RXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP6_RXINTERVAL)          /* USB0 EPn Receive Polling Interval Register */
#define pREG_USB0_EP7_RXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP7_RXINTERVAL)          /* USB0 EPn Receive Polling Interval Register */
#define pREG_USB0_EP8_RXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP8_RXINTERVAL)          /* USB0 EPn Receive Polling Interval Register */
#define pREG_USB0_EP9_RXINTERVAL         ((volatile  uint8_t *)REG_USB0_EP9_RXINTERVAL)          /* USB0 EPn Receive Polling Interval Register */
#define pREG_USB0_EP10_RXINTERVAL        ((volatile  uint8_t *)REG_USB0_EP10_RXINTERVAL)         /* USB0 EPn Receive Polling Interval Register */
#define pREG_USB0_EP11_RXINTERVAL        ((volatile  uint8_t *)REG_USB0_EP11_RXINTERVAL)         /* USB0 EPn Receive Polling Interval Register */
#define pREG_USB0_EP0_CFGDATA0           ((volatile  uint8_t *)REG_USB0_EP0_CFGDATA0)            /* USB0 EP0 Configuration Information Register */
#define pREG_USB0_DMA_IRQ                ((volatile  uint8_t *)REG_USB0_DMA_IRQ)                 /* USB0 DMA Interrupt Register */
#define pREG_USB0_DMA0_CTL               ((volatile uint16_t *)REG_USB0_DMA0_CTL)                /* USB0 DMA Channel n Control Register */
#define pREG_USB0_DMA1_CTL               ((volatile uint16_t *)REG_USB0_DMA1_CTL)                /* USB0 DMA Channel n Control Register */
#define pREG_USB0_DMA2_CTL               ((volatile uint16_t *)REG_USB0_DMA2_CTL)                /* USB0 DMA Channel n Control Register */
#define pREG_USB0_DMA3_CTL               ((volatile uint16_t *)REG_USB0_DMA3_CTL)                /* USB0 DMA Channel n Control Register */
#define pREG_USB0_DMA4_CTL               ((volatile uint16_t *)REG_USB0_DMA4_CTL)                /* USB0 DMA Channel n Control Register */
#define pREG_USB0_DMA5_CTL               ((volatile uint16_t *)REG_USB0_DMA5_CTL)                /* USB0 DMA Channel n Control Register */
#define pREG_USB0_DMA6_CTL               ((volatile uint16_t *)REG_USB0_DMA6_CTL)                /* USB0 DMA Channel n Control Register */
#define pREG_USB0_DMA7_CTL               ((volatile uint16_t *)REG_USB0_DMA7_CTL)                /* USB0 DMA Channel n Control Register */
#define pREG_USB0_DMA0_ADDR              ((void * volatile *)REG_USB0_DMA0_ADDR)                 /* USB0 DMA Channel n Address Register */
#define pREG_USB0_DMA1_ADDR              ((void * volatile *)REG_USB0_DMA1_ADDR)                 /* USB0 DMA Channel n Address Register */
#define pREG_USB0_DMA2_ADDR              ((void * volatile *)REG_USB0_DMA2_ADDR)                 /* USB0 DMA Channel n Address Register */
#define pREG_USB0_DMA3_ADDR              ((void * volatile *)REG_USB0_DMA3_ADDR)                 /* USB0 DMA Channel n Address Register */
#define pREG_USB0_DMA4_ADDR              ((void * volatile *)REG_USB0_DMA4_ADDR)                 /* USB0 DMA Channel n Address Register */
#define pREG_USB0_DMA5_ADDR              ((void * volatile *)REG_USB0_DMA5_ADDR)                 /* USB0 DMA Channel n Address Register */
#define pREG_USB0_DMA6_ADDR              ((void * volatile *)REG_USB0_DMA6_ADDR)                 /* USB0 DMA Channel n Address Register */
#define pREG_USB0_DMA7_ADDR              ((void * volatile *)REG_USB0_DMA7_ADDR)                 /* USB0 DMA Channel n Address Register */
#define pREG_USB0_DMA0_CNT               ((volatile uint32_t *)REG_USB0_DMA0_CNT)                /* USB0 DMA Channel n Count Register */
#define pREG_USB0_DMA1_CNT               ((volatile uint32_t *)REG_USB0_DMA1_CNT)                /* USB0 DMA Channel n Count Register */
#define pREG_USB0_DMA2_CNT               ((volatile uint32_t *)REG_USB0_DMA2_CNT)                /* USB0 DMA Channel n Count Register */
#define pREG_USB0_DMA3_CNT               ((volatile uint32_t *)REG_USB0_DMA3_CNT)                /* USB0 DMA Channel n Count Register */
#define pREG_USB0_DMA4_CNT               ((volatile uint32_t *)REG_USB0_DMA4_CNT)                /* USB0 DMA Channel n Count Register */
#define pREG_USB0_DMA5_CNT               ((volatile uint32_t *)REG_USB0_DMA5_CNT)                /* USB0 DMA Channel n Count Register */
#define pREG_USB0_DMA6_CNT               ((volatile uint32_t *)REG_USB0_DMA6_CNT)                /* USB0 DMA Channel n Count Register */
#define pREG_USB0_DMA7_CNT               ((volatile uint32_t *)REG_USB0_DMA7_CNT)                /* USB0 DMA Channel n Count Register */
#define pREG_USB0_RQPKTCNT0              ((volatile uint16_t *)REG_USB0_RQPKTCNT0)               /* USB0 EPn Request Packet Count Register */
#define pREG_USB0_RQPKTCNT1              ((volatile uint16_t *)REG_USB0_RQPKTCNT1)               /* USB0 EPn Request Packet Count Register */
#define pREG_USB0_RQPKTCNT2              ((volatile uint16_t *)REG_USB0_RQPKTCNT2)               /* USB0 EPn Request Packet Count Register */
#define pREG_USB0_RQPKTCNT3              ((volatile uint16_t *)REG_USB0_RQPKTCNT3)               /* USB0 EPn Request Packet Count Register */
#define pREG_USB0_RQPKTCNT4              ((volatile uint16_t *)REG_USB0_RQPKTCNT4)               /* USB0 EPn Request Packet Count Register */
#define pREG_USB0_RQPKTCNT5              ((volatile uint16_t *)REG_USB0_RQPKTCNT5)               /* USB0 EPn Request Packet Count Register */
#define pREG_USB0_RQPKTCNT6              ((volatile uint16_t *)REG_USB0_RQPKTCNT6)               /* USB0 EPn Request Packet Count Register */
#define pREG_USB0_RQPKTCNT7              ((volatile uint16_t *)REG_USB0_RQPKTCNT7)               /* USB0 EPn Request Packet Count Register */
#define pREG_USB0_RQPKTCNT8              ((volatile uint16_t *)REG_USB0_RQPKTCNT8)               /* USB0 EPn Request Packet Count Register */
#define pREG_USB0_RQPKTCNT9              ((volatile uint16_t *)REG_USB0_RQPKTCNT9)               /* USB0 EPn Request Packet Count Register */
#define pREG_USB0_RQPKTCNT10             ((volatile uint16_t *)REG_USB0_RQPKTCNT10)              /* USB0 EPn Request Packet Count Register */
#define pREG_USB0_CT_UCH                 ((volatile uint16_t *)REG_USB0_CT_UCH)                  /* USB0 Chirp Timeout Register */
#define pREG_USB0_CT_HHSRTN              ((volatile uint16_t *)REG_USB0_CT_HHSRTN)               /* USB0 Host High Speed Return to Normal Register */
#define pREG_USB0_CT_HSBT                ((volatile uint16_t *)REG_USB0_CT_HSBT)                 /* USB0 High Speed Timeout Register */
#define pREG_USB0_LPM_ATTR               ((volatile uint16_t *)REG_USB0_LPM_ATTR)                /* USB0 LPM Attribute Register */
#define pREG_USB0_LPM_CTL                ((volatile  uint8_t *)REG_USB0_LPM_CTL)                 /* USB0 LPM Control Register */
#define pREG_USB0_LPM_IEN                ((volatile  uint8_t *)REG_USB0_LPM_IEN)                 /* USB0 LPM Interrupt Enable Register */
#define pREG_USB0_LPM_IRQ                ((volatile  uint8_t *)REG_USB0_LPM_IRQ)                 /* USB0 LPM Interrupt Status Register */
#define pREG_USB0_LPM_FADDR              ((volatile  uint8_t *)REG_USB0_LPM_FADDR)               /* USB0 LPM Function Address Register */
#define pREG_USB0_VBUS_CTL               ((volatile  uint8_t *)REG_USB0_VBUS_CTL)                /* USB0 VBUS Control Register */
#define pREG_USB0_BAT_CHG                ((volatile  uint8_t *)REG_USB0_BAT_CHG)                 /* USB0 Battery Charging Control Register */
#define pREG_USB0_PHY_CTL                ((volatile  uint8_t *)REG_USB0_PHY_CTL)                 /* USB0 PHY Control Register */
#define pREG_USB0_PLL_OSC                ((volatile uint16_t *)REG_USB0_PLL_OSC)                 /* USB0 PLL and Oscillator Control Register */


/* =========================================================================
       L1DM0
   ========================================================================= */
#define pSRAM_BASE_ADDRESS               ((void * volatile *)SRAM_BASE_ADDRESS)                  /* SRAM Base Address */
#define pDMEM_CONTROL                    ((volatile uint32_t *)DMEM_CONTROL)                     /* Data memory control */
#define pDCPLB_STATUS                    ((volatile uint32_t *)DCPLB_STATUS)                     /* Data Cacheability Protection Lookaside Buffer Status */
#define pDCPLB_FAULT_ADDR                ((void * volatile *)DCPLB_FAULT_ADDR)                   /* Data Cacheability Protection Lookaside Buffer Fault Address */
#define pDCPLB_ADDR0                     ((void * volatile *)DCPLB_ADDR0)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR1                     ((void * volatile *)DCPLB_ADDR1)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR2                     ((void * volatile *)DCPLB_ADDR2)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR3                     ((void * volatile *)DCPLB_ADDR3)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR4                     ((void * volatile *)DCPLB_ADDR4)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR5                     ((void * volatile *)DCPLB_ADDR5)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR6                     ((void * volatile *)DCPLB_ADDR6)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR7                     ((void * volatile *)DCPLB_ADDR7)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR8                     ((void * volatile *)DCPLB_ADDR8)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR9                     ((void * volatile *)DCPLB_ADDR9)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR10                    ((void * volatile *)DCPLB_ADDR10)                       /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR11                    ((void * volatile *)DCPLB_ADDR11)                       /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR12                    ((void * volatile *)DCPLB_ADDR12)                       /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR13                    ((void * volatile *)DCPLB_ADDR13)                       /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR14                    ((void * volatile *)DCPLB_ADDR14)                       /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_ADDR15                    ((void * volatile *)DCPLB_ADDR15)                       /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pDCPLB_DATA0                     ((volatile uint32_t *)DCPLB_DATA0)                      /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA1                     ((volatile uint32_t *)DCPLB_DATA1)                      /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA2                     ((volatile uint32_t *)DCPLB_DATA2)                      /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA3                     ((volatile uint32_t *)DCPLB_DATA3)                      /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA4                     ((volatile uint32_t *)DCPLB_DATA4)                      /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA5                     ((volatile uint32_t *)DCPLB_DATA5)                      /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA6                     ((volatile uint32_t *)DCPLB_DATA6)                      /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA7                     ((volatile uint32_t *)DCPLB_DATA7)                      /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA8                     ((volatile uint32_t *)DCPLB_DATA8)                      /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA9                     ((volatile uint32_t *)DCPLB_DATA9)                      /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA10                    ((volatile uint32_t *)DCPLB_DATA10)                     /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA11                    ((volatile uint32_t *)DCPLB_DATA11)                     /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA12                    ((volatile uint32_t *)DCPLB_DATA12)                     /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA13                    ((volatile uint32_t *)DCPLB_DATA13)                     /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA14                    ((volatile uint32_t *)DCPLB_DATA14)                     /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDCPLB_DATA15                    ((volatile uint32_t *)DCPLB_DATA15)                     /* Cacheability Protection Lookaside Buffer Descriptor Data */
#define pDTEST_COMMAND                   ((volatile uint32_t *)DTEST_COMMAND)                    /* Data Test Command Register */
#define pDTEST_DATA0                     ((volatile uint32_t *)DTEST_DATA0)                      /* Data Test Data Register */
#define pDTEST_DATA1                     ((volatile uint32_t *)DTEST_DATA1)                      /* Data Test Data Register */
#define pL1DBNKA_PELOC                   ((volatile uint32_t *)L1DBNKA_PELOC)                    /* Data Bank A Parity Error Location */
#define pL1DBNKB_PELOC                   ((volatile uint32_t *)L1DBNKB_PELOC)                    /* Data Bank B Parity Error Location */


/* =========================================================================
       L1IM0
   ========================================================================= */
#define pIMEM_CONTROL                    ((volatile uint32_t *)IMEM_CONTROL)                     /* Instruction memory control */
#define pICPLB_STATUS                    ((volatile uint32_t *)ICPLB_STATUS)                     /* Cacheability Protection Lookaside Buffer Status */
#define pICPLB_FAULT_ADDR                ((void * volatile *)ICPLB_FAULT_ADDR)                   /* Cacheability Protection Lookaside Buffer Fault Address */
#define pICPLB_ADDR0                     ((void * volatile *)ICPLB_ADDR0)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR1                     ((void * volatile *)ICPLB_ADDR1)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR2                     ((void * volatile *)ICPLB_ADDR2)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR3                     ((void * volatile *)ICPLB_ADDR3)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR4                     ((void * volatile *)ICPLB_ADDR4)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR5                     ((void * volatile *)ICPLB_ADDR5)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR6                     ((void * volatile *)ICPLB_ADDR6)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR7                     ((void * volatile *)ICPLB_ADDR7)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR8                     ((void * volatile *)ICPLB_ADDR8)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR9                     ((void * volatile *)ICPLB_ADDR9)                        /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR10                    ((void * volatile *)ICPLB_ADDR10)                       /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR11                    ((void * volatile *)ICPLB_ADDR11)                       /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR12                    ((void * volatile *)ICPLB_ADDR12)                       /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR13                    ((void * volatile *)ICPLB_ADDR13)                       /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR14                    ((void * volatile *)ICPLB_ADDR14)                       /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_ADDR15                    ((void * volatile *)ICPLB_ADDR15)                       /* Cacheability Protection Lookaside Buffer Descriptor Address */
#define pICPLB_DATA0                     ((volatile uint32_t *)ICPLB_DATA0)                      /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA1                     ((volatile uint32_t *)ICPLB_DATA1)                      /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA2                     ((volatile uint32_t *)ICPLB_DATA2)                      /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA3                     ((volatile uint32_t *)ICPLB_DATA3)                      /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA4                     ((volatile uint32_t *)ICPLB_DATA4)                      /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA5                     ((volatile uint32_t *)ICPLB_DATA5)                      /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA6                     ((volatile uint32_t *)ICPLB_DATA6)                      /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA7                     ((volatile uint32_t *)ICPLB_DATA7)                      /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA8                     ((volatile uint32_t *)ICPLB_DATA8)                      /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA9                     ((volatile uint32_t *)ICPLB_DATA9)                      /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA10                    ((volatile uint32_t *)ICPLB_DATA10)                     /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA11                    ((volatile uint32_t *)ICPLB_DATA11)                     /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA12                    ((volatile uint32_t *)ICPLB_DATA12)                     /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA13                    ((volatile uint32_t *)ICPLB_DATA13)                     /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA14                    ((volatile uint32_t *)ICPLB_DATA14)                     /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pICPLB_DATA15                    ((volatile uint32_t *)ICPLB_DATA15)                     /* Cacheability Protection Lookaside Buffer Descriptor Status */
#define pITEST_COMMAND                   ((volatile uint32_t *)ITEST_COMMAND)                    /* Instruction Test Command Register */
#define pITEST_DATA0                     ((volatile uint32_t *)ITEST_DATA0)                      /* Instruction Test Data Register */
#define pITEST_DATA1                     ((volatile uint32_t *)ITEST_DATA1)                      /* Instruction Test Data Register */
#define pL1IBNKA_PELOC                   ((volatile uint32_t *)L1IBNKA_PELOC)                    /* Instruction Bank A Parity Error Location */
#define pL1IBNKB_PELOC                   ((volatile uint32_t *)L1IBNKB_PELOC)                    /* Instruction Bank B Parity Error Location */
#define pL1IBNKC_PELOC                   ((volatile uint32_t *)L1IBNKC_PELOC)                    /* Instruction Bank C Parity Error Location */


/* =========================================================================
       ICU0
   ========================================================================= */
#define pEVT0                            ((void * volatile *)EVT0)                               /* Event Vector */
#define pEVT1                            ((void * volatile *)EVT1)                               /* Event Vector */
#define pEVT2                            ((void * volatile *)EVT2)                               /* Event Vector */
#define pEVT3                            ((void * volatile *)EVT3)                               /* Event Vector */
#define pEVT4                            ((void * volatile *)EVT4)                               /* Event Vector */
#define pEVT5                            ((void * volatile *)EVT5)                               /* Event Vector */
#define pEVT6                            ((void * volatile *)EVT6)                               /* Event Vector */
#define pEVT7                            ((void * volatile *)EVT7)                               /* Event Vector */
#define pEVT8                            ((void * volatile *)EVT8)                               /* Event Vector */
#define pEVT9                            ((void * volatile *)EVT9)                               /* Event Vector */
#define pEVT10                           ((void * volatile *)EVT10)                              /* Event Vector */
#define pEVT11                           ((void * volatile *)EVT11)                              /* Event Vector */
#define pEVT12                           ((void * volatile *)EVT12)                              /* Event Vector */
#define pEVT13                           ((void * volatile *)EVT13)                              /* Event Vector */
#define pEVT14                           ((void * volatile *)EVT14)                              /* Event Vector */
#define pEVT15                           ((void * volatile *)EVT15)                              /* Event Vector */
#define pIMASK                           ((volatile uint32_t *)IMASK)                            /* Interrupt Mask Register */
#define pIPEND                           ((volatile uint32_t *)IPEND)                            /* Interrupts Pending Register */
#define pILAT                            ((volatile uint32_t *)ILAT)                             /* Interrupt Latch Register */
#define pIPRIO                           ((volatile uint32_t *)IPRIO)                            /* Interrupt Priority Register */
#define pCEC_SID                         ((volatile uint32_t *)CEC_SID)                          /* Core System Interrupt ID */


/* =========================================================================
       TMR0
   ========================================================================= */
#define pTCNTL                           ((volatile uint32_t *)TCNTL)                            /* Timer Control Register */
#define pTPERIOD                         ((volatile uint32_t *)TPERIOD)                          /* Timer Period Register */
#define pTSCALE                          ((volatile uint32_t *)TSCALE)                           /* Timer Scale Register */
#define pTCOUNT                          ((volatile uint32_t *)TCOUNT)                           /* Timer Count Register */


/* =========================================================================
       DBG0
   ========================================================================= */
#define pDSPID                           ((volatile uint32_t *)DSPID)                            /* DSP Identification Register */


/* =========================================================================
       TB0
   ========================================================================= */
#define pTBUFCTL                         ((volatile uint32_t *)TBUFCTL)                          /* Trace Buffer Control Register */
#define pTBUFSTAT                        ((volatile uint32_t *)TBUFSTAT)                         /* Trace Buffer Status Register */
#define pTBUF                            ((void * volatile *)TBUF)                               /* Trace Buffer */


/* =========================================================================
       WP0
   ========================================================================= */
#define pWPIACTL                         ((volatile uint32_t *)WPIACTL)                          /* Watchpoint Instruction Address Control Register 01 */
#define pWPIA0                           ((void * volatile *)WPIA0)                              /* Watchpoint Instruction Address Register */
#define pWPIA1                           ((void * volatile *)WPIA1)                              /* Watchpoint Instruction Address Register */
#define pWPIA2                           ((void * volatile *)WPIA2)                              /* Watchpoint Instruction Address Register */
#define pWPIA3                           ((void * volatile *)WPIA3)                              /* Watchpoint Instruction Address Register */
#define pWPIA4                           ((void * volatile *)WPIA4)                              /* Watchpoint Instruction Address Register */
#define pWPIA5                           ((void * volatile *)WPIA5)                              /* Watchpoint Instruction Address Register */
#define pWPIACNT0                        ((volatile uint32_t *)WPIACNT0)                         /* Watchpoint Instruction Address Count Register */
#define pWPIACNT1                        ((volatile uint32_t *)WPIACNT1)                         /* Watchpoint Instruction Address Count Register */
#define pWPIACNT2                        ((volatile uint32_t *)WPIACNT2)                         /* Watchpoint Instruction Address Count Register */
#define pWPIACNT3                        ((volatile uint32_t *)WPIACNT3)                         /* Watchpoint Instruction Address Count Register */
#define pWPIACNT4                        ((volatile uint32_t *)WPIACNT4)                         /* Watchpoint Instruction Address Count Register */
#define pWPIACNT5                        ((volatile uint32_t *)WPIACNT5)                         /* Watchpoint Instruction Address Count Register */
#define pWPDACTL                         ((volatile uint32_t *)WPDACTL)                          /* Watchpoint Data Address Control Register */
#define pWPDA0                           ((void * volatile *)WPDA0)                              /* Watchpoint Data Address Register */
#define pWPDA1                           ((void * volatile *)WPDA1)                              /* Watchpoint Data Address Register */
#define pWPDACNT0                        ((volatile uint32_t *)WPDACNT0)                         /* Watchpoint Data Address Count Value Register */
#define pWPDACNT1                        ((volatile uint32_t *)WPDACNT1)                         /* Watchpoint Data Address Count Value Register */
#define pWPSTAT                          ((volatile uint32_t *)WPSTAT)                           /* Watchpoint Status Register */


/* =========================================================================
       PF0
   ========================================================================= */
#define pPFCTL                           ((volatile uint32_t *)PFCTL)                            /* Performance Monitor Control Register */
#define pPFCNTR0                         ((volatile uint32_t *)PFCNTR0)                          /* Performance Monitor Counter 0 */
#define pPFCNTR1                         ((volatile uint32_t *)PFCNTR1)                          /* Performance Monitor Counter 1 */

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif	/* end ifndef _CDEF_BF608_H */
