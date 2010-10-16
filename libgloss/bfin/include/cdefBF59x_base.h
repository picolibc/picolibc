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
** cdefBF59x_base.h
**
** Copyright (C) 2009 Analog Devices Inc., All Rights Reserved.
**
************************************************************************************
**
** This include file contains a list of macro "defines" to enable the programmer
** to use symbolic names for the registers common to the ADSP-BF59x peripherals.
**
***************************************************************/

#ifndef _CDEF_BF59x_H
#define _CDEF_BF59x_H

#include <defBF59x_base.h>
#include <stdint.h>

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


/* Clock and System Control 	(0xFFC00000 - 0xFFC000FF) */
#define pPLL_CTL 			((volatile uint16_t *)PLL_CTL)
#define pPLL_DIV 			((volatile uint16_t *)PLL_DIV)
#define pVR_CTL 			((volatile uint16_t *)VR_CTL)
#define pPLL_STAT 		((volatile uint16_t *)PLL_STAT)
#define pPLL_LOCKCNT 		((volatile uint16_t *)PLL_LOCKCNT)
#define pCHIPID			((volatile uint32_t *)CHIPID)
#define pAUX_REVID			((volatile uint32_t *)AUX_REVID)


/* System Interrupt Controller(0xFFC00100 - 0xFFC001FF) */
#define pSWRST 			((volatile uint16_t *)SWRST)
#define pSYSCR 			((volatile uint16_t *)SYSCR)

#define pSIC_IMASK0 	((volatile uint32_t *)SIC_IMASK0)
#define pSIC_IAR0 		((volatile uint32_t *)SIC_IAR0)
#define pSIC_IAR1 		((volatile uint32_t *)SIC_IAR1)
#define pSIC_IAR2 		((volatile uint32_t *)SIC_IAR2)
#define pSIC_IAR3 		((volatile uint32_t *)SIC_IAR3)
#define pSIC_ISR0 		((volatile uint32_t *)SIC_ISR0)
#define pSIC_IWR0 		((volatile uint32_t *)SIC_IWR0)

/* legacy register name (below) provided for backwards code compatibility */
#define pSIC_IMASK 		((volatile uint32_t *)SIC_IMASK0)
/* legacy register name (below) provided for backwards code compatibility */
#define pSIC_ISR 		((volatile uint32_t *)SIC_ISR0)
/* legacy register name (below) provided for backwards code compatibility */
#define pSIC_IWR 		((volatile uint32_t *)SIC_IWR0)


/* Watchdog Timer 		(0xFFC00200 - 0xFFC002FF) */
#define pWDOG_CTL 		((volatile uint16_t *)WDOG_CTL)
#define pWDOG_CNT 		((volatile uint32_t *)WDOG_CNT)
#define pWDOG_STAT 		((volatile uint32_t *)WDOG_STAT)


/* UART0 Controller 		(0xFFC00400 - 0xFFC004FF) */
#define pUART0_THR 		((volatile uint16_t *)UART0_THR)
#define pUART0_RBR 		((volatile uint16_t *)UART0_RBR)
#define pUART0_DLL 		((volatile uint16_t *)UART0_DLL)
#define pUART0_IER 		((volatile uint16_t *)UART0_IER)
#define pUART0_DLH 		((volatile uint16_t *)UART0_DLH)
#define pUART0_IIR 		((volatile uint16_t *)UART0_IIR)
#define pUART0_LCR 		((volatile uint16_t *)UART0_LCR)
#define pUART0_MCR 		((volatile uint16_t *)UART0_MCR)
#define pUART0_LSR 		((volatile uint16_t *)UART0_LSR)
#define pUART0_SCR 		((volatile uint16_t *)UART0_SCR)
#define pUART0_GCTL 	((volatile uint16_t *)UART0_GCTL)


/* SPI0 Controller  		(0xFFC00500 - 0xFFC005FF)*/
#define pSPI0_CTL 		((volatile uint16_t *)SPI0_CTL)
#define pSPI0_FLG 		((volatile uint16_t *)SPI0_FLG)
#define pSPI0_STAT 		((volatile uint16_t *)SPI0_STAT)
#define pSPI0_TDBR 		((volatile uint16_t *)SPI0_TDBR)
#define pSPI0_RDBR 		((volatile uint16_t *)SPI0_RDBR)
#define pSPI0_BAUD 		((volatile uint16_t *)SPI0_BAUD)
#define pSPI0_SHADOW 	((volatile uint16_t *)SPI0_SHADOW)


