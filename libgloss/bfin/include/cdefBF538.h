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
 * cdefBF538.h
 *
 * (c) Copyright 2006-2009 Analog Devices, Inc.  All rights reserved.
 *
 ************************************************************************/

/* C POINTERS TO SYSTEM MMR REGISTER AND MEMORY MAP FOR ADSP-BF538 */

#ifndef _CDEF_BF538_H
#define _CDEF_BF538_H

/* include all Core registers and bit definitions */
#include <defBF538.h>

/* include core specific register pointer definitions */
#include <cdef_LPBlackfin.h>

/* include common system register pointer definitions from ADSP-BF532 */
#include <cdefBF532.h>

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_19_4:"some macro definitions not MISRA compliant")
#pragma diag(suppress:misra_rule_19_7:"ADI header allows function macros")
#endif /* _MISRA_RULES */

/* System Interrupt Controller (0xFFC00100 - 0xFFC001FF)				*/
/* ADSP-BF538 SIC0 is same as SIC on ADSP-BF532 */
#define pSIC_IMASK0				pSIC_IMASK
#define pSIC_ISR0 				pSIC_ISR
#define pSIC_IWR0 				pSIC_IWR
/* ADSP-BF538 SIC1 Registers */
#define pSIC_IMASK1				((volatile unsigned long  *)SIC_IMASK1)
#define pSIC_ISR1 				((volatile unsigned long  *)SIC_ISR1)
#define pSIC_IWR1 				((volatile unsigned long  *)SIC_IWR1)
#define pSIC_IAR3 				((volatile unsigned long  *)SIC_IAR3)
#define pSIC_IAR4 				((volatile unsigned long  *)SIC_IAR4)
#define pSIC_IAR5 				((volatile unsigned long  *)SIC_IAR5)
#define pSIC_IAR6 				((volatile unsigned long  *)SIC_IAR6)


/* UART0 Controller								*/
/* ADSP-BF538 UART0 is same as UART on ADSP-BF532 */
#define pUART0_THR 				pUART_THR
#define pUART0_RBR 				pUART_RBR
#define pUART0_DLL 				pUART_DLL
#define pUART0_IER 				pUART_IER
#define pUART0_DLH 				pUART_DLH
#define pUART0_IIR 				pUART_IIR
#define pUART0_LCR 				pUART_LCR
#define pUART0_MCR 				pUART_MCR
#define pUART0_LSR 				pUART_LSR
#define pUART0_SCR 				pUART_SCR
#define pUART0_GCTL 			pUART_GCTL


/* SPI0 Controller						*/
/* ADSP-BF538 SPI0 is same as SPI on ADSP-BF532 */
#define pSPI0_CTL 				pSPI_CTL
#define pSPI0_FLG 				pSPI_FLG
#define pSPI0_STAT 				pSPI_STAT
#define pSPI0_TDBR 				pSPI_TDBR
#define pSPI0_RDBR 				pSPI_RDBR
#define pSPI0_BAUD 				pSPI_BAUD
#define pSPI0_SHADOW 			pSPI_SHADOW


/* General-Purpose Input/Output Ports (GPIO)					*/
/* ADSP-BF538 Refers to FIO as GPIO Port F */
#define pPORTFIO 				pFIO_FLAG_D
#define pPORTFIO_CLEAR 			pFIO_FLAG_C
#define pPORTFIO_SET 			pFIO_FLAG_S
#define pPORTFIO_TOGGLE 		pFIO_FLAG_T
#define pPORTFIO_MASKA 			pFIO_MASKA_D
#define pPORTFIO_MASKA_CLEAR 	pFIO_MASKA_C
#define pPORTFIO_MASKA_SET 		pFIO_MASKA_S
#define pPORTFIO_MASKA_TOGGLE 	pFIO_MASKA_T
#define pPORTFIO_MASKB 			pFIO_MASKB_D
#define pPORTFIO_MASKB_CLEAR 	pFIO_MASKB_C
#define pPORTFIO_MASKB_SET 		pFIO_MASKB_S
#define pPORTFIO_MASKB_TOGGLE 	pFIO_MASKB_T
#define pPORTFIO_DIR 			pFIO_DIR
#define pPORTFIO_POLAR	 		pFIO_POLAR
#define pPORTFIO_EDGE 			pFIO_EDGE
#define pPORTFIO_BOTH 			pFIO_BOTH
#define pPORTFIO_INEN 			pFIO_INEN


/* DMA0 Traffic Control Registers											*/
/* ADSP-BF538 DMA0 Controller is same as DMA Controller on ADSP-BF532 */
#define	pDMAC0_TC_PER				pDMA_TC_PER
#define	pDMAC0_TC_CNT				pDMA_TC_CNT

/* Alternate deprecated register names (below) provided for backwards code compatibility */
#define	pDMA0_TC_PER				pDMAC0_TC_PER
#define	pDMA0_TC_CNT				pDMAC0_TC_CNT

/* Alternate deprecated register names (below) provided for backwards code compatibility */
#define pDMA0_TCPER					pDMA0_TC_PER	/* Traffic Control Periods Register			*/
#define pDMA0_TCCNT					pDMA0_TC_CNT	/* Traffic Control Current Counts Register	*/

/* Must Enumerate MemDMA Controllers As Well */
#define pMDMA0_D0_NEXT_DESC_PTR 	pMDMA_D0_NEXT_DESC_PTR
#define pMDMA0_D0_START_ADDR 		pMDMA_D0_START_ADDR
#define pMDMA0_D0_CONFIG 			pMDMA_D0_CONFIG
#define pMDMA0_D0_X_COUNT 			pMDMA_D0_X_COUNT
#define pMDMA0_D0_X_MODIFY 			pMDMA_D0_X_MODIFY
#define pMDMA0_D0_Y_COUNT 			pMDMA_D0_Y_COUNT
#define pMDMA0_D0_Y_MODIFY 			pMDMA_D0_Y_MODIFY
#define pMDMA0_D0_CURR_DESC_PTR 	pMDMA_D0_CURR_DESC_PTR
#define pMDMA0_D0_CURR_ADDR 		pMDMA_D0_CURR_ADDR
#define pMDMA0_D0_IRQ_STATUS 		pMDMA_D0_IRQ_STATUS
#define pMDMA0_D0_PERIPHERAL_MAP 	pMDMA_D0_PERIPHERAL_MAP
#define pMDMA0_D0_CURR_X_COUNT 		pMDMA_D0_CURR_X_COUNT
#define pMDMA0_D0_CURR_Y_COUNT 		pMDMA_D0_CURR_Y_COUNT

#define pMDMA0_S0_NEXT_DESC_PTR 	pMDMA_S0_NEXT_DESC_PTR
#define pMDMA0_S0_START_ADDR 		pMDMA_S0_START_ADDR
#define pMDMA0_S0_CONFIG 			pMDMA_S0_CONFIG
#define pMDMA0_S0_X_COUNT 			pMDMA_S0_X_COUNT
#define pMDMA0_S0_X_MODIFY 			pMDMA_S0_X_MODIFY
#define pMDMA0_S0_Y_COUNT 			pMDMA_S0_Y_COUNT
#define pMDMA0_S0_Y_MODIFY 			pMDMA_S0_Y_MODIFY
#define pMDMA0_S0_CURR_DESC_PTR 	pMDMA_S0_CURR_DESC_PTR
#define pMDMA0_S0_CURR_ADDR 		pMDMA_S0_CURR_ADDR
#define pMDMA0_S0_IRQ_STATUS 		pMDMA_S0_IRQ_STATUS
#define pMDMA0_S0_PERIPHERAL_MAP	pMDMA_S0_PERIPHERAL_MAP
#define pMDMA0_S0_CURR_X_COUNT 		pMDMA_S0_CURR_X_COUNT
#define pMDMA0_S0_CURR_Y_COUNT 		pMDMA_S0_CURR_Y_COUNT

#define pMDMA0_D1_NEXT_DESC_PTR 	pMDMA_D1_NEXT_DESC_PTR
#define pMDMA0_D1_START_ADDR 		pMDMA_D1_START_ADDR
#define pMDMA0_D1_CONFIG 			pMDMA_D1_CONFIG
#define pMDMA0_D1_X_COUNT 			pMDMA_D1_X_COUNT
#define pMDMA0_D1_X_MODIFY 			pMDMA_D1_X_MODIFY
#define pMDMA0_D1_Y_COUNT 			pMDMA_D1_Y_COUNT
#define pMDMA0_D1_Y_MODIFY 			pMDMA_D1_Y_MODIFY
#define pMDMA0_D1_CURR_DESC_PTR 	pMDMA_D1_CURR_DESC_PTR
#define pMDMA0_D1_CURR_ADDR 		pMDMA_D1_CURR_ADDR
#define pMDMA0_D1_IRQ_STATUS 		pMDMA_D1_IRQ_STATUS
#define pMDMA0_D1_PERIPHERAL_MAP 	pMDMA_D1_PERIPHERAL_MAP
#define pMDMA0_D1_CURR_X_COUNT 		pMDMA_D1_CURR_X_COUNT
#define pMDMA0_D1_CURR_Y_COUNT 		pMDMA_D1_CURR_Y_COUNT

#define pMDMA0_S1_NEXT_DESC_PTR 	pMDMA_S1_NEXT_DESC_PTR
#define pMDMA0_S1_START_ADDR 		pMDMA_S1_START_ADDR
#define pMDMA0_S1_CONFIG 			pMDMA_S1_CONFIG
#define pMDMA0_S1_X_COUNT 			pMDMA_S1_X_COUNT
#define pMDMA0_S1_X_MODIFY 			pMDMA_S1_X_MODIFY
#define pMDMA0_S1_Y_COUNT 			pMDMA_S1_Y_COUNT
#define pMDMA0_S1_Y_MODIFY 			pMDMA_S1_Y_MODIFY
#define pMDMA0_S1_CURR_DESC_PTR 	pMDMA_S1_CURR_DESC_PTR
#define pMDMA0_S1_CURR_ADDR 		pMDMA_S1_CURR_ADDR
#define pMDMA0_S1_IRQ_STATUS 		pMDMA_S1_IRQ_STATUS
#define pMDMA0_S1_PERIPHERAL_MAP 	pMDMA_S1_PERIPHERAL_MAP
#define pMDMA0_S1_CURR_X_COUNT 		pMDMA_S1_CURR_X_COUNT
#define pMDMA0_S1_CURR_Y_COUNT 		pMDMA_S1_CURR_Y_COUNT


