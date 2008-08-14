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
** cdefBF52x_base.h
**
** Copyright (C) 2008 Analog Devices, Inc.
**
************************************************************************************
**
** This include file contains a list of macro "defines" to enable the programmer
** to use symbolic names for the registers common to the ADSP-BF52x peripherals.
**
***************************************************************/

#ifndef _CDEF_BF52X_H
#define _CDEF_BF52X_H

#include <defBF52x_base.h>

/* ==== begin from cdefBF534.h ==== */

#ifndef _PTR_TO_VOL_VOID_PTR
#ifndef _USE_LEGACY_CDEF_BEHAVIOUR
#define _PTR_TO_VOL_VOID_PTR (void * volatile *)
#else
#define _PTR_TO_VOL_VOID_PTR (volatile void **)
#endif
#endif


/* Clock and System Control	(0xFFC00000 - 0xFFC000FF)								*/
#define pPLL_CTL 		((volatile unsigned short *)PLL_CTL)
#define pPLL_DIV 		((volatile unsigned short *)PLL_DIV)
#define pVR_CTL 		((volatile unsigned short *)VR_CTL)
#define pPLL_STAT 		((volatile unsigned short *)PLL_STAT)
#define pPLL_LOCKCNT 		((volatile unsigned short *)PLL_LOCKCNT)
#define pCHIPID      ((volatile unsigned long*)CHIPID)


/* System Interrupt Controller (0xFFC00100 - 0xFFC001FF)							*/
#define pSWRST 			((volatile unsigned short *)SWRST)
#define pSYSCR 			((volatile unsigned short *)SYSCR)

#define	pSIC_RVECT		(_PTR_TO_VOL_VOID_PTR SIC_RVECT)
#define pSIC_IMASK0 		((volatile unsigned long  *)SIC_IMASK0)
/* legacy register name (below) provided for backwards code compatibility */
#define pSIC_IMASK 		((volatile unsigned long  *)SIC_IMASK0)

#define pSIC_IAR0 		((volatile unsigned long  *)SIC_IAR0)
#define pSIC_IAR1 		((volatile unsigned long  *)SIC_IAR1)
#define pSIC_IAR2 		((volatile unsigned long  *)SIC_IAR2)
#define pSIC_IAR3 		((volatile unsigned long  *)SIC_IAR3)

#define pSIC_ISR0 		((volatile unsigned long  *)SIC_ISR0)
/* legacy register name (below) provided for backwards code compatibility */
#define pSIC_ISR 		((volatile unsigned long  *)SIC_ISR0)

#define pSIC_IWR0 		((volatile unsigned long  *)SIC_IWR0)
/* legacy register name (below) provided for backwards code compatibility */
#define pSIC_IWR 		((volatile unsigned long  *)SIC_IWR0)

/* SIC Additions to ADSP-BF52x (0xFFC0014C - 0xFFC00162) */

#define pSIC_IMASK1             ((volatile unsigned long  *)SIC_IMASK1)
#define pSIC_IAR4               ((volatile unsigned long  *)SIC_IAR4)
#define pSIC_IAR5               ((volatile unsigned long  *)SIC_IAR5)
#define pSIC_IAR6               ((volatile unsigned long  *)SIC_IAR6)
#define pSIC_IAR7               ((volatile unsigned long  *)SIC_IAR7)
#define pSIC_ISR1               ((volatile unsigned long  *)SIC_ISR1)
#define pSIC_IWR1               ((volatile unsigned long  *)SIC_IWR1)

/* Watchdog Timer		(0xFFC00200 - 0xFFC002FF)									*/
#define pWDOG_CTL 		((volatile unsigned short *)WDOG_CTL)
#define pWDOG_CNT 		((volatile unsigned long  *)WDOG_CNT)
#define pWDOG_STAT 		((volatile unsigned long  *)WDOG_STAT)


/* Real Time Clock		(0xFFC00300 - 0xFFC003FF)									*/
#define pRTC_STAT 		((volatile unsigned long  *)RTC_STAT)
#define pRTC_ICTL 		((volatile unsigned short *)RTC_ICTL)
#define pRTC_ISTAT 		((volatile unsigned short *)RTC_ISTAT)
#define pRTC_SWCNT 		((volatile unsigned short *)RTC_SWCNT)
#define pRTC_ALARM 		((volatile unsigned long  *)RTC_ALARM)
#define pRTC_FAST 		((volatile unsigned short *)RTC_FAST)
#define pRTC_PREN 		((volatile unsigned short *)RTC_PREN)


/* UART0 Controller		(0xFFC00400 - 0xFFC004FF)									*/
#define pUART0_THR 		((volatile unsigned short *)UART0_THR)
#define pUART0_RBR 		((volatile unsigned short *)UART0_RBR)
#define pUART0_DLL 		((volatile unsigned short *)UART0_DLL)
#define pUART0_IER 		((volatile unsigned short *)UART0_IER)
#define pUART0_DLH 		((volatile unsigned short *)UART0_DLH)
#define pUART0_IIR 		((volatile unsigned short *)UART0_IIR)
#define pUART0_LCR 		((volatile unsigned short *)UART0_LCR)
#define pUART0_MCR 		((volatile unsigned short *)UART0_MCR)
#define pUART0_LSR 		((volatile unsigned short *)UART0_LSR)
#define pUART0_MSR		((volatile unsigned short *)UART0_LSR)
#define pUART0_SCR 		((volatile unsigned short *)UART0_SCR)
#define pUART0_GCTL 		((volatile unsigned short *)UART0_GCTL)


/* SPI Controller		(0xFFC00500 - 0xFFC005FF)									*/
#define pSPI_CTL 		((volatile unsigned short *)SPI_CTL)
#define pSPI_FLG 		((volatile unsigned short *)SPI_FLG)
#define pSPI_STAT 		((volatile unsigned short *)SPI_STAT)
#define pSPI_TDBR 		((volatile unsigned short *)SPI_TDBR)
#define pSPI_RDBR 		((volatile unsigned short *)SPI_RDBR)
#define pSPI_BAUD 		((volatile unsigned short *)SPI_BAUD)
#define pSPI_SHADOW 		((volatile unsigned short *)SPI_SHADOW)


/* TIMER0-7 Registers		(0xFFC00600 - 0xFFC006FF)								*/
#define pTIMER0_CONFIG 		((volatile unsigned short *)TIMER0_CONFIG)
#define pTIMER0_COUNTER 	((volatile unsigned long  *)TIMER0_COUNTER)
#define pTIMER0_PERIOD 		((volatile unsigned long  *)TIMER0_PERIOD)
#define pTIMER0_WIDTH 		((volatile unsigned long  *)TIMER0_WIDTH)