/* SPI1 Controller 		(0xFFC01300 - 0xFFC013FF)*/
#define pSPI1_CTL 		((volatile uint16_t *)SPI1_CTL)
#define pSPI1_FLG 		((volatile uint16_t *)SPI1_FLG)
#define pSPI1_STAT 		((volatile uint16_t *)SPI1_STAT)
#define pSPI1_TDBR 		((volatile uint16_t *)SPI1_TDBR)
#define pSPI1_RDBR 		((volatile uint16_t *)SPI1_RDBR)
#define pSPI1_BAUD 		((volatile uint16_t *)SPI1_BAUD)
#define pSPI1_SHADOW 	((volatile uint16_t *)SPI1_SHADOW)


/* TIMER0-2 Registers 		(0xFFC00600 - 0xFFC006FF) */
#define pTIMER0_CONFIG 		((volatile uint16_t *)TIMER0_CONFIG)
#define pTIMER0_COUNTER 	((volatile uint32_t *)TIMER0_COUNTER)
#define pTIMER0_PERIOD 		((volatile uint32_t *)TIMER0_PERIOD)
#define pTIMER0_WIDTH 		((volatile uint32_t *)TIMER0_WIDTH)

#define pTIMER1_CONFIG 		((volatile uint16_t *)TIMER1_CONFIG)
#define pTIMER1_COUNTER 	((volatile uint32_t *)TIMER1_COUNTER)
#define pTIMER1_PERIOD 		((volatile uint32_t *)TIMER1_PERIOD)
#define pTIMER1_WIDTH 		((volatile uint32_t *)TIMER1_WIDTH)

#define pTIMER2_CONFIG 		((volatile uint16_t *)TIMER2_CONFIG)
#define pTIMER2_COUNTER 	((volatile uint32_t *)TIMER2_COUNTER)
#define pTIMER2_PERIOD 		((volatile uint32_t *)TIMER2_PERIOD)
#define pTIMER2_WIDTH 		((volatile uint32_t *)TIMER2_WIDTH)

#define pTIMER_ENABLE 		((volatile uint16_t *)TIMER_ENABLE)
#define pTIMER_DISABLE 		((volatile uint16_t *)TIMER_DISABLE)
#define pTIMER_STATUS		((volatile uint16_t *)TIMER_STATUS)


/* General Purpose I/O Port F (0xFFC00700 - 0xFFC007FF) */
#define pPORTFIO 			((volatile uint16_t *)PORTFIO)
#define pPORTFIO_CLEAR	 	((volatile uint16_t *)PORTFIO_CLEAR)
#define pPORTFIO_SET	 	((volatile uint16_t *)PORTFIO_SET)
#define pPORTFIO_TOGGLE 	((volatile uint16_t *)PORTFIO_TOGGLE)
#define pPORTFIO_MASKA 		((volatile uint16_t *)PORTFIO_MASKA)
#define pPORTFIO_MASKA_CLEAR 	((volatile uint16_t *)PORTFIO_MASKA_CLEAR)
#define pPORTFIO_MASKA_SET 	((volatile uint16_t *)PORTFIO_MASKA_SET)
#define pPORTFIO_MASKA_TOGGLE ((volatile uint16_t *)PORTFIO_MASKA_TOGGLE)
#define pPORTFIO_MASKB	 	((volatile uint16_t *)PORTFIO_MASKB)
#define pPORTFIO_MASKB_CLEAR 	((volatile uint16_t *)PORTFIO_MASKB_CLEAR)
#define pPORTFIO_MASKB_SET 	((volatile uint16_t *)PORTFIO_MASKB_SET)
#define pPORTFIO_MASKB_TOGGLE ((volatile uint16_t *)PORTFIO_MASKB_TOGGLE)
#define pPORTFIO_DIR 		((volatile uint16_t *)PORTFIO_DIR)
#define pPORTFIO_POLAR 		((volatile uint16_t *)PORTFIO_POLAR)
#define pPORTFIO_EDGE 		((volatile uint16_t *)PORTFIO_EDGE)
#define pPORTFIO_BOTH 		((volatile uint16_t *)PORTFIO_BOTH)
#define pPORTFIO_INEN 		((volatile uint16_t *)PORTFIO_INEN)

