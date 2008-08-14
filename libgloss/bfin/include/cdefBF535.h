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
 * cdefBF535.h
 *
 * Copyright (C) 2008 Analog Devices, Inc.
 *
 ************************************************************************/

#ifndef _CDEF_BF535_H
#define _CDEF_BF535_H

/* include all Core registers and bit definitions */
#if defined(__ADSPLPBLACKFIN__)
#warning cdefBF535.h should only be included for 535 compatible chips.
#endif
#include <defBF535.h>

/* include core specific register pointer definitions */
#include <cdefblackfin.h>

#ifndef _PTR_TO_VOL_VOID_PTR
#ifndef _USE_LEGACY_CDEF_BEHAVIOUR
#define _PTR_TO_VOL_VOID_PTR (void * volatile *)
#else
#define _PTR_TO_VOL_VOID_PTR (volatile void **)
#endif
#endif

/* Clock and System Control (0xFFC0 0400-0xFFC0 07FF) */
#define pPLL_CTL ((volatile unsigned long *)PLL_CTL)
#define pPLL_STAT ((volatile unsigned short *)PLL_STAT)
#define pPLL_LOCKCNT ((volatile unsigned short *)PLL_LOCKCNT)
#define pSWRST ((volatile unsigned short *)SWRST)
#define pSYSCR ((volatile unsigned short *)SYSCR)
#define pPLL_IOCKR ((volatile unsigned short *)PLL_IOCKR)
#define pPLL_IOCK ((volatile unsigned short *)PLL_IOCK)

/* JTAG/Debug Communication Channel (0xFFC0 0800-0xFFC0 0BFF) */
#define pCHIPID ((volatile unsigned long *)CHIPID)

/* System Interrupt Controller (0xFFC0 0C00-0xFFC0 0FFF) */
#define pSIC_IAR0 ((volatile unsigned long *)SIC_IAR0)
#define pSIC_IAR1 ((volatile unsigned long *)SIC_IAR1)
#define pSIC_IAR2 ((volatile unsigned long *)SIC_IAR2)
#define pSIC_IMASK ((volatile unsigned long *)SIC_IMASK)
#define pSIC_ISR ((volatile unsigned long *)SIC_ISR)
#define pSIC_IWR ((volatile unsigned long *)SIC_IWR)

/* Watchdog Timer (0xFFC0 1000-0xFFC0 13FF) */
#define pWDOG_CTL ((volatile unsigned short *)WDOG_CTL)
#define pWDOG_CNT ((volatile unsigned long *)WDOG_CNT)
#define pWDOG_STAT ((volatile unsigned long *)WDOG_STAT)

/* Real Time Clock (0xFFC0 1400-0xFFC0 17FF) */
#define pRTC_STAT ((volatile unsigned long *)RTC_STAT)
#define pRTC_ICTL ((volatile unsigned short *)RTC_ICTL)
#define pRTC_ISTAT ((volatile unsigned short *)RTC_ISTAT)
#define pRTC_SWCNT ((volatile unsigned short *)RTC_SWCNT)
#define pRTC_ALARM ((volatile unsigned long *)RTC_ALARM)
#define pRTC_FAST ((volatile unsigned short *)RTC_FAST)

/* General Purpose IO (0xFFC0 2400-0xFFC0 27FF) */
#define pFIO_DIR ((volatile unsigned short *)FIO_DIR)
#define pFIO_FLAG_C ((volatile unsigned short *)FIO_FLAG_C)
#define pFIO_FLAG_S ((volatile unsigned short *)FIO_FLAG_S)
#define pFIO_MASKA_C ((volatile unsigned short *)FIO_MASKA_C)
#define pFIO_MASKA_S ((volatile unsigned short *)FIO_MASKA_S)
#define pFIO_MASKB_C ((volatile unsigned short *)FIO_MASKB_C)
#define pFIO_MASKB_S ((volatile unsigned short *)FIO_MASKB_S)
#define pFIO_POLAR ((volatile unsigned short *)FIO_POLAR)
#define pFIO_EDGE ((volatile unsigned short *)FIO_EDGE)
#define pFIO_BOTH ((volatile unsigned short *)FIO_BOTH)

/* Aysnchronous Memory Controller - External Bus Interface Unit (0xFFC0 3C00-0xFFC0 3FFF) */
#define pEBIU_AMGCTL ((volatile unsigned short *)EBIU_AMGCTL)
#define pEBIU_AMBCTL0 ((volatile unsigned long *)EBIU_AMBCTL0)
#define pEBIU_AMBCTL1 ((volatile unsigned long *)EBIU_AMBCTL1)