#define pTIMER1_CONFIG 		((volatile unsigned short *)TIMER1_CONFIG)
#define pTIMER1_COUNTER 	((volatile unsigned long  *)TIMER1_COUNTER)
#define pTIMER1_PERIOD 		((volatile unsigned long  *)TIMER1_PERIOD)
#define pTIMER1_WIDTH 		((volatile unsigned long  *)TIMER1_WIDTH)

#define pTIMER2_CONFIG 		((volatile unsigned short *)TIMER2_CONFIG)
#define pTIMER2_COUNTER 	((volatile unsigned long  *)TIMER2_COUNTER)
#define pTIMER2_PERIOD 		((volatile unsigned long  *)TIMER2_PERIOD)
#define pTIMER2_WIDTH 		((volatile unsigned long  *)TIMER2_WIDTH)

#define pTIMER3_CONFIG 		((volatile unsigned short *)TIMER3_CONFIG)
#define pTIMER3_COUNTER 	((volatile unsigned long  *)TIMER3_COUNTER)
#define pTIMER3_PERIOD 		((volatile unsigned long  *)TIMER3_PERIOD)
#define pTIMER3_WIDTH 		((volatile unsigned long  *)TIMER3_WIDTH)

#define pTIMER4_CONFIG 		((volatile unsigned short *)TIMER4_CONFIG)
#define pTIMER4_COUNTER 	((volatile unsigned long  *)TIMER4_COUNTER)
#define pTIMER4_PERIOD 		((volatile unsigned long  *)TIMER4_PERIOD)
#define pTIMER4_WIDTH 		((volatile unsigned long  *)TIMER4_WIDTH)

#define pTIMER5_CONFIG 		((volatile unsigned short *)TIMER5_CONFIG)
#define pTIMER5_COUNTER 	((volatile unsigned long  *)TIMER5_COUNTER)
#define pTIMER5_PERIOD 		((volatile unsigned long  *)TIMER5_PERIOD)
#define pTIMER5_WIDTH 		((volatile unsigned long  *)TIMER5_WIDTH)

#define pTIMER6_CONFIG 		((volatile unsigned short *)TIMER6_CONFIG)
#define pTIMER6_COUNTER 	((volatile unsigned long  *)TIMER6_COUNTER)
#define pTIMER6_PERIOD 		((volatile unsigned long  *)TIMER6_PERIOD)
#define pTIMER6_WIDTH 		((volatile unsigned long  *)TIMER6_WIDTH)

#define pTIMER7_CONFIG 		((volatile unsigned short *)TIMER7_CONFIG)
#define pTIMER7_COUNTER 	((volatile unsigned long  *)TIMER7_COUNTER)
#define pTIMER7_PERIOD 		((volatile unsigned long  *)TIMER7_PERIOD)
#define pTIMER7_WIDTH 		((volatile unsigned long  *)TIMER7_WIDTH)

#define pTIMER_ENABLE 		((volatile unsigned short *)TIMER_ENABLE)
#define pTIMER_DISABLE 		((volatile unsigned short *)TIMER_DISABLE)
#define pTIMER_STATUS		((volatile unsigned long  *)TIMER_STATUS)


/* General Purpose I/O Port F (0xFFC00700 - 0xFFC007FF)								*/
#define pPORTFIO	 	((volatile unsigned short *)PORTFIO)
#define pPORTFIO_CLEAR	 	((volatile unsigned short *)PORTFIO_CLEAR)
#define pPORTFIO_SET	 	((volatile unsigned short *)PORTFIO_SET)
#define pPORTFIO_TOGGLE 	((volatile unsigned short *)PORTFIO_TOGGLE)
#define pPORTFIO_MASKA 		((volatile unsigned short *)PORTFIO_MASKA)
#define pPORTFIO_MASKA_CLEAR 	((volatile unsigned short *)PORTFIO_MASKA_CLEAR)
#define pPORTFIO_MASKA_SET 	((volatile unsigned short *)PORTFIO_MASKA_SET)
#define pPORTFIO_MASKA_TOGGLE 	((volatile unsigned short *)PORTFIO_MASKA_TOGGLE)
#define pPORTFIO_MASKB	 	((volatile unsigned short *)PORTFIO_MASKB)
#define pPORTFIO_MASKB_CLEAR 	((volatile unsigned short *)PORTFIO_MASKB_CLEAR)
#define pPORTFIO_MASKB_SET 	((volatile unsigned short *)PORTFIO_MASKB_SET)
#define pPORTFIO_MASKB_TOGGLE 	((volatile unsigned short *)PORTFIO_MASKB_TOGGLE)
#define pPORTFIO_DIR 		((volatile unsigned short *)PORTFIO_DIR)
#define pPORTFIO_POLAR 		((volatile unsigned short *)PORTFIO_POLAR)
#define pPORTFIO_EDGE 		((volatile unsigned short *)PORTFIO_EDGE)
#define pPORTFIO_BOTH 		((volatile unsigned short *)PORTFIO_BOTH)
#define pPORTFIO_INEN 		((volatile unsigned short *)PORTFIO_INEN)


/* SPORT0 Controller		(0xFFC00800 - 0xFFC008FF)								*/
#define pSPORT0_TCR1 		((volatile unsigned short *)SPORT0_TCR1)
#define pSPORT0_TCR2 		((volatile unsigned short *)SPORT0_TCR2)
#define pSPORT0_TCLKDIV 	((volatile unsigned short *)SPORT0_TCLKDIV)
#define pSPORT0_TFSDIV 		((volatile unsigned short *)SPORT0_TFSDIV)
#define pSPORT0_TX 		((volatile unsigned long  *)SPORT0_TX)
#define pSPORT0_RX 		((volatile unsigned long  *)SPORT0_RX)
#define pSPORT0_TX32 		((volatile unsigned long  *)SPORT0_TX)
#define pSPORT0_RX32 		((volatile unsigned long  *)SPORT0_RX)
#define pSPORT0_TX16 		((volatile unsigned short *)SPORT0_TX)
#define pSPORT0_RX16 		((volatile unsigned short *)SPORT0_RX)
#define pSPORT0_RCR1 		((volatile unsigned short *)SPORT0_RCR1)
#define pSPORT0_RCR2 		((volatile unsigned short *)SPORT0_RCR2)
#define pSPORT0_RCLKDIV 	((volatile unsigned short *)SPORT0_RCLKDIV)
#define pSPORT0_RFSDIV 		((volatile unsigned short *)SPORT0_RFSDIV)
#define pSPORT0_STAT 		((volatile unsigned short *)SPORT0_STAT)
#define pSPORT0_CHNL 		((volatile unsigned short *)SPORT0_CHNL)
#define pSPORT0_MCMC1 		((volatile unsigned short *)SPORT0_MCMC1)
#define pSPORT0_MCMC2 		((volatile unsigned short *)SPORT0_MCMC2)
#define pSPORT0_MTCS0 		((volatile unsigned long  *)SPORT0_MTCS0)
#define pSPORT0_MTCS1 		((volatile unsigned long  *)SPORT0_MTCS1)
#define pSPORT0_MTCS2 		((volatile unsigned long  *)SPORT0_MTCS2)
#define pSPORT0_MTCS3 		((volatile unsigned long  *)SPORT0_MTCS3)
#define pSPORT0_MRCS0 		((volatile unsigned long  *)SPORT0_MRCS0)
#define pSPORT0_MRCS1 		((volatile unsigned long  *)SPORT0_MRCS1)
#define pSPORT0_MRCS2 		((volatile unsigned long  *)SPORT0_MRCS2)
#define pSPORT0_MRCS3 		((volatile unsigned long  *)SPORT0_MRCS3)