/* Two-Wire Interface 0						*/
#define pTWI0_CLKDIV		((volatile unsigned short *)TWI0_CLKDIV)
#define pTWI0_CONTROL		((volatile unsigned short *)TWI0_CONTROL)
#define pTWI0_SLAVE_CTRL	((volatile unsigned short *)TWI0_SLAVE_CTRL)
#define pTWI0_SLAVE_STAT	((volatile unsigned short *)TWI0_SLAVE_STAT)
#define pTWI0_SLAVE_ADDR	((volatile unsigned short *)TWI0_SLAVE_ADDR)
#define pTWI0_MASTER_CTRL	((volatile unsigned short *)TWI0_MASTER_CTRL)
#define pTWI0_MASTER_STAT	((volatile unsigned short *)TWI0_MASTER_STAT)
#define pTWI0_MASTER_ADDR	((volatile unsigned short *)TWI0_MASTER_ADDR)
#define pTWI0_INT_STAT		((volatile unsigned short *)TWI0_INT_STAT)
#define pTWI0_INT_MASK		((volatile unsigned short *)TWI0_INT_MASK)
#define pTWI0_FIFO_CTRL		((volatile unsigned short *)TWI0_FIFO_CTRL)
#define pTWI0_FIFO_STAT		((volatile unsigned short *)TWI0_FIFO_STAT)
#define pTWI0_XMT_DATA8		((volatile unsigned short *)TWI0_XMT_DATA8)
#define pTWI0_XMT_DATA16	((volatile unsigned short *)TWI0_XMT_DATA16)
#define pTWI0_RCV_DATA8		((volatile unsigned short *)TWI0_RCV_DATA8)
#define pTWI0_RCV_DATA16	((volatile unsigned short *)TWI0_RCV_DATA16)


/* General Purpose IO Ports C, D, and E						*/
#define pPORTCIO_FER		((volatile unsigned short *)PORTCIO_FER)
#define pPORTCIO			((volatile unsigned short *)PORTCIO)
#define pPORTCIO_CLEAR		((volatile unsigned short *)PORTCIO_CLEAR)
#define pPORTCIO_SET		((volatile unsigned short *)PORTCIO_SET)
#define pPORTCIO_TOGGLE		((volatile unsigned short *)PORTCIO_TOGGLE)
#define pPORTCIO_DIR		((volatile unsigned short *)PORTCIO_DIR)
#define pPORTCIO_INEN		((volatile unsigned short *)PORTCIO_INEN)

#define pPORTDIO_FER		((volatile unsigned short *)PORTDIO_FER)
#define pPORTDIO			((volatile unsigned short *)PORTDIO)
#define pPORTDIO_CLEAR		((volatile unsigned short *)PORTDIO_CLEAR)
#define pPORTDIO_SET		((volatile unsigned short *)PORTDIO_SET)
#define pPORTDIO_TOGGLE		((volatile unsigned short *)PORTDIO_TOGGLE)
#define pPORTDIO_DIR		((volatile unsigned short *)PORTDIO_DIR)
#define pPORTDIO_INEN		((volatile unsigned short *)PORTDIO_INEN)

#define pPORTEIO_FER		((volatile unsigned short *)PORTEIO_FER)
#define pPORTEIO			((volatile unsigned short *)PORTEIO)
#define pPORTEIO_CLEAR		((volatile unsigned short *)PORTEIO_CLEAR)
#define pPORTEIO_SET		((volatile unsigned short *)PORTEIO_SET)
#define pPORTEIO_TOGGLE		((volatile unsigned short *)PORTEIO_TOGGLE)
#define pPORTEIO_DIR		((volatile unsigned short *)PORTEIO_DIR)
#define pPORTEIO_INEN		((volatile unsigned short *)PORTEIO_INEN)


/* DMA1 Traffic Control Registers											*/
#define	pDMAC1_TC_PER				((volatile unsigned short *)DMAC1_TC_PER)
#define	pDMAC1_TC_CNT				((volatile unsigned short *)DMAC1_TC_CNT)
/* Alternate deprecated register names (below) provided for backwards code compatibility */
#define pDMA1_TC_PER					pDMAC1_TC_PER	/* Traffic Control Periods Register			*/
#define pDMA1_TC_CNT					pDMAC1_TC_CNT	/* Traffic Control Current Counts Register	*/
/* Alternate deprecated register names (below) provided for backwards code compatibility */
#define pDMA1_TCPER					pDMA1_TC_PER	/* Traffic Control Periods Register			*/
#define pDMA1_TCCNT					pDMA1_TC_CNT	/* Traffic Control Current Counts Register	*/

/* DMA Controller 1 						*/
#define pDMA8_NEXT_DESC_PTR 		((void * volatile *)DMA8_NEXT_DESC_PTR)
#define pDMA8_START_ADDR 			((void * volatile *)DMA8_START_ADDR)
#define pDMA8_CONFIG 				((volatile unsigned short *)DMA8_CONFIG)
#define pDMA8_X_COUNT 				((volatile unsigned short *)DMA8_X_COUNT)
#define pDMA8_X_MODIFY 				((volatile signed   short *)DMA8_X_MODIFY)
#define pDMA8_Y_COUNT 				((volatile unsigned short *)DMA8_Y_COUNT)
#define pDMA8_Y_MODIFY 				((volatile signed   short *)DMA8_Y_MODIFY)
#define pDMA8_CURR_DESC_PTR 		((void * volatile *)DMA8_CURR_DESC_PTR)
#define pDMA8_CURR_ADDR 			((void * volatile *)DMA8_CURR_ADDR)
#define pDMA8_IRQ_STATUS 			((volatile unsigned short *)DMA8_IRQ_STATUS)
#define pDMA8_PERIPHERAL_MAP 		((volatile unsigned short *)DMA8_PERIPHERAL_MAP)
#define pDMA8_CURR_X_COUNT 			((volatile unsigned short *)DMA8_CURR_X_COUNT)
#define pDMA8_CURR_Y_COUNT 			((volatile unsigned short *)DMA8_CURR_Y_COUNT)

#define pDMA9_NEXT_DESC_PTR 		((void * volatile *)DMA9_NEXT_DESC_PTR)
#define pDMA9_START_ADDR 			((void * volatile *)DMA9_START_ADDR)
#define pDMA9_CONFIG 				((volatile unsigned short *)DMA9_CONFIG)
#define pDMA9_X_COUNT 				((volatile unsigned short *)DMA9_X_COUNT)
#define pDMA9_X_MODIFY 				((volatile signed   short *)DMA9_X_MODIFY)
#define pDMA9_Y_COUNT 				((volatile unsigned short *)DMA9_Y_COUNT)
#define pDMA9_Y_MODIFY 				((volatile signed   short *)DMA9_Y_MODIFY)
#define pDMA9_CURR_DESC_PTR 		((void * volatile *)DMA9_CURR_DESC_PTR)
#define pDMA9_CURR_ADDR 			((void * volatile *)DMA9_CURR_ADDR)
#define pDMA9_IRQ_STATUS 			((volatile unsigned short *)DMA9_IRQ_STATUS)
#define pDMA9_PERIPHERAL_MAP 		((volatile unsigned short *)DMA9_PERIPHERAL_MAP)
#define pDMA9_CURR_X_COUNT 			((volatile unsigned short *)DMA9_CURR_X_COUNT)
#define pDMA9_CURR_Y_COUNT 			((volatile unsigned short *)DMA9_CURR_Y_COUNT)

#define pDMA10_NEXT_DESC_PTR 		((void * volatile *)DMA10_NEXT_DESC_PTR)
#define pDMA10_START_ADDR 			((void * volatile *)DMA10_START_ADDR)
#define pDMA10_CONFIG 				((volatile unsigned short *)DMA10_CONFIG)
#define pDMA10_X_COUNT 				((volatile unsigned short *)DMA10_X_COUNT)
#define pDMA10_X_MODIFY 			((volatile signed   short *)DMA10_X_MODIFY)
#define pDMA10_Y_COUNT 				((volatile unsigned short *)DMA10_Y_COUNT)
#define pDMA10_Y_MODIFY 			((volatile signed   short *)DMA10_Y_MODIFY)
#define pDMA10_CURR_DESC_PTR 		((void * volatile *)DMA10_CURR_DESC_PTR)
#define pDMA10_CURR_ADDR 			((void * volatile *)DMA10_CURR_ADDR)
#define pDMA10_IRQ_STATUS 			((volatile unsigned short *)DMA10_IRQ_STATUS)
#define pDMA10_PERIPHERAL_MAP 		((volatile unsigned short *)DMA10_PERIPHERAL_MAP)
#define pDMA10_CURR_X_COUNT 		((volatile unsigned short *)DMA10_CURR_X_COUNT)
#define pDMA10_CURR_Y_COUNT 		((volatile unsigned short *)DMA10_CURR_Y_COUNT)

#define pDMA11_NEXT_DESC_PTR 		((void * volatile *)DMA11_NEXT_DESC_PTR)
#define pDMA11_START_ADDR 			((void * volatile *)DMA11_START_ADDR)
#define pDMA11_CONFIG 				((volatile unsigned short *)DMA11_CONFIG)
#define pDMA11_X_COUNT 				((volatile unsigned short *)DMA11_X_COUNT)
#define pDMA11_X_MODIFY 			((volatile signed   short *)DMA11_X_MODIFY)
#define pDMA11_Y_COUNT 				((volatile unsigned short *)DMA11_Y_COUNT)
#define pDMA11_Y_MODIFY 			((volatile signed   short *)DMA11_Y_MODIFY)
#define pDMA11_CURR_DESC_PTR 		((void * volatile *)DMA11_CURR_DESC_PTR)
#define pDMA11_CURR_ADDR 			((void * volatile *)DMA11_CURR_ADDR)
#define pDMA11_IRQ_STATUS 			((volatile unsigned short *)DMA11_IRQ_STATUS)
#define pDMA11_PERIPHERAL_MAP 		((volatile unsigned short *)DMA11_PERIPHERAL_MAP)
#define pDMA11_CURR_X_COUNT 		((volatile unsigned short *)DMA11_CURR_X_COUNT)
#define pDMA11_CURR_Y_COUNT 		((volatile unsigned short *)DMA11_CURR_Y_COUNT)