/* USB Registers (0xFFC0 4400 - 0xFFC0 47FF) */
#define pUSBD_ID ((volatile unsigned short *)USBD_ID)
#define pUSBD_FRM ((volatile unsigned short *)USBD_FRM)
#define pUSBD_FRMAT ((volatile unsigned short *)USBD_FRMAT)
#define pUSBD_EPBUF ((volatile unsigned short *)USBD_EPBUF)
#define pUSBD_STAT ((volatile unsigned short *)USBD_STAT)
#define pUSBD_CTRL ((volatile unsigned short *)USBD_CTRL)
#define pUSBD_GINTR ((volatile unsigned short *)USBD_GINTR)
#define pUSBD_GMASK ((volatile unsigned short *)USBD_GMASK)
#define pUSBD_DMACFG ((volatile unsigned short *)USBD_DMACFG)
#define pUSBD_DMABL ((volatile unsigned short *)USBD_DMABL)
#define pUSBD_DMABH ((volatile unsigned short *)USBD_DMABH)
#define pUSBD_DMACT ((volatile unsigned short *)USBD_DMACT)
#define pUSBD_DMAIRQ ((volatile unsigned short *)USBD_DMAIRQ)
#define pUSBD_INTR0 ((volatile unsigned short *)USBD_INTR0)
#define pUSBD_MASK0 ((volatile unsigned short *)USBD_MASK0)
#define pUSBD_EPCFG0 ((volatile unsigned short *)USBD_EPCFG0)
#define pUSBD_EPADR0 ((volatile unsigned short *)USBD_EPADR0)
#define pUSBD_EPLEN0 ((volatile unsigned short *)USBD_EPLEN0)
#define pUSBD_INTR1 ((volatile unsigned short *)USBD_INTR1)
#define pUSBD_MASK1 ((volatile unsigned short *)USBD_MASK1)
#define pUSBD_EPCFG1 ((volatile unsigned short *)USBD_EPCFG1)
#define pUSBD_EPADR1 ((volatile unsigned short *)USBD_EPADR1)
#define pUSBD_EPLEN1 ((volatile unsigned short *)USBD_EPLEN1)
#define pUSBD_INTR2 ((volatile unsigned short *)USBD_INTR2)
#define pUSBD_MASK2 ((volatile unsigned short *)USBD_MASK2)
#define pUSBD_EPCFG2 ((volatile unsigned short *)USBD_EPCFG2)
#define pUSBD_EPADR2 ((volatile unsigned short *)USBD_EPADR2)
#define pUSBD_EPLEN2 ((volatile unsigned short *)USBD_EPLEN2)
#define pUSBD_INTR3 ((volatile unsigned short *)USBD_INTR3)
#define pUSBD_MASK3 ((volatile unsigned short *)USBD_MASK3)
#define pUSBD_EPCFG3 ((volatile unsigned short *)USBD_EPCFG3)
#define pUSBD_EPADR3 ((volatile unsigned short *)USBD_EPADR3)
#define pUSBD_EPLEN3 ((volatile unsigned short *)USBD_EPLEN3)
#define pUSBD_INTR4 ((volatile unsigned short *)USBD_INTR4)
#define pUSBD_MASK4 ((volatile unsigned short *)USBD_MASK4)
#define pUSBD_EPCFG4 ((volatile unsigned short *)USBD_EPCFG4)
#define pUSBD_EPADR4 ((volatile unsigned short *)USBD_EPADR4)
#define pUSBD_EPLEN4 ((volatile unsigned short *)USBD_EPLEN4)
#define pUSBD_INTR5 ((volatile unsigned short *)USBD_INTR5)
#define pUSBD_MASK5 ((volatile unsigned short *)USBD_MASK5)
#define pUSBD_EPCFG5 ((volatile unsigned short *)USBD_EPCFG5)
#define pUSBD_EPADR5 ((volatile unsigned short *)USBD_EPADR5)
#define pUSBD_EPLEN5 ((volatile unsigned short *)USBD_EPLEN5)
#define pUSBD_INTR6 ((volatile unsigned short *)USBD_INTR6)
#define pUSBD_MASK6 ((volatile unsigned short *)USBD_MASK6)
#define pUSBD_EPCFG6 ((volatile unsigned short *)USBD_EPCFG6)
#define pUSBD_EPADR6 ((volatile unsigned short *)USBD_EPADR6)
#define pUSBD_EPLEN6 ((volatile unsigned short *)USBD_EPLEN6)
#define pUSBD_INTR7 ((volatile unsigned short *)USBD_INTR7)
#define pUSBD_MASK7 ((volatile unsigned short *)USBD_MASK7)
#define pUSBD_EPCFG7 ((volatile unsigned short *)USBD_EPCFG7)
#define pUSBD_EPADR7 ((volatile unsigned short *)USBD_EPADR7)
#define pUSBD_EPLEN7 ((volatile unsigned short *)USBD_EPLEN7)

