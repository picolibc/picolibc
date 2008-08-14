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
** cdefBF549.h
**
** Copyright (C) 2008 Analog Devices, Inc.
**
************************************************************************************
**
** This include file contains a list of macro "defines" to enable the programmer
** to use symbolic names for the ADSP-BF549 peripherals.
**
************************************************************************************
** System MMR Register Map
************************************************************************************/

#ifndef _CDEF_BF549_H
#define _CDEF_BF549_H

/* include all Core registers and bit definitions */
#include <defBF549.h>

/* include core specific register pointer definitions */
#include <cdef_LPBlackfin.h>

/* SYSTEM & MMR ADDRESS DEFINITIONS FOR ADSP-BF549 */

/* include cdefBF54x_base.h for the set of #defines that are common to all ADSP-BF54x processors */
#include <cdefBF54x_base.h>

/* The following are the #defines needed by ADSP-BF549 that are not in the common header */

/* Timer Registers */

#define                    pTIMER8_CONFIG ((volatile unsigned short *)TIMER8_CONFIG)
#define                   pTIMER8_COUNTER ((volatile unsigned long *)TIMER8_COUNTER)
#define                    pTIMER8_PERIOD ((volatile unsigned long *)TIMER8_PERIOD)
#define                     pTIMER8_WIDTH ((volatile unsigned long *)TIMER8_WIDTH)
#define                    pTIMER9_CONFIG ((volatile unsigned short *)TIMER9_CONFIG)
#define                   pTIMER9_COUNTER ((volatile unsigned long *)TIMER9_COUNTER)
#define                    pTIMER9_PERIOD ((volatile unsigned long *)TIMER9_PERIOD)
#define                     pTIMER9_WIDTH ((volatile unsigned long *)TIMER9_WIDTH)
#define                   pTIMER10_CONFIG ((volatile unsigned short *)TIMER10_CONFIG)
#define                  pTIMER10_COUNTER ((volatile unsigned long *)TIMER10_COUNTER)
#define                   pTIMER10_PERIOD ((volatile unsigned long *)TIMER10_PERIOD)
#define                    pTIMER10_WIDTH ((volatile unsigned long *)TIMER10_WIDTH)

/* Timer Group of 3 */

#define                    pTIMER_ENABLE1 ((volatile unsigned short *)TIMER_ENABLE1)
#define                   pTIMER_DISABLE1 ((volatile unsigned short *)TIMER_DISABLE1)
#define                    pTIMER_STATUS1 ((volatile unsigned long *)TIMER_STATUS1)

/* SPORT0 Registers */

#define                      pSPORT0_TCR1 ((volatile unsigned short *)SPORT0_TCR1)
#define                      pSPORT0_TCR2 ((volatile unsigned short *)SPORT0_TCR2)
#define                   pSPORT0_TCLKDIV ((volatile unsigned short *)SPORT0_TCLKDIV)
#define                    pSPORT0_TFSDIV ((volatile unsigned short *)SPORT0_TFSDIV)
#define                        pSPORT0_TX ((volatile unsigned long *)SPORT0_TX)
#define                        pSPORT0_RX ((volatile unsigned long *)SPORT0_RX)
#define                      pSPORT0_RCR1 ((volatile unsigned short *)SPORT0_RCR1)
#define                      pSPORT0_RCR2 ((volatile unsigned short *)SPORT0_RCR2)
#define                   pSPORT0_RCLKDIV ((volatile unsigned short *)SPORT0_RCLKDIV)
#define                    pSPORT0_RFSDIV ((volatile unsigned short *)SPORT0_RFSDIV)
#define                      pSPORT0_STAT ((volatile unsigned short *)SPORT0_STAT)
#define                      pSPORT0_CHNL ((volatile unsigned short *)SPORT0_CHNL)
#define                     pSPORT0_MCMC1 ((volatile unsigned short *)SPORT0_MCMC1)
#define                     pSPORT0_MCMC2 ((volatile unsigned short *)SPORT0_MCMC2)
#define                     pSPORT0_MTCS0 ((volatile unsigned long *)SPORT0_MTCS0)
#define                     pSPORT0_MTCS1 ((volatile unsigned long *)SPORT0_MTCS1)
#define                     pSPORT0_MTCS2 ((volatile unsigned long *)SPORT0_MTCS2)
#define                     pSPORT0_MTCS3 ((volatile unsigned long *)SPORT0_MTCS3)
#define                     pSPORT0_MRCS0 ((volatile unsigned long *)SPORT0_MRCS0)
#define                     pSPORT0_MRCS1 ((volatile unsigned long *)SPORT0_MRCS1)
#define                     pSPORT0_MRCS2 ((volatile unsigned long *)SPORT0_MRCS2)
#define                     pSPORT0_MRCS3 ((volatile unsigned long *)SPORT0_MRCS3)

/* EPPI0 Registers */

#define                     pEPPI0_STATUS ((volatile unsigned short *)EPPI0_STATUS)
#define                     pEPPI0_HCOUNT ((volatile unsigned short *)EPPI0_HCOUNT)
#define                     pEPPI0_HDELAY ((volatile unsigned short *)EPPI0_HDELAY)
#define                     pEPPI0_VCOUNT ((volatile unsigned short *)EPPI0_VCOUNT)
#define                     pEPPI0_VDELAY ((volatile unsigned short *)EPPI0_VDELAY)
#define                      pEPPI0_FRAME ((volatile unsigned short *)EPPI0_FRAME)
#define                       pEPPI0_LINE ((volatile unsigned short *)EPPI0_LINE)
#define                     pEPPI0_CLKDIV ((volatile unsigned short *)EPPI0_CLKDIV)
#define                    pEPPI0_CONTROL ((volatile unsigned long *)EPPI0_CONTROL)
#define                   pEPPI0_FS1W_HBL ((volatile unsigned long *)EPPI0_FS1W_HBL)
#define                  pEPPI0_FS1P_AVPL ((volatile unsigned long *)EPPI0_FS1P_AVPL)
#define                   pEPPI0_FS2W_LVB ((volatile unsigned long *)EPPI0_FS2W_LVB)
#define                  pEPPI0_FS2P_LAVF ((volatile unsigned long *)EPPI0_FS2P_LAVF)
#define                       pEPPI0_CLIP ((volatile unsigned long *)EPPI0_CLIP)

/* UART2 Registers */

#define                        pUART2_DLL ((volatile unsigned short *)UART2_DLL)
#define                        pUART2_DLH ((volatile unsigned short *)UART2_DLH)
#define                       pUART2_GCTL ((volatile unsigned short *)UART2_GCTL)
#define                        pUART2_LCR ((volatile unsigned short *)UART2_LCR)
#define                        pUART2_MCR ((volatile unsigned short *)UART2_MCR)
#define                        pUART2_LSR ((volatile unsigned short *)UART2_LSR)
#define                        pUART2_MSR ((volatile unsigned short *)UART2_MSR)
#define                        pUART2_SCR ((volatile unsigned short *)UART2_SCR)
#define                    pUART2_IER_SET ((volatile unsigned short *)UART2_IER_SET)
#define                  pUART2_IER_CLEAR ((volatile unsigned short *)UART2_IER_CLEAR)
#define                        pUART2_THR ((volatile unsigned short *)UART2_THR)
#define                        pUART2_RBR ((volatile unsigned short *)UART2_RBR)

/* Two Wire Interface Registers (TWI1) */

#define                      pTWI1_CLKDIV ((volatile unsigned short *)TWI1_CLKDIV)
#define                     pTWI1_CONTROL ((volatile unsigned short *)TWI1_CONTROL)
#define                   pTWI1_SLAVE_CTL ((volatile unsigned short *)TWI1_SLAVE_CTL)
#define                  pTWI1_SLAVE_STAT ((volatile unsigned short *)TWI1_SLAVE_STAT)
#define                  pTWI1_SLAVE_ADDR ((volatile unsigned short *)TWI1_SLAVE_ADDR)
#define                  pTWI1_MASTER_CTL ((volatile unsigned short *)TWI1_MASTER_CTL)
#define                 pTWI1_MASTER_STAT ((volatile unsigned short *)TWI1_MASTER_STAT)
#define                 pTWI1_MASTER_ADDR ((volatile unsigned short *)TWI1_MASTER_ADDR)
#define                    pTWI1_INT_STAT ((volatile unsigned short *)TWI1_INT_STAT)
#define                    pTWI1_INT_MASK ((volatile unsigned short *)TWI1_INT_MASK)
#define                    pTWI1_FIFO_CTL ((volatile unsigned short *)TWI1_FIFO_CTL)
#define                   pTWI1_FIFO_STAT ((volatile unsigned short *)TWI1_FIFO_STAT)
#define                   pTWI1_XMT_DATA8 ((volatile unsigned short *)TWI1_XMT_DATA8)
#define                  pTWI1_XMT_DATA16 ((volatile unsigned short *)TWI1_XMT_DATA16)
#define                   pTWI1_RCV_DATA8 ((volatile unsigned short *)TWI1_RCV_DATA8)
#define                  pTWI1_RCV_DATA16 ((volatile unsigned short *)TWI1_RCV_DATA16)

/* SPI2 Registers */

#define                         pSPI2_CTL ((volatile unsigned short *)SPI2_CTL)
#define                         pSPI2_FLG ((volatile unsigned short *)SPI2_FLG)
#define                        pSPI2_STAT ((volatile unsigned short *)SPI2_STAT)
#define                        pSPI2_TDBR ((volatile unsigned short *)SPI2_TDBR)
#define                        pSPI2_RDBR ((volatile unsigned short *)SPI2_RDBR)
#define                        pSPI2_BAUD ((volatile unsigned short *)SPI2_BAUD)
#define                      pSPI2_SHADOW ((volatile unsigned short *)SPI2_SHADOW)

/* MXVR Registers */

#define                      pMXVR_CONFIG ((volatile unsigned short *)MXVR_CONFIG)
#define                     pMXVR_STATE_0 ((volatile unsigned long *)MXVR_STATE_0)
#define                     pMXVR_STATE_1 ((volatile unsigned long *)MXVR_STATE_1)
#define                  pMXVR_INT_STAT_0 ((volatile unsigned long *)MXVR_INT_STAT_0)
#define                  pMXVR_INT_STAT_1 ((volatile unsigned long *)MXVR_INT_STAT_1)
#define                    pMXVR_INT_EN_0 ((volatile unsigned long *)MXVR_INT_EN_0)
#define                    pMXVR_INT_EN_1 ((volatile unsigned long *)MXVR_INT_EN_1)
#define                    pMXVR_POSITION ((volatile unsigned short *)MXVR_POSITION)
#define                pMXVR_MAX_POSITION ((volatile unsigned short *)MXVR_MAX_POSITION)
#define                       pMXVR_DELAY ((volatile unsigned short *)MXVR_DELAY)
#define                   pMXVR_MAX_DELAY ((volatile unsigned short *)MXVR_MAX_DELAY)
#define                       pMXVR_LADDR ((volatile unsigned long *)MXVR_LADDR)
#define                       pMXVR_GADDR ((volatile unsigned short *)MXVR_GADDR)
#define                       pMXVR_AADDR ((volatile unsigned long *)MXVR_AADDR)

/* MXVR Allocation Table Registers */