#define pDMA12_NEXT_DESC_PTR 		((void * volatile *)DMA12_NEXT_DESC_PTR)
#define pDMA12_START_ADDR 			((void * volatile *)DMA12_START_ADDR)
#define pDMA12_CONFIG 				((volatile unsigned short *)DMA12_CONFIG)
#define pDMA12_X_COUNT 				((volatile unsigned short *)DMA12_X_COUNT)
#define pDMA12_X_MODIFY 			((volatile signed   short *)DMA12_X_MODIFY)
#define pDMA12_Y_COUNT 				((volatile unsigned short *)DMA12_Y_COUNT)
#define pDMA12_Y_MODIFY 			((volatile signed   short *)DMA12_Y_MODIFY)
#define pDMA12_CURR_DESC_PTR 		((void * volatile *)DMA12_CURR_DESC_PTR)
#define pDMA12_CURR_ADDR 			((void * volatile *)DMA12_CURR_ADDR)
#define pDMA12_IRQ_STATUS 			((volatile unsigned short *)DMA12_IRQ_STATUS)
#define pDMA12_PERIPHERAL_MAP 		((volatile unsigned short *)DMA12_PERIPHERAL_MAP)
#define pDMA12_CURR_X_COUNT 		((volatile unsigned short *)DMA12_CURR_X_COUNT)
#define pDMA12_CURR_Y_COUNT 		((volatile unsigned short *)DMA12_CURR_Y_COUNT)

#define pDMA13_NEXT_DESC_PTR 		((void * volatile *)DMA13_NEXT_DESC_PTR)
#define pDMA13_START_ADDR 			((void * volatile *)DMA13_START_ADDR)
#define pDMA13_CONFIG 				((volatile unsigned short *)DMA13_CONFIG)
#define pDMA13_X_COUNT 				((volatile unsigned short *)DMA13_X_COUNT)
#define pDMA13_X_MODIFY 			((volatile signed   short *)DMA13_X_MODIFY)
#define pDMA13_Y_COUNT 				((volatile unsigned short *)DMA13_Y_COUNT)
#define pDMA13_Y_MODIFY 			((volatile signed   short *)DMA13_Y_MODIFY)
#define pDMA13_CURR_DESC_PTR 		((void * volatile *)DMA13_CURR_DESC_PTR)
#define pDMA13_CURR_ADDR 			((void * volatile *)DMA13_CURR_ADDR)
#define pDMA13_IRQ_STATUS 			((volatile unsigned short *)DMA13_IRQ_STATUS)
#define pDMA13_PERIPHERAL_MAP 		((volatile unsigned short *)DMA13_PERIPHERAL_MAP)
#define pDMA13_CURR_X_COUNT 		((volatile unsigned short *)DMA13_CURR_X_COUNT)
#define pDMA13_CURR_Y_COUNT 		((volatile unsigned short *)DMA13_CURR_Y_COUNT)

#define pDMA14_NEXT_DESC_PTR 		((void * volatile *)DMA14_NEXT_DESC_PTR)
#define pDMA14_START_ADDR 			((void * volatile *)DMA14_START_ADDR)
#define pDMA14_CONFIG 				((volatile unsigned short *)DMA14_CONFIG)
#define pDMA14_X_COUNT 				((volatile unsigned short *)DMA14_X_COUNT)
#define pDMA14_X_MODIFY 			((volatile signed   short *)DMA14_X_MODIFY)
#define pDMA14_Y_COUNT 				((volatile unsigned short *)DMA14_Y_COUNT)
#define pDMA14_Y_MODIFY 			((volatile signed   short *)DMA14_Y_MODIFY)
#define pDMA14_CURR_DESC_PTR 		((void * volatile *)DMA14_CURR_DESC_PTR)
#define pDMA14_CURR_ADDR 			((void * volatile *)DMA14_CURR_ADDR)
#define pDMA14_IRQ_STATUS 			((volatile unsigned short *)DMA14_IRQ_STATUS)
#define pDMA14_PERIPHERAL_MAP 		((volatile unsigned short *)DMA14_PERIPHERAL_MAP)
#define pDMA14_CURR_X_COUNT 		((volatile unsigned short *)DMA14_CURR_X_COUNT)
#define pDMA14_CURR_Y_COUNT 		((volatile unsigned short *)DMA14_CURR_Y_COUNT)

#define pDMA15_NEXT_DESC_PTR 		((void * volatile *)DMA15_NEXT_DESC_PTR)
#define pDMA15_START_ADDR 			((void * volatile *)DMA15_START_ADDR)
#define pDMA15_CONFIG 				((volatile unsigned short *)DMA15_CONFIG)
#define pDMA15_X_COUNT 				((volatile unsigned short *)DMA15_X_COUNT)
#define pDMA15_X_MODIFY 			((volatile signed   short *)DMA15_X_MODIFY)
#define pDMA15_Y_COUNT 				((volatile unsigned short *)DMA15_Y_COUNT)
#define pDMA15_Y_MODIFY 			((volatile signed   short *)DMA15_Y_MODIFY)
#define pDMA15_CURR_DESC_PTR 		((void * volatile *)DMA15_CURR_DESC_PTR)
#define pDMA15_CURR_ADDR 			((void * volatile *)DMA15_CURR_ADDR)
#define pDMA15_IRQ_STATUS 			((volatile unsigned short *)DMA15_IRQ_STATUS)
#define pDMA15_PERIPHERAL_MAP 		((volatile unsigned short *)DMA15_PERIPHERAL_MAP)
#define pDMA15_CURR_X_COUNT 		((volatile unsigned short *)DMA15_CURR_X_COUNT)
#define pDMA15_CURR_Y_COUNT 		((volatile unsigned short *)DMA15_CURR_Y_COUNT)

#define pDMA16_NEXT_DESC_PTR 		((void * volatile *)DMA16_NEXT_DESC_PTR)
#define pDMA16_START_ADDR 			((void * volatile *)DMA16_START_ADDR)
#define pDMA16_CONFIG 				((volatile unsigned short *)DMA16_CONFIG)
#define pDMA16_X_COUNT 				((volatile unsigned short *)DMA16_X_COUNT)
#define pDMA16_X_MODIFY 			((volatile signed   short *)DMA16_X_MODIFY)
#define pDMA16_Y_COUNT 				((volatile unsigned short *)DMA16_Y_COUNT)
#define pDMA16_Y_MODIFY 			((volatile signed   short *)DMA16_Y_MODIFY)
#define pDMA16_CURR_DESC_PTR 		((void * volatile *)DMA16_CURR_DESC_PTR)
#define pDMA16_CURR_ADDR 			((void * volatile *)DMA16_CURR_ADDR)
#define pDMA16_IRQ_STATUS 			((volatile unsigned short *)DMA16_IRQ_STATUS)
#define pDMA16_PERIPHERAL_MAP 		((volatile unsigned short *)DMA16_PERIPHERAL_MAP)
#define pDMA16_CURR_X_COUNT 		((volatile unsigned short *)DMA16_CURR_X_COUNT)
#define pDMA16_CURR_Y_COUNT 		((volatile unsigned short *)DMA16_CURR_Y_COUNT)

#define pDMA17_NEXT_DESC_PTR 		((void * volatile *)DMA17_NEXT_DESC_PTR)
#define pDMA17_START_ADDR 			((void * volatile *)DMA17_START_ADDR)
#define pDMA17_CONFIG 				((volatile unsigned short *)DMA17_CONFIG)
#define pDMA17_X_COUNT 				((volatile unsigned short *)DMA17_X_COUNT)
#define pDMA17_X_MODIFY 			((volatile signed   short *)DMA17_X_MODIFY)
#define pDMA17_Y_COUNT 				((volatile unsigned short *)DMA17_Y_COUNT)
#define pDMA17_Y_MODIFY 			((volatile signed   short *)DMA17_Y_MODIFY)
#define pDMA17_CURR_DESC_PTR 		((void * volatile *)DMA17_CURR_DESC_PTR)
#define pDMA17_CURR_ADDR 			((void * volatile *)DMA17_CURR_ADDR)
#define pDMA17_IRQ_STATUS 			((volatile unsigned short *)DMA17_IRQ_STATUS)
#define pDMA17_PERIPHERAL_MAP 		((volatile unsigned short *)DMA17_PERIPHERAL_MAP)
#define pDMA17_CURR_X_COUNT 		((volatile unsigned short *)DMA17_CURR_X_COUNT)
#define pDMA17_CURR_Y_COUNT 		((volatile unsigned short *)DMA17_CURR_Y_COUNT)

#define pDMA18_NEXT_DESC_PTR 		((void * volatile *)DMA18_NEXT_DESC_PTR)
#define pDMA18_START_ADDR 			((void * volatile *)DMA18_START_ADDR)
#define pDMA18_CONFIG 				((volatile unsigned short *)DMA18_CONFIG)
#define pDMA18_X_COUNT 				((volatile unsigned short *)DMA18_X_COUNT)
#define pDMA18_X_MODIFY 			((volatile signed   short *)DMA18_X_MODIFY)
#define pDMA18_Y_COUNT 				((volatile unsigned short *)DMA18_Y_COUNT)
#define pDMA18_Y_MODIFY 			((volatile signed   short *)DMA18_Y_MODIFY)
#define pDMA18_CURR_DESC_PTR 		((void * volatile *)DMA18_CURR_DESC_PTR)
#define pDMA18_CURR_ADDR 			((void * volatile *)DMA18_CURR_ADDR)
#define pDMA18_IRQ_STATUS 			((volatile unsigned short *)DMA18_IRQ_STATUS)
#define pDMA18_PERIPHERAL_MAP 		((volatile unsigned short *)DMA18_PERIPHERAL_MAP)
#define pDMA18_CURR_X_COUNT 		((volatile unsigned short *)DMA18_CURR_X_COUNT)
#define pDMA18_CURR_Y_COUNT 		((volatile unsigned short *)DMA18_CURR_Y_COUNT)

#define pDMA19_NEXT_DESC_PTR 		((void * volatile *)DMA19_NEXT_DESC_PTR)
#define pDMA19_START_ADDR 			((void * volatile *)DMA19_START_ADDR)
#define pDMA19_CONFIG 				((volatile unsigned short *)DMA19_CONFIG)
#define pDMA19_X_COUNT 				((volatile unsigned short *)DMA19_X_COUNT)
#define pDMA19_X_MODIFY 			((volatile signed   short *)DMA19_X_MODIFY)
#define pDMA19_Y_COUNT 				((volatile unsigned short *)DMA19_Y_COUNT)
#define pDMA19_Y_MODIFY 			((volatile signed   short *)DMA19_Y_MODIFY)
#define pDMA19_CURR_DESC_PTR 		((void * volatile *)DMA19_CURR_DESC_PTR)
#define pDMA19_CURR_ADDR 			((void * volatile *)DMA19_CURR_ADDR)
#define pDMA19_IRQ_STATUS 			((volatile unsigned short *)DMA19_IRQ_STATUS)
#define pDMA19_PERIPHERAL_MAP 		((volatile unsigned short *)DMA19_PERIPHERAL_MAP)
#define pDMA19_CURR_X_COUNT 		((volatile unsigned short *)DMA19_CURR_X_COUNT)
#define pDMA19_CURR_Y_COUNT 		((volatile unsigned short *)DMA19_CURR_Y_COUNT)