/* SDRAM Controller External Bus Interface Unit (0xFFC0 4C00-0xFFC0 4FFF) */
#define pEBIU_SDGCTL ((volatile unsigned long *)EBIU_SDGCTL)
#define pEBIU_SDRRC ((volatile unsigned short *)EBIU_SDRRC)
#define pEBIU_SDSTAT ((volatile unsigned short *)EBIU_SDSTAT)
#define pEBIU_SDBCTL ((volatile unsigned long *)EBIU_SDBCTL)

/* Memory Map */

/* Core MMRs */
#define pCOREMMR_BASE ((volatile void *)COREMMR_BASE)

/* System MMRs */
#define pSYSMMR_BASE ((volatile void *)SYSMMR_BASE)

/* L1 cache/SRAM internal memory */
#define pL1_DATA_A ((void *)L1_DATA_A)
#define pL1_DATA_B ((void *)L1_DATA_B)
#define pL1_CODE ((void *)L1_CODE)
#define pL1_SCRATCH ((void *)L1_SCRATCH)

/* L2 SRAM external memory */
#define pL2_BASE ((void *)L2_BASE)

/* PCI Spaces */
#define pPCI_CONFIG_SPACE_PORT ((volatile void *)PCI_CONFIG_SPACE_PORT)
#define pPCI_CONFIG_BASE ((volatile void *)PCI_CONFIG_BASE)
#define pPCI_IO_BASE ((volatile void *)PCI_IO_BASE)
#define pPCI_MEM_BASE ((volatile void *)PCI_MEM_BASE)

/* Async Memory Banks */
#define pASYNC_BANK3_BASE ((void *)ASYNC_BANK3_BASE)
#define pASYNC_BANK2_BASE ((void *)ASYNC_BANK2_BASE)
#define pASYNC_BANK1_BASE ((void *)ASYNC_BANK1_BASE)
#define pASYNC_BANK0_BASE ((void *)ASYNC_BANK0_BASE)

/* Sync DRAM Banks */
#define pSDRAM_BANK3_BASE ((void *)SDRAM_BANK3_BASE)
#define pSDRAM_BANK2_BASE ((void *)SDRAM_BANK2_BASE)
#define pSDRAM_BANK1_BASE ((void *)SDRAM_BANK1_BASE)
#define pSDRAM_BANK0_BASE ((void *)SDRAM_BANK0_BASE)

/* UART 0 Controller (0xFFC0 1800-0xFFC0 1BFF) */
#define pUART0_THR ((volatile unsigned short *)UART0_THR)
#define pUART0_RBR ((volatile unsigned short *)UART0_RBR)
#define pUART0_DLL ((volatile unsigned short *)UART0_DLL)
#define pUART0_IER ((volatile unsigned short *)UART0_IER)
#define pUART0_DLH ((volatile unsigned short *)UART0_DLH)
#define pUART0_IIR ((volatile unsigned short *)UART0_IIR)
#define pUART0_LCR ((volatile unsigned short *)UART0_LCR)
#define pUART0_MCR ((volatile unsigned short *)UART0_MCR)
#define pUART0_LSR ((volatile unsigned short *)UART0_LSR)
#define pUART0_MSR ((volatile unsigned short *)UART0_MSR)
#define pUART0_SCR ((volatile unsigned short *)UART0_SCR)
#define pUART0_IRCR ((volatile unsigned short *)UART0_IRCR)
#define pUART0_CURR_PTR_RX ((volatile unsigned short *)UART0_CURR_PTR_RX)
#define pUART0_CONFIG_RX ((volatile unsigned short *)UART0_CONFIG_RX)
#define pUART0_START_ADDR_HI_RX ((volatile unsigned short *)UART0_START_ADDR_HI_RX)
#define pUART0_START_ADDR_LO_RX ((volatile unsigned short *)UART0_START_ADDR_LO_RX)
#define pUART0_COUNT_RX ((volatile unsigned short *)UART0_COUNT_RX)
#define pUART0_NEXT_DESCR_RX ((volatile unsigned short *)UART0_NEXT_DESCR_RX)
#define pUART0_DESCR_RDY_RX ((volatile unsigned short *)UART0_DESCR_RDY_RX)
#define pUART0_IRQSTAT_RX ((volatile unsigned short *)UART0_IRQSTAT_RX)
#define pUART0_CURR_PTR_TX ((volatile unsigned short *)UART0_CURR_PTR_TX)
#define pUART0_CONFIG_TX ((volatile unsigned short *)UART0_CONFIG_TX)
#define pUART0_START_ADDR_HI_TX ((volatile unsigned short *)UART0_START_ADDR_HI_TX)
#define pUART0_START_ADDR_LO_TX ((volatile unsigned short *)UART0_START_ADDR_LO_TX)
#define pUART0_COUNT_TX ((volatile unsigned short *)UART0_COUNT_TX)
#define pUART0_NEXT_DESCR_TX ((volatile unsigned short *)UART0_NEXT_DESCR_TX)
#define pUART0_DESCR_RDY_TX ((volatile unsigned short *)UART0_DESCR_RDY_TX)
#define pUART0_IRQSTAT_TX ((volatile unsigned short *)UART0_IRQSTAT_TX)