/* General Purpose I/O Port G (0xFFC01500 - 0xFFC015FF) */
#define pPORTGIO	 		((volatile uint16_t *)PORTGIO)
#define pPORTGIO_CLEAR	 	((volatile uint16_t *)PORTGIO_CLEAR)
#define pPORTGIO_SET	 	((volatile uint16_t *)PORTGIO_SET)
#define pPORTGIO_TOGGLE 	((volatile uint16_t *)PORTGIO_TOGGLE)
#define pPORTGIO_MASKA		((volatile uint16_t *)PORTGIO_MASKA)
#define pPORTGIO_MASKA_CLEAR	((volatile uint16_t *)PORTGIO_MASKA_CLEAR)
#define pPORTGIO_MASKA_SET	((volatile uint16_t *)PORTGIO_MASKA_SET)
#define pPORTGIO_MASKA_TOGGLE	((volatile uint16_t *)PORTGIO_MASKA_TOGGLE)
#define pPORTGIO_MASKB		((volatile uint16_t *)PORTGIO_MASKB)
#define pPORTGIO_MASKB_CLEAR	((volatile uint16_t *)PORTGIO_MASKB_CLEAR)
#define pPORTGIO_MASKB_SET	((volatile uint16_t *)PORTGIO_MASKB_SET)
#define pPORTGIO_MASKB_TOGGLE	((volatile uint16_t *)PORTGIO_MASKB_TOGGLE)
#define pPORTGIO_DIR 		((volatile uint16_t *)PORTGIO_DIR)
#define pPORTGIO_POLAR 		((volatile uint16_t *)PORTGIO_POLAR)
#define pPORTGIO_EDGE 		((volatile uint16_t *)PORTGIO_EDGE)
#define pPORTGIO_BOTH 		((volatile uint16_t *)PORTGIO_BOTH)
#define pPORTGIO_INEN 		((volatile uint16_t *)PORTGIO_INEN)


/* Pin Control Registers 	(0xFFC01100 - 0xFFC012FF) */
#define pPORTF_FER			((volatile uint16_t *)PORTF_FER)
#define pPORTF_MUX 			((volatile uint16_t *)PORTF_MUX)
#define pPORTF_PADCTL 		((volatile uint16_t *)PORTF_PADCTL)
#define pPORTG_FER			((volatile uint16_t *)PORTG_FER)
#define pPORTG_MUX 			((volatile uint16_t *)PORTG_MUX)
#define pPORTG_PADCTL 		((volatile uint16_t *)PORTG_PADCTL)

/* SPORT Clock Gating			(0xFFC0120C) */
#define pSPORT_GATECLK 		((volatile uint16_t *)SPORT_GATECLK)

/* SPORT0 Controller 		(0xFFC00800 - 0xFFC008FF) */
#define pSPORT0_TCR1 		((volatile uint16_t *)SPORT0_TCR1)
#define pSPORT0_TCR2 		((volatile uint16_t *)SPORT0_TCR2)
#define pSPORT0_TCLKDIV 	((volatile uint16_t *)SPORT0_TCLKDIV)
#define pSPORT0_TFSDIV 		((volatile uint16_t *)SPORT0_TFSDIV)
#define pSPORT0_TX 			((volatile uint32_t *)SPORT0_TX)
#define pSPORT0_RX 			((volatile uint32_t *)SPORT0_RX)
#define pSPORT0_TX32 		((volatile uint32_t *)SPORT0_TX)
#define pSPORT0_RX32 		((volatile uint32_t *)SPORT0_RX)
#define pSPORT0_TX16 		((volatile uint16_t *)SPORT0_TX)
#define pSPORT0_RX16 		((volatile uint16_t *)SPORT0_RX)
#define pSPORT0_RCR1 		((volatile uint16_t *)SPORT0_RCR1)
#define pSPORT0_RCR2 		((volatile uint16_t *)SPORT0_RCR2)
#define pSPORT0_RCLKDIV 	((volatile uint16_t *)SPORT0_RCLKDIV)
#define pSPORT0_RFSDIV 		((volatile uint16_t *)SPORT0_RFSDIV)
#define pSPORT0_STAT 		((volatile uint16_t *)SPORT0_STAT)
#define pSPORT0_CHNL 		((volatile uint16_t *)SPORT0_CHNL)
#define pSPORT0_MCMC1 		((volatile uint16_t *)SPORT0_MCMC1)
#define pSPORT0_MCMC2 		((volatile uint16_t *)SPORT0_MCMC2)
#define pSPORT0_MTCS0 		((volatile uint32_t *)SPORT0_MTCS0)
#define pSPORT0_MTCS1 		((volatile uint32_t *)SPORT0_MTCS1)
#define pSPORT0_MTCS2 		((volatile uint32_t *)SPORT0_MTCS2)
#define pSPORT0_MTCS3 		((volatile uint32_t *)SPORT0_MTCS3)
#define pSPORT0_MRCS0 		((volatile uint32_t *)SPORT0_MRCS0)
#define pSPORT0_MRCS1 		((volatile uint32_t *)SPORT0_MRCS1)
#define pSPORT0_MRCS2 		((volatile uint32_t *)SPORT0_MRCS2)
#define pSPORT0_MRCS3 		((volatile uint32_t *)SPORT0_MRCS3)