#define                     pMXVR_ALLOC_0 ((volatile unsigned long *)MXVR_ALLOC_0)
#define                     pMXVR_ALLOC_1 ((volatile unsigned long *)MXVR_ALLOC_1)
#define                     pMXVR_ALLOC_2 ((volatile unsigned long *)MXVR_ALLOC_2)
#define                     pMXVR_ALLOC_3 ((volatile unsigned long *)MXVR_ALLOC_3)
#define                     pMXVR_ALLOC_4 ((volatile unsigned long *)MXVR_ALLOC_4)
#define                     pMXVR_ALLOC_5 ((volatile unsigned long *)MXVR_ALLOC_5)
#define                     pMXVR_ALLOC_6 ((volatile unsigned long *)MXVR_ALLOC_6)
#define                     pMXVR_ALLOC_7 ((volatile unsigned long *)MXVR_ALLOC_7)
#define                     pMXVR_ALLOC_8 ((volatile unsigned long *)MXVR_ALLOC_8)
#define                     pMXVR_ALLOC_9 ((volatile unsigned long *)MXVR_ALLOC_9)
#define                    pMXVR_ALLOC_10 ((volatile unsigned long *)MXVR_ALLOC_10)
#define                    pMXVR_ALLOC_11 ((volatile unsigned long *)MXVR_ALLOC_11)
#define                    pMXVR_ALLOC_12 ((volatile unsigned long *)MXVR_ALLOC_12)
#define                    pMXVR_ALLOC_13 ((volatile unsigned long *)MXVR_ALLOC_13)
#define                    pMXVR_ALLOC_14 ((volatile unsigned long *)MXVR_ALLOC_14)

/* MXVR Channel Assign Registers */

#define                pMXVR_SYNC_LCHAN_0 ((volatile unsigned long *)MXVR_SYNC_LCHAN_0)
#define                pMXVR_SYNC_LCHAN_1 ((volatile unsigned long *)MXVR_SYNC_LCHAN_1)
#define                pMXVR_SYNC_LCHAN_2 ((volatile unsigned long *)MXVR_SYNC_LCHAN_2)
#define                pMXVR_SYNC_LCHAN_3 ((volatile unsigned long *)MXVR_SYNC_LCHAN_3)
#define                pMXVR_SYNC_LCHAN_4 ((volatile unsigned long *)MXVR_SYNC_LCHAN_4)
#define                pMXVR_SYNC_LCHAN_5 ((volatile unsigned long *)MXVR_SYNC_LCHAN_5)
#define                pMXVR_SYNC_LCHAN_6 ((volatile unsigned long *)MXVR_SYNC_LCHAN_6)
#define                pMXVR_SYNC_LCHAN_7 ((volatile unsigned long *)MXVR_SYNC_LCHAN_7)

/* MXVR DMA0 Registers */

#define                 pMXVR_DMA0_CONFIG ((volatile unsigned long *)MXVR_DMA0_CONFIG)
#define             pMXVR_DMA0_START_ADDR ((void *volatile *)MXVR_DMA0_START_ADDR)
#define                  pMXVR_DMA0_COUNT ((volatile unsigned short *)MXVR_DMA0_COUNT)
#define              pMXVR_DMA0_CURR_ADDR ((void *volatile *)MXVR_DMA0_CURR_ADDR)
#define             pMXVR_DMA0_CURR_COUNT ((volatile unsigned short *)MXVR_DMA0_CURR_COUNT)

/* MXVR DMA1 Registers */

#define                 pMXVR_DMA1_CONFIG ((volatile unsigned long *)MXVR_DMA1_CONFIG)
#define             pMXVR_DMA1_START_ADDR ((void *volatile *)MXVR_DMA1_START_ADDR)
#define                  pMXVR_DMA1_COUNT ((volatile unsigned short *)MXVR_DMA1_COUNT)
#define              pMXVR_DMA1_CURR_ADDR ((void *volatile *)MXVR_DMA1_CURR_ADDR)
#define             pMXVR_DMA1_CURR_COUNT ((volatile unsigned short *)MXVR_DMA1_CURR_COUNT)

/* MXVR DMA2 Registers */

#define                 pMXVR_DMA2_CONFIG ((volatile unsigned long *)MXVR_DMA2_CONFIG)
#define             pMXVR_DMA2_START_ADDR ((void *volatile *)MXVR_DMA2_START_ADDR)
#define                  pMXVR_DMA2_COUNT ((volatile unsigned short *)MXVR_DMA2_COUNT)
#define              pMXVR_DMA2_CURR_ADDR ((void *volatile *)MXVR_DMA2_CURR_ADDR)
#define             pMXVR_DMA2_CURR_COUNT ((volatile unsigned short *)MXVR_DMA2_CURR_COUNT)

/* MXVR DMA3 Registers */

#define                 pMXVR_DMA3_CONFIG ((volatile unsigned long *)MXVR_DMA3_CONFIG)
#define             pMXVR_DMA3_START_ADDR ((void *volatile *)MXVR_DMA3_START_ADDR)
#define                  pMXVR_DMA3_COUNT ((volatile unsigned short *)MXVR_DMA3_COUNT)
#define              pMXVR_DMA3_CURR_ADDR ((void *volatile *)MXVR_DMA3_CURR_ADDR)
#define             pMXVR_DMA3_CURR_COUNT ((volatile unsigned short *)MXVR_DMA3_CURR_COUNT)

/* MXVR DMA4 Registers */

#define                 pMXVR_DMA4_CONFIG ((volatile unsigned long *)MXVR_DMA4_CONFIG)
#define             pMXVR_DMA4_START_ADDR ((void *volatile *)MXVR_DMA4_START_ADDR)
#define                  pMXVR_DMA4_COUNT ((volatile unsigned short *)MXVR_DMA4_COUNT)
#define              pMXVR_DMA4_CURR_ADDR ((void *volatile *)MXVR_DMA4_CURR_ADDR)
#define             pMXVR_DMA4_CURR_COUNT ((volatile unsigned short *)MXVR_DMA4_CURR_COUNT)

/* MXVR DMA5 Registers */

#define                 pMXVR_DMA5_CONFIG ((volatile unsigned long *)MXVR_DMA5_CONFIG)
#define             pMXVR_DMA5_START_ADDR ((void *volatile *)MXVR_DMA5_START_ADDR)
#define                  pMXVR_DMA5_COUNT ((volatile unsigned short *)MXVR_DMA5_COUNT)
#define              pMXVR_DMA5_CURR_ADDR ((void *volatile *)MXVR_DMA5_CURR_ADDR)
#define             pMXVR_DMA5_CURR_COUNT ((volatile unsigned short *)MXVR_DMA5_CURR_COUNT)

/* MXVR DMA6 Registers */

#define                 pMXVR_DMA6_CONFIG ((volatile unsigned long *)MXVR_DMA6_CONFIG)
#define             pMXVR_DMA6_START_ADDR ((void *volatile *)MXVR_DMA6_START_ADDR)
#define                  pMXVR_DMA6_COUNT ((volatile unsigned short *)MXVR_DMA6_COUNT)
#define              pMXVR_DMA6_CURR_ADDR ((void *volatile *)MXVR_DMA6_CURR_ADDR)
#define             pMXVR_DMA6_CURR_COUNT ((volatile unsigned short *)MXVR_DMA6_CURR_COUNT)

/* MXVR DMA7 Registers */

#define                 pMXVR_DMA7_CONFIG ((volatile unsigned long *)MXVR_DMA7_CONFIG)
#define             pMXVR_DMA7_START_ADDR ((void *volatile *)MXVR_DMA7_START_ADDR)
#define                  pMXVR_DMA7_COUNT ((volatile unsigned short *)MXVR_DMA7_COUNT)
#define              pMXVR_DMA7_CURR_ADDR ((void *volatile *)MXVR_DMA7_CURR_ADDR)
#define             pMXVR_DMA7_CURR_COUNT ((volatile unsigned short *)MXVR_DMA7_CURR_COUNT)

/* MXVR Asynch Packet Registers */

#define                      pMXVR_AP_CTL ((volatile unsigned short *)MXVR_AP_CTL)
#define             pMXVR_APRB_START_ADDR ((void *volatile *)MXVR_APRB_START_ADDR)
#define              pMXVR_APRB_CURR_ADDR ((void *volatile *)MXVR_APRB_CURR_ADDR)
#define             pMXVR_APTB_START_ADDR ((void *volatile *)MXVR_APTB_START_ADDR)
#define              pMXVR_APTB_CURR_ADDR ((void *volatile *)MXVR_APTB_CURR_ADDR)

/* MXVR Control Message Registers */

#define                      pMXVR_CM_CTL ((volatile unsigned long *)MXVR_CM_CTL)
#define             pMXVR_CMRB_START_ADDR ((void *volatile *)MXVR_CMRB_START_ADDR)
#define              pMXVR_CMRB_CURR_ADDR ((void *volatile *)MXVR_CMRB_CURR_ADDR)
#define             pMXVR_CMTB_START_ADDR ((void *volatile *)MXVR_CMTB_START_ADDR)
#define              pMXVR_CMTB_CURR_ADDR ((void *volatile *)MXVR_CMTB_CURR_ADDR)

/* MXVR Remote Read Registers */

#define             pMXVR_RRDB_START_ADDR ((void *volatile *)MXVR_RRDB_START_ADDR)
#define              pMXVR_RRDB_CURR_ADDR ((void *volatile *)MXVR_RRDB_CURR_ADDR)

/* MXVR Pattern Data Registers */

#define                  pMXVR_PAT_DATA_0 ((volatile unsigned long *)MXVR_PAT_DATA_0)
#define                    pMXVR_PAT_EN_0 ((volatile unsigned long *)MXVR_PAT_EN_0)
#define                  pMXVR_PAT_DATA_1 ((volatile unsigned long *)MXVR_PAT_DATA_1)
#define                    pMXVR_PAT_EN_1 ((volatile unsigned long *)MXVR_PAT_EN_1)

/* MXVR Frame Counter Registers */

#define                 pMXVR_FRAME_CNT_0 ((volatile unsigned short *)MXVR_FRAME_CNT_0)
#define                 pMXVR_FRAME_CNT_1 ((volatile unsigned short *)MXVR_FRAME_CNT_1)

/* MXVR Routing Table Registers */

#define                   pMXVR_ROUTING_0 ((volatile unsigned long *)MXVR_ROUTING_0)
#define                   pMXVR_ROUTING_1 ((volatile unsigned long *)MXVR_ROUTING_1)
#define                   pMXVR_ROUTING_2 ((volatile unsigned long *)MXVR_ROUTING_2)
#define                   pMXVR_ROUTING_3 ((volatile unsigned long *)MXVR_ROUTING_3)
#define                   pMXVR_ROUTING_4 ((volatile unsigned long *)MXVR_ROUTING_4)
#define                   pMXVR_ROUTING_5 ((volatile unsigned long *)MXVR_ROUTING_5)
#define                   pMXVR_ROUTING_6 ((volatile unsigned long *)MXVR_ROUTING_6)
#define                   pMXVR_ROUTING_7 ((volatile unsigned long *)MXVR_ROUTING_7)
#define                   pMXVR_ROUTING_8 ((volatile unsigned long *)MXVR_ROUTING_8)
#define                   pMXVR_ROUTING_9 ((volatile unsigned long *)MXVR_ROUTING_9)
#define                  pMXVR_ROUTING_10 ((volatile unsigned long *)MXVR_ROUTING_10)
#define                  pMXVR_ROUTING_11 ((volatile unsigned long *)MXVR_ROUTING_11)
#define                  pMXVR_ROUTING_12 ((volatile unsigned long *)MXVR_ROUTING_12)
#define                  pMXVR_ROUTING_13 ((volatile unsigned long *)MXVR_ROUTING_13)
#define                  pMXVR_ROUTING_14 ((volatile unsigned long *)MXVR_ROUTING_14)

/* MXVR Counter-Clock-Control Registers */