/* SPORT1 Controller		(0xFFC00900 - 0xFFC009FF)								*/
#define pSPORT1_TCR1 		((volatile unsigned short *)SPORT1_TCR1)
#define pSPORT1_TCR2 		((volatile unsigned short *)SPORT1_TCR2)
#define pSPORT1_TCLKDIV 	((volatile unsigned short *)SPORT1_TCLKDIV)
#define pSPORT1_TFSDIV 		((volatile unsigned short *)SPORT1_TFSDIV)
#define pSPORT1_TX 		((volatile unsigned long  *)SPORT1_TX)
#define pSPORT1_RX 		((volatile unsigned long  *)SPORT1_RX)
#define pSPORT1_TX32 		((volatile unsigned long  *)SPORT1_TX)
#define pSPORT1_RX32 		((volatile unsigned long  *)SPORT1_RX)
#define pSPORT1_TX16 		((volatile unsigned short *)SPORT1_TX)
#define pSPORT1_RX16 		((volatile unsigned short *)SPORT1_RX)
#define pSPORT1_RCR1 		((volatile unsigned short *)SPORT1_RCR1)
#define pSPORT1_RCR2 		((volatile unsigned short *)SPORT1_RCR2)
#define pSPORT1_RCLKDIV 	((volatile unsigned short *)SPORT1_RCLKDIV)
#define pSPORT1_RFSDIV 		((volatile unsigned short *)SPORT1_RFSDIV)
#define pSPORT1_STAT 		((volatile unsigned short *)SPORT1_STAT)
#define pSPORT1_CHNL 		((volatile unsigned short *)SPORT1_CHNL)
#define pSPORT1_MCMC1 		((volatile unsigned short *)SPORT1_MCMC1)
#define pSPORT1_MCMC2 		((volatile unsigned short *)SPORT1_MCMC2)
#define pSPORT1_MTCS0 		((volatile unsigned long  *)SPORT1_MTCS0)
#define pSPORT1_MTCS1 		((volatile unsigned long  *)SPORT1_MTCS1)
#define pSPORT1_MTCS2 		((volatile unsigned long  *)SPORT1_MTCS2)
#define pSPORT1_MTCS3 		((volatile unsigned long  *)SPORT1_MTCS3)
#define pSPORT1_MRCS0 		((volatile unsigned long  *)SPORT1_MRCS0)
#define pSPORT1_MRCS1 		((volatile unsigned long  *)SPORT1_MRCS1)
#define pSPORT1_MRCS2 		((volatile unsigned long  *)SPORT1_MRCS2)
#define pSPORT1_MRCS3 		((volatile unsigned long  *)SPORT1_MRCS3)


/* External Bus Interface Unit (0xFFC00A00 - 0xFFC00AFF)							*/
#define pEBIU_AMGCTL 		((volatile unsigned short *)EBIU_AMGCTL)
#define pEBIU_AMBCTL0 		((volatile unsigned long  *)EBIU_AMBCTL0)
#define pEBIU_AMBCTL1 		((volatile unsigned long  *)EBIU_AMBCTL1)
#define pEBIU_SDGCTL 		((volatile unsigned long  *)EBIU_SDGCTL)
#define pEBIU_SDBCTL 		((volatile unsigned short *)EBIU_SDBCTL)
#define pEBIU_SDRRC 		((volatile unsigned short *)EBIU_SDRRC)
#define pEBIU_SDSTAT 		((volatile unsigned short *)EBIU_SDSTAT)


/* DMA Traffic Control Registers													*/
#define	pDMA_TC_PER		((volatile unsigned short *)DMA_TC_PER)
#define	pDMA_TC_CNT		((volatile unsigned short *)DMA_TC_CNT)

/* Alternate deprecated register names (below) provided for backwards code compatibility */
#define	pDMA_TCPER		((volatile unsigned short *)DMA_TCPER)
#define	pDMA_TCCNT		((volatile unsigned short *)DMA_TCCNT)

/* DMA Controller																	*/
#define pDMA0_CONFIG 		((volatile unsigned short *)DMA0_CONFIG)
#define pDMA0_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA0_NEXT_DESC_PTR)
#define pDMA0_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA0_START_ADDR)
#define pDMA0_X_COUNT 		((volatile unsigned short *)DMA0_X_COUNT)
#define pDMA0_Y_COUNT 		((volatile unsigned short *)DMA0_Y_COUNT)
#define pDMA0_X_MODIFY 		((volatile signed   short *)DMA0_X_MODIFY)
#define pDMA0_Y_MODIFY 		((volatile signed   short *)DMA0_Y_MODIFY)
#define pDMA0_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA0_CURR_DESC_PTR)
#define pDMA0_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA0_CURR_ADDR)
#define pDMA0_CURR_X_COUNT 	((volatile unsigned short *)DMA0_CURR_X_COUNT)
#define pDMA0_CURR_Y_COUNT 	((volatile unsigned short *)DMA0_CURR_Y_COUNT)
#define pDMA0_IRQ_STATUS 	((volatile unsigned short *)DMA0_IRQ_STATUS)
#define pDMA0_PERIPHERAL_MAP 	((volatile unsigned short *)DMA0_PERIPHERAL_MAP)