/* SPORT1 Controller		(0xFFC00900 - 0xFFC009FF) */
#define pSPORT1_TCR1 		((volatile uint16_t *)SPORT1_TCR1)
#define pSPORT1_TCR2 		((volatile uint16_t *)SPORT1_TCR2)
#define pSPORT1_TCLKDIV 	((volatile uint16_t *)SPORT1_TCLKDIV)
#define pSPORT1_TFSDIV 		((volatile uint16_t *)SPORT1_TFSDIV)
#define pSPORT1_TX 		((volatile uint32_t *)SPORT1_TX)
#define pSPORT1_RX 		((volatile uint32_t *)SPORT1_RX)
#define pSPORT1_TX32 		((volatile uint32_t *)SPORT1_TX)
#define pSPORT1_RX32 		((volatile uint32_t *)SPORT1_RX)
#define pSPORT1_TX16 		((volatile uint16_t *)SPORT1_TX)
#define pSPORT1_RX16 		((volatile uint16_t *)SPORT1_RX)
#define pSPORT1_RCR1 		((volatile uint16_t *)SPORT1_RCR1)
#define pSPORT1_RCR2 		((volatile uint16_t *)SPORT1_RCR2)
#define pSPORT1_RCLKDIV 	((volatile uint16_t *)SPORT1_RCLKDIV)
#define pSPORT1_RFSDIV 		((volatile uint16_t *)SPORT1_RFSDIV)
#define pSPORT1_STAT 		((volatile uint16_t *)SPORT1_STAT)
#define pSPORT1_CHNL 		((volatile uint16_t *)SPORT1_CHNL)
#define pSPORT1_MCMC1 		((volatile uint16_t *)SPORT1_MCMC1)
#define pSPORT1_MCMC2 		((volatile uint16_t *)SPORT1_MCMC2)
#define pSPORT1_MTCS0 		((volatile uint32_t *)SPORT1_MTCS0)
#define pSPORT1_MTCS1 		((volatile uint32_t *)SPORT1_MTCS1)
#define pSPORT1_MTCS2 		((volatile uint32_t *)SPORT1_MTCS2)
#define pSPORT1_MTCS3 		((volatile uint32_t *)SPORT1_MTCS3)
#define pSPORT1_MRCS0 		((volatile uint32_t *)SPORT1_MRCS0)
#define pSPORT1_MRCS1 		((volatile uint32_t *)SPORT1_MRCS1)
#define pSPORT1_MRCS2 		((volatile uint32_t *)SPORT1_MRCS2)
#define pSPORT1_MRCS3 		((volatile uint32_t *)SPORT1_MRCS3)


/* DMA Traffic Control Registers	(0xFFC00B00 - 0xFFC00BFF) */
#define	pDMA_TC_PER		((volatile uint16_t *)DMA_TC_PER)
#define	pDMA_TC_CNT		((volatile uint16_t *)DMA_TC_CNT)

/* Alternate deprecated register names (below) provided for backwards code compatibility */
#define	pDMA_TCPER		((volatile uint16_t *)DMA_TCPER)
#define	pDMA_TCCNT		((volatile uint16_t *)DMA_TCCNT)

/* DMA Controller			(0xFFC00C00 - FFC00FFF)*/
#define pDMA0_CONFIG 		((volatile uint16_t *)DMA0_CONFIG)
#define pDMA0_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA0_NEXT_DESC_PTR)
#define pDMA0_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA0_START_ADDR)
#define pDMA0_X_COUNT 		((volatile uint16_t *)DMA0_X_COUNT)
#define pDMA0_Y_COUNT 		((volatile uint16_t *)DMA0_Y_COUNT)
#define pDMA0_X_MODIFY 		((volatile signed   short *)DMA0_X_MODIFY)
#define pDMA0_Y_MODIFY 		((volatile signed   short *)DMA0_Y_MODIFY)
#define pDMA0_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA0_CURR_DESC_PTR)
#define pDMA0_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA0_CURR_ADDR)
#define pDMA0_CURR_X_COUNT 	((volatile uint16_t *)DMA0_CURR_X_COUNT)
#define pDMA0_CURR_Y_COUNT 	((volatile uint16_t *)DMA0_CURR_Y_COUNT)
#define pDMA0_IRQ_STATUS 	((volatile uint16_t *)DMA0_IRQ_STATUS)
#define pDMA0_PERIPHERAL_MAP 	((volatile uint16_t *)DMA0_PERIPHERAL_MAP)