#define                   pMXVR_BLOCK_CNT ((volatile unsigned short *)MXVR_BLOCK_CNT)
#define                     pMXVR_CLK_CTL ((volatile unsigned long *)MXVR_CLK_CTL)
#define                  pMXVR_CDRPLL_CTL ((volatile unsigned long *)MXVR_CDRPLL_CTL)
#define                   pMXVR_FMPLL_CTL ((volatile unsigned long *)MXVR_FMPLL_CTL)
#define                     pMXVR_PIN_CTL ((volatile unsigned short *)MXVR_PIN_CTL)
#define                    pMXVR_SCLK_CNT ((volatile unsigned short *)MXVR_SCLK_CNT)

/* CAN Controller 1 Config 1 Registers */

#define                         pCAN1_MC1 ((volatile unsigned short *)CAN1_MC1)
#define                         pCAN1_MD1 ((volatile unsigned short *)CAN1_MD1)
#define                        pCAN1_TRS1 ((volatile unsigned short *)CAN1_TRS1)
#define                        pCAN1_TRR1 ((volatile unsigned short *)CAN1_TRR1)
#define                         pCAN1_TA1 ((volatile unsigned short *)CAN1_TA1)
#define                         pCAN1_AA1 ((volatile unsigned short *)CAN1_AA1)
#define                        pCAN1_RMP1 ((volatile unsigned short *)CAN1_RMP1)
#define                        pCAN1_RML1 ((volatile unsigned short *)CAN1_RML1)
#define                      pCAN1_MBTIF1 ((volatile unsigned short *)CAN1_MBTIF1)
#define                      pCAN1_MBRIF1 ((volatile unsigned short *)CAN1_MBRIF1)
#define                       pCAN1_MBIM1 ((volatile unsigned short *)CAN1_MBIM1)
#define                        pCAN1_RFH1 ((volatile unsigned short *)CAN1_RFH1)
#define                       pCAN1_OPSS1 ((volatile unsigned short *)CAN1_OPSS1)

/* CAN Controller 1 Config 2 Registers */

#define                         pCAN1_MC2 ((volatile unsigned short *)CAN1_MC2)
#define                         pCAN1_MD2 ((volatile unsigned short *)CAN1_MD2)
#define                        pCAN1_TRS2 ((volatile unsigned short *)CAN1_TRS2)
#define                        pCAN1_TRR2 ((volatile unsigned short *)CAN1_TRR2)
#define                         pCAN1_TA2 ((volatile unsigned short *)CAN1_TA2)
#define                         pCAN1_AA2 ((volatile unsigned short *)CAN1_AA2)
#define                        pCAN1_RMP2 ((volatile unsigned short *)CAN1_RMP2)
#define                        pCAN1_RML2 ((volatile unsigned short *)CAN1_RML2)
#define                      pCAN1_MBTIF2 ((volatile unsigned short *)CAN1_MBTIF2)
#define                      pCAN1_MBRIF2 ((volatile unsigned short *)CAN1_MBRIF2)
#define                       pCAN1_MBIM2 ((volatile unsigned short *)CAN1_MBIM2)
#define                        pCAN1_RFH2 ((volatile unsigned short *)CAN1_RFH2)
#define                       pCAN1_OPSS2 ((volatile unsigned short *)CAN1_OPSS2)

/* CAN Controller 1 Clock/Interrupt/Counter Registers */

#define                       pCAN1_CLOCK ((volatile unsigned short *)CAN1_CLOCK)
#define                      pCAN1_TIMING ((volatile unsigned short *)CAN1_TIMING)
#define                       pCAN1_DEBUG ((volatile unsigned short *)CAN1_DEBUG)
#define                      pCAN1_STATUS ((volatile unsigned short *)CAN1_STATUS)
#define                         pCAN1_CEC ((volatile unsigned short *)CAN1_CEC)
#define                         pCAN1_GIS ((volatile unsigned short *)CAN1_GIS)
#define                         pCAN1_GIM ((volatile unsigned short *)CAN1_GIM)
#define                         pCAN1_GIF ((volatile unsigned short *)CAN1_GIF)
#define                     pCAN1_CONTROL ((volatile unsigned short *)CAN1_CONTROL)
#define                        pCAN1_INTR ((volatile unsigned short *)CAN1_INTR)
#define                        pCAN1_MBTD ((volatile unsigned short *)CAN1_MBTD)
#define                         pCAN1_EWR ((volatile unsigned short *)CAN1_EWR)
#define                         pCAN1_ESR ((volatile unsigned short *)CAN1_ESR)
#define                       pCAN1_UCCNT ((volatile unsigned short *)CAN1_UCCNT)
#define                        pCAN1_UCRC ((volatile unsigned short *)CAN1_UCRC)
#define                       pCAN1_UCCNF ((volatile unsigned short *)CAN1_UCCNF)

/* CAN Controller 1 Mailbox Acceptance Registers */

#define                       pCAN1_AM00L ((volatile unsigned short *)CAN1_AM00L)
#define                       pCAN1_AM00H ((volatile unsigned short *)CAN1_AM00H)
#define                       pCAN1_AM01L ((volatile unsigned short *)CAN1_AM01L)
#define                       pCAN1_AM01H ((volatile unsigned short *)CAN1_AM01H)
#define                       pCAN1_AM02L ((volatile unsigned short *)CAN1_AM02L)
#define                       pCAN1_AM02H ((volatile unsigned short *)CAN1_AM02H)
#define                       pCAN1_AM03L ((volatile unsigned short *)CAN1_AM03L)
#define                       pCAN1_AM03H ((volatile unsigned short *)CAN1_AM03H)
#define                       pCAN1_AM04L ((volatile unsigned short *)CAN1_AM04L)
#define                       pCAN1_AM04H ((volatile unsigned short *)CAN1_AM04H)
#define                       pCAN1_AM05L ((volatile unsigned short *)CAN1_AM05L)
#define                       pCAN1_AM05H ((volatile unsigned short *)CAN1_AM05H)
#define                       pCAN1_AM06L ((volatile unsigned short *)CAN1_AM06L)
#define                       pCAN1_AM06H ((volatile unsigned short *)CAN1_AM06H)
#define                       pCAN1_AM07L ((volatile unsigned short *)CAN1_AM07L)
#define                       pCAN1_AM07H ((volatile unsigned short *)CAN1_AM07H)
#define                       pCAN1_AM08L ((volatile unsigned short *)CAN1_AM08L)
#define                       pCAN1_AM08H ((volatile unsigned short *)CAN1_AM08H)
#define                       pCAN1_AM09L ((volatile unsigned short *)CAN1_AM09L)
#define                       pCAN1_AM09H ((volatile unsigned short *)CAN1_AM09H)
#define                       pCAN1_AM10L ((volatile unsigned short *)CAN1_AM10L)
#define                       pCAN1_AM10H ((volatile unsigned short *)CAN1_AM10H)
#define                       pCAN1_AM11L ((volatile unsigned short *)CAN1_AM11L)
#define                       pCAN1_AM11H ((volatile unsigned short *)CAN1_AM11H)
#define                       pCAN1_AM12L ((volatile unsigned short *)CAN1_AM12L)
#define                       pCAN1_AM12H ((volatile unsigned short *)CAN1_AM12H)
#define                       pCAN1_AM13L ((volatile unsigned short *)CAN1_AM13L)
#define                       pCAN1_AM13H ((volatile unsigned short *)CAN1_AM13H)
#define                       pCAN1_AM14L ((volatile unsigned short *)CAN1_AM14L)
#define                       pCAN1_AM14H ((volatile unsigned short *)CAN1_AM14H)
#define                       pCAN1_AM15L ((volatile unsigned short *)CAN1_AM15L)
#define                       pCAN1_AM15H ((volatile unsigned short *)CAN1_AM15H)

/* CAN Controller 1 Mailbox Acceptance Registers */

#define                       pCAN1_AM16L ((volatile unsigned short *)CAN1_AM16L)
#define                       pCAN1_AM16H ((volatile unsigned short *)CAN1_AM16H)
#define                       pCAN1_AM17L ((volatile unsigned short *)CAN1_AM17L)
#define                       pCAN1_AM17H ((volatile unsigned short *)CAN1_AM17H)
#define                       pCAN1_AM18L ((volatile unsigned short *)CAN1_AM18L)
#define                       pCAN1_AM18H ((volatile unsigned short *)CAN1_AM18H)
#define                       pCAN1_AM19L ((volatile unsigned short *)CAN1_AM19L)
#define                       pCAN1_AM19H ((volatile unsigned short *)CAN1_AM19H)
#define                       pCAN1_AM20L ((volatile unsigned short *)CAN1_AM20L)
#define                       pCAN1_AM20H ((volatile unsigned short *)CAN1_AM20H)
#define                       pCAN1_AM21L ((volatile unsigned short *)CAN1_AM21L)
#define                       pCAN1_AM21H ((volatile unsigned short *)CAN1_AM21H)
#define                       pCAN1_AM22L ((volatile unsigned short *)CAN1_AM22L)
#define                       pCAN1_AM22H ((volatile unsigned short *)CAN1_AM22H)
#define                       pCAN1_AM23L ((volatile unsigned short *)CAN1_AM23L)
#define                       pCAN1_AM23H ((volatile unsigned short *)CAN1_AM23H)
#define                       pCAN1_AM24L ((volatile unsigned short *)CAN1_AM24L)
#define                       pCAN1_AM24H ((volatile unsigned short *)CAN1_AM24H)
#define                       pCAN1_AM25L ((volatile unsigned short *)CAN1_AM25L)
#define                       pCAN1_AM25H ((volatile unsigned short *)CAN1_AM25H)
#define                       pCAN1_AM26L ((volatile unsigned short *)CAN1_AM26L)
#define                       pCAN1_AM26H ((volatile unsigned short *)CAN1_AM26H)
#define                       pCAN1_AM27L ((volatile unsigned short *)CAN1_AM27L)
#define                       pCAN1_AM27H ((volatile unsigned short *)CAN1_AM27H)
#define                       pCAN1_AM28L ((volatile unsigned short *)CAN1_AM28L)
#define                       pCAN1_AM28H ((volatile unsigned short *)CAN1_AM28H)
#define                       pCAN1_AM29L ((volatile unsigned short *)CAN1_AM29L)
#define                       pCAN1_AM29H ((volatile unsigned short *)CAN1_AM29H)
#define                       pCAN1_AM30L ((volatile unsigned short *)CAN1_AM30L)
#define                       pCAN1_AM30H ((volatile unsigned short *)CAN1_AM30H)
#define                       pCAN1_AM31L ((volatile unsigned short *)CAN1_AM31L)
#define                       pCAN1_AM31H ((volatile unsigned short *)CAN1_AM31H)

/* CAN Controller 1 Mailbox Data Registers */