/* UART 1 Controller (0xFFC0 1C00-0xFFC0 1FFF) */
#define pUART1_THR ((volatile unsigned short *)UART1_THR)
#define pUART1_RBR ((volatile unsigned short *)UART1_RBR)
#define pUART1_DLL ((volatile unsigned short *)UART1_DLL)
#define pUART1_IER ((volatile unsigned short *)UART1_IER)
#define pUART1_DLH ((volatile unsigned short *)UART1_DLH)
#define pUART1_IIR ((volatile unsigned short *)UART1_IIR)
#define pUART1_LCR ((volatile unsigned short *)UART1_LCR)
#define pUART1_MCR ((volatile unsigned short *)UART1_MCR)
#define pUART1_LSR ((volatile unsigned short *)UART1_LSR)
#define pUART1_MSR ((volatile unsigned short *)UART1_MSR)
#define pUART1_SCR ((volatile unsigned short *)UART1_SCR)
#define pUART1_CURR_PTR_RX ((volatile unsigned short *)UART1_CURR_PTR_RX)
#define pUART1_CONFIG_RX ((volatile unsigned short *)UART1_CONFIG_RX)
#define pUART1_START_ADDR_HI_RX ((volatile unsigned short *)UART1_START_ADDR_HI_RX)
#define pUART1_START_ADDR_LO_RX ((volatile unsigned short *)UART1_START_ADDR_LO_RX)
#define pUART1_COUNT_RX ((volatile unsigned short *)UART1_COUNT_RX)
#define pUART1_NEXT_DESCR_RX ((volatile unsigned short *)UART1_NEXT_DESCR_RX)
#define pUART1_DESCR_RDY_RX ((volatile unsigned short *)UART1_DESCR_RDY_RX)
#define pUART1_IRQSTAT_RX ((volatile unsigned short *)UART1_IRQSTAT_RX)
#define pUART1_CURR_PTR_TX ((volatile unsigned short *)UART1_CURR_PTR_TX)
#define pUART1_CONFIG_TX ((volatile unsigned short *)UART1_CONFIG_TX)
#define pUART1_START_ADDR_HI_TX ((volatile unsigned short *)UART1_START_ADDR_HI_TX)
#define pUART1_START_ADDR_LO_TX ((volatile unsigned short *)UART1_START_ADDR_LO_TX)
#define pUART1_COUNT_TX ((volatile unsigned short *)UART1_COUNT_TX)
#define pUART1_NEXT_DESCR_TX ((volatile unsigned short *)UART1_NEXT_DESCR_TX)
#define pUART1_DESCR_RDY_TX ((volatile unsigned short *)UART1_DESCR_RDY_TX)
#define pUART1_IRQSTAT_TX ((volatile unsigned short *)UART1_IRQSTAT_TX)