#define pDMA1_CONFIG 		((volatile uint16_t *)DMA1_CONFIG)
#define pDMA1_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA1_NEXT_DESC_PTR)
#define pDMA1_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA1_START_ADDR)
#define pDMA1_X_COUNT 		((volatile uint16_t *)DMA1_X_COUNT)
#define pDMA1_Y_COUNT 		((volatile uint16_t *)DMA1_Y_COUNT)
#define pDMA1_X_MODIFY 		((volatile signed   short *)DMA1_X_MODIFY)
#define pDMA1_Y_MODIFY 		((volatile signed   short *)DMA1_Y_MODIFY)
#define pDMA1_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA1_CURR_DESC_PTR)
#define pDMA1_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA1_CURR_ADDR)
#define pDMA1_CURR_X_COUNT 	((volatile uint16_t *)DMA1_CURR_X_COUNT)
#define pDMA1_CURR_Y_COUNT 	((volatile uint16_t *)DMA1_CURR_Y_COUNT)
#define pDMA1_IRQ_STATUS 	((volatile uint16_t *)DMA1_IRQ_STATUS)
#define pDMA1_PERIPHERAL_MAP 	((volatile uint16_t *)DMA1_PERIPHERAL_MAP)

#define pDMA2_CONFIG 		((volatile uint16_t *)DMA2_CONFIG)
#define pDMA2_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA2_NEXT_DESC_PTR)
#define pDMA2_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA2_START_ADDR)
#define pDMA2_X_COUNT 		((volatile uint16_t *)DMA2_X_COUNT)
#define pDMA2_Y_COUNT 		((volatile uint16_t *)DMA2_Y_COUNT)
#define pDMA2_X_MODIFY 		((volatile signed   short *)DMA2_X_MODIFY)
#define pDMA2_Y_MODIFY 		((volatile signed   short *)DMA2_Y_MODIFY)
#define pDMA2_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA2_CURR_DESC_PTR)
#define pDMA2_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA2_CURR_ADDR)
#define pDMA2_CURR_X_COUNT 	((volatile uint16_t *)DMA2_CURR_X_COUNT)
#define pDMA2_CURR_Y_COUNT 	((volatile uint16_t *)DMA2_CURR_Y_COUNT)
#define pDMA2_IRQ_STATUS 	((volatile uint16_t *)DMA2_IRQ_STATUS)
#define pDMA2_PERIPHERAL_MAP 	((volatile uint16_t *)DMA2_PERIPHERAL_MAP)

#define pDMA3_CONFIG 		((volatile uint16_t *)DMA3_CONFIG)
#define pDMA3_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA3_NEXT_DESC_PTR)
#define pDMA3_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA3_START_ADDR)
#define pDMA3_X_COUNT 		((volatile uint16_t *)DMA3_X_COUNT)
#define pDMA3_Y_COUNT 		((volatile uint16_t *)DMA3_Y_COUNT)
#define pDMA3_X_MODIFY 		((volatile signed   short *)DMA3_X_MODIFY)
#define pDMA3_Y_MODIFY 		((volatile signed   short *)DMA3_Y_MODIFY)
#define pDMA3_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA3_CURR_DESC_PTR)
#define pDMA3_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA3_CURR_ADDR)
#define pDMA3_CURR_X_COUNT 	((volatile uint16_t *)DMA3_CURR_X_COUNT)
#define pDMA3_CURR_Y_COUNT 	((volatile uint16_t *)DMA3_CURR_Y_COUNT)
#define pDMA3_IRQ_STATUS 	((volatile uint16_t *)DMA3_IRQ_STATUS)
#define pDMA3_PERIPHERAL_MAP 	((volatile uint16_t *)DMA3_PERIPHERAL_MAP)

#define pDMA4_CONFIG 		((volatile uint16_t *)DMA4_CONFIG)
#define pDMA4_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA4_NEXT_DESC_PTR)
#define pDMA4_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA4_START_ADDR)
#define pDMA4_X_COUNT 		((volatile uint16_t *)DMA4_X_COUNT)
#define pDMA4_Y_COUNT 		((volatile uint16_t *)DMA4_Y_COUNT)
#define pDMA4_X_MODIFY 		((volatile signed   short *)DMA4_X_MODIFY)
#define pDMA4_Y_MODIFY 		((volatile signed   short *)DMA4_Y_MODIFY)
#define pDMA4_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA4_CURR_DESC_PTR)
#define pDMA4_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA4_CURR_ADDR)
#define pDMA4_CURR_X_COUNT 	((volatile uint16_t *)DMA4_CURR_X_COUNT)
#define pDMA4_CURR_Y_COUNT 	((volatile uint16_t *)DMA4_CURR_Y_COUNT)
#define pDMA4_IRQ_STATUS 	((volatile uint16_t *)DMA4_IRQ_STATUS)
#define pDMA4_PERIPHERAL_MAP 	((volatile uint16_t *)DMA4_PERIPHERAL_MAP)