#define pMDMA1_D0_NEXT_DESC_PTR 	((void * volatile *)MDMA1_D0_NEXT_DESC_PTR)
#define pMDMA1_D0_START_ADDR 		((void * volatile *)MDMA1_D0_START_ADDR)
#define pMDMA1_D0_CONFIG 			((volatile unsigned short *)MDMA1_D0_CONFIG)
#define pMDMA1_D0_X_COUNT 			((volatile unsigned short *)MDMA1_D0_X_COUNT)
#define pMDMA1_D0_X_MODIFY 			((volatile signed   short *)MDMA1_D0_X_MODIFY)
#define pMDMA1_D0_Y_COUNT 			((volatile unsigned short *)MDMA1_D0_Y_COUNT)
#define pMDMA1_D0_Y_MODIFY 			((volatile signed   short *)MDMA1_D0_Y_MODIFY)
#define pMDMA1_D0_CURR_DESC_PTR 	((void * volatile *)MDMA1_D0_CURR_DESC_PTR)
#define pMDMA1_D0_CURR_ADDR 		((void * volatile *)MDMA1_D0_CURR_ADDR)
#define pMDMA1_D0_IRQ_STATUS 		((volatile unsigned short *)MDMA1_D0_IRQ_STATUS)
#define pMDMA1_D0_PERIPHERAL_MAP	((volatile unsigned short *)MDMA1_D0_PERIPHERAL_MAP)
#define pMDMA1_D0_CURR_X_COUNT 		((volatile unsigned short *)MDMA1_D0_CURR_X_COUNT)
#define pMDMA1_D0_CURR_Y_COUNT 		((volatile unsigned short *)MDMA1_D0_CURR_Y_COUNT)

#define pMDMA1_S0_NEXT_DESC_PTR 	((void * volatile *)MDMA1_S0_NEXT_DESC_PTR)
#define pMDMA1_S0_START_ADDR 		((void * volatile *)MDMA1_S0_START_ADDR)
#define pMDMA1_S0_CONFIG 			((volatile unsigned short *)MDMA1_S0_CONFIG)
#define pMDMA1_S0_X_COUNT 			((volatile unsigned short *)MDMA1_S0_X_COUNT)
#define pMDMA1_S0_X_MODIFY 			((volatile signed   short *)MDMA1_S0_X_MODIFY)
#define pMDMA1_S0_Y_COUNT 			((volatile unsigned short *)MDMA1_S0_Y_COUNT)
#define pMDMA1_S0_Y_MODIFY 			((volatile signed   short *)MDMA1_S0_Y_MODIFY)
#define pMDMA1_S0_CURR_DESC_PTR 	((void * volatile *)MDMA1_S0_CURR_DESC_PTR)
#define pMDMA1_S0_CURR_ADDR 		((void * volatile *)MDMA1_S0_CURR_ADDR)
#define pMDMA1_S0_IRQ_STATUS 		((volatile unsigned short *)MDMA1_S0_IRQ_STATUS)
#define pMDMA1_S0_PERIPHERAL_MAP	((volatile unsigned short *)MDMA1_S0_PERIPHERAL_MAP)
#define pMDMA1_S0_CURR_X_COUNT		((volatile unsigned short *)MDMA1_S0_CURR_X_COUNT)
#define pMDMA1_S0_CURR_Y_COUNT		((volatile unsigned short *)MDMA1_S0_CURR_Y_COUNT)

#define pMDMA1_D1_NEXT_DESC_PTR 	((void * volatile *)MDMA1_D1_NEXT_DESC_PTR)
#define pMDMA1_D1_START_ADDR 		((void * volatile *)MDMA1_D1_START_ADDR)
#define pMDMA1_D1_CONFIG 			((volatile unsigned short *)MDMA1_D1_CONFIG)
#define pMDMA1_D1_X_COUNT 			((volatile unsigned short *)MDMA1_D1_X_COUNT)
#define pMDMA1_D1_X_MODIFY 			((volatile signed   short *)MDMA1_D1_X_MODIFY)
#define pMDMA1_D1_Y_COUNT 			((volatile unsigned short *)MDMA1_D1_Y_COUNT)
#define pMDMA1_D1_Y_MODIFY 			((volatile signed   short *)MDMA1_D1_Y_MODIFY)
#define pMDMA1_D1_CURR_DESC_PTR 	((void * volatile *)MDMA1_D1_CURR_DESC_PTR)
#define pMDMA1_D1_CURR_ADDR 		((void * volatile *)MDMA1_D1_CURR_ADDR)
#define pMDMA1_D1_IRQ_STATUS 		((volatile unsigned short *)MDMA1_D1_IRQ_STATUS)
#define pMDMA1_D1_PERIPHERAL_MAP	((volatile unsigned short *)MDMA1_D1_PERIPHERAL_MAP)
#define pMDMA1_D1_CURR_X_COUNT 		((volatile unsigned short *)MDMA1_D1_CURR_X_COUNT)
#define pMDMA1_D1_CURR_Y_COUNT 		((volatile unsigned short *)MDMA1_D1_CURR_Y_COUNT)

#define pMDMA1_S1_NEXT_DESC_PTR 	((void * volatile *)MDMA1_S1_NEXT_DESC_PTR)
#define pMDMA1_S1_START_ADDR 		((void * volatile *)MDMA1_S1_START_ADDR)
#define pMDMA1_S1_CONFIG 			((volatile unsigned short *)MDMA1_S1_CONFIG)
#define pMDMA1_S1_X_COUNT 			((volatile unsigned short *)MDMA1_S1_X_COUNT)
#define pMDMA1_S1_X_MODIFY 			((volatile signed   short *)MDMA1_S1_X_MODIFY)
#define pMDMA1_S1_Y_COUNT 			((volatile unsigned short *)MDMA1_S1_Y_COUNT)
#define pMDMA1_S1_Y_MODIFY 			((volatile signed   short *)MDMA1_S1_Y_MODIFY)
#define pMDMA1_S1_CURR_DESC_PTR 	((void * volatile *)MDMA1_S1_CURR_DESC_PTR)
#define pMDMA1_S1_CURR_ADDR 		((void * volatile *)MDMA1_S1_CURR_ADDR)
#define pMDMA1_S1_IRQ_STATUS 		((volatile unsigned short *)MDMA1_S1_IRQ_STATUS)
#define pMDMA1_S1_PERIPHERAL_MAP	((volatile unsigned short *)MDMA1_S1_PERIPHERAL_MAP)
#define pMDMA1_S1_CURR_X_COUNT 		((volatile unsigned short *)MDMA1_S1_CURR_X_COUNT)
#define pMDMA1_S1_CURR_Y_COUNT 		((volatile unsigned short *)MDMA1_S1_CURR_Y_COUNT)


/* UART1 Controller							*/
#define pUART1_THR 			((volatile unsigned short *)UART1_THR)
#define pUART1_RBR 			((volatile unsigned short *)UART1_RBR)
#define pUART1_DLL 			((volatile unsigned short *)UART1_DLL)
#define pUART1_IER 			((volatile unsigned short *)UART1_IER)
#define pUART1_DLH 			((volatile unsigned short *)UART1_DLH)
#define pUART1_IIR 			((volatile unsigned short *)UART1_IIR)
#define pUART1_LCR 			((volatile unsigned short *)UART1_LCR)
#define pUART1_MCR 			((volatile unsigned short *)UART1_MCR)
#define pUART1_LSR 			((volatile unsigned short *)UART1_LSR)
#define pUART1_SCR 			((volatile unsigned short *)UART1_SCR)
#define pUART1_GCTL 		((volatile unsigned short *)UART1_GCTL)


/* UART2 Controller						*/
#define pUART2_THR 			((volatile unsigned short *)UART2_THR)
#define pUART2_RBR 			((volatile unsigned short *)UART2_RBR)
#define pUART2_DLL 			((volatile unsigned short *)UART2_DLL)
#define pUART2_IER 			((volatile unsigned short *)UART2_IER)
#define pUART2_DLH 			((volatile unsigned short *)UART2_DLH)
#define pUART2_IIR 			((volatile unsigned short *)UART2_IIR)
#define pUART2_LCR 			((volatile unsigned short *)UART2_LCR)
#define pUART2_MCR 			((volatile unsigned short *)UART2_MCR)
#define pUART2_LSR 			((volatile unsigned short *)UART2_LSR)
#define pUART2_SCR 			((volatile unsigned short *)UART2_SCR)
#define pUART2_GCTL 		((volatile unsigned short *)UART2_GCTL)


/* Two-Wire Interface 1						*/
#define pTWI1_CLKDIV		((volatile unsigned short *)TWI1_CLKDIV)
#define pTWI1_CONTROL		((volatile unsigned short *)TWI1_CONTROL)
#define pTWI1_SLAVE_CTRL	((volatile unsigned short *)TWI1_SLAVE_CTRL)
#define pTWI1_SLAVE_STAT	((volatile unsigned short *)TWI1_SLAVE_STAT)
#define pTWI1_SLAVE_ADDR	((volatile unsigned short *)TWI1_SLAVE_ADDR)
#define pTWI1_MASTER_CTRL	((volatile unsigned short *)TWI1_MASTER_CTRL)
#define pTWI1_MASTER_STAT	((volatile unsigned short *)TWI1_MASTER_STAT)
#define pTWI1_MASTER_ADDR	((volatile unsigned short *)TWI1_MASTER_ADDR)
#define pTWI1_INT_STAT		((volatile unsigned short *)TWI1_INT_STAT)
#define pTWI1_INT_MASK		((volatile unsigned short *)TWI1_INT_MASK)
#define pTWI1_FIFO_CTRL		((volatile unsigned short *)TWI1_FIFO_CTRL)
#define pTWI1_FIFO_STAT		((volatile unsigned short *)TWI1_FIFO_STAT)
#define pTWI1_XMT_DATA8		((volatile unsigned short *)TWI1_XMT_DATA8)
#define pTWI1_XMT_DATA16	((volatile unsigned short *)TWI1_XMT_DATA16)
#define pTWI1_RCV_DATA8		((volatile unsigned short *)TWI1_RCV_DATA8)
#define pTWI1_RCV_DATA16	((volatile unsigned short *)TWI1_RCV_DATA16)


/* SPI1 Controller					*/
#define pSPI1_CTL 			((volatile unsigned short *)SPI1_CTL)
#define pSPI1_FLG 			((volatile unsigned short *)SPI1_FLG)
#define pSPI1_STAT 			((volatile unsigned short *)SPI1_STAT)
#define pSPI1_TDBR 			((volatile unsigned short *)SPI1_TDBR)
#define pSPI1_RDBR 			((volatile unsigned short *)SPI1_RDBR)
#define pSPI1_BAUD 			((volatile unsigned short *)SPI1_BAUD)
#define pSPI1_SHADOW 		((volatile unsigned short *)SPI1_SHADOW)