#define                  pCAN1_MB00_DATA0 ((volatile unsigned short *)CAN1_MB00_DATA0)
#define                  pCAN1_MB00_DATA1 ((volatile unsigned short *)CAN1_MB00_DATA1)
#define                  pCAN1_MB00_DATA2 ((volatile unsigned short *)CAN1_MB00_DATA2)
#define                  pCAN1_MB00_DATA3 ((volatile unsigned short *)CAN1_MB00_DATA3)
#define                 pCAN1_MB00_LENGTH ((volatile unsigned short *)CAN1_MB00_LENGTH)
#define              pCAN1_MB00_TIMESTAMP ((volatile unsigned short *)CAN1_MB00_TIMESTAMP)
#define                    pCAN1_MB00_ID0 ((volatile unsigned short *)CAN1_MB00_ID0)
#define                    pCAN1_MB00_ID1 ((volatile unsigned short *)CAN1_MB00_ID1)
#define                  pCAN1_MB01_DATA0 ((volatile unsigned short *)CAN1_MB01_DATA0)
#define                  pCAN1_MB01_DATA1 ((volatile unsigned short *)CAN1_MB01_DATA1)
#define                  pCAN1_MB01_DATA2 ((volatile unsigned short *)CAN1_MB01_DATA2)
#define                  pCAN1_MB01_DATA3 ((volatile unsigned short *)CAN1_MB01_DATA3)
#define                 pCAN1_MB01_LENGTH ((volatile unsigned short *)CAN1_MB01_LENGTH)
#define              pCAN1_MB01_TIMESTAMP ((volatile unsigned short *)CAN1_MB01_TIMESTAMP)
#define                    pCAN1_MB01_ID0 ((volatile unsigned short *)CAN1_MB01_ID0)
#define                    pCAN1_MB01_ID1 ((volatile unsigned short *)CAN1_MB01_ID1)
#define                  pCAN1_MB02_DATA0 ((volatile unsigned short *)CAN1_MB02_DATA0)
#define                  pCAN1_MB02_DATA1 ((volatile unsigned short *)CAN1_MB02_DATA1)
#define                  pCAN1_MB02_DATA2 ((volatile unsigned short *)CAN1_MB02_DATA2)
#define                  pCAN1_MB02_DATA3 ((volatile unsigned short *)CAN1_MB02_DATA3)
#define                 pCAN1_MB02_LENGTH ((volatile unsigned short *)CAN1_MB02_LENGTH)
#define              pCAN1_MB02_TIMESTAMP ((volatile unsigned short *)CAN1_MB02_TIMESTAMP)
#define                    pCAN1_MB02_ID0 ((volatile unsigned short *)CAN1_MB02_ID0)
#define                    pCAN1_MB02_ID1 ((volatile unsigned short *)CAN1_MB02_ID1)
#define                  pCAN1_MB03_DATA0 ((volatile unsigned short *)CAN1_MB03_DATA0)
#define                  pCAN1_MB03_DATA1 ((volatile unsigned short *)CAN1_MB03_DATA1)
#define                  pCAN1_MB03_DATA2 ((volatile unsigned short *)CAN1_MB03_DATA2)
#define                  pCAN1_MB03_DATA3 ((volatile unsigned short *)CAN1_MB03_DATA3)
#define                 pCAN1_MB03_LENGTH ((volatile unsigned short *)CAN1_MB03_LENGTH)
#define              pCAN1_MB03_TIMESTAMP ((volatile unsigned short *)CAN1_MB03_TIMESTAMP)
#define                    pCAN1_MB03_ID0 ((volatile unsigned short *)CAN1_MB03_ID0)
#define                    pCAN1_MB03_ID1 ((volatile unsigned short *)CAN1_MB03_ID1)
#define                  pCAN1_MB04_DATA0 ((volatile unsigned short *)CAN1_MB04_DATA0)
#define                  pCAN1_MB04_DATA1 ((volatile unsigned short *)CAN1_MB04_DATA1)
#define                  pCAN1_MB04_DATA2 ((volatile unsigned short *)CAN1_MB04_DATA2)
#define                  pCAN1_MB04_DATA3 ((volatile unsigned short *)CAN1_MB04_DATA3)
#define                 pCAN1_MB04_LENGTH ((volatile unsigned short *)CAN1_MB04_LENGTH)
#define              pCAN1_MB04_TIMESTAMP ((volatile unsigned short *)CAN1_MB04_TIMESTAMP)
#define                    pCAN1_MB04_ID0 ((volatile unsigned short *)CAN1_MB04_ID0)
#define                    pCAN1_MB04_ID1 ((volatile unsigned short *)CAN1_MB04_ID1)
#define                  pCAN1_MB05_DATA0 ((volatile unsigned short *)CAN1_MB05_DATA0)
#define                  pCAN1_MB05_DATA1 ((volatile unsigned short *)CAN1_MB05_DATA1)
#define                  pCAN1_MB05_DATA2 ((volatile unsigned short *)CAN1_MB05_DATA2)
#define                  pCAN1_MB05_DATA3 ((volatile unsigned short *)CAN1_MB05_DATA3)
#define                 pCAN1_MB05_LENGTH ((volatile unsigned short *)CAN1_MB05_LENGTH)
#define              pCAN1_MB05_TIMESTAMP ((volatile unsigned short *)CAN1_MB05_TIMESTAMP)
#define                    pCAN1_MB05_ID0 ((volatile unsigned short *)CAN1_MB05_ID0)
#define                    pCAN1_MB05_ID1 ((volatile unsigned short *)CAN1_MB05_ID1)
#define                  pCAN1_MB06_DATA0 ((volatile unsigned short *)CAN1_MB06_DATA0)
#define                  pCAN1_MB06_DATA1 ((volatile unsigned short *)CAN1_MB06_DATA1)
#define                  pCAN1_MB06_DATA2 ((volatile unsigned short *)CAN1_MB06_DATA2)
#define                  pCAN1_MB06_DATA3 ((volatile unsigned short *)CAN1_MB06_DATA3)
#define                 pCAN1_MB06_LENGTH ((volatile unsigned short *)CAN1_MB06_LENGTH)
#define              pCAN1_MB06_TIMESTAMP ((volatile unsigned short *)CAN1_MB06_TIMESTAMP)
#define                    pCAN1_MB06_ID0 ((volatile unsigned short *)CAN1_MB06_ID0)
#define                    pCAN1_MB06_ID1 ((volatile unsigned short *)CAN1_MB06_ID1)
#define                  pCAN1_MB07_DATA0 ((volatile unsigned short *)CAN1_MB07_DATA0)
#define                  pCAN1_MB07_DATA1 ((volatile unsigned short *)CAN1_MB07_DATA1)
#define                  pCAN1_MB07_DATA2 ((volatile unsigned short *)CAN1_MB07_DATA2)
#define                  pCAN1_MB07_DATA3 ((volatile unsigned short *)CAN1_MB07_DATA3)
#define                 pCAN1_MB07_LENGTH ((volatile unsigned short *)CAN1_MB07_LENGTH)
#define              pCAN1_MB07_TIMESTAMP ((volatile unsigned short *)CAN1_MB07_TIMESTAMP)
#define                    pCAN1_MB07_ID0 ((volatile unsigned short *)CAN1_MB07_ID0)
#define                    pCAN1_MB07_ID1 ((volatile unsigned short *)CAN1_MB07_ID1)
#define                  pCAN1_MB08_DATA0 ((volatile unsigned short *)CAN1_MB08_DATA0)
#define                  pCAN1_MB08_DATA1 ((volatile unsigned short *)CAN1_MB08_DATA1)
#define                  pCAN1_MB08_DATA2 ((volatile unsigned short *)CAN1_MB08_DATA2)
#define                  pCAN1_MB08_DATA3 ((volatile unsigned short *)CAN1_MB08_DATA3)
#define                 pCAN1_MB08_LENGTH ((volatile unsigned short *)CAN1_MB08_LENGTH)
#define              pCAN1_MB08_TIMESTAMP ((volatile unsigned short *)CAN1_MB08_TIMESTAMP)
#define                    pCAN1_MB08_ID0 ((volatile unsigned short *)CAN1_MB08_ID0)
#define                    pCAN1_MB08_ID1 ((volatile unsigned short *)CAN1_MB08_ID1)
#define                  pCAN1_MB09_DATA0 ((volatile unsigned short *)CAN1_MB09_DATA0)
#define                  pCAN1_MB09_DATA1 ((volatile unsigned short *)CAN1_MB09_DATA1)
#define                  pCAN1_MB09_DATA2 ((volatile unsigned short *)CAN1_MB09_DATA2)
#define                  pCAN1_MB09_DATA3 ((volatile unsigned short *)CAN1_MB09_DATA3)
#define                 pCAN1_MB09_LENGTH ((volatile unsigned short *)CAN1_MB09_LENGTH)
#define              pCAN1_MB09_TIMESTAMP ((volatile unsigned short *)CAN1_MB09_TIMESTAMP)
#define                    pCAN1_MB09_ID0 ((volatile unsigned short *)CAN1_MB09_ID0)
#define                    pCAN1_MB09_ID1 ((volatile unsigned short *)CAN1_MB09_ID1)
#define                  pCAN1_MB10_DATA0 ((volatile unsigned short *)CAN1_MB10_DATA0)
#define                  pCAN1_MB10_DATA1 ((volatile unsigned short *)CAN1_MB10_DATA1)
#define                  pCAN1_MB10_DATA2 ((volatile unsigned short *)CAN1_MB10_DATA2)
#define                  pCAN1_MB10_DATA3 ((volatile unsigned short *)CAN1_MB10_DATA3)
#define                 pCAN1_MB10_LENGTH ((volatile unsigned short *)CAN1_MB10_LENGTH)
#define              pCAN1_MB10_TIMESTAMP ((volatile unsigned short *)CAN1_MB10_TIMESTAMP)
#define                    pCAN1_MB10_ID0 ((volatile unsigned short *)CAN1_MB10_ID0)
#define                    pCAN1_MB10_ID1 ((volatile unsigned short *)CAN1_MB10_ID1)
#define                  pCAN1_MB11_DATA0 ((volatile unsigned short *)CAN1_MB11_DATA0)
#define                  pCAN1_MB11_DATA1 ((volatile unsigned short *)CAN1_MB11_DATA1)
#define                  pCAN1_MB11_DATA2 ((volatile unsigned short *)CAN1_MB11_DATA2)
#define                  pCAN1_MB11_DATA3 ((volatile unsigned short *)CAN1_MB11_DATA3)
#define                 pCAN1_MB11_LENGTH ((volatile unsigned short *)CAN1_MB11_LENGTH)
#define              pCAN1_MB11_TIMESTAMP ((volatile unsigned short *)CAN1_MB11_TIMESTAMP)
#define                    pCAN1_MB11_ID0 ((volatile unsigned short *)CAN1_MB11_ID0)
#define                    pCAN1_MB11_ID1 ((volatile unsigned short *)CAN1_MB11_ID1)
#define                  pCAN1_MB12_DATA0 ((volatile unsigned short *)CAN1_MB12_DATA0)
#define                  pCAN1_MB12_DATA1 ((volatile unsigned short *)CAN1_MB12_DATA1)
#define                  pCAN1_MB12_DATA2 ((volatile unsigned short *)CAN1_MB12_DATA2)
#define                  pCAN1_MB12_DATA3 ((volatile unsigned short *)CAN1_MB12_DATA3)
#define                 pCAN1_MB12_LENGTH ((volatile unsigned short *)CAN1_MB12_LENGTH)
#define              pCAN1_MB12_TIMESTAMP ((volatile unsigned short *)CAN1_MB12_TIMESTAMP)
#define                    pCAN1_MB12_ID0 ((volatile unsigned short *)CAN1_MB12_ID0)
#define                    pCAN1_MB12_ID1 ((volatile unsigned short *)CAN1_MB12_ID1)
#define                  pCAN1_MB13_DATA0 ((volatile unsigned short *)CAN1_MB13_DATA0)
#define                  pCAN1_MB13_DATA1 ((volatile unsigned short *)CAN1_MB13_DATA1)
#define                  pCAN1_MB13_DATA2 ((volatile unsigned short *)CAN1_MB13_DATA2)
#define                  pCAN1_MB13_DATA3 ((volatile unsigned short *)CAN1_MB13_DATA3)
#define                 pCAN1_MB13_LENGTH ((volatile unsigned short *)CAN1_MB13_LENGTH)
#define              pCAN1_MB13_TIMESTAMP ((volatile unsigned short *)CAN1_MB13_TIMESTAMP)
#define                    pCAN1_MB13_ID0 ((volatile unsigned short *)CAN1_MB13_ID0)
#define                    pCAN1_MB13_ID1 ((volatile unsigned short *)CAN1_MB13_ID1)
#define                  pCAN1_MB14_DATA0 ((volatile unsigned short *)CAN1_MB14_DATA0)
#define                  pCAN1_MB14_DATA1 ((volatile unsigned short *)CAN1_MB14_DATA1)
#define                  pCAN1_MB14_DATA2 ((volatile unsigned short *)CAN1_MB14_DATA2)
#define                  pCAN1_MB14_DATA3 ((volatile unsigned short *)CAN1_MB14_DATA3)
#define                 pCAN1_MB14_LENGTH ((volatile unsigned short *)CAN1_MB14_LENGTH)
#define              pCAN1_MB14_TIMESTAMP ((volatile unsigned short *)CAN1_MB14_TIMESTAMP)
#define                    pCAN1_MB14_ID0 ((volatile unsigned short *)CAN1_MB14_ID0)
#define                    pCAN1_MB14_ID1 ((volatile unsigned short *)CAN1_MB14_ID1)
#define                  pCAN1_MB15_DATA0 ((volatile unsigned short *)CAN1_MB15_DATA0)
#define                  pCAN1_MB15_DATA1 ((volatile unsigned short *)CAN1_MB15_DATA1)
#define                  pCAN1_MB15_DATA2 ((volatile unsigned short *)CAN1_MB15_DATA2)
#define                  pCAN1_MB15_DATA3 ((volatile unsigned short *)CAN1_MB15_DATA3)
#define                 pCAN1_MB15_LENGTH ((volatile unsigned short *)CAN1_MB15_LENGTH)
#define              pCAN1_MB15_TIMESTAMP ((volatile unsigned short *)CAN1_MB15_TIMESTAMP)
#define                    pCAN1_MB15_ID0 ((volatile unsigned short *)CAN1_MB15_ID0)
#define                    pCAN1_MB15_ID1 ((volatile unsigned short *)CAN1_MB15_ID1)