#define pDMA5_CONFIG 		((volatile uint16_t *)DMA5_CONFIG)
#define pDMA5_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA5_NEXT_DESC_PTR)
#define pDMA5_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA5_START_ADDR)
#define pDMA5_X_COUNT 		((volatile uint16_t *)DMA5_X_COUNT)
#define pDMA5_Y_COUNT 		((volatile uint16_t *)DMA5_Y_COUNT)
#define pDMA5_X_MODIFY 		((volatile signed   short *)DMA5_X_MODIFY)
#define pDMA5_Y_MODIFY 		((volatile signed   short *)DMA5_Y_MODIFY)
#define pDMA5_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA5_CURR_DESC_PTR)
#define pDMA5_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA5_CURR_ADDR)
#define pDMA5_CURR_X_COUNT 	((volatile uint16_t *)DMA5_CURR_X_COUNT)
#define pDMA5_CURR_Y_COUNT 	((volatile uint16_t *)DMA5_CURR_Y_COUNT)
#define pDMA5_IRQ_STATUS 	((volatile uint16_t *)DMA5_IRQ_STATUS)
#define pDMA5_PERIPHERAL_MAP 	((volatile uint16_t *)DMA5_PERIPHERAL_MAP)

#define pDMA6_CONFIG 		((volatile uint16_t *)DMA6_CONFIG)
#define pDMA6_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA6_NEXT_DESC_PTR)
#define pDMA6_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA6_START_ADDR)
#define pDMA6_X_COUNT 		((volatile uint16_t *)DMA6_X_COUNT)
#define pDMA6_Y_COUNT 		((volatile uint16_t *)DMA6_Y_COUNT)
#define pDMA6_X_MODIFY 		((volatile signed   short *)DMA6_X_MODIFY)
#define pDMA6_Y_MODIFY 		((volatile signed   short *)DMA6_Y_MODIFY)
#define pDMA6_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA6_CURR_DESC_PTR)
#define pDMA6_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA6_CURR_ADDR)
#define pDMA6_CURR_X_COUNT 	((volatile uint16_t *)DMA6_CURR_X_COUNT)
#define pDMA6_CURR_Y_COUNT 	((volatile uint16_t *)DMA6_CURR_Y_COUNT)
#define pDMA6_IRQ_STATUS 	((volatile uint16_t *)DMA6_IRQ_STATUS)
#define pDMA6_PERIPHERAL_MAP 	((volatile uint16_t *)DMA6_PERIPHERAL_MAP)

#define pDMA7_CONFIG 		((volatile uint16_t *)DMA7_CONFIG)
#define pDMA7_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA7_NEXT_DESC_PTR)
#define pDMA7_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA7_START_ADDR)
#define pDMA7_X_COUNT 		((volatile uint16_t *)DMA7_X_COUNT)
#define pDMA7_Y_COUNT 		((volatile uint16_t *)DMA7_Y_COUNT)
#define pDMA7_X_MODIFY 		((volatile signed   short *)DMA7_X_MODIFY)
#define pDMA7_Y_MODIFY 		((volatile signed   short *)DMA7_Y_MODIFY)
#define pDMA7_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA7_CURR_DESC_PTR)
#define pDMA7_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA7_CURR_ADDR)
#define pDMA7_CURR_X_COUNT 	((volatile uint16_t *)DMA7_CURR_X_COUNT)
#define pDMA7_CURR_Y_COUNT 	((volatile uint16_t *)DMA7_CURR_Y_COUNT)
#define pDMA7_IRQ_STATUS 	((volatile uint16_t *)DMA7_IRQ_STATUS)
#define pDMA7_PERIPHERAL_MAP 	((volatile uint16_t *)DMA7_PERIPHERAL_MAP)

#define pDMA8_CONFIG 		((volatile uint16_t *)DMA8_CONFIG)
#define pDMA8_NEXT_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA8_NEXT_DESC_PTR)
#define pDMA8_START_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA8_START_ADDR)
#define pDMA8_X_COUNT 		((volatile uint16_t *)DMA8_X_COUNT)
#define pDMA8_Y_COUNT 		((volatile uint16_t *)DMA8_Y_COUNT)
#define pDMA8_X_MODIFY 		((volatile signed   short *)DMA8_X_MODIFY)
#define pDMA8_Y_MODIFY 		((volatile signed   short *)DMA8_Y_MODIFY)
#define pDMA8_CURR_DESC_PTR 	(_PTR_TO_VOL_VOID_PTR DMA8_CURR_DESC_PTR)
#define pDMA8_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR DMA8_CURR_ADDR)
#define pDMA8_CURR_X_COUNT 	((volatile uint16_t *)DMA8_CURR_X_COUNT)
#define pDMA8_CURR_Y_COUNT 	((volatile uint16_t *)DMA8_CURR_Y_COUNT)
#define pDMA8_IRQ_STATUS 	((volatile uint16_t *)DMA8_IRQ_STATUS)
#define pDMA8_PERIPHERAL_MAP 	((volatile uint16_t *)DMA8_PERIPHERAL_MAP)