/* SPI2 Controller								*/
#define pSPI2_CTL 			((volatile unsigned short *)SPI2_CTL)
#define pSPI2_FLG 			((volatile unsigned short *)SPI2_FLG)
#define pSPI2_STAT 			((volatile unsigned short *)SPI2_STAT)
#define pSPI2_TDBR 			((volatile unsigned short *)SPI2_TDBR)
#define pSPI2_RDBR 			((volatile unsigned short *)SPI2_RDBR)
#define pSPI2_BAUD 			((volatile unsigned short *)SPI2_BAUD)
#define pSPI2_SHADOW 		((volatile unsigned short *)SPI2_SHADOW)


/* SPORT2 Controller					*/
#define pSPORT2_TCR1 		((volatile unsigned short *)SPORT2_TCR1)
#define pSPORT2_TCR2 		((volatile unsigned short *)SPORT2_TCR2)
#define pSPORT2_TCLKDIV 	((volatile unsigned short *)SPORT2_TCLKDIV)
#define pSPORT2_TFSDIV 		((volatile unsigned short *)SPORT2_TFSDIV)
#define pSPORT2_TX 			((volatile unsigned long  *)SPORT2_TX)
#define pSPORT2_RX 			((volatile unsigned long  *)SPORT2_RX)
#define pSPORT2_TX32 		((volatile unsigned long  *)SPORT2_TX)
#define pSPORT2_RX32 		((volatile unsigned long  *)SPORT2_RX)
#define pSPORT2_TX16 		((volatile unsigned short *)SPORT2_TX)
#define pSPORT2_RX16 		((volatile unsigned short *)SPORT2_RX)
#define pSPORT2_RCR1 		((volatile unsigned short *)SPORT2_RCR1)
#define pSPORT2_RCR2 		((volatile unsigned short *)SPORT2_RCR2)
#define pSPORT2_RCLKDIV 	((volatile unsigned short *)SPORT2_RCLKDIV)
#define pSPORT2_RFSDIV 		((volatile unsigned short *)SPORT2_RFSDIV)
#define pSPORT2_STAT 		((volatile unsigned short *)SPORT2_STAT)
#define pSPORT2_CHNL 		((volatile unsigned short *)SPORT2_CHNL)
#define pSPORT2_MCMC1 		((volatile unsigned short *)SPORT2_MCMC1)
#define pSPORT2_MCMC2 		((volatile unsigned short *)SPORT2_MCMC2)
#define pSPORT2_MTCS0 		((volatile unsigned long  *)SPORT2_MTCS0)
#define pSPORT2_MTCS1 		((volatile unsigned long  *)SPORT2_MTCS1)
#define pSPORT2_MTCS2 		((volatile unsigned long  *)SPORT2_MTCS2)
#define pSPORT2_MTCS3 		((volatile unsigned long  *)SPORT2_MTCS3)
#define pSPORT2_MRCS0 		((volatile unsigned long  *)SPORT2_MRCS0)
#define pSPORT2_MRCS1 		((volatile unsigned long  *)SPORT2_MRCS1)
#define pSPORT2_MRCS2 		((volatile unsigned long  *)SPORT2_MRCS2)
#define pSPORT2_MRCS3 		((volatile unsigned long  *)SPORT2_MRCS3)


/* SPORT3 Controller						*/
#define pSPORT3_TCR1 		((volatile unsigned short *)SPORT3_TCR1)
#define pSPORT3_TCR2 		((volatile unsigned short *)SPORT3_TCR2)
#define pSPORT3_TCLKDIV 	((volatile unsigned short *)SPORT3_TCLKDIV)
#define pSPORT3_TFSDIV 		((volatile unsigned short *)SPORT3_TFSDIV)
#define pSPORT3_TX 			((volatile unsigned long  *)SPORT3_TX)
#define pSPORT3_RX 			((volatile unsigned long  *)SPORT3_RX)
#define pSPORT3_TX32 		((volatile unsigned long  *)SPORT3_TX)
#define pSPORT3_RX32 		((volatile unsigned long  *)SPORT3_RX)
#define pSPORT3_TX16 		((volatile unsigned short *)SPORT3_TX)
#define pSPORT3_RX16 		((volatile unsigned short *)SPORT3_RX)
#define pSPORT3_RCR1 		((volatile unsigned short *)SPORT3_RCR1)
#define pSPORT3_RCR2 		((volatile unsigned short *)SPORT3_RCR2)
#define pSPORT3_RCLKDIV 	((volatile unsigned short *)SPORT3_RCLKDIV)
#define pSPORT3_RFSDIV 		((volatile unsigned short *)SPORT3_RFSDIV)
#define pSPORT3_STAT 		((volatile unsigned short *)SPORT3_STAT)
#define pSPORT3_CHNL 		((volatile unsigned short *)SPORT3_CHNL)
#define pSPORT3_MCMC1 		((volatile unsigned short *)SPORT3_MCMC1)
#define pSPORT3_MCMC2 		((volatile unsigned short *)SPORT3_MCMC2)
#define pSPORT3_MTCS0 		((volatile unsigned long  *)SPORT3_MTCS0)
#define pSPORT3_MTCS1 		((volatile unsigned long  *)SPORT3_MTCS1)
#define pSPORT3_MTCS2 		((volatile unsigned long  *)SPORT3_MTCS2)
#define pSPORT3_MTCS3 		((volatile unsigned long  *)SPORT3_MTCS3)
#define pSPORT3_MRCS0 		((volatile unsigned long  *)SPORT3_MRCS0)
#define pSPORT3_MRCS1 		((volatile unsigned long  *)SPORT3_MRCS1)
#define pSPORT3_MRCS2 		((volatile unsigned long  *)SPORT3_MRCS2)
#define pSPORT3_MRCS3 		((volatile unsigned long  *)SPORT3_MRCS3)


/* CAN Controller						*/
/* For Mailboxes 0-15													*/
#define pCAN_MC1			((volatile unsigned short *)CAN_MC1)
#define pCAN_MD1			((volatile unsigned short *)CAN_MD1)
#define pCAN_TRS1			((volatile unsigned short *)CAN_TRS1)
#define pCAN_TRR1			((volatile unsigned short *)CAN_TRR1)
#define pCAN_TA1			((volatile unsigned short *)CAN_TA1)
#define pCAN_AA1			((volatile unsigned short *)CAN_AA1)
#define pCAN_RMP1			((volatile unsigned short *)CAN_RMP1)
#define pCAN_RML1			((volatile unsigned short *)CAN_RML1)
#define pCAN_MBTIF1			((volatile unsigned short *)CAN_MBTIF1)
#define pCAN_MBRIF1			((volatile unsigned short *)CAN_MBRIF1)
#define pCAN_MBIM1			((volatile unsigned short *)CAN_MBIM1)
#define pCAN_RFH1			((volatile unsigned short *)CAN_RFH1)
#define pCAN_OPSS1			((volatile unsigned short *)CAN_OPSS1)

/* For Mailboxes 16-31   												*/
#define pCAN_MC2			((volatile unsigned short *)CAN_MC2)
#define pCAN_MD2			((volatile unsigned short *)CAN_MD2)
#define pCAN_TRS2			((volatile unsigned short *)CAN_TRS2)
#define pCAN_TRR2			((volatile unsigned short *)CAN_TRR2)
#define pCAN_TA2			((volatile unsigned short *)CAN_TA2)
#define pCAN_AA2			((volatile unsigned short *)CAN_AA2)
#define pCAN_RMP2			((volatile unsigned short *)CAN_RMP2)
#define pCAN_RML2			((volatile unsigned short *)CAN_RML2)
#define pCAN_MBTIF2			((volatile unsigned short *)CAN_MBTIF2)
#define pCAN_MBRIF2			((volatile unsigned short *)CAN_MBRIF2)
#define pCAN_MBIM2			((volatile unsigned short *)CAN_MBIM2)
#define pCAN_RFH2			((volatile unsigned short *)CAN_RFH2)
#define pCAN_OPSS2			((volatile unsigned short *)CAN_OPSS2)

#define pCAN_CLOCK			((volatile unsigned short *)CAN_CLOCK)
#define pCAN_TIMING			((volatile unsigned short *)CAN_TIMING)
#define pCAN_DEBUG			((volatile unsigned short *)CAN_DEBUG)
#define pCAN_STATUS			((volatile unsigned short *)CAN_STATUS)
#define pCAN_CEC			((volatile unsigned short *)CAN_CEC)
#define pCAN_GIS			((volatile unsigned short *)CAN_GIS)
#define pCAN_GIM			((volatile unsigned short *)CAN_GIM)
#define pCAN_GIF			((volatile unsigned short *)CAN_GIF)
#define pCAN_CONTROL		((volatile unsigned short *)CAN_CONTROL)
#define pCAN_INTR			((volatile unsigned short *)CAN_INTR)
#define pCAN_MBTD			((volatile unsigned short *)CAN_MBTD)
#define pCAN_EWR			((volatile unsigned short *)CAN_EWR)
#define pCAN_ESR			((volatile unsigned short *)CAN_ESR)
#define pCAN_UCCNT			((volatile unsigned short *)CAN_UCCNT)
#define pCAN_UCRC			((volatile unsigned short *)CAN_UCRC)
#define pCAN_UCCNF			((volatile unsigned short *)CAN_UCCNF)

/* Mailbox Acceptance Masks 											*/
#define pCAN_AM00L			((volatile unsigned short *)CAN_AM00L)
#define pCAN_AM00H			((volatile unsigned short *)CAN_AM00H)
#define pCAN_AM01L			((volatile unsigned short *)CAN_AM01L)
#define pCAN_AM01H			((volatile unsigned short *)CAN_AM01H)
#define pCAN_AM02L			((volatile unsigned short *)CAN_AM02L)
#define pCAN_AM02H			((volatile unsigned short *)CAN_AM02H)
#define pCAN_AM03L			((volatile unsigned short *)CAN_AM03L)
#define pCAN_AM03H			((volatile unsigned short *)CAN_AM03H)
#define pCAN_AM04L			((volatile unsigned short *)CAN_AM04L)
#define pCAN_AM04H			((volatile unsigned short *)CAN_AM04H)
#define pCAN_AM05L			((volatile unsigned short *)CAN_AM05L)
#define pCAN_AM05H			((volatile unsigned short *)CAN_AM05H)
#define pCAN_AM06L			((volatile unsigned short *)CAN_AM06L)
#define pCAN_AM06H			((volatile unsigned short *)CAN_AM06H)
#define pCAN_AM07L			((volatile unsigned short *)CAN_AM07L)
#define pCAN_AM07H			((volatile unsigned short *)CAN_AM07H)
#define pCAN_AM08L			((volatile unsigned short *)CAN_AM08L)
#define pCAN_AM08H			((volatile unsigned short *)CAN_AM08H)
#define pCAN_AM09L			((volatile unsigned short *)CAN_AM09L)
#define pCAN_AM09H			((volatile unsigned short *)CAN_AM09H)
#define pCAN_AM10L			((volatile unsigned short *)CAN_AM10L)
#define pCAN_AM10H			((volatile unsigned short *)CAN_AM10H)
#define pCAN_AM11L			((volatile unsigned short *)CAN_AM11L)
#define pCAN_AM11H			((volatile unsigned short *)CAN_AM11H)
#define pCAN_AM12L			((volatile unsigned short *)CAN_AM12L)
#define pCAN_AM12H			((volatile unsigned short *)CAN_AM12H)
#define pCAN_AM13L			((volatile unsigned short *)CAN_AM13L)
#define pCAN_AM13H			((volatile unsigned short *)CAN_AM13H)
#define pCAN_AM14L			((volatile unsigned short *)CAN_AM14L)
#define pCAN_AM14H			((volatile unsigned short *)CAN_AM14H)
#define pCAN_AM15L			((volatile unsigned short *)CAN_AM15L)
#define pCAN_AM15H			((volatile unsigned short *)CAN_AM15H)