/* CAN Controller 1 Mailbox Data Registers */

#define                  pCAN1_MB16_DATA0 ((volatile unsigned short *)CAN1_MB16_DATA0)
#define                  pCAN1_MB16_DATA1 ((volatile unsigned short *)CAN1_MB16_DATA1)
#define                  pCAN1_MB16_DATA2 ((volatile unsigned short *)CAN1_MB16_DATA2)
#define                  pCAN1_MB16_DATA3 ((volatile unsigned short *)CAN1_MB16_DATA3)
#define                 pCAN1_MB16_LENGTH ((volatile unsigned short *)CAN1_MB16_LENGTH)
#define              pCAN1_MB16_TIMESTAMP ((volatile unsigned short *)CAN1_MB16_TIMESTAMP)
#define                    pCAN1_MB16_ID0 ((volatile unsigned short *)CAN1_MB16_ID0)
#define                    pCAN1_MB16_ID1 ((volatile unsigned short *)CAN1_MB16_ID1)
#define                  pCAN1_MB17_DATA0 ((volatile unsigned short *)CAN1_MB17_DATA0)
#define                  pCAN1_MB17_DATA1 ((volatile unsigned short *)CAN1_MB17_DATA1)
#define                  pCAN1_MB17_DATA2 ((volatile unsigned short *)CAN1_MB17_DATA2)
#define                  pCAN1_MB17_DATA3 ((volatile unsigned short *)CAN1_MB17_DATA3)
#define                 pCAN1_MB17_LENGTH ((volatile unsigned short *)CAN1_MB17_LENGTH)
#define              pCAN1_MB17_TIMESTAMP ((volatile unsigned short *)CAN1_MB17_TIMESTAMP)
#define                    pCAN1_MB17_ID0 ((volatile unsigned short *)CAN1_MB17_ID0)
#define                    pCAN1_MB17_ID1 ((volatile unsigned short *)CAN1_MB17_ID1)
#define                  pCAN1_MB18_DATA0 ((volatile unsigned short *)CAN1_MB18_DATA0)
#define                  pCAN1_MB18_DATA1 ((volatile unsigned short *)CAN1_MB18_DATA1)
#define                  pCAN1_MB18_DATA2 ((volatile unsigned short *)CAN1_MB18_DATA2)
#define                  pCAN1_MB18_DATA3 ((volatile unsigned short *)CAN1_MB18_DATA3)
#define                 pCAN1_MB18_LENGTH ((volatile unsigned short *)CAN1_MB18_LENGTH)
#define              pCAN1_MB18_TIMESTAMP ((volatile unsigned short *)CAN1_MB18_TIMESTAMP)
#define                    pCAN1_MB18_ID0 ((volatile unsigned short *)CAN1_MB18_ID0)
#define                    pCAN1_MB18_ID1 ((volatile unsigned short *)CAN1_MB18_ID1)
#define                  pCAN1_MB19_DATA0 ((volatile unsigned short *)CAN1_MB19_DATA0)
#define                  pCAN1_MB19_DATA1 ((volatile unsigned short *)CAN1_MB19_DATA1)
#define                  pCAN1_MB19_DATA2 ((volatile unsigned short *)CAN1_MB19_DATA2)
#define                  pCAN1_MB19_DATA3 ((volatile unsigned short *)CAN1_MB19_DATA3)
#define                 pCAN1_MB19_LENGTH ((volatile unsigned short *)CAN1_MB19_LENGTH)
#define              pCAN1_MB19_TIMESTAMP ((volatile unsigned short *)CAN1_MB19_TIMESTAMP)
#define                    pCAN1_MB19_ID0 ((volatile unsigned short *)CAN1_MB19_ID0)
#define                    pCAN1_MB19_ID1 ((volatile unsigned short *)CAN1_MB19_ID1)
#define                  pCAN1_MB20_DATA0 ((volatile unsigned short *)CAN1_MB20_DATA0)
#define                  pCAN1_MB20_DATA1 ((volatile unsigned short *)CAN1_MB20_DATA1)
#define                  pCAN1_MB20_DATA2 ((volatile unsigned short *)CAN1_MB20_DATA2)
#define                  pCAN1_MB20_DATA3 ((volatile unsigned short *)CAN1_MB20_DATA3)
#define                 pCAN1_MB20_LENGTH ((volatile unsigned short *)CAN1_MB20_LENGTH)
#define              pCAN1_MB20_TIMESTAMP ((volatile unsigned short *)CAN1_MB20_TIMESTAMP)
#define                    pCAN1_MB20_ID0 ((volatile unsigned short *)CAN1_MB20_ID0)
#define                    pCAN1_MB20_ID1 ((volatile unsigned short *)CAN1_MB20_ID1)
#define                  pCAN1_MB21_DATA0 ((volatile unsigned short *)CAN1_MB21_DATA0)
#define                  pCAN1_MB21_DATA1 ((volatile unsigned short *)CAN1_MB21_DATA1)
#define                  pCAN1_MB21_DATA2 ((volatile unsigned short *)CAN1_MB21_DATA2)
#define                  pCAN1_MB21_DATA3 ((volatile unsigned short *)CAN1_MB21_DATA3)
#define                 pCAN1_MB21_LENGTH ((volatile unsigned short *)CAN1_MB21_LENGTH)
#define              pCAN1_MB21_TIMESTAMP ((volatile unsigned short *)CAN1_MB21_TIMESTAMP)
#define                    pCAN1_MB21_ID0 ((volatile unsigned short *)CAN1_MB21_ID0)
#define                    pCAN1_MB21_ID1 ((volatile unsigned short *)CAN1_MB21_ID1)
#define                  pCAN1_MB22_DATA0 ((volatile unsigned short *)CAN1_MB22_DATA0)
#define                  pCAN1_MB22_DATA1 ((volatile unsigned short *)CAN1_MB22_DATA1)
#define                  pCAN1_MB22_DATA2 ((volatile unsigned short *)CAN1_MB22_DATA2)
#define                  pCAN1_MB22_DATA3 ((volatile unsigned short *)CAN1_MB22_DATA3)
#define                 pCAN1_MB22_LENGTH ((volatile unsigned short *)CAN1_MB22_LENGTH)
#define              pCAN1_MB22_TIMESTAMP ((volatile unsigned short *)CAN1_MB22_TIMESTAMP)
#define                    pCAN1_MB22_ID0 ((volatile unsigned short *)CAN1_MB22_ID0)
#define                    pCAN1_MB22_ID1 ((volatile unsigned short *)CAN1_MB22_ID1)
#define                  pCAN1_MB23_DATA0 ((volatile unsigned short *)CAN1_MB23_DATA0)
#define                  pCAN1_MB23_DATA1 ((volatile unsigned short *)CAN1_MB23_DATA1)
#define                  pCAN1_MB23_DATA2 ((volatile unsigned short *)CAN1_MB23_DATA2)
#define                  pCAN1_MB23_DATA3 ((volatile unsigned short *)CAN1_MB23_DATA3)
#define                 pCAN1_MB23_LENGTH ((volatile unsigned short *)CAN1_MB23_LENGTH)
#define              pCAN1_MB23_TIMESTAMP ((volatile unsigned short *)CAN1_MB23_TIMESTAMP)
#define                    pCAN1_MB23_ID0 ((volatile unsigned short *)CAN1_MB23_ID0)
#define                    pCAN1_MB23_ID1 ((volatile unsigned short *)CAN1_MB23_ID1)
#define                  pCAN1_MB24_DATA0 ((volatile unsigned short *)CAN1_MB24_DATA0)
#define                  pCAN1_MB24_DATA1 ((volatile unsigned short *)CAN1_MB24_DATA1)
#define                  pCAN1_MB24_DATA2 ((volatile unsigned short *)CAN1_MB24_DATA2)
#define                  pCAN1_MB24_DATA3 ((volatile unsigned short *)CAN1_MB24_DATA3)
#define                 pCAN1_MB24_LENGTH ((volatile unsigned short *)CAN1_MB24_LENGTH)
#define              pCAN1_MB24_TIMESTAMP ((volatile unsigned short *)CAN1_MB24_TIMESTAMP)
#define                    pCAN1_MB24_ID0 ((volatile unsigned short *)CAN1_MB24_ID0)
#define                    pCAN1_MB24_ID1 ((volatile unsigned short *)CAN1_MB24_ID1)
#define                  pCAN1_MB25_DATA0 ((volatile unsigned short *)CAN1_MB25_DATA0)
#define                  pCAN1_MB25_DATA1 ((volatile unsigned short *)CAN1_MB25_DATA1)
#define                  pCAN1_MB25_DATA2 ((volatile unsigned short *)CAN1_MB25_DATA2)
#define                  pCAN1_MB25_DATA3 ((volatile unsigned short *)CAN1_MB25_DATA3)
#define                 pCAN1_MB25_LENGTH ((volatile unsigned short *)CAN1_MB25_LENGTH)
#define              pCAN1_MB25_TIMESTAMP ((volatile unsigned short *)CAN1_MB25_TIMESTAMP)
#define                    pCAN1_MB25_ID0 ((volatile unsigned short *)CAN1_MB25_ID0)
#define                    pCAN1_MB25_ID1 ((volatile unsigned short *)CAN1_MB25_ID1)
#define                  pCAN1_MB26_DATA0 ((volatile unsigned short *)CAN1_MB26_DATA0)
#define                  pCAN1_MB26_DATA1 ((volatile unsigned short *)CAN1_MB26_DATA1)
#define                  pCAN1_MB26_DATA2 ((volatile unsigned short *)CAN1_MB26_DATA2)
#define                  pCAN1_MB26_DATA3 ((volatile unsigned short *)CAN1_MB26_DATA3)
#define                 pCAN1_MB26_LENGTH ((volatile unsigned short *)CAN1_MB26_LENGTH)
#define              pCAN1_MB26_TIMESTAMP ((volatile unsigned short *)CAN1_MB26_TIMESTAMP)
#define                    pCAN1_MB26_ID0 ((volatile unsigned short *)CAN1_MB26_ID0)
#define                    pCAN1_MB26_ID1 ((volatile unsigned short *)CAN1_MB26_ID1)
#define                  pCAN1_MB27_DATA0 ((volatile unsigned short *)CAN1_MB27_DATA0)
#define                  pCAN1_MB27_DATA1 ((volatile unsigned short *)CAN1_MB27_DATA1)
#define                  pCAN1_MB27_DATA2 ((volatile unsigned short *)CAN1_MB27_DATA2)
#define                  pCAN1_MB27_DATA3 ((volatile unsigned short *)CAN1_MB27_DATA3)
#define                 pCAN1_MB27_LENGTH ((volatile unsigned short *)CAN1_MB27_LENGTH)
#define              pCAN1_MB27_TIMESTAMP ((volatile unsigned short *)CAN1_MB27_TIMESTAMP)
#define                    pCAN1_MB27_ID0 ((volatile unsigned short *)CAN1_MB27_ID0)
#define                    pCAN1_MB27_ID1 ((volatile unsigned short *)CAN1_MB27_ID1)
#define                  pCAN1_MB28_DATA0 ((volatile unsigned short *)CAN1_MB28_DATA0)
#define                  pCAN1_MB28_DATA1 ((volatile unsigned short *)CAN1_MB28_DATA1)
#define                  pCAN1_MB28_DATA2 ((volatile unsigned short *)CAN1_MB28_DATA2)
#define                  pCAN1_MB28_DATA3 ((volatile unsigned short *)CAN1_MB28_DATA3)
#define                 pCAN1_MB28_LENGTH ((volatile unsigned short *)CAN1_MB28_LENGTH)
#define              pCAN1_MB28_TIMESTAMP ((volatile unsigned short *)CAN1_MB28_TIMESTAMP)
#define                    pCAN1_MB28_ID0 ((volatile unsigned short *)CAN1_MB28_ID0)
#define                    pCAN1_MB28_ID1 ((volatile unsigned short *)CAN1_MB28_ID1)
#define                  pCAN1_MB29_DATA0 ((volatile unsigned short *)CAN1_MB29_DATA0)
#define                  pCAN1_MB29_DATA1 ((volatile unsigned short *)CAN1_MB29_DATA1)
#define                  pCAN1_MB29_DATA2 ((volatile unsigned short *)CAN1_MB29_DATA2)
#define                  pCAN1_MB29_DATA3 ((volatile unsigned short *)CAN1_MB29_DATA3)
#define                 pCAN1_MB29_LENGTH ((volatile unsigned short *)CAN1_MB29_LENGTH)
#define              pCAN1_MB29_TIMESTAMP ((volatile unsigned short *)CAN1_MB29_TIMESTAMP)
#define                    pCAN1_MB29_ID0 ((volatile unsigned short *)CAN1_MB29_ID0)
#define                    pCAN1_MB29_ID1 ((volatile unsigned short *)CAN1_MB29_ID1)
#define                  pCAN1_MB30_DATA0 ((volatile unsigned short *)CAN1_MB30_DATA0)
#define                  pCAN1_MB30_DATA1 ((volatile unsigned short *)CAN1_MB30_DATA1)
#define                  pCAN1_MB30_DATA2 ((volatile unsigned short *)CAN1_MB30_DATA2)
#define                  pCAN1_MB30_DATA3 ((volatile unsigned short *)CAN1_MB30_DATA3)
#define                 pCAN1_MB30_LENGTH ((volatile unsigned short *)CAN1_MB30_LENGTH)
#define              pCAN1_MB30_TIMESTAMP ((volatile unsigned short *)CAN1_MB30_TIMESTAMP)
#define                    pCAN1_MB30_ID0 ((volatile unsigned short *)CAN1_MB30_ID0)
#define                    pCAN1_MB30_ID1 ((volatile unsigned short *)CAN1_MB30_ID1)
#define                  pCAN1_MB31_DATA0 ((volatile unsigned short *)CAN1_MB31_DATA0)
#define                  pCAN1_MB31_DATA1 ((volatile unsigned short *)CAN1_MB31_DATA1)
#define                  pCAN1_MB31_DATA2 ((volatile unsigned short *)CAN1_MB31_DATA2)
#define                  pCAN1_MB31_DATA3 ((volatile unsigned short *)CAN1_MB31_DATA3)
#define                 pCAN1_MB31_LENGTH ((volatile unsigned short *)CAN1_MB31_LENGTH)
#define              pCAN1_MB31_TIMESTAMP ((volatile unsigned short *)CAN1_MB31_TIMESTAMP)
#define                    pCAN1_MB31_ID0 ((volatile unsigned short *)CAN1_MB31_ID0)
#define                    pCAN1_MB31_ID1 ((volatile unsigned short *)CAN1_MB31_ID1)