#define pMDMA_D0_CONFIG 	((volatile uint16_t *)MDMA_D0_CONFIG)
#define pMDMA_D0_NEXT_DESC_PTR  (_PTR_TO_VOL_VOID_PTR MDMA_D0_NEXT_DESC_PTR)
#define pMDMA_D0_START_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_D0_START_ADDR)
#define pMDMA_D0_X_COUNT 	((volatile uint16_t *)MDMA_D0_X_COUNT)
#define pMDMA_D0_Y_COUNT 	((volatile uint16_t *)MDMA_D0_Y_COUNT)
#define pMDMA_D0_X_MODIFY 	((volatile signed   short *)MDMA_D0_X_MODIFY)
#define pMDMA_D0_Y_MODIFY 	((volatile signed   short *)MDMA_D0_Y_MODIFY)
#define pMDMA_D0_CURR_DESC_PTR  (_PTR_TO_VOL_VOID_PTR MDMA_D0_CURR_DESC_PTR)
#define pMDMA_D0_CURR_ADDR 	  (_PTR_TO_VOL_VOID_PTR MDMA_D0_CURR_ADDR)
#define pMDMA_D0_CURR_X_COUNT ((volatile uint16_t *)MDMA_D0_CURR_X_COUNT)
#define pMDMA_D0_CURR_Y_COUNT ((volatile uint16_t *)MDMA_D0_CURR_Y_COUNT)
#define pMDMA_D0_IRQ_STATUS 	((volatile uint16_t *)MDMA_D0_IRQ_STATUS)
#define pMDMA_D0_PERIPHERAL_MAP ((volatile uint16_t *)MDMA_D0_PERIPHERAL_MAP)

#define pMDMA_S0_CONFIG 	((volatile uint16_t *)MDMA_S0_CONFIG)
#define pMDMA_S0_NEXT_DESC_PTR  (_PTR_TO_VOL_VOID_PTR MDMA_S0_NEXT_DESC_PTR)
#define pMDMA_S0_START_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_S0_START_ADDR)
#define pMDMA_S0_X_COUNT 	((volatile uint16_t *)MDMA_S0_X_COUNT)
#define pMDMA_S0_Y_COUNT 	((volatile uint16_t *)MDMA_S0_Y_COUNT)
#define pMDMA_S0_X_MODIFY 	((volatile signed   short *)MDMA_S0_X_MODIFY)
#define pMDMA_S0_Y_MODIFY 	((volatile signed   short *)MDMA_S0_Y_MODIFY)
#define pMDMA_S0_CURR_DESC_PTR  (_PTR_TO_VOL_VOID_PTR MDMA_S0_CURR_DESC_PTR)
#define pMDMA_S0_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_S0_CURR_ADDR)
#define pMDMA_S0_CURR_X_COUNT ((volatile uint16_t *)MDMA_S0_CURR_X_COUNT)
#define pMDMA_S0_CURR_Y_COUNT ((volatile uint16_t *)MDMA_S0_CURR_Y_COUNT)
#define pMDMA_S0_IRQ_STATUS 	((volatile uint16_t *)MDMA_S0_IRQ_STATUS)
#define pMDMA_S0_PERIPHERAL_MAP ((volatile uint16_t *)MDMA_S0_PERIPHERAL_MAP)

#define pMDMA_D1_CONFIG 	((volatile uint16_t *)MDMA_D1_CONFIG)
#define pMDMA_D1_NEXT_DESC_PTR  (_PTR_TO_VOL_VOID_PTR MDMA_D1_NEXT_DESC_PTR)
#define pMDMA_D1_START_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_D1_START_ADDR)
#define pMDMA_D1_X_COUNT 	((volatile uint16_t *)MDMA_D1_X_COUNT)
#define pMDMA_D1_Y_COUNT 	((volatile uint16_t *)MDMA_D1_Y_COUNT)
#define pMDMA_D1_X_MODIFY 	((volatile signed   short *)MDMA_D1_X_MODIFY)
#define pMDMA_D1_Y_MODIFY 	((volatile signed   short *)MDMA_D1_Y_MODIFY)
#define pMDMA_D1_CURR_DESC_PTR  (_PTR_TO_VOL_VOID_PTR MDMA_D1_CURR_DESC_PTR)
#define pMDMA_D1_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_D1_CURR_ADDR)
#define pMDMA_D1_CURR_X_COUNT ((volatile uint16_t *)MDMA_D1_CURR_X_COUNT)
#define pMDMA_D1_CURR_Y_COUNT ((volatile uint16_t *)MDMA_D1_CURR_Y_COUNT)
#define pMDMA_D1_IRQ_STATUS 	((volatile uint16_t *)MDMA_D1_IRQ_STATUS)
#define pMDMA_D1_PERIPHERAL_MAP ((volatile uint16_t *)MDMA_D1_PERIPHERAL_MAP)