#define pCAN_AM16L			((volatile unsigned short *)CAN_AM16L)
#define pCAN_AM16H			((volatile unsigned short *)CAN_AM16H)
#define pCAN_AM17L			((volatile unsigned short *)CAN_AM17L)
#define pCAN_AM17H			((volatile unsigned short *)CAN_AM17H)
#define pCAN_AM18L			((volatile unsigned short *)CAN_AM18L)
#define pCAN_AM18H			((volatile unsigned short *)CAN_AM18H)
#define pCAN_AM19L			((volatile unsigned short *)CAN_AM19L)
#define pCAN_AM19H			((volatile unsigned short *)CAN_AM19H)
#define pCAN_AM20L			((volatile unsigned short *)CAN_AM20L)
#define pCAN_AM20H			((volatile unsigned short *)CAN_AM20H)
#define pCAN_AM21L			((volatile unsigned short *)CAN_AM21L)
#define pCAN_AM21H			((volatile unsigned short *)CAN_AM21H)
#define pCAN_AM22L			((volatile unsigned short *)CAN_AM22L)
#define pCAN_AM22H			((volatile unsigned short *)CAN_AM22H)
#define pCAN_AM23L			((volatile unsigned short *)CAN_AM23L)
#define pCAN_AM23H			((volatile unsigned short *)CAN_AM23H)
#define pCAN_AM24L			((volatile unsigned short *)CAN_AM24L)
#define pCAN_AM24H			((volatile unsigned short *)CAN_AM24H)
#define pCAN_AM25L			((volatile unsigned short *)CAN_AM25L)
#define pCAN_AM25H			((volatile unsigned short *)CAN_AM25H)
#define pCAN_AM26L			((volatile unsigned short *)CAN_AM26L)
#define pCAN_AM26H			((volatile unsigned short *)CAN_AM26H)
#define pCAN_AM27L			((volatile unsigned short *)CAN_AM27L)
#define pCAN_AM27H			((volatile unsigned short *)CAN_AM27H)
#define pCAN_AM28L			((volatile unsigned short *)CAN_AM28L)
#define pCAN_AM28H			((volatile unsigned short *)CAN_AM28H)
#define pCAN_AM29L			((volatile unsigned short *)CAN_AM29L)
#define pCAN_AM29H			((volatile unsigned short *)CAN_AM29H)
#define pCAN_AM30L			((volatile unsigned short *)CAN_AM30L)
#define pCAN_AM30H			((volatile unsigned short *)CAN_AM30H)
#define pCAN_AM31L			((volatile unsigned short *)CAN_AM31L)
#define pCAN_AM31H			((volatile unsigned short *)CAN_AM31H)

/* CAN Acceptance Mask Area Macros	*/
#define pCAN_AM_L(x)		((volatile unsigned short *)CAN_AM_L(x))
#define pCAN_AM_H(x)		((volatile unsigned short *)CAN_AM_H(x))

/* Mailbox Registers														*/
#define pCAN_MB00_DATA0		((volatile unsigned short *)CAN_MB00_DATA0)
#define pCAN_MB00_DATA1		((volatile unsigned short *)CAN_MB00_DATA1)
#define pCAN_MB00_DATA2		((volatile unsigned short *)CAN_MB00_DATA2)
#define pCAN_MB00_DATA3		((volatile unsigned short *)CAN_MB00_DATA3)
#define pCAN_MB00_LENGTH	((volatile unsigned short *)CAN_MB00_LENGTH)
#define pCAN_MB00_TIMESTAMP	((volatile unsigned short *)CAN_MB00_TIMESTAMP)
#define pCAN_MB00_ID0		((volatile unsigned short *)CAN_MB00_ID0)
#define pCAN_MB00_ID1		((volatile unsigned short *)CAN_MB00_ID1)

#define pCAN_MB01_DATA0		((volatile unsigned short *)CAN_MB01_DATA0)
#define pCAN_MB01_DATA1		((volatile unsigned short *)CAN_MB01_DATA1)
#define pCAN_MB01_DATA2		((volatile unsigned short *)CAN_MB01_DATA2)
#define pCAN_MB01_DATA3		((volatile unsigned short *)CAN_MB01_DATA3)
#define pCAN_MB01_LENGTH	((volatile unsigned short *)CAN_MB01_LENGTH)
#define pCAN_MB01_TIMESTAMP	((volatile unsigned short *)CAN_MB01_TIMESTAMP)
#define pCAN_MB01_ID0		((volatile unsigned short *)CAN_MB01_ID0)
#define pCAN_MB01_ID1		((volatile unsigned short *)CAN_MB01_ID1)

#define pCAN_MB02_DATA0		((volatile unsigned short *)CAN_MB02_DATA0)
#define pCAN_MB02_DATA1		((volatile unsigned short *)CAN_MB02_DATA1)
#define pCAN_MB02_DATA2		((volatile unsigned short *)CAN_MB02_DATA2)
#define pCAN_MB02_DATA3		((volatile unsigned short *)CAN_MB02_DATA3)
#define pCAN_MB02_LENGTH	((volatile unsigned short *)CAN_MB02_LENGTH)
#define pCAN_MB02_TIMESTAMP	((volatile unsigned short *)CAN_MB02_TIMESTAMP)
#define pCAN_MB02_ID0		((volatile unsigned short *)CAN_MB02_ID0)
#define pCAN_MB02_ID1		((volatile unsigned short *)CAN_MB02_ID1)

#define pCAN_MB03_DATA0		((volatile unsigned short *)CAN_MB03_DATA0)
#define pCAN_MB03_DATA1		((volatile unsigned short *)CAN_MB03_DATA1)
#define pCAN_MB03_DATA2		((volatile unsigned short *)CAN_MB03_DATA2)
#define pCAN_MB03_DATA3		((volatile unsigned short *)CAN_MB03_DATA3)
#define pCAN_MB03_LENGTH	((volatile unsigned short *)CAN_MB03_LENGTH)
#define pCAN_MB03_TIMESTAMP	((volatile unsigned short *)CAN_MB03_TIMESTAMP)
#define pCAN_MB03_ID0		((volatile unsigned short *)CAN_MB03_ID0)
#define pCAN_MB03_ID1		((volatile unsigned short *)CAN_MB03_ID1)

#define pCAN_MB04_DATA0		((volatile unsigned short *)CAN_MB04_DATA0)
#define pCAN_MB04_DATA1		((volatile unsigned short *)CAN_MB04_DATA1)
#define pCAN_MB04_DATA2		((volatile unsigned short *)CAN_MB04_DATA2)
#define pCAN_MB04_DATA3		((volatile unsigned short *)CAN_MB04_DATA3)
#define pCAN_MB04_LENGTH	((volatile unsigned short *)CAN_MB04_LENGTH)
#define pCAN_MB04_TIMESTAMP	((volatile unsigned short *)CAN_MB04_TIMESTAMP)
#define pCAN_MB04_ID0		((volatile unsigned short *)CAN_MB04_ID0)
#define pCAN_MB04_ID1		((volatile unsigned short *)CAN_MB04_ID1)

#define pCAN_MB05_DATA0		((volatile unsigned short *)CAN_MB05_DATA0)
#define pCAN_MB05_DATA1		((volatile unsigned short *)CAN_MB05_DATA1)
#define pCAN_MB05_DATA2		((volatile unsigned short *)CAN_MB05_DATA2)
#define pCAN_MB05_DATA3		((volatile unsigned short *)CAN_MB05_DATA3)
#define pCAN_MB05_LENGTH	((volatile unsigned short *)CAN_MB05_LENGTH)
#define pCAN_MB05_TIMESTAMP	((volatile unsigned short *)CAN_MB05_TIMESTAMP)
#define pCAN_MB05_ID0		((volatile unsigned short *)CAN_MB05_ID0)
#define pCAN_MB05_ID1		((volatile unsigned short *)CAN_MB05_ID1)

#define pCAN_MB06_DATA0		((volatile unsigned short *)CAN_MB06_DATA0)
#define pCAN_MB06_DATA1		((volatile unsigned short *)CAN_MB06_DATA1)
#define pCAN_MB06_DATA2		((volatile unsigned short *)CAN_MB06_DATA2)
#define pCAN_MB06_DATA3		((volatile unsigned short *)CAN_MB06_DATA3)
#define pCAN_MB06_LENGTH	((volatile unsigned short *)CAN_MB06_LENGTH)
#define pCAN_MB06_TIMESTAMP	((volatile unsigned short *)CAN_MB06_TIMESTAMP)
#define pCAN_MB06_ID0		((volatile unsigned short *)CAN_MB06_ID0)
#define pCAN_MB06_ID1		((volatile unsigned short *)CAN_MB06_ID1)

#define pCAN_MB07_DATA0		((volatile unsigned short *)CAN_MB07_DATA0)
#define pCAN_MB07_DATA1		((volatile unsigned short *)CAN_MB07_DATA1)
#define pCAN_MB07_DATA2		((volatile unsigned short *)CAN_MB07_DATA2)
#define pCAN_MB07_DATA3		((volatile unsigned short *)CAN_MB07_DATA3)
#define pCAN_MB07_LENGTH	((volatile unsigned short *)CAN_MB07_LENGTH)
#define pCAN_MB07_TIMESTAMP	((volatile unsigned short *)CAN_MB07_TIMESTAMP)
#define pCAN_MB07_ID0		((volatile unsigned short *)CAN_MB07_ID0)
#define pCAN_MB07_ID1		((volatile unsigned short *)CAN_MB07_ID1)