/* ATAPI Registers */

#define                    pATAPI_CONTROL ((volatile unsigned short *)ATAPI_CONTROL)
#define                     pATAPI_STATUS ((volatile unsigned short *)ATAPI_STATUS)
#define                   pATAPI_DEV_ADDR ((volatile unsigned short *)ATAPI_DEV_ADDR)
#define                  pATAPI_DEV_TXBUF ((volatile unsigned short *)ATAPI_DEV_TXBUF)
#define                  pATAPI_DEV_RXBUF ((volatile unsigned short *)ATAPI_DEV_RXBUF)
#define                   pATAPI_INT_MASK ((volatile unsigned short *)ATAPI_INT_MASK)
#define                 pATAPI_INT_STATUS ((volatile unsigned short *)ATAPI_INT_STATUS)
#define                   pATAPI_XFER_LEN ((volatile unsigned short *)ATAPI_XFER_LEN)
#define                pATAPI_LINE_STATUS ((volatile unsigned short *)ATAPI_LINE_STATUS)
#define                   pATAPI_SM_STATE ((volatile unsigned short *)ATAPI_SM_STATE)
#define                  pATAPI_TERMINATE ((volatile unsigned short *)ATAPI_TERMINATE)
#define                 pATAPI_PIO_TFRCNT ((volatile unsigned short *)ATAPI_PIO_TFRCNT)
#define                 pATAPI_DMA_TFRCNT ((volatile unsigned short *)ATAPI_DMA_TFRCNT)
#define               pATAPI_UMAIN_TFRCNT ((volatile unsigned short *)ATAPI_UMAIN_TFRCNT)
#define             pATAPI_UDMAOUT_TFRCNT ((volatile unsigned short *)ATAPI_UDMAOUT_TFRCNT)
#define                  pATAPI_REG_TIM_0 ((volatile unsigned short *)ATAPI_REG_TIM_0)
#define                  pATAPI_PIO_TIM_0 ((volatile unsigned short *)ATAPI_PIO_TIM_0)
#define                  pATAPI_PIO_TIM_1 ((volatile unsigned short *)ATAPI_PIO_TIM_1)
#define                pATAPI_MULTI_TIM_0 ((volatile unsigned short *)ATAPI_MULTI_TIM_0)
#define                pATAPI_MULTI_TIM_1 ((volatile unsigned short *)ATAPI_MULTI_TIM_1)
#define                pATAPI_MULTI_TIM_2 ((volatile unsigned short *)ATAPI_MULTI_TIM_2)
#define                pATAPI_ULTRA_TIM_0 ((volatile unsigned short *)ATAPI_ULTRA_TIM_0)
#define                pATAPI_ULTRA_TIM_1 ((volatile unsigned short *)ATAPI_ULTRA_TIM_1)
#define                pATAPI_ULTRA_TIM_2 ((volatile unsigned short *)ATAPI_ULTRA_TIM_2)
#define                pATAPI_ULTRA_TIM_3 ((volatile unsigned short *)ATAPI_ULTRA_TIM_3)

/* SDH Registers */

#define                      pSDH_PWR_CTL ((volatile unsigned short *)SDH_PWR_CTL)
#define                      pSDH_CLK_CTL ((volatile unsigned short *)SDH_CLK_CTL)
#define                     pSDH_ARGUMENT ((volatile unsigned long *)SDH_ARGUMENT)
#define                      pSDH_COMMAND ((volatile unsigned short *)SDH_COMMAND)
#define                     pSDH_RESP_CMD ((volatile unsigned short *)SDH_RESP_CMD)
#define                    pSDH_RESPONSE0 ((volatile unsigned long *)SDH_RESPONSE0)
#define                    pSDH_RESPONSE1 ((volatile unsigned long *)SDH_RESPONSE1)
#define                    pSDH_RESPONSE2 ((volatile unsigned long *)SDH_RESPONSE2)
#define                    pSDH_RESPONSE3 ((volatile unsigned long *)SDH_RESPONSE3)
#define                   pSDH_DATA_TIMER ((volatile unsigned long *)SDH_DATA_TIMER)
#define                    pSDH_DATA_LGTH ((volatile unsigned short *)SDH_DATA_LGTH)
#define                     pSDH_DATA_CTL ((volatile unsigned short *)SDH_DATA_CTL)
#define                     pSDH_DATA_CNT ((volatile unsigned short *)SDH_DATA_CNT)
#define                       pSDH_STATUS ((volatile unsigned long *)SDH_STATUS)
#define                   pSDH_STATUS_CLR ((volatile unsigned short *)SDH_STATUS_CLR)
#define                        pSDH_MASK0 ((volatile unsigned long *)SDH_MASK0)
#define                        pSDH_MASK1 ((volatile unsigned long *)SDH_MASK1)
#define                     pSDH_FIFO_CNT ((volatile unsigned short *)SDH_FIFO_CNT)
#define                         pSDH_FIFO ((volatile unsigned long *)SDH_FIFO)
#define                     pSDH_E_STATUS ((volatile unsigned short *)SDH_E_STATUS)
#define                       pSDH_E_MASK ((volatile unsigned short *)SDH_E_MASK)
#define                          pSDH_CFG ((volatile unsigned short *)SDH_CFG)
#define                   pSDH_RD_WAIT_EN ((volatile unsigned short *)SDH_RD_WAIT_EN)
#define                         pSDH_PID0 ((volatile unsigned short *)SDH_PID0)
#define                         pSDH_PID1 ((volatile unsigned short *)SDH_PID1)
#define                         pSDH_PID2 ((volatile unsigned short *)SDH_PID2)
#define                         pSDH_PID3 ((volatile unsigned short *)SDH_PID3)
#define                         pSDH_PID4 ((volatile unsigned short *)SDH_PID4)
#define                         pSDH_PID5 ((volatile unsigned short *)SDH_PID5)
#define                         pSDH_PID6 ((volatile unsigned short *)SDH_PID6)
#define                         pSDH_PID7 ((volatile unsigned short *)SDH_PID7)

/* HOST Port Registers */

#define                     pHOST_CONTROL ((volatile unsigned short *)HOST_CONTROL)
#define                      pHOST_STATUS ((volatile unsigned short *)HOST_STATUS)
#define                     pHOST_TIMEOUT ((volatile unsigned short *)HOST_TIMEOUT)

/* USB Control Registers */