#define pMDMA_S1_CONFIG 	((volatile uint16_t *)MDMA_S1_CONFIG)
#define pMDMA_S1_NEXT_DESC_PTR  (_PTR_TO_VOL_VOID_PTR MDMA_S1_NEXT_DESC_PTR)
#define pMDMA_S1_START_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_S1_START_ADDR)
#define pMDMA_S1_X_COUNT 	((volatile uint16_t *)MDMA_S1_X_COUNT)
#define pMDMA_S1_Y_COUNT 	((volatile uint16_t *)MDMA_S1_Y_COUNT)
#define pMDMA_S1_X_MODIFY 	((volatile signed   short *)MDMA_S1_X_MODIFY)
#define pMDMA_S1_Y_MODIFY 	((volatile signed   short *)MDMA_S1_Y_MODIFY)
#define pMDMA_S1_CURR_DESC_PTR  (_PTR_TO_VOL_VOID_PTR MDMA_S1_CURR_DESC_PTR)
#define pMDMA_S1_CURR_ADDR 	(_PTR_TO_VOL_VOID_PTR MDMA_S1_CURR_ADDR)
#define pMDMA_S1_CURR_X_COUNT ((volatile uint16_t *)MDMA_S1_CURR_X_COUNT)
#define pMDMA_S1_CURR_Y_COUNT ((volatile uint16_t *)MDMA_S1_CURR_Y_COUNT)
#define pMDMA_S1_IRQ_STATUS 	((volatile uint16_t *)MDMA_S1_IRQ_STATUS)
#define pMDMA_S1_PERIPHERAL_MAP ((volatile uint16_t *)MDMA_S1_PERIPHERAL_MAP)


/* Parallel Peripheral Interface (0xFFC01000 - 0xFFC010FF) */
#define pPPI_CONTROL 		((volatile uint16_t *)PPI_CONTROL)
#define pPPI_STATUS 		((volatile uint16_t *)PPI_STATUS)
#define pPPI_DELAY 		((volatile uint16_t *)PPI_DELAY)
#define pPPI_COUNT 		((volatile uint16_t *)PPI_COUNT)
#define pPPI_FRAME 		((volatile uint16_t *)PPI_FRAME)


/* Two-Wire Interface 		(0xFFC01400 - 0xFFC014FF) */
#define pTWI_CLKDIV		((volatile uint16_t *)TWI_CLKDIV)
#define pTWI_CONTROL		((volatile uint16_t *)TWI_CONTROL)
#define pTWI_SLAVE_CTL		((volatile uint16_t *)TWI_SLAVE_CTL)
#define pTWI_SLAVE_STAT		((volatile uint16_t *)TWI_SLAVE_STAT)
#define pTWI_SLAVE_ADDR		((volatile uint16_t *)TWI_SLAVE_ADDR)
#define pTWI_MASTER_CTL		((volatile uint16_t *)TWI_MASTER_CTL)
#define pTWI_MASTER_STAT	((volatile uint16_t *)TWI_MASTER_STAT)
#define pTWI_MASTER_ADDR	((volatile uint16_t *)TWI_MASTER_ADDR)
#define pTWI_INT_STAT		((volatile uint16_t *)TWI_INT_STAT)
#define pTWI_INT_MASK		((volatile uint16_t *)TWI_INT_MASK)
#define pTWI_FIFO_CTL		((volatile uint16_t *)TWI_FIFO_CTL)
#define pTWI_FIFO_STAT		((volatile uint16_t *)TWI_FIFO_STAT)
#define pTWI_XMT_DATA8		((volatile uint16_t *)TWI_XMT_DATA8)
#define pTWI_XMT_DATA16		((volatile uint16_t *)TWI_XMT_DATA16)
#define pTWI_RCV_DATA8		((volatile uint16_t *)TWI_RCV_DATA8)
#define pTWI_RCV_DATA16		((volatile uint16_t *)TWI_RCV_DATA16)


#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */


#endif  /*_CDEF_BF59x_H*/