#define pDMA1_CONFIG 		((volatile unsigned short *)DMA1_CONFIG)
#define pDMA1_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA1_NEXT_DESC_PTR)
#define pDMA1_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA1_START_ADDR)
#define pDMA1_X_COUNT 		((volatile unsigned short *)DMA1_X_COUNT)
#define pDMA1_Y_COUNT 		((volatile unsigned short *)DMA1_Y_COUNT)
#define pDMA1_X_MODIFY 		((volatile signed   short *)DMA1_X_MODIFY)
#define pDMA1_Y_MODIFY 		((volatile signed   short *)DMA1_Y_MODIFY)
#define pDMA1_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA1_CURR_DESC_PTR)
#define pDMA1_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA1_CURR_ADDR)
#define pDMA1_CURR_X_COUNT 	((volatile unsigned short *)DMA1_CURR_X_COUNT)
#define pDMA1_CURR_Y_COUNT 	((volatile unsigned short *)DMA1_CURR_Y_COUNT)
#define pDMA1_IRQ_STATUS 	((volatile unsigned short *)DMA1_IRQ_STATUS)
#define pDMA1_PERIPHERAL_MAP 	((volatile unsigned short *)DMA1_PERIPHERAL_MAP)

#define pDMA2_CONFIG 		((volatile unsigned short *)DMA2_CONFIG)
#define pDMA2_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA2_NEXT_DESC_PTR)
#define pDMA2_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA2_START_ADDR)
#define pDMA2_X_COUNT 		((volatile unsigned short *)DMA2_X_COUNT)
#define pDMA2_Y_COUNT 		((volatile unsigned short *)DMA2_Y_COUNT)
#define pDMA2_X_MODIFY 		((volatile signed   short *)DMA2_X_MODIFY)
#define pDMA2_Y_MODIFY 		((volatile signed   short *)DMA2_Y_MODIFY)
#define pDMA2_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA2_CURR_DESC_PTR)
#define pDMA2_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA2_CURR_ADDR)
#define pDMA2_CURR_X_COUNT 	((volatile unsigned short *)DMA2_CURR_X_COUNT)
#define pDMA2_CURR_Y_COUNT 	((volatile unsigned short *)DMA2_CURR_Y_COUNT)
#define pDMA2_IRQ_STATUS 	((volatile unsigned short *)DMA2_IRQ_STATUS)
#define pDMA2_PERIPHERAL_MAP 	((volatile unsigned short *)DMA2_PERIPHERAL_MAP)

#define pDMA3_CONFIG 		((volatile unsigned short *)DMA3_CONFIG)
#define pDMA3_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA3_NEXT_DESC_PTR)
#define pDMA3_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA3_START_ADDR)
#define pDMA3_X_COUNT 		((volatile unsigned short *)DMA3_X_COUNT)
#define pDMA3_Y_COUNT 		((volatile unsigned short *)DMA3_Y_COUNT)
#define pDMA3_X_MODIFY 		((volatile signed   short *)DMA3_X_MODIFY)
#define pDMA3_Y_MODIFY 		((volatile signed   short *)DMA3_Y_MODIFY)
#define pDMA3_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA3_CURR_DESC_PTR)
#define pDMA3_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA3_CURR_ADDR)
#define pDMA3_CURR_X_COUNT 	((volatile unsigned short *)DMA3_CURR_X_COUNT)
#define pDMA3_CURR_Y_COUNT 	((volatile unsigned short *)DMA3_CURR_Y_COUNT)
#define pDMA3_IRQ_STATUS 	((volatile unsigned short *)DMA3_IRQ_STATUS)
#define pDMA3_PERIPHERAL_MAP 	((volatile unsigned short *)DMA3_PERIPHERAL_MAP)

#define pDMA4_CONFIG 		((volatile unsigned short *)DMA4_CONFIG)
#define pDMA4_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA4_NEXT_DESC_PTR)
#define pDMA4_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA4_START_ADDR)
#define pDMA4_X_COUNT 		((volatile unsigned short *)DMA4_X_COUNT)
#define pDMA4_Y_COUNT 		((volatile unsigned short *)DMA4_Y_COUNT)
#define pDMA4_X_MODIFY 		((volatile signed   short *)DMA4_X_MODIFY)
#define pDMA4_Y_MODIFY 		((volatile signed   short *)DMA4_Y_MODIFY)
#define pDMA4_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA4_CURR_DESC_PTR)
#define pDMA4_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA4_CURR_ADDR)
#define pDMA4_CURR_X_COUNT 	((volatile unsigned short *)DMA4_CURR_X_COUNT)
#define pDMA4_CURR_Y_COUNT 	((volatile unsigned short *)DMA4_CURR_Y_COUNT)
#define pDMA4_IRQ_STATUS 	((volatile unsigned short *)DMA4_IRQ_STATUS)
#define pDMA4_PERIPHERAL_MAP 	((volatile unsigned short *)DMA4_PERIPHERAL_MAP)

#define pDMA5_CONFIG 		((volatile unsigned short *)DMA5_CONFIG)
#define pDMA5_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA5_NEXT_DESC_PTR)
#define pDMA5_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA5_START_ADDR)
#define pDMA5_X_COUNT 		((volatile unsigned short *)DMA5_X_COUNT)
#define pDMA5_Y_COUNT 		((volatile unsigned short *)DMA5_Y_COUNT)
#define pDMA5_X_MODIFY 		((volatile signed   short *)DMA5_X_MODIFY)
#define pDMA5_Y_MODIFY 		((volatile signed   short *)DMA5_Y_MODIFY)
#define pDMA5_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA5_CURR_DESC_PTR)
#define pDMA5_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA5_CURR_ADDR)
#define pDMA5_CURR_X_COUNT 	((volatile unsigned short *)DMA5_CURR_X_COUNT)
#define pDMA5_CURR_Y_COUNT 	((volatile unsigned short *)DMA5_CURR_Y_COUNT)
#define pDMA5_IRQ_STATUS 	((volatile unsigned short *)DMA5_IRQ_STATUS)
#define pDMA5_PERIPHERAL_MAP 	((volatile unsigned short *)DMA5_PERIPHERAL_MAP)

#define pDMA6_CONFIG 		((volatile unsigned short *)DMA6_CONFIG)
#define pDMA6_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA6_NEXT_DESC_PTR)
#define pDMA6_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA6_START_ADDR)
#define pDMA6_X_COUNT 		((volatile unsigned short *)DMA6_X_COUNT)
#define pDMA6_Y_COUNT 		((volatile unsigned short *)DMA6_Y_COUNT)
#define pDMA6_X_MODIFY 		((volatile signed   short *)DMA6_X_MODIFY)
#define pDMA6_Y_MODIFY 		((volatile signed   short *)DMA6_Y_MODIFY)
#define pDMA6_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA6_CURR_DESC_PTR)
#define pDMA6_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA6_CURR_ADDR)
#define pDMA6_CURR_X_COUNT 	((volatile unsigned short *)DMA6_CURR_X_COUNT)
#define pDMA6_CURR_Y_COUNT 	((volatile unsigned short *)DMA6_CURR_Y_COUNT)
#define pDMA6_IRQ_STATUS 	((volatile unsigned short *)DMA6_IRQ_STATUS)
#define pDMA6_PERIPHERAL_MAP 	((volatile unsigned short *)DMA6_PERIPHERAL_MAP)