/* TIMER 0, 1, 2 Registers (0xFFC0 2000-0xFFC0 23FF) */
#define pTIMER0_STATUS ((volatile unsigned short *)TIMER0_STATUS)
#define pTIMER0_CONFIG ((volatile unsigned short *)TIMER0_CONFIG)
#define pTIMER0_COUNTER_LO ((volatile unsigned short *)TIMER0_COUNTER_LO)
#define pTIMER0_COUNTER_HI ((volatile unsigned short *)TIMER0_COUNTER_HI)
#define pTIMER0_PERIOD_LO ((volatile unsigned short *)TIMER0_PERIOD_LO)
#define pTIMER0_PERIOD_HI ((volatile unsigned short *)TIMER0_PERIOD_HI)
#define pTIMER0_WIDTH_LO ((volatile unsigned short *)TIMER0_WIDTH_LO)
#define pTIMER0_WIDTH_HI ((volatile unsigned short *)TIMER0_WIDTH_HI)
#define pTIMER1_STATUS ((volatile unsigned short *)TIMER1_STATUS)
#define pTIMER1_CONFIG ((volatile unsigned short *)TIMER1_CONFIG)
#define pTIMER1_COUNTER_LO ((volatile unsigned short *)TIMER1_COUNTER_LO)
#define pTIMER1_COUNTER_HI ((volatile unsigned short *)TIMER1_COUNTER_HI)
#define pTIMER1_PERIOD_LO ((volatile unsigned short *)TIMER1_PERIOD_LO)
#define pTIMER1_PERIOD_HI ((volatile unsigned short *)TIMER1_PERIOD_HI)
#define pTIMER1_WIDTH_LO ((volatile unsigned short *)TIMER1_WIDTH_LO)
#define pTIMER1_WIDTH_HI ((volatile unsigned short *)TIMER1_WIDTH_HI)
#define pTIMER2_STATUS ((volatile unsigned short *)TIMER2_STATUS)
#define pTIMER2_CONFIG ((volatile unsigned short *)TIMER2_CONFIG)
#define pTIMER2_COUNTER_LO ((volatile unsigned short *)TIMER2_COUNTER_LO)
#define pTIMER2_COUNTER_HI ((volatile unsigned short *)TIMER2_COUNTER_HI)
#define pTIMER2_PERIOD_LO ((volatile unsigned short *)TIMER2_PERIOD_LO)
#define pTIMER2_PERIOD_HI ((volatile unsigned short *)TIMER2_PERIOD_HI)
#define pTIMER2_WIDTH_LO ((volatile unsigned short *)TIMER2_WIDTH_LO)
#define pTIMER2_WIDTH_HI ((volatile unsigned short *)TIMER2_WIDTH_HI)

/* SPORT0 Controller (0xFFC0 2800-0xFFC0 2BFF) */
#define pSPORT0_TX_CONFIG ((volatile unsigned short *)SPORT0_TX_CONFIG)
#define pSPORT0_RX_CONFIG ((volatile unsigned short *)SPORT0_RX_CONFIG)
#define pSPORT0_TX ((volatile short *)SPORT0_TX)
#define pSPORT0_RX ((volatile short *)SPORT0_RX)
#define pSPORT0_TSCLKDIV ((volatile unsigned short *)SPORT0_TSCLKDIV)
#define pSPORT0_RSCLKDIV ((volatile unsigned short *)SPORT0_RSCLKDIV)
#define pSPORT0_TFSDIV ((volatile unsigned short *)SPORT0_TFSDIV)
#define pSPORT0_RFSDIV ((volatile unsigned short *)SPORT0_RFSDIV)
#define pSPORT0_STAT ((volatile unsigned short *)SPORT0_STAT)
#define pSPORT0_MTCS0 ((volatile unsigned short *)SPORT0_MTCS0)
#define pSPORT0_MTCS1 ((volatile unsigned short *)SPORT0_MTCS1)
#define pSPORT0_MTCS2 ((volatile unsigned short *)SPORT0_MTCS2)
#define pSPORT0_MTCS3 ((volatile unsigned short *)SPORT0_MTCS3)
#define pSPORT0_MTCS4 ((volatile unsigned short *)SPORT0_MTCS4)
#define pSPORT0_MTCS5 ((volatile unsigned short *)SPORT0_MTCS5)
#define pSPORT0_MTCS6 ((volatile unsigned short *)SPORT0_MTCS6)
#define pSPORT0_MTCS7 ((volatile unsigned short *)SPORT0_MTCS7)
#define pSPORT0_MRCS0 ((volatile unsigned short *)SPORT0_MRCS0)
#define pSPORT0_MRCS1 ((volatile unsigned short *)SPORT0_MRCS1)
#define pSPORT0_MRCS2 ((volatile unsigned short *)SPORT0_MRCS2)
#define pSPORT0_MRCS3 ((volatile unsigned short *)SPORT0_MRCS3)
#define pSPORT0_MRCS4 ((volatile unsigned short *)SPORT0_MRCS4)
#define pSPORT0_MRCS5 ((volatile unsigned short *)SPORT0_MRCS5)
#define pSPORT0_MRCS6 ((volatile unsigned short *)SPORT0_MRCS6)
#define pSPORT0_MRCS7 ((volatile unsigned short *)SPORT0_MRCS7)
#define pSPORT0_MCMC1 ((volatile unsigned short *)SPORT0_MCMC1)
#define pSPORT0_MCMC2 ((volatile unsigned short *)SPORT0_MCMC2)
#define pSPORT0_CURR_PTR_RX ((volatile unsigned short *)SPORT0_CURR_PTR_RX)
#define pSPORT0_CONFIG_DMA_RX ((volatile unsigned short *)SPORT0_CONFIG_DMA_RX)
#define pSPORT0_START_ADDR_HI_RX ((volatile unsigned short *)SPORT0_START_ADDR_HI_RX)
#define pSPORT0_START_ADDR_LO_RX ((volatile unsigned short *)SPORT0_START_ADDR_LO_RX)
#define pSPORT0_COUNT_RX ((volatile unsigned short *)SPORT0_COUNT_RX)
#define pSPORT0_NEXT_DESCR_RX ((volatile unsigned short *)SPORT0_NEXT_DESCR_RX)
#define pSPORT0_DESCR_RDY_RX ((volatile unsigned short *)SPORT0_DESCR_RDY_RX)
#define pSPORT0_IRQSTAT_RX ((volatile unsigned short *)SPORT0_IRQSTAT_RX)
#define pSPORT0_CURR_PTR_TX ((volatile unsigned short *)SPORT0_CURR_PTR_TX)
#define pSPORT0_CONFIG_DMA_TX ((volatile unsigned short *)SPORT0_CONFIG_DMA_TX)
#define pSPORT0_START_ADDR_HI_TX ((volatile unsigned short *)SPORT0_START_ADDR_HI_TX)
#define pSPORT0_START_ADDR_LO_TX ((volatile unsigned short *)SPORT0_START_ADDR_LO_TX)
#define pSPORT0_COUNT_TX ((volatile unsigned short *)SPORT0_COUNT_TX)
#define pSPORT0_NEXT_DESCR_TX ((volatile unsigned short *)SPORT0_NEXT_DESCR_TX)
#define pSPORT0_DESCR_RDY_TX ((volatile unsigned short *)SPORT0_DESCR_RDY_TX)
#define pSPORT0_IRQSTAT_TX ((volatile unsigned short *)SPORT0_IRQSTAT_TX)