#define                        pUSB_FADDR ((volatile unsigned short *)USB_FADDR)
#define                        pUSB_POWER ((volatile unsigned short *)USB_POWER)
#define                       pUSB_INTRTX ((volatile unsigned short *)USB_INTRTX)
#define                       pUSB_INTRRX ((volatile unsigned short *)USB_INTRRX)
#define                      pUSB_INTRTXE ((volatile unsigned short *)USB_INTRTXE)
#define                      pUSB_INTRRXE ((volatile unsigned short *)USB_INTRRXE)
#define                      pUSB_INTRUSB ((volatile unsigned short *)USB_INTRUSB)
#define                     pUSB_INTRUSBE ((volatile unsigned short *)USB_INTRUSBE)
#define                        pUSB_FRAME ((volatile unsigned short *)USB_FRAME)
#define                        pUSB_INDEX ((volatile unsigned short *)USB_INDEX)
#define                     pUSB_TESTMODE ((volatile unsigned short *)USB_TESTMODE)
#define                     pUSB_GLOBINTR ((volatile unsigned short *)USB_GLOBINTR)
#define                   pUSB_GLOBAL_CTL ((volatile unsigned short *)USB_GLOBAL_CTL)

/* USB Packet Control Registers */

#define                pUSB_TX_MAX_PACKET ((volatile unsigned short *)USB_TX_MAX_PACKET)
#define                         pUSB_CSR0 ((volatile unsigned short *)USB_CSR0)
#define                        pUSB_TXCSR ((volatile unsigned short *)USB_TXCSR)
#define                pUSB_RX_MAX_PACKET ((volatile unsigned short *)USB_RX_MAX_PACKET)
#define                        pUSB_RXCSR ((volatile unsigned short *)USB_RXCSR)
#define                       pUSB_COUNT0 ((volatile unsigned short *)USB_COUNT0)
#define                      pUSB_RXCOUNT ((volatile unsigned short *)USB_RXCOUNT)
#define                       pUSB_TXTYPE ((volatile unsigned short *)USB_TXTYPE)
#define                    pUSB_NAKLIMIT0 ((volatile unsigned short *)USB_NAKLIMIT0)
#define                   pUSB_TXINTERVAL ((volatile unsigned short *)USB_TXINTERVAL)
#define                       pUSB_RXTYPE ((volatile unsigned short *)USB_RXTYPE)
#define                   pUSB_RXINTERVAL ((volatile unsigned short *)USB_RXINTERVAL)
#define                      pUSB_TXCOUNT ((volatile unsigned short *)USB_TXCOUNT)

/* USB Endpoint FIFO Registers */

#define                     pUSB_EP0_FIFO ((volatile unsigned short *)USB_EP0_FIFO)
#define                     pUSB_EP1_FIFO ((volatile unsigned short *)USB_EP1_FIFO)
#define                     pUSB_EP2_FIFO ((volatile unsigned short *)USB_EP2_FIFO)
#define                     pUSB_EP3_FIFO ((volatile unsigned short *)USB_EP3_FIFO)
#define                     pUSB_EP4_FIFO ((volatile unsigned short *)USB_EP4_FIFO)
#define                     pUSB_EP5_FIFO ((volatile unsigned short *)USB_EP5_FIFO)
#define                     pUSB_EP6_FIFO ((volatile unsigned short *)USB_EP6_FIFO)
#define                     pUSB_EP7_FIFO ((volatile unsigned short *)USB_EP7_FIFO)

/* USB OTG Control Registers */

#define                  pUSB_OTG_DEV_CTL ((volatile unsigned short *)USB_OTG_DEV_CTL)
#define                 pUSB_OTG_VBUS_IRQ ((volatile unsigned short *)USB_OTG_VBUS_IRQ)
#define                pUSB_OTG_VBUS_MASK ((volatile unsigned short *)USB_OTG_VBUS_MASK)

/* USB Phy Control Registers */

#define                     pUSB_LINKINFO ((volatile unsigned short *)USB_LINKINFO)
#define                        pUSB_VPLEN ((volatile unsigned short *)USB_VPLEN)
#define                      pUSB_HS_EOF1 ((volatile unsigned short *)USB_HS_EOF1)
#define                      pUSB_FS_EOF1 ((volatile unsigned short *)USB_FS_EOF1)
#define                      pUSB_LS_EOF1 ((volatile unsigned short *)USB_LS_EOF1)

/* (APHY_CNTRL is for ADI usage only) */

#define                   pUSB_APHY_CNTRL ((volatile unsigned short *)USB_APHY_CNTRL)

/* (APHY_CALIB is for ADI usage only) */

#define                   pUSB_APHY_CALIB ((volatile unsigned short *)USB_APHY_CALIB)
#define                  pUSB_APHY_CNTRL2 ((volatile unsigned short *)USB_APHY_CNTRL2)

/* (PHY_TEST is for ADI usage only) */

#define                     pUSB_PHY_TEST ((volatile unsigned short *)USB_PHY_TEST)
#define                  pUSB_PLLOSC_CTRL ((volatile unsigned short *)USB_PLLOSC_CTRL)
#define                   pUSB_SRP_CLKDIV ((volatile unsigned short *)USB_SRP_CLKDIV)

/* USB Endpoint 0 Control Registers */

#define                pUSB_EP_NI0_TXMAXP ((volatile unsigned short *)USB_EP_NI0_TXMAXP)
#define                 pUSB_EP_NI0_TXCSR ((volatile unsigned short *)USB_EP_NI0_TXCSR)
#define                pUSB_EP_NI0_RXMAXP ((volatile unsigned short *)USB_EP_NI0_RXMAXP)
#define                 pUSB_EP_NI0_RXCSR ((volatile unsigned short *)USB_EP_NI0_RXCSR)
#define               pUSB_EP_NI0_RXCOUNT ((volatile unsigned short *)USB_EP_NI0_RXCOUNT)
#define                pUSB_EP_NI0_TXTYPE ((volatile unsigned short *)USB_EP_NI0_TXTYPE)
#define            pUSB_EP_NI0_TXINTERVAL ((volatile unsigned short *)USB_EP_NI0_TXINTERVAL)
#define                pUSB_EP_NI0_RXTYPE ((volatile unsigned short *)USB_EP_NI0_RXTYPE)
#define            pUSB_EP_NI0_RXINTERVAL ((volatile unsigned short *)USB_EP_NI0_RXINTERVAL)

/* USB Endpoint 1 Control Registers */

#define               pUSB_EP_NI0_TXCOUNT ((volatile unsigned short *)USB_EP_NI0_TXCOUNT)
#define                pUSB_EP_NI1_TXMAXP ((volatile unsigned short *)USB_EP_NI1_TXMAXP)
#define                 pUSB_EP_NI1_TXCSR ((volatile unsigned short *)USB_EP_NI1_TXCSR)
#define                pUSB_EP_NI1_RXMAXP ((volatile unsigned short *)USB_EP_NI1_RXMAXP)
#define                 pUSB_EP_NI1_RXCSR ((volatile unsigned short *)USB_EP_NI1_RXCSR)
#define               pUSB_EP_NI1_RXCOUNT ((volatile unsigned short *)USB_EP_NI1_RXCOUNT)
#define                pUSB_EP_NI1_TXTYPE ((volatile unsigned short *)USB_EP_NI1_TXTYPE)
#define            pUSB_EP_NI1_TXINTERVAL ((volatile unsigned short *)USB_EP_NI1_TXINTERVAL)
#define                pUSB_EP_NI1_RXTYPE ((volatile unsigned short *)USB_EP_NI1_RXTYPE)
#define            pUSB_EP_NI1_RXINTERVAL ((volatile unsigned short *)USB_EP_NI1_RXINTERVAL)

/* USB Endpoint 2 Control Registers */

#define               pUSB_EP_NI1_TXCOUNT ((volatile unsigned short *)USB_EP_NI1_TXCOUNT)
#define                pUSB_EP_NI2_TXMAXP ((volatile unsigned short *)USB_EP_NI2_TXMAXP)
#define                 pUSB_EP_NI2_TXCSR ((volatile unsigned short *)USB_EP_NI2_TXCSR)
#define                pUSB_EP_NI2_RXMAXP ((volatile unsigned short *)USB_EP_NI2_RXMAXP)
#define                 pUSB_EP_NI2_RXCSR ((volatile unsigned short *)USB_EP_NI2_RXCSR)
#define               pUSB_EP_NI2_RXCOUNT ((volatile unsigned short *)USB_EP_NI2_RXCOUNT)
#define                pUSB_EP_NI2_TXTYPE ((volatile unsigned short *)USB_EP_NI2_TXTYPE)
#define            pUSB_EP_NI2_TXINTERVAL ((volatile unsigned short *)USB_EP_NI2_TXINTERVAL)
#define                pUSB_EP_NI2_RXTYPE ((volatile unsigned short *)USB_EP_NI2_RXTYPE)
#define            pUSB_EP_NI2_RXINTERVAL ((volatile unsigned short *)USB_EP_NI2_RXINTERVAL)

/* USB Endpoint 3 Control Registers */

#define               pUSB_EP_NI2_TXCOUNT ((volatile unsigned short *)USB_EP_NI2_TXCOUNT)
#define                pUSB_EP_NI3_TXMAXP ((volatile unsigned short *)USB_EP_NI3_TXMAXP)
#define                 pUSB_EP_NI3_TXCSR ((volatile unsigned short *)USB_EP_NI3_TXCSR)
#define                pUSB_EP_NI3_RXMAXP ((volatile unsigned short *)USB_EP_NI3_RXMAXP)
#define                 pUSB_EP_NI3_RXCSR ((volatile unsigned short *)USB_EP_NI3_RXCSR)
#define               pUSB_EP_NI3_RXCOUNT ((volatile unsigned short *)USB_EP_NI3_RXCOUNT)
#define                pUSB_EP_NI3_TXTYPE ((volatile unsigned short *)USB_EP_NI3_TXTYPE)
#define            pUSB_EP_NI3_TXINTERVAL ((volatile unsigned short *)USB_EP_NI3_TXINTERVAL)
#define                pUSB_EP_NI3_RXTYPE ((volatile unsigned short *)USB_EP_NI3_RXTYPE)
#define            pUSB_EP_NI3_RXINTERVAL ((volatile unsigned short *)USB_EP_NI3_RXINTERVAL)

/* USB Endpoint 4 Control Registers */

#define               pUSB_EP_NI3_TXCOUNT ((volatile unsigned short *)USB_EP_NI3_TXCOUNT)
#define                pUSB_EP_NI4_TXMAXP ((volatile unsigned short *)USB_EP_NI4_TXMAXP)
#define                 pUSB_EP_NI4_TXCSR ((volatile unsigned short *)USB_EP_NI4_TXCSR)
#define                pUSB_EP_NI4_RXMAXP ((volatile unsigned short *)USB_EP_NI4_RXMAXP)
#define                 pUSB_EP_NI4_RXCSR ((volatile unsigned short *)USB_EP_NI4_RXCSR)
#define               pUSB_EP_NI4_RXCOUNT ((volatile unsigned short *)USB_EP_NI4_RXCOUNT)
#define                pUSB_EP_NI4_TXTYPE ((volatile unsigned short *)USB_EP_NI4_TXTYPE)
#define            pUSB_EP_NI4_TXINTERVAL ((volatile unsigned short *)USB_EP_NI4_TXINTERVAL)
#define                pUSB_EP_NI4_RXTYPE ((volatile unsigned short *)USB_EP_NI4_RXTYPE)
#define            pUSB_EP_NI4_RXINTERVAL ((volatile unsigned short *)USB_EP_NI4_RXINTERVAL)

/* USB Endpoint 5 Control Registers */