#define pCAN_MB08_DATA0		((volatile unsigned short *)CAN_MB08_DATA0)
#define pCAN_MB08_DATA1		((volatile unsigned short *)CAN_MB08_DATA1)
#define pCAN_MB08_DATA2		((volatile unsigned short *)CAN_MB08_DATA2)
#define pCAN_MB08_DATA3		((volatile unsigned short *)CAN_MB08_DATA3)
#define pCAN_MB08_LENGTH	((volatile unsigned short *)CAN_MB08_LENGTH)
#define pCAN_MB08_TIMESTAMP	((volatile unsigned short *)CAN_MB08_TIMESTAMP)
#define pCAN_MB08_ID0		((volatile unsigned short *)CAN_MB08_ID0)
#define pCAN_MB08_ID1		((volatile unsigned short *)CAN_MB08_ID1)

#define pCAN_MB09_DATA0		((volatile unsigned short *)CAN_MB09_DATA0)
#define pCAN_MB09_DATA1		((volatile unsigned short *)CAN_MB09_DATA1)
#define pCAN_MB09_DATA2		((volatile unsigned short *)CAN_MB09_DATA2)
#define pCAN_MB09_DATA3		((volatile unsigned short *)CAN_MB09_DATA3)
#define pCAN_MB09_LENGTH	((volatile unsigned short *)CAN_MB09_LENGTH)
#define pCAN_MB09_TIMESTAMP	((volatile unsigned short *)CAN_MB09_TIMESTAMP)
#define pCAN_MB09_ID0		((volatile unsigned short *)CAN_MB09_ID0)
#define pCAN_MB09_ID1		((volatile unsigned short *)CAN_MB09_ID1)

#define pCAN_MB10_DATA0		((volatile unsigned short *)CAN_MB10_DATA0)
#define pCAN_MB10_DATA1		((volatile unsigned short *)CAN_MB10_DATA1)
#define pCAN_MB10_DATA2		((volatile unsigned short *)CAN_MB10_DATA2)
#define pCAN_MB10_DATA3		((volatile unsigned short *)CAN_MB10_DATA3)
#define pCAN_MB10_LENGTH	((volatile unsigned short *)CAN_MB10_LENGTH)
#define pCAN_MB10_TIMESTAMP	((volatile unsigned short *)CAN_MB10_TIMESTAMP)
#define pCAN_MB10_ID0		((volatile unsigned short *)CAN_MB10_ID0)
#define pCAN_MB10_ID1		((volatile unsigned short *)CAN_MB10_ID1)

#define pCAN_MB11_DATA0		((volatile unsigned short *)CAN_MB11_DATA0)
#define pCAN_MB11_DATA1		((volatile unsigned short *)CAN_MB11_DATA1)
#define pCAN_MB11_DATA2		((volatile unsigned short *)CAN_MB11_DATA2)
#define pCAN_MB11_DATA3		((volatile unsigned short *)CAN_MB11_DATA3)
#define pCAN_MB11_LENGTH	((volatile unsigned short *)CAN_MB11_LENGTH)
#define pCAN_MB11_TIMESTAMP	((volatile unsigned short *)CAN_MB11_TIMESTAMP)
#define pCAN_MB11_ID0		((volatile unsigned short *)CAN_MB11_ID0)
#define pCAN_MB11_ID1		((volatile unsigned short *)CAN_MB11_ID1)

#define pCAN_MB12_DATA0		((volatile unsigned short *)CAN_MB12_DATA0)
#define pCAN_MB12_DATA1		((volatile unsigned short *)CAN_MB12_DATA1)
#define pCAN_MB12_DATA2		((volatile unsigned short *)CAN_MB12_DATA2)
#define pCAN_MB12_DATA3		((volatile unsigned short *)CAN_MB12_DATA3)
#define pCAN_MB12_LENGTH	((volatile unsigned short *)CAN_MB12_LENGTH)
#define pCAN_MB12_TIMESTAMP	((volatile unsigned short *)CAN_MB12_TIMESTAMP)
#define pCAN_MB12_ID0		((volatile unsigned short *)CAN_MB12_ID0)
#define pCAN_MB12_ID1		((volatile unsigned short *)CAN_MB12_ID1)

#define pCAN_MB13_DATA0		((volatile unsigned short *)CAN_MB13_DATA0)
#define pCAN_MB13_DATA1		((volatile unsigned short *)CAN_MB13_DATA1)
#define pCAN_MB13_DATA2		((volatile unsigned short *)CAN_MB13_DATA2)
#define pCAN_MB13_DATA3		((volatile unsigned short *)CAN_MB13_DATA3)
#define pCAN_MB13_LENGTH	((volatile unsigned short *)CAN_MB13_LENGTH)
#define pCAN_MB13_TIMESTAMP	((volatile unsigned short *)CAN_MB13_TIMESTAMP)
#define pCAN_MB13_ID0		((volatile unsigned short *)CAN_MB13_ID0)
#define pCAN_MB13_ID1		((volatile unsigned short *)CAN_MB13_ID1)

#define pCAN_MB14_DATA0		((volatile unsigned short *)CAN_MB14_DATA0)
#define pCAN_MB14_DATA1		((volatile unsigned short *)CAN_MB14_DATA1)
#define pCAN_MB14_DATA2		((volatile unsigned short *)CAN_MB14_DATA2)
#define pCAN_MB14_DATA3		((volatile unsigned short *)CAN_MB14_DATA3)
#define pCAN_MB14_LENGTH	((volatile unsigned short *)CAN_MB14_LENGTH)
#define pCAN_MB14_TIMESTAMP	((volatile unsigned short *)CAN_MB14_TIMESTAMP)
#define pCAN_MB14_ID0		((volatile unsigned short *)CAN_MB14_ID0)
#define pCAN_MB14_ID1		((volatile unsigned short *)CAN_MB14_ID1)

#define pCAN_MB15_DATA0		((volatile unsigned short *)CAN_MB15_DATA0)
#define pCAN_MB15_DATA1		((volatile unsigned short *)CAN_MB15_DATA1)
#define pCAN_MB15_DATA2		((volatile unsigned short *)CAN_MB15_DATA2)
#define pCAN_MB15_DATA3		((volatile unsigned short *)CAN_MB15_DATA3)
#define pCAN_MB15_LENGTH	((volatile unsigned short *)CAN_MB15_LENGTH)
#define pCAN_MB15_TIMESTAMP	((volatile unsigned short *)CAN_MB15_TIMESTAMP)
#define pCAN_MB15_ID0		((volatile unsigned short *)CAN_MB15_ID0)
#define pCAN_MB15_ID1		((volatile unsigned short *)CAN_MB15_ID1)

#define pCAN_MB16_DATA0		((volatile unsigned short *)CAN_MB16_DATA0)
#define pCAN_MB16_DATA1		((volatile unsigned short *)CAN_MB16_DATA1)
#define pCAN_MB16_DATA2		((volatile unsigned short *)CAN_MB16_DATA2)
#define pCAN_MB16_DATA3		((volatile unsigned short *)CAN_MB16_DATA3)
#define pCAN_MB16_LENGTH	((volatile unsigned short *)CAN_MB16_LENGTH)
#define pCAN_MB16_TIMESTAMP	((volatile unsigned short *)CAN_MB16_TIMESTAMP)
#define pCAN_MB16_ID0		((volatile unsigned short *)CAN_MB16_ID0)
#define pCAN_MB16_ID1		((volatile unsigned short *)CAN_MB16_ID1)

#define pCAN_MB17_DATA0		((volatile unsigned short *)CAN_MB17_DATA0)
#define pCAN_MB17_DATA1		((volatile unsigned short *)CAN_MB17_DATA1)
#define pCAN_MB17_DATA2		((volatile unsigned short *)CAN_MB17_DATA2)
#define pCAN_MB17_DATA3		((volatile unsigned short *)CAN_MB17_DATA3)
#define pCAN_MB17_LENGTH	((volatile unsigned short *)CAN_MB17_LENGTH)
#define pCAN_MB17_TIMESTAMP	((volatile unsigned short *)CAN_MB17_TIMESTAMP)
#define pCAN_MB17_ID0		((volatile unsigned short *)CAN_MB17_ID0)
#define pCAN_MB17_ID1		((volatile unsigned short *)CAN_MB17_ID1)

#define pCAN_MB18_DATA0		((volatile unsigned short *)CAN_MB18_DATA0)
#define pCAN_MB18_DATA1		((volatile unsigned short *)CAN_MB18_DATA1)
#define pCAN_MB18_DATA2		((volatile unsigned short *)CAN_MB18_DATA2)
#define pCAN_MB18_DATA3		((volatile unsigned short *)CAN_MB18_DATA3)
#define pCAN_MB18_LENGTH	((volatile unsigned short *)CAN_MB18_LENGTH)
#define pCAN_MB18_TIMESTAMP	((volatile unsigned short *)CAN_MB18_TIMESTAMP)
#define pCAN_MB18_ID0		((volatile unsigned short *)CAN_MB18_ID0)
#define pCAN_MB18_ID1		((volatile unsigned short *)CAN_MB18_ID1)

#define pCAN_MB19_DATA0		((volatile unsigned short *)CAN_MB19_DATA0)
#define pCAN_MB19_DATA1		((volatile unsigned short *)CAN_MB19_DATA1)
#define pCAN_MB19_DATA2		((volatile unsigned short *)CAN_MB19_DATA2)
#define pCAN_MB19_DATA3		((volatile unsigned short *)CAN_MB19_DATA3)
#define pCAN_MB19_LENGTH	((volatile unsigned short *)CAN_MB19_LENGTH)
#define pCAN_MB19_TIMESTAMP	((volatile unsigned short *)CAN_MB19_TIMESTAMP)
#define pCAN_MB19_ID0		((volatile unsigned short *)CAN_MB19_ID0)
#define pCAN_MB19_ID1		((volatile unsigned short *)CAN_MB19_ID1)

#define pCAN_MB20_DATA0		((volatile unsigned short *)CAN_MB20_DATA0)
#define pCAN_MB20_DATA1		((volatile unsigned short *)CAN_MB20_DATA1)
#define pCAN_MB20_DATA2		((volatile unsigned short *)CAN_MB20_DATA2)
#define pCAN_MB20_DATA3		((volatile unsigned short *)CAN_MB20_DATA3)
#define pCAN_MB20_LENGTH	((volatile unsigned short *)CAN_MB20_LENGTH)
#define pCAN_MB20_TIMESTAMP	((volatile unsigned short *)CAN_MB20_TIMESTAMP)
#define pCAN_MB20_ID0		((volatile unsigned short *)CAN_MB20_ID0)
#define pCAN_MB20_ID1		((volatile unsigned short *)CAN_MB20_ID1)