/* SPORT1 Controller (0xFFC0 2C00-0xFFC0 2FFF) */
#define pSPORT1_TX_CONFIG ((volatile unsigned short *)SPORT1_TX_CONFIG)
#define pSPORT1_RX_CONFIG ((volatile unsigned short *)SPORT1_RX_CONFIG)
#define pSPORT1_TX ((volatile short *)SPORT1_TX)
#define pSPORT1_RX ((volatile short *)SPORT1_RX)
#define pSPORT1_TSCLKDIV ((volatile unsigned short *)SPORT1_TSCLKDIV)
#define pSPORT1_RSCLKDIV ((volatile unsigned short *)SPORT1_RSCLKDIV)
#define pSPORT1_TFSDIV ((volatile unsigned short *)SPORT1_TFSDIV)
#define pSPORT1_RFSDIV ((volatile unsigned short *)SPORT1_RFSDIV)
#define pSPORT1_STAT ((volatile unsigned short *)SPORT1_STAT)
#define pSPORT1_MTCS0 ((volatile unsigned short *)SPORT1_MTCS0)
#define pSPORT1_MTCS1 ((volatile unsigned short *)SPORT1_MTCS1)
#define pSPORT1_MTCS2 ((volatile unsigned short *)SPORT1_MTCS2)
#define pSPORT1_MTCS3 ((volatile unsigned short *)SPORT1_MTCS3)
#define pSPORT1_MTCS4 ((volatile unsigned short *)SPORT1_MTCS4)
#define pSPORT1_MTCS5 ((volatile unsigned short *)SPORT1_MTCS5)
#define pSPORT1_MTCS6 ((volatile unsigned short *)SPORT1_MTCS6)
#define pSPORT1_MTCS7 ((volatile unsigned short *)SPORT1_MTCS7)
#define pSPORT1_MRCS0 ((volatile unsigned short *)SPORT1_MRCS0)
#define pSPORT1_MRCS1 ((volatile unsigned short *)SPORT1_MRCS1)
#define pSPORT1_MRCS2 ((volatile unsigned short *)SPORT1_MRCS2)
#define pSPORT1_MRCS3 ((volatile unsigned short *)SPORT1_MRCS3)
#define pSPORT1_MRCS4 ((volatile unsigned short *)SPORT1_MRCS4)
#define pSPORT1_MRCS5 ((volatile unsigned short *)SPORT1_MRCS5)
#define pSPORT1_MRCS6 ((volatile unsigned short *)SPORT1_MRCS6)
#define pSPORT1_MRCS7 ((volatile unsigned short *)SPORT1_MRCS7)
#define pSPORT1_MCMC1 ((volatile unsigned short *)SPORT1_MCMC1)
#define pSPORT1_MCMC2 ((volatile unsigned short *)SPORT1_MCMC2)
#define pSPORT1_CURR_PTR_RX ((volatile unsigned short *)SPORT1_CURR_PTR_RX)
#define pSPORT1_CONFIG_DMA_RX ((volatile unsigned short *)SPORT1_CONFIG_DMA_RX)
#define pSPORT1_START_ADDR_HI_RX ((volatile unsigned short *)SPORT1_START_ADDR_HI_RX)
#define pSPORT1_START_ADDR_LO_RX ((volatile unsigned short *)SPORT1_START_ADDR_LO_RX)
#define pSPORT1_COUNT_RX ((volatile unsigned short *)SPORT1_COUNT_RX)
#define pSPORT1_NEXT_DESCR_RX ((volatile unsigned short *)SPORT1_NEXT_DESCR_RX)
#define pSPORT1_DESCR_RDY_RX ((volatile unsigned short *)SPORT1_DESCR_RDY_RX)
#define pSPORT1_IRQSTAT_RX ((volatile unsigned short *)SPORT1_IRQSTAT_RX)
#define pSPORT1_CURR_PTR_TX ((volatile unsigned short *)SPORT1_CURR_PTR_TX)
#define pSPORT1_CONFIG_DMA_TX ((volatile unsigned short *)SPORT1_CONFIG_DMA_TX)
#define pSPORT1_START_ADDR_HI_TX ((volatile unsigned short *)SPORT1_START_ADDR_HI_TX)
#define pSPORT1_START_ADDR_LO_TX ((volatile unsigned short *)SPORT1_START_ADDR_LO_TX)
#define pSPORT1_COUNT_TX ((volatile unsigned short *)SPORT1_COUNT_TX)
#define pSPORT1_NEXT_DESCR_TX ((volatile unsigned short *)SPORT1_NEXT_DESCR_TX)
#define pSPORT1_DESCR_RDY_TX ((volatile unsigned short *)SPORT1_DESCR_RDY_TX)
#define pSPORT1_IRQSTAT_TX ((volatile unsigned short *)SPORT1_IRQSTAT_TX)