#define pDMA7_CONFIG 		((volatile unsigned short *)DMA7_CONFIG)
#define pDMA7_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA7_NEXT_DESC_PTR)
#define pDMA7_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA7_START_ADDR)
#define pDMA7_X_COUNT 		((volatile unsigned short *)DMA7_X_COUNT)
#define pDMA7_Y_COUNT 		((volatile unsigned short *)DMA7_Y_COUNT)
#define pDMA7_X_MODIFY 		((volatile signed   short *)DMA7_X_MODIFY)
#define pDMA7_Y_MODIFY 		((volatile signed   short *)DMA7_Y_MODIFY)
#define pDMA7_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA7_CURR_DESC_PTR)
#define pDMA7_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA7_CURR_ADDR)
#define pDMA7_CURR_X_COUNT 	((volatile unsigned short *)DMA7_CURR_X_COUNT)
#define pDMA7_CURR_Y_COUNT 	((volatile unsigned short *)DMA7_CURR_Y_COUNT)
#define pDMA7_IRQ_STATUS 	((volatile unsigned short *)DMA7_IRQ_STATUS)
#define pDMA7_PERIPHERAL_MAP 	((volatile unsigned short *)DMA7_PERIPHERAL_MAP)

#define pDMA8_CONFIG 		((volatile unsigned short *)DMA8_CONFIG)
#define pDMA8_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA8_NEXT_DESC_PTR)
#define pDMA8_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA8_START_ADDR)
#define pDMA8_X_COUNT 		((volatile unsigned short *)DMA8_X_COUNT)
#define pDMA8_Y_COUNT 		((volatile unsigned short *)DMA8_Y_COUNT)
#define pDMA8_X_MODIFY 		((volatile signed   short *)DMA8_X_MODIFY)
#define pDMA8_Y_MODIFY 		((volatile signed   short *)DMA8_Y_MODIFY)
#define pDMA8_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA8_CURR_DESC_PTR)
#define pDMA8_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA8_CURR_ADDR)
#define pDMA8_CURR_X_COUNT 	((volatile unsigned short *)DMA8_CURR_X_COUNT)
#define pDMA8_CURR_Y_COUNT 	((volatile unsigned short *)DMA8_CURR_Y_COUNT)
#define pDMA8_IRQ_STATUS 	((volatile unsigned short *)DMA8_IRQ_STATUS)
#define pDMA8_PERIPHERAL_MAP 	((volatile unsigned short *)DMA8_PERIPHERAL_MAP)

#define pDMA9_CONFIG 		((volatile unsigned short *)DMA9_CONFIG)
#define pDMA9_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA9_NEXT_DESC_PTR)
#define pDMA9_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA9_START_ADDR)
#define pDMA9_X_COUNT 		((volatile unsigned short *)DMA9_X_COUNT)
#define pDMA9_Y_COUNT 		((volatile unsigned short *)DMA9_Y_COUNT)
#define pDMA9_X_MODIFY 		((volatile signed   short *)DMA9_X_MODIFY)
#define pDMA9_Y_MODIFY 		((volatile signed   short *)DMA9_Y_MODIFY)
#define pDMA9_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA9_CURR_DESC_PTR)
#define pDMA9_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA9_CURR_ADDR)
#define pDMA9_CURR_X_COUNT 	((volatile unsigned short *)DMA9_CURR_X_COUNT)
#define pDMA9_CURR_Y_COUNT 	((volatile unsigned short *)DMA9_CURR_Y_COUNT)
#define pDMA9_IRQ_STATUS 	((volatile unsigned short *)DMA9_IRQ_STATUS)
#define pDMA9_PERIPHERAL_MAP 	((volatile unsigned short *)DMA9_PERIPHERAL_MAP)

#define pDMA10_CONFIG 		((volatile unsigned short *)DMA10_CONFIG)
#define pDMA10_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA10_NEXT_DESC_PTR)
#define pDMA10_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA10_START_ADDR)
#define pDMA10_X_COUNT 		((volatile unsigned short *)DMA10_X_COUNT)
#define pDMA10_Y_COUNT 		((volatile unsigned short *)DMA10_Y_COUNT)
#define pDMA10_X_MODIFY 	((volatile signed   short *)DMA10_X_MODIFY)
#define pDMA10_Y_MODIFY 	((volatile signed   short *)DMA10_Y_MODIFY)
#define pDMA10_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA10_CURR_DESC_PTR)
#define pDMA10_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA10_CURR_ADDR)
#define pDMA10_CURR_X_COUNT 	((volatile unsigned short *)DMA10_CURR_X_COUNT)
#define pDMA10_CURR_Y_COUNT 	((volatile unsigned short *)DMA10_CURR_Y_COUNT)
#define pDMA10_IRQ_STATUS 	((volatile unsigned short *)DMA10_IRQ_STATUS)
#define pDMA10_PERIPHERAL_MAP 	((volatile unsigned short *)DMA10_PERIPHERAL_MAP)

#define pDMA11_CONFIG 		((volatile unsigned short *)DMA11_CONFIG)
#define pDMA11_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA11_NEXT_DESC_PTR)
#define pDMA11_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA11_START_ADDR)
#define pDMA11_X_COUNT 		((volatile unsigned short *)DMA11_X_COUNT)
#define pDMA11_Y_COUNT 		((volatile unsigned short *)DMA11_Y_COUNT)
#define pDMA11_X_MODIFY 	((volatile signed   short *)DMA11_X_MODIFY)
#define pDMA11_Y_MODIFY 	((volatile signed   short *)DMA11_Y_MODIFY)
#define pDMA11_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA11_CURR_DESC_PTR)
#define pDMA11_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA11_CURR_ADDR)
#define pDMA11_CURR_X_COUNT 	((volatile unsigned short *)DMA11_CURR_X_COUNT)
#define pDMA11_CURR_Y_COUNT 	((volatile unsigned short *)DMA11_CURR_Y_COUNT)
#define pDMA11_IRQ_STATUS 	((volatile unsigned short *)DMA11_IRQ_STATUS)
#define pDMA11_PERIPHERAL_MAP 	((volatile unsigned short *)DMA11_PERIPHERAL_MAP)