#define pCAN_MB21_DATA0		((volatile unsigned short *)CAN_MB21_DATA0)
#define pCAN_MB21_DATA1		((volatile unsigned short *)CAN_MB21_DATA1)
#define pCAN_MB21_DATA2		((volatile unsigned short *)CAN_MB21_DATA2)
#define pCAN_MB21_DATA3		((volatile unsigned short *)CAN_MB21_DATA3)
#define pCAN_MB21_LENGTH	((volatile unsigned short *)CAN_MB21_LENGTH)
#define pCAN_MB21_TIMESTAMP	((volatile unsigned short *)CAN_MB21_TIMESTAMP)
#define pCAN_MB21_ID0		((volatile unsigned short *)CAN_MB21_ID0)
#define pCAN_MB21_ID1		((volatile unsigned short *)CAN_MB21_ID1)

#define pCAN_MB22_DATA0		((volatile unsigned short *)CAN_MB22_DATA0)
#define pCAN_MB22_DATA1		((volatile unsigned short *)CAN_MB22_DATA1)
#define pCAN_MB22_DATA2		((volatile unsigned short *)CAN_MB22_DATA2)
#define pCAN_MB22_DATA3		((volatile unsigned short *)CAN_MB22_DATA3)
#define pCAN_MB22_LENGTH	((volatile unsigned short *)CAN_MB22_LENGTH)
#define pCAN_MB22_TIMESTAMP	((volatile unsigned short *)CAN_MB22_TIMESTAMP)
#define pCAN_MB22_ID0		((volatile unsigned short *)CAN_MB22_ID0)
#define pCAN_MB22_ID1		((volatile unsigned short *)CAN_MB22_ID1)

#define pCAN_MB23_DATA0		((volatile unsigned short *)CAN_MB23_DATA0)
#define pCAN_MB23_DATA1		((volatile unsigned short *)CAN_MB23_DATA1)
#define pCAN_MB23_DATA2		((volatile unsigned short *)CAN_MB23_DATA2)
#define pCAN_MB23_DATA3		((volatile unsigned short *)CAN_MB23_DATA3)
#define pCAN_MB23_LENGTH	((volatile unsigned short *)CAN_MB23_LENGTH)
#define pCAN_MB23_TIMESTAMP	((volatile unsigned short *)CAN_MB23_TIMESTAMP)
#define pCAN_MB23_ID0		((volatile unsigned short *)CAN_MB23_ID0)
#define pCAN_MB23_ID1		((volatile unsigned short *)CAN_MB23_ID1)

#define pCAN_MB24_DATA0		((volatile unsigned short *)CAN_MB24_DATA0)
#define pCAN_MB24_DATA1		((volatile unsigned short *)CAN_MB24_DATA1)
#define pCAN_MB24_DATA2		((volatile unsigned short *)CAN_MB24_DATA2)
#define pCAN_MB24_DATA3		((volatile unsigned short *)CAN_MB24_DATA3)
#define pCAN_MB24_LENGTH	((volatile unsigned short *)CAN_MB24_LENGTH)
#define pCAN_MB24_TIMESTAMP	((volatile unsigned short *)CAN_MB24_TIMESTAMP)
#define pCAN_MB24_ID0		((volatile unsigned short *)CAN_MB24_ID0)
#define pCAN_MB24_ID1		((volatile unsigned short *)CAN_MB24_ID1)

#define pCAN_MB25_DATA0		((volatile unsigned short *)CAN_MB25_DATA0)
#define pCAN_MB25_DATA1		((volatile unsigned short *)CAN_MB25_DATA1)
#define pCAN_MB25_DATA2		((volatile unsigned short *)CAN_MB25_DATA2)
#define pCAN_MB25_DATA3		((volatile unsigned short *)CAN_MB25_DATA3)
#define pCAN_MB25_LENGTH	((volatile unsigned short *)CAN_MB25_LENGTH)
#define pCAN_MB25_TIMESTAMP	((volatile unsigned short *)CAN_MB25_TIMESTAMP)
#define pCAN_MB25_ID0		((volatile unsigned short *)CAN_MB25_ID0)
#define pCAN_MB25_ID1		((volatile unsigned short *)CAN_MB25_ID1)

#define pCAN_MB26_DATA0		((volatile unsigned short *)CAN_MB26_DATA0)
#define pCAN_MB26_DATA1		((volatile unsigned short *)CAN_MB26_DATA1)
#define pCAN_MB26_DATA2		((volatile unsigned short *)CAN_MB26_DATA2)
#define pCAN_MB26_DATA3		((volatile unsigned short *)CAN_MB26_DATA3)
#define pCAN_MB26_LENGTH	((volatile unsigned short *)CAN_MB26_LENGTH)
#define pCAN_MB26_TIMESTAMP	((volatile unsigned short *)CAN_MB26_TIMESTAMP)
#define pCAN_MB26_ID0		((volatile unsigned short *)CAN_MB26_ID0)
#define pCAN_MB26_ID1		((volatile unsigned short *)CAN_MB26_ID1)

#define pCAN_MB27_DATA0		((volatile unsigned short *)CAN_MB27_DATA0)
#define pCAN_MB27_DATA1		((volatile unsigned short *)CAN_MB27_DATA1)
#define pCAN_MB27_DATA2		((volatile unsigned short *)CAN_MB27_DATA2)
#define pCAN_MB27_DATA3		((volatile unsigned short *)CAN_MB27_DATA3)
#define pCAN_MB27_LENGTH	((volatile unsigned short *)CAN_MB27_LENGTH)
#define pCAN_MB27_TIMESTAMP	((volatile unsigned short *)CAN_MB27_TIMESTAMP)
#define pCAN_MB27_ID0		((volatile unsigned short *)CAN_MB27_ID0)
#define pCAN_MB27_ID1		((volatile unsigned short *)CAN_MB27_ID1)

#define pCAN_MB28_DATA0		((volatile unsigned short *)CAN_MB28_DATA0)
#define pCAN_MB28_DATA1		((volatile unsigned short *)CAN_MB28_DATA1)
#define pCAN_MB28_DATA2		((volatile unsigned short *)CAN_MB28_DATA2)
#define pCAN_MB28_DATA3		((volatile unsigned short *)CAN_MB28_DATA3)
#define pCAN_MB28_LENGTH	((volatile unsigned short *)CAN_MB28_LENGTH)
#define pCAN_MB28_TIMESTAMP	((volatile unsigned short *)CAN_MB28_TIMESTAMP)
#define pCAN_MB28_ID0		((volatile unsigned short *)CAN_MB28_ID0)
#define pCAN_MB28_ID1		((volatile unsigned short *)CAN_MB28_ID1)

#define pCAN_MB29_DATA0		((volatile unsigned short *)CAN_MB29_DATA0)
#define pCAN_MB29_DATA1		((volatile unsigned short *)CAN_MB29_DATA1)
#define pCAN_MB29_DATA2		((volatile unsigned short *)CAN_MB29_DATA2)
#define pCAN_MB29_DATA3		((volatile unsigned short *)CAN_MB29_DATA3)
#define pCAN_MB29_LENGTH	((volatile unsigned short *)CAN_MB29_LENGTH)
#define pCAN_MB29_TIMESTAMP	((volatile unsigned short *)CAN_MB29_TIMESTAMP)
#define pCAN_MB29_ID0		((volatile unsigned short *)CAN_MB29_ID0)
#define pCAN_MB29_ID1		((volatile unsigned short *)CAN_MB29_ID1)

#define pCAN_MB30_DATA0		((volatile unsigned short *)CAN_MB30_DATA0)
#define pCAN_MB30_DATA1		((volatile unsigned short *)CAN_MB30_DATA1)
#define pCAN_MB30_DATA2		((volatile unsigned short *)CAN_MB30_DATA2)
#define pCAN_MB30_DATA3		((volatile unsigned short *)CAN_MB30_DATA3)
#define pCAN_MB30_LENGTH	((volatile unsigned short *)CAN_MB30_LENGTH)
#define pCAN_MB30_TIMESTAMP	((volatile unsigned short *)CAN_MB30_TIMESTAMP)
#define pCAN_MB30_ID0		((volatile unsigned short *)CAN_MB30_ID0)
#define pCAN_MB30_ID1		((volatile unsigned short *)CAN_MB30_ID1)

#define pCAN_MB31_DATA0		((volatile unsigned short *)CAN_MB31_DATA0)
#define pCAN_MB31_DATA1		((volatile unsigned short *)CAN_MB31_DATA1)
#define pCAN_MB31_DATA2		((volatile unsigned short *)CAN_MB31_DATA2)
#define pCAN_MB31_DATA3		((volatile unsigned short *)CAN_MB31_DATA3)
#define pCAN_MB31_LENGTH	((volatile unsigned short *)CAN_MB31_LENGTH)
#define pCAN_MB31_TIMESTAMP	((volatile unsigned short *)CAN_MB31_TIMESTAMP)
#define pCAN_MB31_ID0		((volatile unsigned short *)CAN_MB31_ID0)
#define pCAN_MB31_ID1		((volatile unsigned short *)CAN_MB31_ID1)


/* CAN Mailbox Area Macros		*/
#define pCAN_MB_ID1(x)			((volatile unsigned short *)CAN_MB_ID1(x))
#define pCAN_MB_ID0(x)			((volatile unsigned short *)CAN_MB_ID0(x))
#define pCAN_MB_TIMESTAMP(x)	((volatile unsigned short *)CAN_MB_TIMESTAMP(x))
#define pCAN_MB_LENGTH(x)		((volatile unsigned short *)CAN_MB_LENGTH(x))
#define pCAN_MB_DATA3(x)		((volatile unsigned short *)CAN_MB_DATA3(x))
#define pCAN_MB_DATA2(x)		((volatile unsigned short *)CAN_MB_DATA2(x))
#define pCAN_MB_DATA1(x)		((volatile unsigned short *)CAN_MB_DATA1(x))
#define pCAN_MB_DATA0(x)		((volatile unsigned short *)CAN_MB_DATA0(x))


/* Alternate Deprecated Macros Provided For Backwards Code Compatibility */
#define pCAN_CNF         	pCAN_DEBUG
#define pTWI0_PRESCALE   	pTWI0_CONTROL
#define pTWI0_INT_SRC	 	pTWI0_INT_STAT
#define pTWI0_INT_ENABLE 	pTWI0_INT_MASK
#define pTWI1_PRESCALE		pTWI1_CONTROL
#define pTWI1_INT_SRC		pTWI1_INT_STAT
#define pTWI1_INT_ENABLE	pTWI1_INT_MASK

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* _CDEF_BF538_H */