/* SPI 0 Controller (0xFFC0 3000-0xFFC0 33FF) */
#define pSPI0_CTL ((volatile unsigned short *)SPI0_CTL)
#define pSPI0_FLG ((volatile unsigned short *)SPI0_FLG)
#define pSPI0_ST ((volatile unsigned short *)SPI0_ST)
#define pSPI0_TDBR ((volatile unsigned short *)SPI0_TDBR)
#define pSPI0_RDBR ((volatile unsigned short *)SPI0_RDBR)
#define pSPI0_BAUD ((volatile unsigned short *)SPI0_BAUD)
#define pSPI0_SHADOW ((volatile unsigned short *)SPI0_SHADOW)
#define pSPI0_CURR_PTR ((volatile unsigned short *)SPI0_CURR_PTR)
#define pSPI0_CONFIG ((volatile unsigned short *)SPI0_CONFIG)
#define pSPI0_START_ADDR_HI ((volatile unsigned short *)SPI0_START_ADDR_HI)
#define pSPI0_START_ADDR_LO ((volatile unsigned short *)SPI0_START_ADDR_LO)
#define pSPI0_COUNT ((volatile unsigned short *)SPI0_COUNT)
#define pSPI0_NEXT_DESCR ((volatile unsigned short *)SPI0_NEXT_DESCR)
#define pSPI0_DESCR_RDY ((volatile unsigned short *)SPI0_DESCR_RDY)
#define pSPI0_DMA_INT ((volatile unsigned short *)SPI0_DMA_INT)

/* SPI 1 Controller (0xFFC0 3400-0xFFC0 37FF) */
#define pSPI1_CTL ((volatile unsigned short *)SPI1_CTL)
#define pSPI1_FLG ((volatile unsigned short *)SPI1_FLG)
#define pSPI1_ST ((volatile unsigned short *)SPI1_ST)
#define pSPI1_TDBR ((volatile unsigned short *)SPI1_TDBR)
#define pSPI1_RDBR ((volatile unsigned short *)SPI1_RDBR)
#define pSPI1_BAUD ((volatile unsigned short *)SPI1_BAUD)
#define pSPI1_SHADOW ((volatile unsigned short *)SPI1_SHADOW)
#define pSPI1_CURR_PTR ((volatile unsigned short *)SPI1_CURR_PTR)
#define pSPI1_CONFIG ((volatile unsigned short *)SPI1_CONFIG)
#define pSPI1_START_ADDR_HI ((volatile unsigned short *)SPI1_START_ADDR_HI)
#define pSPI1_START_ADDR_LO ((volatile unsigned short *)SPI1_START_ADDR_LO)
#define pSPI1_COUNT ((volatile unsigned short *)SPI1_COUNT)
#define pSPI1_NEXT_DESCR ((volatile unsigned short *)SPI1_NEXT_DESCR)
#define pSPI1_DESCR_RDY ((volatile unsigned short *)SPI1_DESCR_RDY)
#define pSPI1_DMA_INT ((volatile unsigned short *)SPI1_DMA_INT)