#define pMDMA_D0_CONFIG 	((volatile unsigned short *)MDMA_D0_CONFIG)
#define pMDMA_D0_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR MDMA_D0_NEXT_DESC_PTR)
#define pMDMA_D0_START_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_D0_START_ADDR)
#define pMDMA_D0_X_COUNT 	((volatile unsigned short *)MDMA_D0_X_COUNT)
#define pMDMA_D0_Y_COUNT 	((volatile unsigned short *)MDMA_D0_Y_COUNT)
#define pMDMA_D0_X_MODIFY 	((volatile signed   short *)MDMA_D0_X_MODIFY)
#define pMDMA_D0_Y_MODIFY 	((volatile signed   short *)MDMA_D0_Y_MODIFY)
#define pMDMA_D0_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR MDMA_D0_CURR_DESC_PTR)
#define pMDMA_D0_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_D0_CURR_ADDR)
#define pMDMA_D0_CURR_X_COUNT 	((volatile unsigned short *)MDMA_D0_CURR_X_COUNT)
#define pMDMA_D0_CURR_Y_COUNT 	((volatile unsigned short *)MDMA_D0_CURR_Y_COUNT)
#define pMDMA_D0_IRQ_STATUS 	((volatile unsigned short *)MDMA_D0_IRQ_STATUS)
#define pMDMA_D0_PERIPHERAL_MAP ((volatile unsigned short *)MDMA_D0_PERIPHERAL_MAP)

#define pMDMA_S0_CONFIG 	((volatile unsigned short *)MDMA_S0_CONFIG)
#define pMDMA_S0_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR MDMA_S0_NEXT_DESC_PTR)
#define pMDMA_S0_START_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_S0_START_ADDR)
#define pMDMA_S0_X_COUNT 	((volatile unsigned short *)MDMA_S0_X_COUNT)
#define pMDMA_S0_Y_COUNT 	((volatile unsigned short *)MDMA_S0_Y_COUNT)
#define pMDMA_S0_X_MODIFY 	((volatile signed   short *)MDMA_S0_X_MODIFY)
#define pMDMA_S0_Y_MODIFY 	((volatile signed   short *)MDMA_S0_Y_MODIFY)
#define pMDMA_S0_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR MDMA_S0_CURR_DESC_PTR)
#define pMDMA_S0_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_S0_CURR_ADDR)
#define pMDMA_S0_CURR_X_COUNT 	((volatile unsigned short *)MDMA_S0_CURR_X_COUNT)
#define pMDMA_S0_CURR_Y_COUNT 	((volatile unsigned short *)MDMA_S0_CURR_Y_COUNT)
#define pMDMA_S0_IRQ_STATUS 	((volatile unsigned short *)MDMA_S0_IRQ_STATUS)
#define pMDMA_S0_PERIPHERAL_MAP	((volatile unsigned short *)MDMA_S0_PERIPHERAL_MAP)

#define pMDMA_D1_CONFIG 	((volatile unsigned short *)MDMA_D1_CONFIG)
#define pMDMA_D1_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR MDMA_D1_NEXT_DESC_PTR)
#define pMDMA_D1_START_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_D1_START_ADDR)
#define pMDMA_D1_X_COUNT 	((volatile unsigned short *)MDMA_D1_X_COUNT)
#define pMDMA_D1_Y_COUNT 	((volatile unsigned short *)MDMA_D1_Y_COUNT)
#define pMDMA_D1_X_MODIFY 	((volatile signed   short *)MDMA_D1_X_MODIFY)
#define pMDMA_D1_Y_MODIFY 	((volatile signed   short *)MDMA_D1_Y_MODIFY)
#define pMDMA_D1_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR MDMA_D1_CURR_DESC_PTR)
#define pMDMA_D1_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_D1_CURR_ADDR)
#define pMDMA_D1_CURR_X_COUNT 	((volatile unsigned short *)MDMA_D1_CURR_X_COUNT)
#define pMDMA_D1_CURR_Y_COUNT 	((volatile unsigned short *)MDMA_D1_CURR_Y_COUNT)
#define pMDMA_D1_IRQ_STATUS 	((volatile unsigned short *)MDMA_D1_IRQ_STATUS)
#define pMDMA_D1_PERIPHERAL_MAP ((volatile unsigned short *)MDMA_D1_PERIPHERAL_MAP)

#define pMDMA_S1_CONFIG 	((volatile unsigned short *)MDMA_S1_CONFIG)
#define pMDMA_S1_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR MDMA_S1_NEXT_DESC_PTR)
#define pMDMA_S1_START_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_S1_START_ADDR)
#define pMDMA_S1_X_COUNT 	((volatile unsigned short *)MDMA_S1_X_COUNT)
#define pMDMA_S1_Y_COUNT 	((volatile unsigned short *)MDMA_S1_Y_COUNT)
#define pMDMA_S1_X_MODIFY 	((volatile signed   short *)MDMA_S1_X_MODIFY)
#define pMDMA_S1_Y_MODIFY 	((volatile signed   short *)MDMA_S1_Y_MODIFY)
#define pMDMA_S1_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR MDMA_S1_CURR_DESC_PTR)
#define pMDMA_S1_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_S1_CURR_ADDR)
#define pMDMA_S1_CURR_X_COUNT 	((volatile unsigned short *)MDMA_S1_CURR_X_COUNT)
#define pMDMA_S1_CURR_Y_COUNT 	((volatile unsigned short *)MDMA_S1_CURR_Y_COUNT)
#define pMDMA_S1_IRQ_STATUS 	((volatile unsigned short *)MDMA_S1_IRQ_STATUS)
#define pMDMA_S1_PERIPHERAL_MAP ((volatile unsigned short *)MDMA_S1_PERIPHERAL_MAP)


/* Parallel Peripheral Interface (0xFFC01000 - 0xFFC010FF)							*/
#define pPPI_CONTROL 		((volatile unsigned short *)PPI_CONTROL)
#define pPPI_STATUS 		((volatile unsigned short *)PPI_STATUS)
#define pPPI_DELAY 		((volatile unsigned short *)PPI_DELAY)
#define pPPI_COUNT 		((volatile unsigned short *)PPI_COUNT)
#define pPPI_FRAME 		((volatile unsigned short *)PPI_FRAME)