#define               pUSB_EP_NI4_TXCOUNT ((volatile unsigned short *)USB_EP_NI4_TXCOUNT)
#define                pUSB_EP_NI5_TXMAXP ((volatile unsigned short *)USB_EP_NI5_TXMAXP)
#define                 pUSB_EP_NI5_TXCSR ((volatile unsigned short *)USB_EP_NI5_TXCSR)
#define                pUSB_EP_NI5_RXMAXP ((volatile unsigned short *)USB_EP_NI5_RXMAXP)
#define                 pUSB_EP_NI5_RXCSR ((volatile unsigned short *)USB_EP_NI5_RXCSR)
#define               pUSB_EP_NI5_RXCOUNT ((volatile unsigned short *)USB_EP_NI5_RXCOUNT)
#define                pUSB_EP_NI5_TXTYPE ((volatile unsigned short *)USB_EP_NI5_TXTYPE)
#define            pUSB_EP_NI5_TXINTERVAL ((volatile unsigned short *)USB_EP_NI5_TXINTERVAL)
#define                pUSB_EP_NI5_RXTYPE ((volatile unsigned short *)USB_EP_NI5_RXTYPE)
#define            pUSB_EP_NI5_RXINTERVAL ((volatile unsigned short *)USB_EP_NI5_RXINTERVAL)

/* USB Endpoint 6 Control Registers */

#define               pUSB_EP_NI5_TXCOUNT ((volatile unsigned short *)USB_EP_NI5_TXCOUNT)
#define                pUSB_EP_NI6_TXMAXP ((volatile unsigned short *)USB_EP_NI6_TXMAXP)
#define                 pUSB_EP_NI6_TXCSR ((volatile unsigned short *)USB_EP_NI6_TXCSR)
#define                pUSB_EP_NI6_RXMAXP ((volatile unsigned short *)USB_EP_NI6_RXMAXP)
#define                 pUSB_EP_NI6_RXCSR ((volatile unsigned short *)USB_EP_NI6_RXCSR)
#define               pUSB_EP_NI6_RXCOUNT ((volatile unsigned short *)USB_EP_NI6_RXCOUNT)
#define                pUSB_EP_NI6_TXTYPE ((volatile unsigned short *)USB_EP_NI6_TXTYPE)
#define            pUSB_EP_NI6_TXINTERVAL ((volatile unsigned short *)USB_EP_NI6_TXINTERVAL)
#define                pUSB_EP_NI6_RXTYPE ((volatile unsigned short *)USB_EP_NI6_RXTYPE)
#define            pUSB_EP_NI6_RXINTERVAL ((volatile unsigned short *)USB_EP_NI6_RXINTERVAL)

/* USB Endpoint 7 Control Registers */

#define               pUSB_EP_NI6_TXCOUNT ((volatile unsigned short *)USB_EP_NI6_TXCOUNT)
#define                pUSB_EP_NI7_TXMAXP ((volatile unsigned short *)USB_EP_NI7_TXMAXP)
#define                 pUSB_EP_NI7_TXCSR ((volatile unsigned short *)USB_EP_NI7_TXCSR)
#define                pUSB_EP_NI7_RXMAXP ((volatile unsigned short *)USB_EP_NI7_RXMAXP)
#define                 pUSB_EP_NI7_RXCSR ((volatile unsigned short *)USB_EP_NI7_RXCSR)
#define               pUSB_EP_NI7_RXCOUNT ((volatile unsigned short *)USB_EP_NI7_RXCOUNT)
#define                pUSB_EP_NI7_TXTYPE ((volatile unsigned short *)USB_EP_NI7_TXTYPE)
#define            pUSB_EP_NI7_TXINTERVAL ((volatile unsigned short *)USB_EP_NI7_TXINTERVAL)
#define                pUSB_EP_NI7_RXTYPE ((volatile unsigned short *)USB_EP_NI7_RXTYPE)
#define            pUSB_EP_NI7_RXINTERVAL ((volatile unsigned short *)USB_EP_NI7_RXINTERVAL)
#define               pUSB_EP_NI7_TXCOUNT ((volatile unsigned short *)USB_EP_NI7_TXCOUNT)
#define                pUSB_DMA_INTERRUPT ((volatile unsigned short *)USB_DMA_INTERRUPT)

/* USB Channel 0 Config Registers */

#define                  pUSB_DMA0CONTROL ((volatile unsigned short *)USB_DMA0CONTROL)
#define                  pUSB_DMA0ADDRLOW ((volatile unsigned short *)USB_DMA0ADDRLOW)
#define                 pUSB_DMA0ADDRHIGH ((volatile unsigned short *)USB_DMA0ADDRHIGH)
#define                 pUSB_DMA0COUNTLOW ((volatile unsigned short *)USB_DMA0COUNTLOW)
#define                pUSB_DMA0COUNTHIGH ((volatile unsigned short *)USB_DMA0COUNTHIGH)

/* USB Channel 1 Config Registers */

#define                  pUSB_DMA1CONTROL ((volatile unsigned short *)USB_DMA1CONTROL)
#define                  pUSB_DMA1ADDRLOW ((volatile unsigned short *)USB_DMA1ADDRLOW)
#define                 pUSB_DMA1ADDRHIGH ((volatile unsigned short *)USB_DMA1ADDRHIGH)
#define                 pUSB_DMA1COUNTLOW ((volatile unsigned short *)USB_DMA1COUNTLOW)
#define                pUSB_DMA1COUNTHIGH ((volatile unsigned short *)USB_DMA1COUNTHIGH)

/* USB Channel 2 Config Registers */

#define                  pUSB_DMA2CONTROL ((volatile unsigned short *)USB_DMA2CONTROL)
#define                  pUSB_DMA2ADDRLOW ((volatile unsigned short *)USB_DMA2ADDRLOW)
#define                 pUSB_DMA2ADDRHIGH ((volatile unsigned short *)USB_DMA2ADDRHIGH)
#define                 pUSB_DMA2COUNTLOW ((volatile unsigned short *)USB_DMA2COUNTLOW)
#define                pUSB_DMA2COUNTHIGH ((volatile unsigned short *)USB_DMA2COUNTHIGH)

/* USB Channel 3 Config Registers */

#define                  pUSB_DMA3CONTROL ((volatile unsigned short *)USB_DMA3CONTROL)
#define                  pUSB_DMA3ADDRLOW ((volatile unsigned short *)USB_DMA3ADDRLOW)
#define                 pUSB_DMA3ADDRHIGH ((volatile unsigned short *)USB_DMA3ADDRHIGH)
#define                 pUSB_DMA3COUNTLOW ((volatile unsigned short *)USB_DMA3COUNTLOW)
#define                pUSB_DMA3COUNTHIGH ((volatile unsigned short *)USB_DMA3COUNTHIGH)

/* USB Channel 4 Config Registers */

#define                  pUSB_DMA4CONTROL ((volatile unsigned short *)USB_DMA4CONTROL)
#define                  pUSB_DMA4ADDRLOW ((volatile unsigned short *)USB_DMA4ADDRLOW)
#define                 pUSB_DMA4ADDRHIGH ((volatile unsigned short *)USB_DMA4ADDRHIGH)
#define                 pUSB_DMA4COUNTLOW ((volatile unsigned short *)USB_DMA4COUNTLOW)
#define                pUSB_DMA4COUNTHIGH ((volatile unsigned short *)USB_DMA4COUNTHIGH)

/* USB Channel 5 Config Registers */

#define                  pUSB_DMA5CONTROL ((volatile unsigned short *)USB_DMA5CONTROL)
#define                  pUSB_DMA5ADDRLOW ((volatile unsigned short *)USB_DMA5ADDRLOW)
#define                 pUSB_DMA5ADDRHIGH ((volatile unsigned short *)USB_DMA5ADDRHIGH)
#define                 pUSB_DMA5COUNTLOW ((volatile unsigned short *)USB_DMA5COUNTLOW)
#define                pUSB_DMA5COUNTHIGH ((volatile unsigned short *)USB_DMA5COUNTHIGH)

/* USB Channel 6 Config Registers */

#define                  pUSB_DMA6CONTROL ((volatile unsigned short *)USB_DMA6CONTROL)
#define                  pUSB_DMA6ADDRLOW ((volatile unsigned short *)USB_DMA6ADDRLOW)
#define                 pUSB_DMA6ADDRHIGH ((volatile unsigned short *)USB_DMA6ADDRHIGH)
#define                 pUSB_DMA6COUNTLOW ((volatile unsigned short *)USB_DMA6COUNTLOW)
#define                pUSB_DMA6COUNTHIGH ((volatile unsigned short *)USB_DMA6COUNTHIGH)

/* USB Channel 7 Config Registers */

#define                  pUSB_DMA7CONTROL ((volatile unsigned short *)USB_DMA7CONTROL)
#define                  pUSB_DMA7ADDRLOW ((volatile unsigned short *)USB_DMA7ADDRLOW)
#define                 pUSB_DMA7ADDRHIGH ((volatile unsigned short *)USB_DMA7ADDRHIGH)
#define                 pUSB_DMA7COUNTLOW ((volatile unsigned short *)USB_DMA7COUNTLOW)
#define                pUSB_DMA7COUNTHIGH ((volatile unsigned short *)USB_DMA7COUNTHIGH)

/* Keypad Registers */

#define                         pKPAD_CTL ((volatile unsigned short *)KPAD_CTL)
#define                    pKPAD_PRESCALE ((volatile unsigned short *)KPAD_PRESCALE)
#define                        pKPAD_MSEL ((volatile unsigned short *)KPAD_MSEL)
#define                      pKPAD_ROWCOL ((volatile unsigned short *)KPAD_ROWCOL)
#define                        pKPAD_STAT ((volatile unsigned short *)KPAD_STAT)
#define                    pKPAD_SOFTEVAL ((volatile unsigned short *)KPAD_SOFTEVAL)

/* Pixel Compositor (PIXC) Registers */

#define                         pPIXC_CTL ((volatile unsigned short *)PIXC_CTL)
#define                         pPIXC_PPL ((volatile unsigned short *)PIXC_PPL)
#define                         pPIXC_LPF ((volatile unsigned short *)PIXC_LPF)
#define                     pPIXC_AHSTART ((volatile unsigned short *)PIXC_AHSTART)
#define                       pPIXC_AHEND ((volatile unsigned short *)PIXC_AHEND)
#define                     pPIXC_AVSTART ((volatile unsigned short *)PIXC_AVSTART)
#define                       pPIXC_AVEND ((volatile unsigned short *)PIXC_AVEND)
#define                     pPIXC_ATRANSP ((volatile unsigned short *)PIXC_ATRANSP)
#define                     pPIXC_BHSTART ((volatile unsigned short *)PIXC_BHSTART)
#define                       pPIXC_BHEND ((volatile unsigned short *)PIXC_BHEND)
#define                     pPIXC_BVSTART ((volatile unsigned short *)PIXC_BVSTART)
#define                       pPIXC_BVEND ((volatile unsigned short *)PIXC_BVEND)
#define                     pPIXC_BTRANSP ((volatile unsigned short *)PIXC_BTRANSP)
#define                    pPIXC_INTRSTAT ((volatile unsigned short *)PIXC_INTRSTAT)
#define                       pPIXC_RYCON ((volatile unsigned long *)PIXC_RYCON)
#define                       pPIXC_GUCON ((volatile unsigned long *)PIXC_GUCON)
#define                       pPIXC_BVCON ((volatile unsigned long *)PIXC_BVCON)
#define                      pPIXC_CCBIAS ((volatile unsigned long *)PIXC_CCBIAS)
#define                          pPIXC_TC ((volatile unsigned long *)PIXC_TC)

#endif /* _CDEF_BF549_H */