/* Memory DMA Controller (0xFFC0 3800-0xFFC0 3BFF) */
#define pMDD_DCP ((volatile unsigned short *)MDD_DCP)
#define pMDD_DCFG ((volatile unsigned short *)MDD_DCFG)
#define pMDD_DSAH ((volatile unsigned short *)MDD_DSAH)
#define pMDD_DSAL ((volatile unsigned short *)MDD_DSAL)
#define pMDD_DCT ((volatile unsigned short *)MDD_DCT)
#define pMDD_DND ((volatile unsigned short *)MDD_DND)
#define pMDD_DDR ((volatile unsigned short *)MDD_DDR)
#define pMDD_DI ((volatile unsigned short *)MDD_DI)
#define pMDS_DCP ((volatile unsigned short *)MDS_DCP)
#define pMDS_DCFG ((volatile unsigned short *)MDS_DCFG)
#define pMDS_DSAH ((volatile unsigned short *)MDS_DSAH)
#define pMDS_DSAL ((volatile unsigned short *)MDS_DSAL)
#define pMDS_DCT ((volatile unsigned short *)MDS_DCT)
#define pMDS_DND ((volatile unsigned short *)MDS_DND)
#define pMDS_DDR ((volatile unsigned short *)MDS_DDR)
#define pMDS_DI ((volatile unsigned short *)MDS_DI)

/* PCI Bridge PAB Registers (0xFFC0 4000-0xFFC0 43FF) */
#define pPCI_CTL ((volatile unsigned short *)PCI_CTL)
#define pPCI_STAT ((volatile unsigned long *)PCI_STAT)
#define pPCI_ICTL ((volatile unsigned long *)PCI_ICTL)
#define pPCI_MBAP (_PTR_TO_VOL_VOID_PTR PCI_MBAP)
#define pPCI_IBAP (_PTR_TO_VOL_VOID_PTR PCI_IBAP)
#define pPCI_CBAP (_PTR_TO_VOL_VOID_PTR PCI_CBAP)
#define pPCI_TMBAP (_PTR_TO_VOL_VOID_PTR PCI_TMBAP)
#define pPCI_TIBAP (_PTR_TO_VOL_VOID_PTR PCI_TIBAP)

/* PCI Bridge External Access Bus Registers (0xEEFF FF00-0xEEFF FFFF) */
#define pPCI_DMBARM ((volatile unsigned long *)PCI_DMBARM)
#define pPCI_DIBARM ((volatile unsigned long *)PCI_DIBARM)
#define pPCI_CFG_DIC ((volatile unsigned long *)PCI_CFG_DIC)
#define pPCI_CFG_VIC ((volatile unsigned long *)PCI_CFG_VIC)
#define pPCI_CFG_STAT ((volatile unsigned long *)PCI_CFG_STAT)
#define pPCI_CFG_CMD ((volatile unsigned long *)PCI_CFG_CMD)
#define pPCI_CFG_CC ((volatile unsigned long *)PCI_CFG_CC)
#define pPCI_CFG_RID ((volatile unsigned long *)PCI_CFG_RID)
#define pPCI_CFG_BIST ((volatile unsigned long *)PCI_CFG_BIST)
#define pPCI_CFG_HT ((volatile unsigned long *)PCI_CFG_HT)
#define pPCI_CFG_MLT ((volatile unsigned long *)PCI_CFG_MLT)
#define pPCI_CFG_CLS ((volatile unsigned long *)PCI_CFG_CLS)
#define pPCI_CFG_MBAR ((volatile unsigned long *)PCI_CFG_MBAR)
#define pPCI_CFG_IBAR ((volatile unsigned long *)PCI_CFG_IBAR)
#define pPCI_CFG_SID ((volatile unsigned long *)PCI_CFG_SID)
#define pPCI_CFG_SVID ((volatile unsigned long *)PCI_CFG_SVID)
#define pPCI_CFG_MAXL ((volatile unsigned long *)PCI_CFG_MAXL)
#define pPCI_CFG_MING ((volatile unsigned long *)PCI_CFG_MING)
#define pPCI_CFG_IP ((volatile unsigned long *)PCI_CFG_IP)
#define pPCI_CFG_IL ((volatile unsigned long *)PCI_CFG_IL)
#define pPCI_HMCTL ((volatile unsigned long *)PCI_HMCTL)

/* System Bus Interface Unit (0xFFC0 4800-0xFFC0 4FFF) */
#define pDMA_DBP ((volatile unsigned short *)DMA_DBP)
#define pDB_ACOMP (_PTR_TO_VOL_VOID_PTR DB_ACOMP)
#define pDB_CCOMP ((volatile unsigned long *)DB_CCOMP)

#endif /* _CDEF_BF535_H */