/* Two-Wire Interface		(0xFFC01400 - 0xFFC014FF)								*/
#define pTWI_CLKDIV		((volatile unsigned short *)TWI_CLKDIV)
#define pTWI_CONTROL		((volatile unsigned short *)TWI_CONTROL)
#define pTWI_SLAVE_CTL		((volatile unsigned short *)TWI_SLAVE_CTL)
#define pTWI_SLAVE_STAT		((volatile unsigned short *)TWI_SLAVE_STAT)
#define pTWI_SLAVE_ADDR		((volatile unsigned short *)TWI_SLAVE_ADDR)
#define pTWI_MASTER_CTL		((volatile unsigned short *)TWI_MASTER_CTL)
#define pTWI_MASTER_STAT	((volatile unsigned short *)TWI_MASTER_STAT)
#define pTWI_MASTER_ADDR	((volatile unsigned short *)TWI_MASTER_ADDR)
#define pTWI_INT_STAT		((volatile unsigned short *)TWI_INT_STAT)
#define pTWI_INT_MASK		((volatile unsigned short *)TWI_INT_MASK)
#define pTWI_FIFO_CTL		((volatile unsigned short *)TWI_FIFO_CTL)
#define pTWI_FIFO_STAT		((volatile unsigned short *)TWI_FIFO_STAT)
#define pTWI_XMT_DATA8		((volatile unsigned short *)TWI_XMT_DATA8)
#define pTWI_XMT_DATA16		((volatile unsigned short *)TWI_XMT_DATA16)
#define pTWI_RCV_DATA8		((volatile unsigned short *)TWI_RCV_DATA8)
#define pTWI_RCV_DATA16		((volatile unsigned short *)TWI_RCV_DATA16)


/* General Purpose I/O Port G (0xFFC01500 - 0xFFC015FF)								*/
#define pPORTGIO	 	((volatile unsigned short *)PORTGIO)
#define pPORTGIO_CLEAR	 	((volatile unsigned short *)PORTGIO_CLEAR)
#define pPORTGIO_SET	 	((volatile unsigned short *)PORTGIO_SET)
#define pPORTGIO_TOGGLE 	((volatile unsigned short *)PORTGIO_TOGGLE)
#define pPORTGIO_MASKA		((volatile unsigned short *)PORTGIO_MASKA)
#define pPORTGIO_MASKA_CLEAR	((volatile unsigned short *)PORTGIO_MASKA_CLEAR)
#define pPORTGIO_MASKA_SET	((volatile unsigned short *)PORTGIO_MASKA_SET)
#define pPORTGIO_MASKA_TOGGLE	((volatile unsigned short *)PORTGIO_MASKA_TOGGLE)
#define pPORTGIO_MASKB		((volatile unsigned short *)PORTGIO_MASKB)
#define pPORTGIO_MASKB_CLEAR	((volatile unsigned short *)PORTGIO_MASKB_CLEAR)
#define pPORTGIO_MASKB_SET	((volatile unsigned short *)PORTGIO_MASKB_SET)
#define pPORTGIO_MASKB_TOGGLE	((volatile unsigned short *)PORTGIO_MASKB_TOGGLE)
#define pPORTGIO_DIR 		((volatile unsigned short *)PORTGIO_DIR)
#define pPORTGIO_POLAR 		((volatile unsigned short *)PORTGIO_POLAR)
#define pPORTGIO_EDGE 		((volatile unsigned short *)PORTGIO_EDGE)
#define pPORTGIO_BOTH 		((volatile unsigned short *)PORTGIO_BOTH)
#define pPORTGIO_INEN 		((volatile unsigned short *)PORTGIO_INEN)


/* General Purpose I/O Port H (0xFFC01700 - 0xFFC017FF)								*/
#define pPORTHIO	 	((volatile unsigned short *)PORTHIO)
#define pPORTHIO_CLEAR	 	((volatile unsigned short *)PORTHIO_CLEAR)
#define pPORTHIO_SET	 	((volatile unsigned short *)PORTHIO_SET)
#define pPORTHIO_TOGGLE 	((volatile unsigned short *)PORTHIO_TOGGLE)
#define pPORTHIO_MASKA		((volatile unsigned short *)PORTHIO_MASKA)
#define pPORTHIO_MASKA_CLEAR	((volatile unsigned short *)PORTHIO_MASKA_CLEAR)
#define pPORTHIO_MASKA_SET	((volatile unsigned short *)PORTHIO_MASKA_SET)
#define pPORTHIO_MASKA_TOGGLE	((volatile unsigned short *)PORTHIO_MASKA_TOGGLE)
#define pPORTHIO_MASKB		((volatile unsigned short *)PORTHIO_MASKB)
#define pPORTHIO_MASKB_CLEAR	((volatile unsigned short *)PORTHIO_MASKB_CLEAR)
#define pPORTHIO_MASKB_SET	((volatile unsigned short *)PORTHIO_MASKB_SET)
#define pPORTHIO_MASKB_TOGGLE	((volatile unsigned short *)PORTHIO_MASKB_TOGGLE)
#define pPORTHIO_DIR 		((volatile unsigned short *)PORTHIO_DIR)
#define pPORTHIO_POLAR 		((volatile unsigned short *)PORTHIO_POLAR)
#define pPORTHIO_EDGE 		((volatile unsigned short *)PORTHIO_EDGE)
#define pPORTHIO_BOTH 		((volatile unsigned short *)PORTHIO_BOTH)
#define pPORTHIO_INEN 		((volatile unsigned short *)PORTHIO_INEN)


/* UART1 Controller		(0xFFC02000 - 0xFFC020FF)								*/
#define pUART1_THR 		((volatile unsigned short *)UART1_THR)
#define pUART1_RBR 		((volatile unsigned short *)UART1_RBR)
#define pUART1_DLL 		((volatile unsigned short *)UART1_DLL)
#define pUART1_IER 		((volatile unsigned short *)UART1_IER)
#define pUART1_DLH 		((volatile unsigned short *)UART1_DLH)
#define pUART1_IIR 		((volatile unsigned short *)UART1_IIR)
#define pUART1_LCR 		((volatile unsigned short *)UART1_LCR)
#define pUART1_MCR 		((volatile unsigned short *)UART1_MCR)
#define pUART1_LSR 		((volatile unsigned short *)UART1_LSR)
#define pUART1_MSR		((volatile unsigned short *)UART1_LSR)
#define pUART1_SCR 		((volatile unsigned short *)UART1_SCR)
#define pUART1_GCTL 		((volatile unsigned short *)UART1_GCTL)

/* Omit CAN register sets from the cdefBF534.h (CAN is not in the ADSP-BF52x processor) */

/* Pin Control Registers	(0xFFC03200 - 0xFFC032FF)								*/
#define pPORTF_FER		((volatile unsigned short *)PORTF_FER)
#define pPORTG_FER		((volatile unsigned short *)PORTG_FER)
#define pPORTH_FER		((volatile unsigned short *)PORTH_FER)


/* Handshake MDMA Registers	(0xFFC03300 - 0xFFC033FF)								*/
#define pHMDMA0_CONTROL		((volatile unsigned short *)HMDMA0_CONTROL)
#define pHMDMA0_ECINIT		((volatile unsigned short *)HMDMA0_ECINIT)
#define pHMDMA0_BCINIT		((volatile unsigned short *)HMDMA0_BCINIT)
#define pHMDMA0_ECURGENT	((volatile unsigned short *)HMDMA0_ECURGENT)
#define pHMDMA0_ECOVERFLOW	((volatile unsigned short *)HMDMA0_ECOVERFLOW)
#define pHMDMA0_ECOUNT		((volatile unsigned short *)HMDMA0_ECOUNT)
#define pHMDMA0_BCOUNT		((volatile unsigned short *)HMDMA0_BCOUNT)

#define pHMDMA1_CONTROL		((volatile unsigned short *)HMDMA1_CONTROL)
#define pHMDMA1_ECINIT		((volatile unsigned short *)HMDMA1_ECINIT)
#define pHMDMA1_BCINIT		((volatile unsigned short *)HMDMA1_BCINIT)
#define pHMDMA1_ECURGENT	((volatile unsigned short *)HMDMA1_ECURGENT)
#define pHMDMA1_ECOVERFLOW	((volatile unsigned short *)HMDMA1_ECOVERFLOW)
#define pHMDMA1_ECOUNT		((volatile unsigned short *)HMDMA1_ECOUNT)
#define pHMDMA1_BCOUNT		((volatile unsigned short *)HMDMA1_BCOUNT)

/* ==== end from cdefBF534.h ==== */

/* GPIO PIN mux (0xFFC03210 - OxFFC03288) */

#define                        pPORTF_MUX ((volatile unsigned short *)PORTF_MUX)
#define                        pPORTG_MUX ((volatile unsigned short *)PORTG_MUX)
#define                        pPORTH_MUX ((volatile unsigned short *)PORTH_MUX)

#define                      pPORTF_DRIVE ((volatile unsigned short *)PORTF_DRIVE)
#define                      pPORTG_DRIVE ((volatile unsigned short *)PORTG_DRIVE)
#define                      pPORTH_DRIVE ((volatile unsigned short *)PORTH_DRIVE)
#define                       pPORTF_SLEW ((volatile unsigned short *)PORTF_SLEW)
#define                       pPORTG_SLEW ((volatile unsigned short *)PORTG_SLEW)
#define                       pPORTH_SLEW ((volatile unsigned short *)PORTH_SLEW)
#define                 pPORTF_HYSTERESIS ((volatile unsigned short *)PORTF_HYSTERESIS)
#define                 pPORTG_HYSTERESIS ((volatile unsigned short *)PORTG_HYSTERESIS)
#define                 pPORTH_HYSTERESIS ((volatile unsigned short *)PORTH_HYSTERESIS)
#define                   pNONGPIO_DRIVE ((volatile unsigned short *)NONGPIO_DRIVE)
#define                    pNONGPIO_SLEW ((volatile unsigned short *)NONGPIO_SLEW)
#define              pNONGPIO_HYSTERESIS ((volatile unsigned short *)NONGPIO_HYSTERESIS)

/* HOST Port Registers */

#define                     pHOST_CONTROL ((volatile unsigned short *)HOST_CONTROL)
#define                      pHOST_STATUS ((volatile unsigned short *)HOST_STATUS)
#define                     pHOST_TIMEOUT ((volatile unsigned short *)HOST_TIMEOUT)

/* Counter Registers */

#define                       pCNT_CONFIG ((volatile unsigned short *)CNT_CONFIG)
#define                        pCNT_IMASK ((volatile unsigned short *)CNT_IMASK)
#define                       pCNT_STATUS ((volatile unsigned short *)CNT_STATUS)
#define                      pCNT_COMMAND ((volatile unsigned short *)CNT_COMMAND)
#define                     pCNT_DEBOUNCE ((volatile unsigned short *)CNT_DEBOUNCE)
#define                      pCNT_COUNTER ((volatile unsigned long *)CNT_COUNTER)
#define                          pCNT_MAX ((volatile unsigned long *)CNT_MAX)
#define                          pCNT_MIN ((volatile unsigned long *)CNT_MIN)

/* OTP/FUSE Registers */

#define                      pOTP_CONTROL ((volatile unsigned short *)OTP_CONTROL)
#define                          pOTP_BEN ((volatile unsigned short *)OTP_BEN)
#define                       pOTP_STATUS ((volatile unsigned short *)OTP_STATUS)
#define                       pOTP_TIMING ((volatile unsigned long *)OTP_TIMING)

/* Security Registers */

#define                    pSECURE_SYSSWT ((volatile unsigned long *)SECURE_SYSSWT)
#define                   pSECURE_CONTROL ((volatile unsigned short *)SECURE_CONTROL)
#define                    pSECURE_STATUS ((volatile unsigned short *)SECURE_STATUS)

/* OTP Read/Write Data Buffer Registers */

#define                        pOTP_DATA0 ((volatile unsigned long *)OTP_DATA0)
#define                        pOTP_DATA1 ((volatile unsigned long *)OTP_DATA1)
#define                        pOTP_DATA2 ((volatile unsigned long *)OTP_DATA2)
#define                        pOTP_DATA3 ((volatile unsigned long *)OTP_DATA3)

/* NFC Registers */

#define                          pNFC_CTL ((volatile unsigned short *)NFC_CTL)
#define                         pNFC_STAT ((volatile unsigned short *)NFC_STAT)
#define                      pNFC_IRQSTAT ((volatile unsigned short *)NFC_IRQSTAT)
#define                      pNFC_IRQMASK ((volatile unsigned short *)NFC_IRQMASK)
#define                         pNFC_ECC0 ((volatile unsigned short *)NFC_ECC0)
#define                         pNFC_ECC1 ((volatile unsigned short *)NFC_ECC1)
#define                         pNFC_ECC2 ((volatile unsigned short *)NFC_ECC2)
#define                         pNFC_ECC3 ((volatile unsigned short *)NFC_ECC3)
#define                        pNFC_COUNT ((volatile unsigned short *)NFC_COUNT)
#define                          pNFC_RST ((volatile unsigned short *)NFC_RST)
#define                        pNFC_PGCTL ((volatile unsigned short *)NFC_PGCTL)
#define                         pNFC_READ ((volatile unsigned short *)NFC_READ)
#define                         pNFC_ADDR ((volatile unsigned short *)NFC_ADDR)
#define                          pNFC_CMD ((volatile unsigned short *)NFC_CMD)
#define                      pNFC_DATA_WR ((volatile unsigned short *)NFC_DATA_WR)
#define                      pNFC_DATA_RD ((volatile unsigned short *)NFC_DATA_RD)

#endif /* _CDEF_BF52X_H */
