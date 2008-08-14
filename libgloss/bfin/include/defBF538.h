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
**
** defBF538.h
**
** Copyright (C) 2008 Analog Devices, Inc.
**
*************************************************************************/

/*
** This include file contains a list of macro "defines" to enable the
** programmer to use symbolic names for ADSP-BF538 peripherals.
*/

#ifndef _DEF_BF538_H
#define _DEF_BF538_H

/* Include all Core registers and bit definitions */
#include <def_LPBlackfin.h>

/* Include all System registers and bit definitions common to ADSP-BF532 */
#include <defBF532.h>

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_19_4)
#pragma diag(suppress:misra_rule_19_6)
#pragma diag(suppress:misra_rule_19_7)
#endif /* _MISRA_RULES */


/********************************************************************************
 * System MMR Register Map
 ********************************************************************************/
/* Define MMR Space for Additional BF538 Peripherals */
/* System Interrupt Controller (0xFFC00100 - 0xFFC001FF)						*/
/* SIC0 on ADSP-BF538 Is Same As SIC on ADSP-BF532		*/
#define SIC_IMASK0			SIC_IMASK	/* Interrupt Mask Register 0			*/
#define SIC_ISR0			SIC_ISR	/* Interrupt Status Register 0			*/
#define SIC_IWR0			SIC_IWR	/* Interrupt Wakeup Register 0			*/

/* Add SIC1 MMRs for ADSP-BF538 Processors		*/
#define SIC_IMASK1			0xFFC00128	/* Interrupt Mask Register 1			*/
#define SIC_ISR1			0xFFC0012C	/* Interrupt Status Register 1			*/
#define SIC_IWR1			0xFFC00130	/* Interrupt Wakeup Register 1			*/

/* Add SIC1 Interrupt Sources for ADSP-BF538 Processors		*/
#define SIC_IAR3			0xFFC0011C	/* Interrupt Assignment Register 3		*/
#define SIC_IAR4			0xFFC00134	/* Interrupt Assignment Register 4		*/
#define SIC_IAR5			0xFFC00138	/* Interrupt Assignment Register 5		*/
#define SIC_IAR6			0xFFC0013C	/* Interrupt Assignment Register 6		*/


/* UART0 Controller		(0xFFC00400 - 0xFFC004FF)								*/
/* UART0 on ADSP-BF538 Is Same As UART on ADSP-BF532	*/
#define UART0_THR			UART_THR	/* Transmit Holding register			*/
#define UART0_RBR			UART_RBR	/* Receive Buffer register				*/
#define UART0_DLL			UART_DLL	/* Divisor Latch (Low-Byte)				*/
#define UART0_IER			UART_IER	/* Interrupt Enable Register			*/
#define UART0_DLH			UART_DLH	/* Divisor Latch (High-Byte)			*/
#define UART0_IIR			UART_IIR	/* Interrupt Identification Register	*/
#define UART0_LCR			UART_LCR	/* Line Control Register				*/
#define UART0_MCR			UART_MCR	/* Modem Control Register				*/
#define UART0_LSR			UART_LSR	/* Line Status Register					*/
#define UART0_SCR			UART_SCR	/* SCR Scratch Register					*/
#define UART0_GCTL			UART_GCTL	/* Global Control Register				*/


/* SPI0 Controller		(0xFFC00500 - 0xFFC005FF)								*/
/* SPI0 on ADSP-BF538 Is Same As SPI on ADSP-BF532	*/
#define SPI0_CTL			SPI_CTL	/* SPI0 Control Register				*/
#define SPI0_FLG			SPI_FLG	/* SPI0 Flag register					*/
#define SPI0_STAT			SPI_STAT	/* SPI0 Status register					*/
#define SPI0_TDBR			SPI_TDBR	/* SPI0 Transmit Data Buffer Register	*/
#define SPI0_RDBR			SPI_RDBR	/* SPI0 Receive Data Buffer Register	*/
#define SPI0_BAUD			SPI_BAUD	/* SPI0 Baud rate Register				*/
#define SPI0_SHADOW			SPI_SHADOW	/* SPI0_RDBR Shadow Register			*/


/* General-Purpose Port F	(0xFFC00700 - 0xFFC007FF)										*/
/* ADSP-BF538 Refers to FIO as GPIO Port F */
#define PORTFIO					FIO_FLAG_D	/* GPIO Port F Pin State Specify Register				*/
#define PORTFIO_CLEAR			FIO_FLAG_C	/* Peripheral Interrupt GPIO Clear Register		*/
#define PORTFIO_SET				FIO_FLAG_S	/* Peripheral Interrupt GPIO Set Register		*/
#define PORTFIO_TOGGLE			FIO_FLAG_T	/* GPIO Port F Pin State Toggle Register				*/
#define PORTFIO_MASKA			FIO_MASKA_D	/* GPIO Port F Mask State Specify Interrupt A Register	*/
#define PORTFIO_MASKA_CLEAR		FIO_MASKA_C	/* GPIO Port F Mask Disable Interrupt A Register		*/
#define PORTFIO_MASKA_SET		FIO_MASKA_S	/* GPIO Port F Mask Enable Interrupt A Register		*/
#define PORTFIO_MASKA_TOGGLE	FIO_MASKA_T	/* GPIO Port F Mask Toggle Enable Interrupt A Register	*/
#define PORTFIO_MASKB			FIO_MASKB_D	/* GPIO Port F Mask State Specify Interrupt B Register	*/
#define PORTFIO_MASKB_CLEAR		FIO_MASKB_C	/* GPIO Port F Mask Disable Interrupt B Register		*/
#define PORTFIO_MASKB_SET		FIO_MASKB_S	/* GPIO Port F Mask Enable Interrupt B Register		*/
#define PORTFIO_MASKB_TOGGLE	FIO_MASKB_T	/* GPIO Port F Mask Toggle Enable Interrupt B Register	*/
#define PORTFIO_DIR				FIO_DIR	/* GPIO Port F Direction Register			*/
#define PORTFIO_POLAR			FIO_POLAR	/* GPIO Port F Source Polarity Register				*/
#define PORTFIO_EDGE			FIO_EDGE	/* GPIO Port F Source Sensitivity Register				*/
#define PORTFIO_BOTH			FIO_BOTH	/* GPIO Port F Set on BOTH Edges Register				*/
#define PORTFIO_INEN			FIO_INEN	/* GPIO Port F Input Enable Register					*/


/* DMA0 Test Registers (0xFFC00B00 - 0xFFC00BFF)										*/
/* ADSP-BF538 DMA0 Controller Is Same As ADSP-BF532 DMA Controller	*/
#define DMAC0_TC_PER 			0xFFC00B0C	/* Traffic Control Periods Register */
#define DMAC0_TC_CNT 			0xFFC00B10	/* Traffic Control Current Counts Register */
/* Alternate deprecated register names (below) provided for backwards code compatibility */
#define DMA0_TC_PER				DMAC0_TC_PER	/* Traffic Control Periods Register			*/
#define DMA0_TC_CNT				DMAC0_TC_CNT	/* Traffic Control Current Counts Register	*/
/* Alternate deprecated register names (below) provided for backwards code compatibility */
#define DMA0_TCPER				DMA0_TC_PER	/* Traffic Control Periods Register			*/
#define DMA0_TCCNT				DMA0_TC_CNT	/* Traffic Control Current Counts Register	*/

/* ADSP-BF538 Must Enumerate Memory DMA Channels By Controller */
#define MDMA0_D0_NEXT_DESC_PTR	MDMA_D0_NEXT_DESC_PTR	/* MemDMA0 Stream 0 Destination Next Descriptor Pointer Register		*/
#define MDMA0_D0_START_ADDR		MDMA_D0_START_ADDR	/* MemDMA0 Stream 0 Destination Start Address Register				*/
#define MDMA0_D0_CONFIG			MDMA_D0_CONFIG	/* MemDMA0 Stream 0 Destination Configuration Register				*/
#define MDMA0_D0_X_COUNT		MDMA_D0_X_COUNT	/* MemDMA0 Stream 0 Destination X Count Register						*/
#define MDMA0_D0_X_MODIFY		MDMA_D0_X_MODIFY	/* MemDMA0 Stream 0 Destination X Modify Register					*/
#define MDMA0_D0_Y_COUNT		MDMA_D0_Y_COUNT	/* MemDMA0 Stream 0 Destination Y Count Register						*/
#define MDMA0_D0_Y_MODIFY		MDMA_D0_Y_MODIFY	/* MemDMA0 Stream 0 Destination Y Modify Register					*/
#define MDMA0_D0_CURR_DESC_PTR	MDMA_D0_CURR_DESC_PTR	/* MemDMA0 Stream 0 Destination Current Descriptor Pointer Register	*/
#define MDMA0_D0_CURR_ADDR		MDMA_D0_CURR_ADDR	/* MemDMA0 Stream 0 Destination Current Address Register				*/
#define MDMA0_D0_IRQ_STATUS		MDMA_D0_IRQ_STATUS	/* MemDMA0 Stream 0 Destination Interrupt/Status Register			*/
#define MDMA0_D0_PERIPHERAL_MAP	MDMA_D0_PERIPHERAL_MAP	/* MemDMA0 Stream 0 Destination Peripheral Map Register				*/
#define MDMA0_D0_CURR_X_COUNT	MDMA_D0_CURR_X_COUNT	/* MemDMA0 Stream 0 Destination Current X Count Register				*/
#define MDMA0_D0_CURR_Y_COUNT	MDMA_D0_CURR_Y_COUNT	/* MemDMA0 Stream 0 Destination Current Y Count Register				*/

#define MDMA0_S0_NEXT_DESC_PTR	MDMA_S0_NEXT_DESC_PTR	/* MemDMA0 Stream 0 Source Next Descriptor Pointer Register		*/
#define MDMA0_S0_START_ADDR		MDMA_S0_START_ADDR	/* MemDMA0 Stream 0 Source Start Address Register				*/
#define MDMA0_S0_CONFIG			MDMA_S0_CONFIG	/* MemDMA0 Stream 0 Source Configuration Register				*/
#define MDMA0_S0_X_COUNT		MDMA_S0_X_COUNT	/* MemDMA Stream 0 Source X Count Register						*/
#define MDMA0_S0_X_MODIFY		MDMA_S0_X_MODIFY	/* MemDMA0 Stream 0 Source X Modify Register					*/
#define MDMA0_S0_Y_COUNT		MDMA_S0_Y_COUNT	/* MemDMA0 Stream 0 Source Y Count Register						*/
#define MDMA0_S0_Y_MODIFY		MDMA_S0_Y_MODIFY	/* MemDMA0 Stream 0 Source Y Modify Register					*/
#define MDMA0_S0_CURR_DESC_PTR	MDMA_S0_CURR_DESC_PTR	/* MemDMA0 Stream 0 Source Current Descriptor Pointer Register	*/
#define MDMA0_S0_CURR_ADDR		MDMA_S0_CURR_ADDR	/* MemDMA0 Stream 0 Source Current Address Register				*/
#define MDMA0_S0_IRQ_STATUS		MDMA_S0_IRQ_STATUS	/* MemDMA0 Stream 0 Source Interrupt/Status Register			*/
#define MDMA0_S0_PERIPHERAL_MAP	MDMA_S0_PERIPHERAL_MAP	/* MemDMA0 Stream 0 Source Peripheral Map Register				*/
#define MDMA0_S0_CURR_X_COUNT	MDMA_S0_CURR_X_COUNT	/* MemDMA0 Stream 0 Source Current X Count Register				*/
#define MDMA0_S0_CURR_Y_COUNT	MDMA_S0_CURR_Y_COUNT	/* MemDMA0 Stream 0 Source Current Y Count Register				*/

#define MDMA0_D1_NEXT_DESC_PTR	MDMA_D1_NEXT_DESC_PTR	/* MemDMA0 Stream 1 Destination Next Descriptor Pointer Register		*/
#define MDMA0_D1_START_ADDR		MDMA_D1_START_ADDR	/* MemDMA0 Stream 1 Destination Start Address Register				*/
#define MDMA0_D1_CONFIG			MDMA_D1_CONFIG	/* MemDMA0 Stream 1 Destination Configuration Register				*/
#define MDMA0_D1_X_COUNT		MDMA_D1_X_COUNT	/* MemDMA0 Stream 1 Destination X Count Register						*/
#define MDMA0_D1_X_MODIFY		MDMA_D1_X_MODIFY	/* MemDMA0 Stream 1 Destination X Modify Register					*/
#define MDMA0_D1_Y_COUNT		MDMA_D1_Y_COUNT	/* MemDMA0 Stream 1 Destination Y Count Register						*/
#define MDMA0_D1_Y_MODIFY		MDMA_D1_Y_MODIFY	/* MemDMA0 Stream 1 Destination Y Modify Register					*/
#define MDMA0_D1_CURR_DESC_PTR	MDMA_D1_CURR_DESC_PTR	/* MemDMA0 Stream 1 Destination Current Descriptor Pointer Register	*/
#define MDMA0_D1_CURR_ADDR		MDMA_D1_CURR_ADDR	/* MemDMA0 Stream 1 Destination Current Address Register				*/
#define MDMA0_D1_IRQ_STATUS		MDMA_D1_IRQ_STATUS	/* MemDMA0 Stream 1 Destination Interrupt/Status Register			*/
#define MDMA0_D1_PERIPHERAL_MAP	MDMA_D1_PERIPHERAL_MAP	/* MemDMA0 Stream 1 Destination Peripheral Map Register				*/
#define MDMA0_D1_CURR_X_COUNT	MDMA_D1_CURR_X_COUNT	/* MemDMA0 Stream 1 Destination Current X Count Register				*/
#define MDMA0_D1_CURR_Y_COUNT	MDMA_D1_CURR_Y_COUNT	/* MemDMA0 Stream 1 Destination Current Y Count Register				*/

#define MDMA0_S1_NEXT_DESC_PTR	MDMA_S1_NEXT_DESC_PTR	/* MemDMA0 Stream 1 Source Next Descriptor Pointer Register		*/
#define MDMA0_S1_START_ADDR		MDMA_S1_START_ADDR	/* MemDMA0 Stream 1 Source Start Address Register				*/
#define MDMA0_S1_CONFIG			MDMA_S1_CONFIG	/* MemDMA0 Stream 1 Source Configuration Register				*/
#define MDMA0_S1_X_COUNT		MDMA_S1_X_COUNT	/* MemDMA0 Stream 1 Source X Count Register						*/
#define MDMA0_S1_X_MODIFY		MDMA_S1_X_MODIFY	/* MemDMA0 Stream 1 Source X Modify Register					*/
#define MDMA0_S1_Y_COUNT		MDMA_S1_Y_COUNT	/* MemDMA0 Stream 1 Source Y Count Register						*/
#define MDMA0_S1_Y_MODIFY		MDMA_S1_Y_MODIFY	/* MemDMA0 Stream 1 Source Y Modify Register					*/
#define MDMA0_S1_CURR_DESC_PTR	MDMA_S1_CURR_DESC_PTR	/* MemDMA0 Stream 1 Source Current Descriptor Pointer Register	*/
#define MDMA0_S1_CURR_ADDR		MDMA_S1_CURR_ADDR	/* MemDMA0 Stream 1 Source Current Address Register				*/
#define MDMA0_S1_IRQ_STATUS		MDMA_S1_IRQ_STATUS	/* MemDMA0 Stream 1 Source Interrupt/Status Register			*/
#define MDMA0_S1_PERIPHERAL_MAP	MDMA_S1_PERIPHERAL_MAP	/* MemDMA0 Stream 1 Source Peripheral Map Register				*/
#define MDMA0_S1_CURR_X_COUNT	MDMA_S1_CURR_X_COUNT	/* MemDMA0 Stream 1 Source Current X Count Register				*/
#define MDMA0_S1_CURR_Y_COUNT	MDMA_S1_CURR_Y_COUNT	/* MemDMA0 Stream 1 Source Current Y Count Register				*/


/* Two-Wire Interface 0	(0xFFC01400 - 0xFFC014FF)										*/
#define TWI0_CLKDIV			0xFFC01400	/* Serial Clock Divider Register				*/
#define TWI0_CONTROL		0xFFC01404	/* TWI0 Master Internal Time Reference Register	*/
#define TWI0_SLAVE_CTRL		0xFFC01408	/* Slave Mode Control Register					*/
#define TWI0_SLAVE_STAT		0xFFC0140C	/* Slave Mode Status Register					*/
#define TWI0_SLAVE_ADDR		0xFFC01410	/* Slave Mode Address Register					*/
#define TWI0_MASTER_CTRL	0xFFC01414	/* Master Mode Control Register					*/
#define TWI0_MASTER_STAT	0xFFC01418	/* Master Mode Status Register					*/
#define TWI0_MASTER_ADDR	0xFFC0141C	/* Master Mode Address Register					*/
#define TWI0_INT_STAT		0xFFC01420	/* TWI0 Master Interrupt Register				*/
#define TWI0_INT_MASK		0xFFC01424	/* TWI0 Master Interrupt Mask Register			*/
#define TWI0_FIFO_CTRL		0xFFC01428	/* FIFO Control Register						*/
#define TWI0_FIFO_STAT		0xFFC0142C	/* FIFO Status Register							*/
#define TWI0_XMT_DATA8		0xFFC01480	/* FIFO Transmit Data Single Byte Register		*/
#define TWI0_XMT_DATA16		0xFFC01484	/* FIFO Transmit Data Double Byte Register		*/
#define TWI0_RCV_DATA8		0xFFC01488	/* FIFO Receive Data Single Byte Register		*/
#define TWI0_RCV_DATA16		0xFFC0148C	/* FIFO Receive Data Double Byte Register		*/


/* General-Purpose Ports  (0xFFC01500 - 0xFFC015FF)								*/
/* Port C */
#define PORTCIO_FER			0xFFC01500	/* GPIO Pin Port C Configuration Register	*/
#define PORTCIO				0xFFC01510	/* GPIO Pin Port C Data Register			*/
#define PORTCIO_CLEAR		0xFFC01520	/* Clear GPIO Pin Port C Register			*/
#define PORTCIO_SET			0xFFC01530	/* Set GPIO Pin Port C Register				*/
#define PORTCIO_TOGGLE		0xFFC01540	/* Toggle GPIO Pin Port C Register			*/
#define PORTCIO_DIR			0xFFC01550	/* GPIO Pin Port C Direction Register		*/
#define PORTCIO_INEN		0xFFC01560	/* GPIO Pin Port C Input Enable Register	*/

/* Port D */
#define PORTDIO_FER			0xFFC01504	/* GPIO Pin Port D Configuration Register	*/
#define PORTDIO				0xFFC01514	/* GPIO Pin Port D Data Register			*/
#define PORTDIO_CLEAR		0xFFC01524	/* Clear GPIO Pin Port D Register			*/
#define PORTDIO_SET			0xFFC01534	/* Set GPIO Pin Port D Register				*/
#define PORTDIO_TOGGLE		0xFFC01544	/* Toggle GPIO Pin Port D Register			*/
#define PORTDIO_DIR			0xFFC01554	/* GPIO Pin Port D Direction Register		*/
#define PORTDIO_INEN		0xFFC01564	/* GPIO Pin Port D Input Enable Register	*/

/* Port E */
#define PORTEIO_FER			0xFFC01508	/* GPIO Pin Port E Configuration Register	*/
#define PORTEIO				0xFFC01518	/* GPIO Pin Port E Data Register			*/
#define PORTEIO_CLEAR		0xFFC01528	/* Clear GPIO Pin Port E Register			*/
#define PORTEIO_SET			0xFFC01538	/* Set GPIO Pin Port E Register				*/
#define PORTEIO_TOGGLE		0xFFC01548	/* Toggle GPIO Pin Port E Register			*/
#define PORTEIO_DIR			0xFFC01558	/* GPIO Pin Port E Direction Register		*/
#define PORTEIO_INEN		0xFFC01568	/* GPIO Pin Port E Input Enable Register	*/


/* ADSP-BF538 Adds DMA1 Controller	*/
/* DMA1 Test Registers (0xFFC01B00 - 0xFFC01BFF)										*/
#define DMAC1_TC_PER				0xFFC01B0C	/* Traffic Control Periods Register			*/
#define DMAC1_TC_CNT				0xFFC01B10	/* Traffic Control Current Counts Register	*/
/* Alternate deprecated register names (below) provided for backwards code compatibility */
#define DMA1_TC_PER				DMAC1_TC_PER	/* Traffic Control Periods Register			*/
#define DMA1_TC_CNT				DMAC1_TC_CNT	/* Traffic Control Current Counts Register	*/
/* Alternate deprecated register names (below) provided for backwards code compatibility */
#define DMA1_TCPER				DMA1_TC_PER	/* Traffic Control Periods Register			*/
#define DMA1_TCCNT				DMA1_TC_CNT	/* Traffic Control Current Counts Register	*/

/* DMA Controller 1 (0xFFC01C00 - 0xFFC01FFF)														*/
#define DMA8_NEXT_DESC_PTR		0xFFC01C00	/* DMA Channel 8 Next Descriptor Pointer Register		*/
#define DMA8_START_ADDR			0xFFC01C04	/* DMA Channel 8 Start Address Register					*/
#define DMA8_CONFIG				0xFFC01C08	/* DMA Channel 8 Configuration Register					*/
#define DMA8_X_COUNT			0xFFC01C10	/* DMA Channel 8 X Count Register						*/
#define DMA8_X_MODIFY			0xFFC01C14	/* DMA Channel 8 X Modify Register						*/
#define DMA8_Y_COUNT			0xFFC01C18	/* DMA Channel 8 Y Count Register						*/
#define DMA8_Y_MODIFY			0xFFC01C1C	/* DMA Channel 8 Y Modify Register						*/
#define DMA8_CURR_DESC_PTR		0xFFC01C20	/* DMA Channel 8 Current Descriptor Pointer Register	*/
#define DMA8_CURR_ADDR			0xFFC01C24	/* DMA Channel 8 Current Address Register				*/
#define DMA8_IRQ_STATUS			0xFFC01C28	/* DMA Channel 8 Interrupt/Status Register				*/
#define DMA8_PERIPHERAL_MAP		0xFFC01C2C	/* DMA Channel 8 Peripheral Map Register				*/
#define DMA8_CURR_X_COUNT		0xFFC01C30	/* DMA Channel 8 Current X Count Register				*/
#define DMA8_CURR_Y_COUNT		0xFFC01C38	/* DMA Channel 8 Current Y Count Register				*/

#define DMA9_NEXT_DESC_PTR		0xFFC01C40	/* DMA Channel 9 Next Descriptor Pointer Register		*/
#define DMA9_START_ADDR			0xFFC01C44	/* DMA Channel 9 Start Address Register					*/
#define DMA9_CONFIG				0xFFC01C48	/* DMA Channel 9 Configuration Register					*/
#define DMA9_X_COUNT			0xFFC01C50	/* DMA Channel 9 X Count Register						*/
#define DMA9_X_MODIFY			0xFFC01C54	/* DMA Channel 9 X Modify Register						*/
#define DMA9_Y_COUNT			0xFFC01C58	/* DMA Channel 9 Y Count Register						*/
#define DMA9_Y_MODIFY			0xFFC01C5C	/* DMA Channel 9 Y Modify Register						*/
#define DMA9_CURR_DESC_PTR		0xFFC01C60	/* DMA Channel 9 Current Descriptor Pointer Register	*/
#define DMA9_CURR_ADDR			0xFFC01C64	/* DMA Channel 9 Current Address Register				*/
#define DMA9_IRQ_STATUS			0xFFC01C68	/* DMA Channel 9 Interrupt/Status Register				*/
#define DMA9_PERIPHERAL_MAP		0xFFC01C6C	/* DMA Channel 9 Peripheral Map Register				*/
#define DMA9_CURR_X_COUNT		0xFFC01C70	/* DMA Channel 9 Current X Count Register				*/
#define DMA9_CURR_Y_COUNT		0xFFC01C78	/* DMA Channel 9 Current Y Count Register				*/

#define DMA10_NEXT_DESC_PTR		0xFFC01C80	/* DMA Channel 10 Next Descriptor Pointer Register		*/
#define DMA10_START_ADDR		0xFFC01C84	/* DMA Channel 10 Start Address Register				*/
#define DMA10_CONFIG			0xFFC01C88	/* DMA Channel 10 Configuration Register				*/
#define DMA10_X_COUNT			0xFFC01C90	/* DMA Channel 10 X Count Register						*/
#define DMA10_X_MODIFY			0xFFC01C94	/* DMA Channel 10 X Modify Register						*/
#define DMA10_Y_COUNT			0xFFC01C98	/* DMA Channel 10 Y Count Register						*/
#define DMA10_Y_MODIFY			0xFFC01C9C	/* DMA Channel 10 Y Modify Register						*/
#define DMA10_CURR_DESC_PTR		0xFFC01CA0	/* DMA Channel 10 Current Descriptor Pointer Register	*/
#define DMA10_CURR_ADDR			0xFFC01CA4	/* DMA Channel 10 Current Address Register				*/
#define DMA10_IRQ_STATUS		0xFFC01CA8	/* DMA Channel 10 Interrupt/Status Register				*/
#define DMA10_PERIPHERAL_MAP	0xFFC01CAC	/* DMA Channel 10 Peripheral Map Register				*/
#define DMA10_CURR_X_COUNT		0xFFC01CB0	/* DMA Channel 10 Current X Count Register				*/
#define DMA10_CURR_Y_COUNT		0xFFC01CB8	/* DMA Channel 10 Current Y Count Register				*/

#define DMA11_NEXT_DESC_PTR		0xFFC01CC0	/* DMA Channel 11 Next Descriptor Pointer Register		*/
#define DMA11_START_ADDR		0xFFC01CC4	/* DMA Channel 11 Start Address Register				*/
#define DMA11_CONFIG			0xFFC01CC8	/* DMA Channel 11 Configuration Register				*/
#define DMA11_X_COUNT			0xFFC01CD0	/* DMA Channel 11 X Count Register						*/
#define DMA11_X_MODIFY			0xFFC01CD4	/* DMA Channel 11 X Modify Register						*/
#define DMA11_Y_COUNT			0xFFC01CD8	/* DMA Channel 11 Y Count Register						*/
#define DMA11_Y_MODIFY			0xFFC01CDC	/* DMA Channel 11 Y Modify Register						*/
#define DMA11_CURR_DESC_PTR		0xFFC01CE0	/* DMA Channel 11 Current Descriptor Pointer Register	*/
#define DMA11_CURR_ADDR			0xFFC01CE4	/* DMA Channel 11 Current Address Register				*/
#define DMA11_IRQ_STATUS		0xFFC01CE8	/* DMA Channel 11 Interrupt/Status Register				*/
#define DMA11_PERIPHERAL_MAP	0xFFC01CEC	/* DMA Channel 11 Peripheral Map Register				*/
#define DMA11_CURR_X_COUNT		0xFFC01CF0	/* DMA Channel 11 Current X Count Register				*/
#define DMA11_CURR_Y_COUNT		0xFFC01CF8	/* DMA Channel 11 Current Y Count Register				*/

#define DMA12_NEXT_DESC_PTR		0xFFC01D00	/* DMA Channel 12 Next Descriptor Pointer Register		*/
#define DMA12_START_ADDR		0xFFC01D04	/* DMA Channel 12 Start Address Register				*/
#define DMA12_CONFIG			0xFFC01D08	/* DMA Channel 12 Configuration Register				*/
#define DMA12_X_COUNT			0xFFC01D10	/* DMA Channel 12 X Count Register						*/
#define DMA12_X_MODIFY			0xFFC01D14	/* DMA Channel 12 X Modify Register						*/
#define DMA12_Y_COUNT			0xFFC01D18	/* DMA Channel 12 Y Count Register						*/
#define DMA12_Y_MODIFY			0xFFC01D1C	/* DMA Channel 12 Y Modify Register						*/
#define DMA12_CURR_DESC_PTR		0xFFC01D20	/* DMA Channel 12 Current Descriptor Pointer Register	*/
#define DMA12_CURR_ADDR			0xFFC01D24	/* DMA Channel 12 Current Address Register				*/
#define DMA12_IRQ_STATUS		0xFFC01D28	/* DMA Channel 12 Interrupt/Status Register				*/
#define DMA12_PERIPHERAL_MAP	0xFFC01D2C	/* DMA Channel 12 Peripheral Map Register				*/
#define DMA12_CURR_X_COUNT		0xFFC01D30	/* DMA Channel 12 Current X Count Register				*/
#define DMA12_CURR_Y_COUNT		0xFFC01D38	/* DMA Channel 12 Current Y Count Register				*/

#define DMA13_NEXT_DESC_PTR		0xFFC01D40	/* DMA Channel 13 Next Descriptor Pointer Register		*/
#define DMA13_START_ADDR		0xFFC01D44	/* DMA Channel 13 Start Address Register				*/
#define DMA13_CONFIG			0xFFC01D48	/* DMA Channel 13 Configuration Register				*/
#define DMA13_X_COUNT			0xFFC01D50	/* DMA Channel 13 X Count Register						*/
#define DMA13_X_MODIFY			0xFFC01D54	/* DMA Channel 13 X Modify Register						*/
#define DMA13_Y_COUNT			0xFFC01D58	/* DMA Channel 13 Y Count Register						*/
#define DMA13_Y_MODIFY			0xFFC01D5C	/* DMA Channel 13 Y Modify Register						*/
#define DMA13_CURR_DESC_PTR		0xFFC01D60	/* DMA Channel 13 Current Descriptor Pointer Register	*/
#define DMA13_CURR_ADDR			0xFFC01D64	/* DMA Channel 13 Current Address Register				*/
#define DMA13_IRQ_STATUS		0xFFC01D68	/* DMA Channel 13 Interrupt/Status Register				*/
#define DMA13_PERIPHERAL_MAP	0xFFC01D6C	/* DMA Channel 13 Peripheral Map Register				*/
#define DMA13_CURR_X_COUNT		0xFFC01D70	/* DMA Channel 13 Current X Count Register				*/
#define DMA13_CURR_Y_COUNT		0xFFC01D78	/* DMA Channel 13 Current Y Count Register				*/

#define DMA14_NEXT_DESC_PTR		0xFFC01D80	/* DMA Channel 14 Next Descriptor Pointer Register		*/
#define DMA14_START_ADDR		0xFFC01D84	/* DMA Channel 14 Start Address Register				*/
#define DMA14_CONFIG			0xFFC01D88	/* DMA Channel 14 Configuration Register				*/
#define DMA14_X_COUNT			0xFFC01D90	/* DMA Channel 14 X Count Register						*/
#define DMA14_X_MODIFY			0xFFC01D94	/* DMA Channel 14 X Modify Register						*/
#define DMA14_Y_COUNT			0xFFC01D98	/* DMA Channel 14 Y Count Register						*/
#define DMA14_Y_MODIFY			0xFFC01D9C	/* DMA Channel 14 Y Modify Register						*/
#define DMA14_CURR_DESC_PTR		0xFFC01DA0	/* DMA Channel 14 Current Descriptor Pointer Register	*/
#define DMA14_CURR_ADDR			0xFFC01DA4	/* DMA Channel 14 Current Address Register				*/
#define DMA14_IRQ_STATUS		0xFFC01DA8	/* DMA Channel 14 Interrupt/Status Register				*/
#define DMA14_PERIPHERAL_MAP	0xFFC01DAC	/* DMA Channel 14 Peripheral Map Register				*/
#define DMA14_CURR_X_COUNT		0xFFC01DB0	/* DMA Channel 14 Current X Count Register				*/
#define DMA14_CURR_Y_COUNT		0xFFC01DB8	/* DMA Channel 14 Current Y Count Register				*/

#define DMA15_NEXT_DESC_PTR		0xFFC01DC0	/* DMA Channel 15 Next Descriptor Pointer Register		*/
#define DMA15_START_ADDR		0xFFC01DC4	/* DMA Channel 15 Start Address Register				*/
#define DMA15_CONFIG			0xFFC01DC8	/* DMA Channel 15 Configuration Register				*/
#define DMA15_X_COUNT			0xFFC01DD0	/* DMA Channel 15 X Count Register						*/
#define DMA15_X_MODIFY			0xFFC01DD4	/* DMA Channel 15 X Modify Register						*/
#define DMA15_Y_COUNT			0xFFC01DD8	/* DMA Channel 15 Y Count Register						*/
#define DMA15_Y_MODIFY			0xFFC01DDC	/* DMA Channel 15 Y Modify Register						*/
#define DMA15_CURR_DESC_PTR		0xFFC01DE0	/* DMA Channel 15 Current Descriptor Pointer Register	*/
#define DMA15_CURR_ADDR			0xFFC01DE4	/* DMA Channel 15 Current Address Register				*/
#define DMA15_IRQ_STATUS		0xFFC01DE8	/* DMA Channel 15 Interrupt/Status Register				*/
#define DMA15_PERIPHERAL_MAP	0xFFC01DEC	/* DMA Channel 15 Peripheral Map Register				*/
#define DMA15_CURR_X_COUNT		0xFFC01DF0	/* DMA Channel 15 Current X Count Register				*/
#define DMA15_CURR_Y_COUNT		0xFFC01DF8	/* DMA Channel 15 Current Y Count Register				*/

#define DMA16_NEXT_DESC_PTR		0xFFC01E00	/* DMA Channel 16 Next Descriptor Pointer Register		*/
#define DMA16_START_ADDR		0xFFC01E04	/* DMA Channel 16 Start Address Register				*/
#define DMA16_CONFIG			0xFFC01E08	/* DMA Channel 16 Configuration Register				*/
#define DMA16_X_COUNT			0xFFC01E10	/* DMA Channel 16 X Count Register						*/
#define DMA16_X_MODIFY			0xFFC01E14	/* DMA Channel 16 X Modify Register						*/
#define DMA16_Y_COUNT			0xFFC01E18	/* DMA Channel 16 Y Count Register						*/
#define DMA16_Y_MODIFY			0xFFC01E1C	/* DMA Channel 16 Y Modify Register						*/
#define DMA16_CURR_DESC_PTR		0xFFC01E20	/* DMA Channel 16 Current Descriptor Pointer Register	*/
#define DMA16_CURR_ADDR			0xFFC01E24	/* DMA Channel 16 Current Address Register				*/
#define DMA16_IRQ_STATUS		0xFFC01E28	/* DMA Channel 16 Interrupt/Status Register				*/
#define DMA16_PERIPHERAL_MAP	0xFFC01E2C	/* DMA Channel 16 Peripheral Map Register				*/
#define DMA16_CURR_X_COUNT		0xFFC01E30	/* DMA Channel 16 Current X Count Register				*/
#define DMA16_CURR_Y_COUNT		0xFFC01E38	/* DMA Channel 16 Current Y Count Register				*/

#define DMA17_NEXT_DESC_PTR		0xFFC01E40	/* DMA Channel 17 Next Descriptor Pointer Register		*/
#define DMA17_START_ADDR		0xFFC01E44	/* DMA Channel 17 Start Address Register				*/
#define DMA17_CONFIG			0xFFC01E48	/* DMA Channel 17 Configuration Register				*/
#define DMA17_X_COUNT			0xFFC01E50	/* DMA Channel 17 X Count Register						*/
#define DMA17_X_MODIFY			0xFFC01E54	/* DMA Channel 17 X Modify Register						*/
#define DMA17_Y_COUNT			0xFFC01E58	/* DMA Channel 17 Y Count Register						*/
#define DMA17_Y_MODIFY			0xFFC01E5C	/* DMA Channel 17 Y Modify Register						*/
#define DMA17_CURR_DESC_PTR		0xFFC01E60	/* DMA Channel 17 Current Descriptor Pointer Register	*/
#define DMA17_CURR_ADDR			0xFFC01E64	/* DMA Channel 17 Current Address Register				*/
#define DMA17_IRQ_STATUS		0xFFC01E68	/* DMA Channel 17 Interrupt/Status Register				*/
#define DMA17_PERIPHERAL_MAP	0xFFC01E6C	/* DMA Channel 17 Peripheral Map Register				*/
#define DMA17_CURR_X_COUNT		0xFFC01E70	/* DMA Channel 17 Current X Count Register				*/
#define DMA17_CURR_Y_COUNT		0xFFC01E78	/* DMA Channel 17 Current Y Count Register				*/

#define DMA18_NEXT_DESC_PTR		0xFFC01E80	/* DMA Channel 18 Next Descriptor Pointer Register		*/
#define DMA18_START_ADDR		0xFFC01E84	/* DMA Channel 18 Start Address Register				*/
#define DMA18_CONFIG			0xFFC01E88	/* DMA Channel 18 Configuration Register				*/
#define DMA18_X_COUNT			0xFFC01E90	/* DMA Channel 18 X Count Register						*/
#define DMA18_X_MODIFY			0xFFC01E94	/* DMA Channel 18 X Modify Register						*/
#define DMA18_Y_COUNT			0xFFC01E98	/* DMA Channel 18 Y Count Register						*/
#define DMA18_Y_MODIFY			0xFFC01E9C	/* DMA Channel 18 Y Modify Register						*/
#define DMA18_CURR_DESC_PTR		0xFFC01EA0	/* DMA Channel 18 Current Descriptor Pointer Register	*/
#define DMA18_CURR_ADDR			0xFFC01EA4	/* DMA Channel 18 Current Address Register				*/
#define DMA18_IRQ_STATUS		0xFFC01EA8	/* DMA Channel 18 Interrupt/Status Register				*/
#define DMA18_PERIPHERAL_MAP	0xFFC01EAC	/* DMA Channel 18 Peripheral Map Register				*/
#define DMA18_CURR_X_COUNT		0xFFC01EB0	/* DMA Channel 18 Current X Count Register				*/
#define DMA18_CURR_Y_COUNT		0xFFC01EB8	/* DMA Channel 18 Current Y Count Register				*/

#define DMA19_NEXT_DESC_PTR		0xFFC01EC0	/* DMA Channel 19 Next Descriptor Pointer Register		*/
#define DMA19_START_ADDR		0xFFC01EC4	/* DMA Channel 19 Start Address Register				*/
#define DMA19_CONFIG			0xFFC01EC8	/* DMA Channel 19 Configuration Register				*/
#define DMA19_X_COUNT			0xFFC01ED0	/* DMA Channel 19 X Count Register						*/
#define DMA19_X_MODIFY			0xFFC01ED4	/* DMA Channel 19 X Modify Register						*/
#define DMA19_Y_COUNT			0xFFC01ED8	/* DMA Channel 19 Y Count Register						*/
#define DMA19_Y_MODIFY			0xFFC01EDC	/* DMA Channel 19 Y Modify Register						*/
#define DMA19_CURR_DESC_PTR		0xFFC01EE0	/* DMA Channel 19 Current Descriptor Pointer Register	*/
#define DMA19_CURR_ADDR			0xFFC01EE4	/* DMA Channel 19 Current Address Register				*/
#define DMA19_IRQ_STATUS		0xFFC01EE8	/* DMA Channel 19 Interrupt/Status Register				*/
#define DMA19_PERIPHERAL_MAP	0xFFC01EEC	/* DMA Channel 19 Peripheral Map Register				*/
#define DMA19_CURR_X_COUNT		0xFFC01EF0	/* DMA Channel 19 Current X Count Register				*/
#define DMA19_CURR_Y_COUNT		0xFFC01EF8	/* DMA Channel 19 Current Y Count Register				*/

#define MDMA1_D0_NEXT_DESC_PTR	0xFFC01F00	/* MemDMA1 Stream 0 Destination Next Descriptor Pointer Register		*/
#define MDMA1_D0_START_ADDR		0xFFC01F04	/* MemDMA1 Stream 0 Destination Start Address Register				*/
#define MDMA1_D0_CONFIG			0xFFC01F08	/* MemDMA1 Stream 0 Destination Configuration Register				*/
#define MDMA1_D0_X_COUNT		0xFFC01F10	/* MemDMA1 Stream 0 Destination X Count Register						*/
#define MDMA1_D0_X_MODIFY		0xFFC01F14	/* MemDMA1 Stream 0 Destination X Modify Register					*/
#define MDMA1_D0_Y_COUNT		0xFFC01F18	/* MemDMA1 Stream 0 Destination Y Count Register						*/
#define MDMA1_D0_Y_MODIFY		0xFFC01F1C	/* MemDMA1 Stream 0 Destination Y Modify Register					*/
#define MDMA1_D0_CURR_DESC_PTR	0xFFC01F20	/* MemDMA1 Stream 0 Destination Current Descriptor Pointer Register	*/
#define MDMA1_D0_CURR_ADDR		0xFFC01F24	/* MemDMA1 Stream 0 Destination Current Address Register				*/
#define MDMA1_D0_IRQ_STATUS		0xFFC01F28	/* MemDMA1 Stream 0 Destination Interrupt/Status Register			*/
#define MDMA1_D0_PERIPHERAL_MAP	0xFFC01F2C	/* MemDMA1 Stream 0 Destination Peripheral Map Register				*/
#define MDMA1_D0_CURR_X_COUNT	0xFFC01F30	/* MemDMA1 Stream 0 Destination Current X Count Register				*/
#define MDMA1_D0_CURR_Y_COUNT	0xFFC01F38	/* MemDMA1 Stream 0 Destination Current Y Count Register				*/

#define MDMA1_S0_NEXT_DESC_PTR	0xFFC01F40	/* MemDMA1 Stream 0 Source Next Descriptor Pointer Register			*/
#define MDMA1_S0_START_ADDR		0xFFC01F44	/* MemDMA1 Stream 0 Source Start Address Register					*/
#define MDMA1_S0_CONFIG			0xFFC01F48	/* MemDMA1 Stream 0 Source Configuration Register					*/
#define MDMA1_S0_X_COUNT		0xFFC01F50	/* MemDMA1 Stream 0 Source X Count Register							*/
#define MDMA1_S0_X_MODIFY		0xFFC01F54	/* MemDMA1 Stream 0 Source X Modify Register							*/
#define MDMA1_S0_Y_COUNT		0xFFC01F58	/* MemDMA1 Stream 0 Source Y Count Register							*/
#define MDMA1_S0_Y_MODIFY		0xFFC01F5C	/* MemDMA1 Stream 0 Source Y Modify Register							*/
#define MDMA1_S0_CURR_DESC_PTR	0xFFC01F60	/* MemDMA1 Stream 0 Source Current Descriptor Pointer Register		*/
#define MDMA1_S0_CURR_ADDR		0xFFC01F64	/* MemDMA1 Stream 0 Source Current Address Register					*/
#define MDMA1_S0_IRQ_STATUS		0xFFC01F68	/* MemDMA1 Stream 0 Source Interrupt/Status Register					*/
#define MDMA1_S0_PERIPHERAL_MAP	0xFFC01F6C	/* MemDMA1 Stream 0 Source Peripheral Map Register					*/
#define MDMA1_S0_CURR_X_COUNT	0xFFC01F70	/* MemDMA1 Stream 0 Source Current X Count Register					*/
#define MDMA1_S0_CURR_Y_COUNT	0xFFC01F78	/* MemDMA1 Stream 0 Source Current Y Count Register					*/

#define MDMA1_D1_NEXT_DESC_PTR	0xFFC01F80	/* MemDMA1 Stream 1 Destination Next Descriptor Pointer Register		*/
#define MDMA1_D1_START_ADDR		0xFFC01F84	/* MemDMA1 Stream 1 Destination Start Address Register				*/
#define MDMA1_D1_CONFIG			0xFFC01F88	/* MemDMA1 Stream 1 Destination Configuration Register				*/
#define MDMA1_D1_X_COUNT		0xFFC01F90	/* MemDMA1 Stream 1 Destination X Count Register						*/
#define MDMA1_D1_X_MODIFY		0xFFC01F94	/* MemDMA1 Stream 1 Destination X Modify Register					*/
#define MDMA1_D1_Y_COUNT		0xFFC01F98	/* MemDMA1 Stream 1 Destination Y Count Register						*/
#define MDMA1_D1_Y_MODIFY		0xFFC01F9C	/* MemDMA1 Stream 1 Destination Y Modify Register					*/
#define MDMA1_D1_CURR_DESC_PTR	0xFFC01FA0	/* MemDMA1 Stream 1 Destination Current Descriptor Pointer Register	*/
#define MDMA1_D1_CURR_ADDR		0xFFC01FA4	/* MemDMA1 Stream 1 Destination Current Address Register				*/
#define MDMA1_D1_IRQ_STATUS		0xFFC01FA8	/* MemDMA1 Stream 1 Destination Interrupt/Status Register			*/
#define MDMA1_D1_PERIPHERAL_MAP	0xFFC01FAC	/* MemDMA1 Stream 1 Destination Peripheral Map Register				*/
#define MDMA1_D1_CURR_X_COUNT	0xFFC01FB0	/* MemDMA1 Stream 1 Destination Current X Count Register				*/
#define MDMA1_D1_CURR_Y_COUNT	0xFFC01FB8	/* MemDMA1 Stream 1 Destination Current Y Count Register				*/

#define MDMA1_S1_NEXT_DESC_PTR	0xFFC01FC0	/* MemDMA1 Stream 1 Source Next Descriptor Pointer Register			*/
#define MDMA1_S1_START_ADDR		0xFFC01FC4	/* MemDMA1 Stream 1 Source Start Address Register					*/
#define MDMA1_S1_CONFIG			0xFFC01FC8	/* MemDMA1 Stream 1 Source Configuration Register					*/
#define MDMA1_S1_X_COUNT		0xFFC01FD0	/* MemDMA1 Stream 1 Source X Count Register							*/
#define MDMA1_S1_X_MODIFY		0xFFC01FD4	/* MemDMA1 Stream 1 Source X Modify Register							*/
#define MDMA1_S1_Y_COUNT		0xFFC01FD8	/* MemDMA1 Stream 1 Source Y Count Register							*/
#define MDMA1_S1_Y_MODIFY		0xFFC01FDC	/* MemDMA1 Stream 1 Source Y Modify Register							*/
#define MDMA1_S1_CURR_DESC_PTR	0xFFC01FE0	/* MemDMA1 Stream 1 Source Current Descriptor Pointer Register		*/
#define MDMA1_S1_CURR_ADDR		0xFFC01FE4	/* MemDMA1 Stream 1 Source Current Address Register					*/
#define MDMA1_S1_IRQ_STATUS		0xFFC01FE8	/* MemDMA1 Stream 1 Source Interrupt/Status Register					*/
#define MDMA1_S1_PERIPHERAL_MAP	0xFFC01FEC	/* MemDMA1 Stream 1 Source Peripheral Map Register					*/
#define MDMA1_S1_CURR_X_COUNT	0xFFC01FF0	/* MemDMA1 Stream 1 Source Current X Count Register					*/
#define MDMA1_S1_CURR_Y_COUNT	0xFFC01FF8	/* MemDMA1 Stream 1 Source Current Y Count Register					*/


/* UART1 Controller		(0xFFC02000 - 0xFFC020FF)								*/
#define UART1_THR			0xFFC02000	/* Transmit Holding register			*/
#define UART1_RBR			0xFFC02000	/* Receive Buffer register				*/
#define UART1_DLL			0xFFC02000	/* Divisor Latch (Low-Byte)				*/
#define UART1_IER			0xFFC02004	/* Interrupt Enable Register			*/
#define UART1_DLH			0xFFC02004	/* Divisor Latch (High-Byte)			*/
#define UART1_IIR			0xFFC02008	/* Interrupt Identification Register	*/
#define UART1_LCR			0xFFC0200C	/* Line Control Register				*/
#define UART1_MCR			0xFFC02010	/* Modem Control Register				*/
#define UART1_LSR			0xFFC02014	/* Line Status Register					*/
#define UART1_SCR			0xFFC0201C	/* SCR Scratch Register					*/
#define UART1_GCTL			0xFFC02024	/* Global Control Register				*/


/* UART2 Controller		(0xFFC02100 - 0xFFC021FF)								*/
#define UART2_THR			0xFFC02100	/* Transmit Holding register			*/
#define UART2_RBR			0xFFC02100	/* Receive Buffer register				*/
#define UART2_DLL			0xFFC02100	/* Divisor Latch (Low-Byte)				*/
#define UART2_IER			0xFFC02104	/* Interrupt Enable Register			*/
#define UART2_DLH			0xFFC02104	/* Divisor Latch (High-Byte)			*/
#define UART2_IIR			0xFFC02108	/* Interrupt Identification Register	*/
#define UART2_LCR			0xFFC0210C	/* Line Control Register				*/
#define UART2_MCR			0xFFC02110	/* Modem Control Register				*/
#define UART2_LSR			0xFFC02114	/* Line Status Register					*/
#define UART2_SCR			0xFFC0211C	/* SCR Scratch Register					*/
#define UART2_GCTL			0xFFC02124	/* Global Control Register				*/


/* Two-Wire Interface 1	(0xFFC02200 - 0xFFC022FF)										*/
#define TWI1_CLKDIV			0xFFC02200	/* Serial Clock Divider Register				*/
#define TWI1_CONTROL		0xFFC02204	/* TWI1 Master Internal Time Reference Register	*/
#define TWI1_SLAVE_CTRL		0xFFC02208	/* Slave Mode Control Register					*/
#define TWI1_SLAVE_STAT		0xFFC0220C	/* Slave Mode Status Register					*/
#define TWI1_SLAVE_ADDR		0xFFC02210	/* Slave Mode Address Register					*/
#define TWI1_MASTER_CTRL	0xFFC02214	/* Master Mode Control Register					*/
#define TWI1_MASTER_STAT	0xFFC02218	/* Master Mode Status Register					*/
#define TWI1_MASTER_ADDR	0xFFC0221C	/* Master Mode Address Register					*/
#define TWI1_INT_STAT		0xFFC02220	/* TWI1 Master Interrupt Register				*/
#define TWI1_INT_MASK		0xFFC02224	/* TWI1 Master Interrupt Mask Register			*/
#define TWI1_FIFO_CTRL		0xFFC02228	/* FIFO Control Register						*/
#define TWI1_FIFO_STAT		0xFFC0222C	/* FIFO Status Register							*/
#define TWI1_XMT_DATA8		0xFFC02280	/* FIFO Transmit Data Single Byte Register		*/
#define TWI1_XMT_DATA16		0xFFC02284	/* FIFO Transmit Data Double Byte Register		*/
#define TWI1_RCV_DATA8		0xFFC02288	/* FIFO Receive Data Single Byte Register		*/
#define TWI1_RCV_DATA16		0xFFC0228C	/* FIFO Receive Data Double Byte Register		*/


/* SPI1 Controller		(0xFFC02300 - 0xFFC023FF)								*/
#define SPI1_CTL			0xFFC02300  /* SPI1 Control Register				*/
#define SPI1_FLG			0xFFC02304  /* SPI1 Flag register					*/
#define SPI1_STAT			0xFFC02308  /* SPI1 Status register					*/
#define SPI1_TDBR			0xFFC0230C  /* SPI1 Transmit Data Buffer Register	*/
#define SPI1_RDBR			0xFFC02310  /* SPI1 Receive Data Buffer Register	*/
#define SPI1_BAUD			0xFFC02314  /* SPI1 Baud rate Register				*/
#define SPI1_SHADOW			0xFFC02318  /* SPI1_RDBR Shadow Register			*/


/* SPI2 Controller		(0xFFC02400 - 0xFFC024FF)								*/
#define SPI2_CTL			0xFFC02400  /* SPI2 Control Register				*/
#define SPI2_FLG			0xFFC02404  /* SPI2 Flag register					*/
#define SPI2_STAT			0xFFC02408  /* SPI2 Status register					*/
#define SPI2_TDBR			0xFFC0240C  /* SPI2 Transmit Data Buffer Register	*/
#define SPI2_RDBR			0xFFC02410  /* SPI2 Receive Data Buffer Register	*/
#define SPI2_BAUD			0xFFC02414  /* SPI2 Baud rate Register				*/
#define SPI2_SHADOW			0xFFC02418  /* SPI2_RDBR Shadow Register			*/


/* SPORT2 Controller		(0xFFC02500 - 0xFFC025FF)										*/
#define SPORT2_TCR1			0xFFC02500	/* SPORT2 Transmit Configuration 1 Register			*/
#define SPORT2_TCR2			0xFFC02504	/* SPORT2 Transmit Configuration 2 Register			*/
#define SPORT2_TCLKDIV		0xFFC02508	/* SPORT2 Transmit Clock Divider					*/
#define SPORT2_TFSDIV		0xFFC0250C	/* SPORT2 Transmit Frame Sync Divider				*/
#define SPORT2_TX			0xFFC02510	/* SPORT2 TX Data Register							*/
#define SPORT2_RX			0xFFC02518	/* SPORT2 RX Data Register							*/
#define SPORT2_RCR1			0xFFC02520	/* SPORT2 Transmit Configuration 1 Register			*/
#define SPORT2_RCR2			0xFFC02524	/* SPORT2 Transmit Configuration 2 Register			*/
#define SPORT2_RCLKDIV		0xFFC02528	/* SPORT2 Receive Clock Divider						*/
#define SPORT2_RFSDIV		0xFFC0252C	/* SPORT2 Receive Frame Sync Divider				*/
#define SPORT2_STAT			0xFFC02530	/* SPORT2 Status Register							*/
#define SPORT2_CHNL			0xFFC02534	/* SPORT2 Current Channel Register					*/
#define SPORT2_MCMC1		0xFFC02538	/* SPORT2 Multi-Channel Configuration Register 1	*/
#define SPORT2_MCMC2		0xFFC0253C	/* SPORT2 Multi-Channel Configuration Register 2	*/
#define SPORT2_MTCS0		0xFFC02540	/* SPORT2 Multi-Channel Transmit Select Register 0	*/
#define SPORT2_MTCS1		0xFFC02544	/* SPORT2 Multi-Channel Transmit Select Register 1	*/
#define SPORT2_MTCS2		0xFFC02548	/* SPORT2 Multi-Channel Transmit Select Register 2	*/
#define SPORT2_MTCS3		0xFFC0254C	/* SPORT2 Multi-Channel Transmit Select Register 3	*/
#define SPORT2_MRCS0		0xFFC02550	/* SPORT2 Multi-Channel Receive Select Register 0	*/
#define SPORT2_MRCS1		0xFFC02554	/* SPORT2 Multi-Channel Receive Select Register 1	*/
#define SPORT2_MRCS2		0xFFC02558	/* SPORT2 Multi-Channel Receive Select Register 2	*/
#define SPORT2_MRCS3		0xFFC0255C	/* SPORT2 Multi-Channel Receive Select Register 3	*/


/* SPORT3 Controller		(0xFFC02600 - 0xFFC026FF)										*/
#define SPORT3_TCR1			0xFFC02600	/* SPORT3 Transmit Configuration 1 Register			*/
#define SPORT3_TCR2			0xFFC02604	/* SPORT3 Transmit Configuration 2 Register			*/
#define SPORT3_TCLKDIV		0xFFC02608	/* SPORT3 Transmit Clock Divider					*/
#define SPORT3_TFSDIV		0xFFC0260C	/* SPORT3 Transmit Frame Sync Divider				*/
#define SPORT3_TX			0xFFC02610	/* SPORT3 TX Data Register							*/
#define SPORT3_RX			0xFFC02618	/* SPORT3 RX Data Register							*/
#define SPORT3_RCR1			0xFFC02620	/* SPORT3 Transmit Configuration 1 Register			*/
#define SPORT3_RCR2			0xFFC02624	/* SPORT3 Transmit Configuration 2 Register			*/
#define SPORT3_RCLKDIV		0xFFC02628	/* SPORT3 Receive Clock Divider						*/
#define SPORT3_RFSDIV		0xFFC0262C	/* SPORT3 Receive Frame Sync Divider				*/
#define SPORT3_STAT			0xFFC02630	/* SPORT3 Status Register							*/
#define SPORT3_CHNL			0xFFC02634	/* SPORT3 Current Channel Register					*/
#define SPORT3_MCMC1		0xFFC02638	/* SPORT3 Multi-Channel Configuration Register 1	*/
#define SPORT3_MCMC2		0xFFC0263C	/* SPORT3 Multi-Channel Configuration Register 2	*/
#define SPORT3_MTCS0		0xFFC02640	/* SPORT3 Multi-Channel Transmit Select Register 0	*/
#define SPORT3_MTCS1		0xFFC02644	/* SPORT3 Multi-Channel Transmit Select Register 1	*/
#define SPORT3_MTCS2		0xFFC02648	/* SPORT3 Multi-Channel Transmit Select Register 2	*/
#define SPORT3_MTCS3		0xFFC0264C	/* SPORT3 Multi-Channel Transmit Select Register 3	*/
#define SPORT3_MRCS0		0xFFC02650	/* SPORT3 Multi-Channel Receive Select Register 0	*/
#define SPORT3_MRCS1		0xFFC02654	/* SPORT3 Multi-Channel Receive Select Register 1	*/
#define SPORT3_MRCS2		0xFFC02658	/* SPORT3 Multi-Channel Receive Select Register 2	*/
#define SPORT3_MRCS3		0xFFC0265C	/* SPORT3 Multi-Channel Receive Select Register 3	*/


/* CAN Controller		(0xFFC02A00 - 0xFFC02FFF)											*/
/* For Mailboxes 0-15																		*/
#define CAN_MC1				0xFFC02A00	/* Mailbox config reg 1								*/
#define CAN_MD1				0xFFC02A04	/* Mailbox direction reg 1							*/
#define CAN_TRS1			0xFFC02A08	/* Transmit Request Set reg 1						*/
#define CAN_TRR1			0xFFC02A0C	/* Transmit Request Reset reg 1						*/
#define CAN_TA1				0xFFC02A10	/* Transmit Acknowledge reg 1						*/
#define CAN_AA1				0xFFC02A14	/* Transmit Abort Acknowledge reg 1					*/
#define CAN_RMP1			0xFFC02A18	/* Receive Message Pending reg 1					*/
#define CAN_RML1			0xFFC02A1C	/* Receive Message Lost reg 1						*/
#define CAN_MBTIF1			0xFFC02A20	/* Mailbox Transmit Interrupt Flag reg 1			*/
#define CAN_MBRIF1			0xFFC02A24	/* Mailbox Receive  Interrupt Flag reg 1			*/
#define CAN_MBIM1			0xFFC02A28	/* Mailbox Interrupt Mask reg 1						*/
#define CAN_RFH1			0xFFC02A2C	/* Remote Frame Handling reg 1						*/
#define CAN_OPSS1			0xFFC02A30	/* Overwrite Protection Single Shot Xmission reg 1	*/

/* For Mailboxes 16-31   																	*/
#define CAN_MC2				0xFFC02A40	/* Mailbox config reg 2								*/
#define CAN_MD2				0xFFC02A44	/* Mailbox direction reg 2							*/
#define CAN_TRS2			0xFFC02A48	/* Transmit Request Set reg 2						*/
#define CAN_TRR2			0xFFC02A4C	/* Transmit Request Reset reg 2						*/
#define CAN_TA2				0xFFC02A50	/* Transmit Acknowledge reg 2						*/
#define CAN_AA2				0xFFC02A54	/* Transmit Abort Acknowledge reg 2					*/
#define CAN_RMP2			0xFFC02A58	/* Receive Message Pending reg 2					*/
#define CAN_RML2			0xFFC02A5C	/* Receive Message Lost reg 2						*/
#define CAN_MBTIF2			0xFFC02A60	/* Mailbox Transmit Interrupt Flag reg 2			*/
#define CAN_MBRIF2			0xFFC02A64	/* Mailbox Receive  Interrupt Flag reg 2			*/
#define CAN_MBIM2			0xFFC02A68	/* Mailbox Interrupt Mask reg 2						*/
#define CAN_RFH2			0xFFC02A6C	/* Remote Frame Handling reg 2						*/
#define CAN_OPSS2			0xFFC02A70	/* Overwrite Protection Single Shot Xmission reg 2	*/

#define CAN_CLOCK			0xFFC02A80	/* Bit Timing Configuration register 0				*/
#define CAN_TIMING			0xFFC02A84	/* Bit Timing Configuration register 1				*/
#define CAN_DEBUG			0xFFC02A88	/* Debug Register									*/
#define CAN_STATUS			0xFFC02A8C	/* Global Status Register							*/
#define CAN_CEC				0xFFC02A90	/* Error Counter Register							*/
#define CAN_GIS				0xFFC02A94	/* Global Interrupt Status Register					*/
#define CAN_GIM				0xFFC02A98	/* Global Interrupt Mask Register					*/
#define CAN_GIF				0xFFC02A9C	/* Global Interrupt Flag Register					*/
#define CAN_CONTROL			0xFFC02AA0	/* Master Control Register							*/
#define CAN_INTR			0xFFC02AA4	/* Interrupt Pending Register						*/
#define CAN_MBTD			0xFFC02AAC	/* Mailbox Temporary Disable Feature				*/
#define CAN_EWR				0xFFC02AB0	/* Programmable Warning Level						*/
#define CAN_ESR				0xFFC02AB4	/* Error Status Register							*/
#define CAN_UCREG			0xFFC02AC0	/* Universal Counter Register/Capture Register		*/
#define CAN_UCCNT			0xFFC02AC4	/* Universal Counter								*/
#define CAN_UCRC			0xFFC02AC8	/* Universal Counter Force Reload Register			*/
#define CAN_UCCNF			0xFFC02ACC	/* Universal Counter Configuration Register			*/

/* Mailbox Acceptance Masks 												*/
#define CAN_AM00L			0xFFC02B00	/* Mailbox 0 Low Acceptance Mask	*/
#define CAN_AM00H			0xFFC02B04	/* Mailbox 0 High Acceptance Mask	*/
#define CAN_AM01L			0xFFC02B08	/* Mailbox 1 Low Acceptance Mask	*/
#define CAN_AM01H			0xFFC02B0C	/* Mailbox 1 High Acceptance Mask	*/
#define CAN_AM02L			0xFFC02B10	/* Mailbox 2 Low Acceptance Mask 	*/
#define CAN_AM02H			0xFFC02B14	/* Mailbox 2 High Acceptance Mask	*/
#define CAN_AM03L			0xFFC02B18	/* Mailbox 3 Low Acceptance Mask 	*/
#define CAN_AM03H			0xFFC02B1C	/* Mailbox 3 High Acceptance Mask	*/
#define CAN_AM04L			0xFFC02B20	/* Mailbox 4 Low Acceptance Mask 	*/
#define CAN_AM04H			0xFFC02B24	/* Mailbox 4 High Acceptance Mask	*/
#define CAN_AM05L			0xFFC02B28	/* Mailbox 5 Low Acceptance Mask 	*/
#define CAN_AM05H			0xFFC02B2C	/* Mailbox 5 High Acceptance Mask	*/
#define CAN_AM06L			0xFFC02B30	/* Mailbox 6 Low Acceptance Mask 	*/
#define CAN_AM06H			0xFFC02B34	/* Mailbox 6 High Acceptance Mask	*/
#define CAN_AM07L			0xFFC02B38	/* Mailbox 7 Low Acceptance Mask 	*/
#define CAN_AM07H			0xFFC02B3C	/* Mailbox 7 High Acceptance Mask	*/
#define CAN_AM08L			0xFFC02B40	/* Mailbox 8 Low Acceptance Mask 	*/
#define CAN_AM08H			0xFFC02B44	/* Mailbox 8 High Acceptance Mask	*/
#define CAN_AM09L			0xFFC02B48	/* Mailbox 9 Low Acceptance Mask 	*/
#define CAN_AM09H			0xFFC02B4C	/* Mailbox 9 High Acceptance Mask	*/
#define CAN_AM10L			0xFFC02B50	/* Mailbox 10 Low Acceptance Mask 	*/
#define CAN_AM10H			0xFFC02B54	/* Mailbox 10 High Acceptance Mask	*/
#define CAN_AM11L			0xFFC02B58	/* Mailbox 11 Low Acceptance Mask 	*/
#define CAN_AM11H			0xFFC02B5C	/* Mailbox 11 High Acceptance Mask	*/
#define CAN_AM12L			0xFFC02B60	/* Mailbox 12 Low Acceptance Mask 	*/
#define CAN_AM12H			0xFFC02B64	/* Mailbox 12 High Acceptance Mask	*/
#define CAN_AM13L			0xFFC02B68	/* Mailbox 13 Low Acceptance Mask 	*/
#define CAN_AM13H			0xFFC02B6C	/* Mailbox 13 High Acceptance Mask	*/
#define CAN_AM14L			0xFFC02B70	/* Mailbox 14 Low Acceptance Mask 	*/
#define CAN_AM14H			0xFFC02B74	/* Mailbox 14 High Acceptance Mask	*/
#define CAN_AM15L			0xFFC02B78	/* Mailbox 15 Low Acceptance Mask 	*/
#define CAN_AM15H			0xFFC02B7C	/* Mailbox 15 High Acceptance Mask	*/

#define CAN_AM16L			0xFFC02B80	/* Mailbox 16 Low Acceptance Mask 	*/
#define CAN_AM16H			0xFFC02B84	/* Mailbox 16 High Acceptance Mask	*/
#define CAN_AM17L			0xFFC02B88	/* Mailbox 17 Low Acceptance Mask 	*/
#define CAN_AM17H			0xFFC02B8C	/* Mailbox 17 High Acceptance Mask	*/
#define CAN_AM18L			0xFFC02B90	/* Mailbox 18 Low Acceptance Mask 	*/
#define CAN_AM18H			0xFFC02B94	/* Mailbox 18 High Acceptance Mask	*/
#define CAN_AM19L			0xFFC02B98	/* Mailbox 19 Low Acceptance Mask 	*/
#define CAN_AM19H			0xFFC02B9C	/* Mailbox 19 High Acceptance Mask	*/
#define CAN_AM20L			0xFFC02BA0	/* Mailbox 20 Low Acceptance Mask 	*/
#define CAN_AM20H			0xFFC02BA4	/* Mailbox 20 High Acceptance Mask	*/
#define CAN_AM21L			0xFFC02BA8	/* Mailbox 21 Low Acceptance Mask 	*/
#define CAN_AM21H			0xFFC02BAC	/* Mailbox 21 High Acceptance Mask	*/
#define CAN_AM22L			0xFFC02BB0	/* Mailbox 22 Low Acceptance Mask 	*/
#define CAN_AM22H			0xFFC02BB4	/* Mailbox 22 High Acceptance Mask	*/
#define CAN_AM23L			0xFFC02BB8	/* Mailbox 23 Low Acceptance Mask 	*/
#define CAN_AM23H			0xFFC02BBC	/* Mailbox 23 High Acceptance Mask	*/
#define CAN_AM24L			0xFFC02BC0	/* Mailbox 24 Low Acceptance Mask 	*/
#define CAN_AM24H			0xFFC02BC4	/* Mailbox 24 High Acceptance Mask	*/
#define CAN_AM25L			0xFFC02BC8	/* Mailbox 25 Low Acceptance Mask 	*/
#define CAN_AM25H			0xFFC02BCC	/* Mailbox 25 High Acceptance Mask	*/
#define CAN_AM26L			0xFFC02BD0	/* Mailbox 26 Low Acceptance Mask 	*/
#define CAN_AM26H			0xFFC02BD4	/* Mailbox 26 High Acceptance Mask	*/
#define CAN_AM27L			0xFFC02BD8	/* Mailbox 27 Low Acceptance Mask 	*/
#define CAN_AM27H			0xFFC02BDC	/* Mailbox 27 High Acceptance Mask	*/
#define CAN_AM28L			0xFFC02BE0	/* Mailbox 28 Low Acceptance Mask 	*/
#define CAN_AM28H			0xFFC02BE4	/* Mailbox 28 High Acceptance Mask	*/
#define CAN_AM29L			0xFFC02BE8	/* Mailbox 29 Low Acceptance Mask 	*/
#define CAN_AM29H			0xFFC02BEC	/* Mailbox 29 High Acceptance Mask	*/
#define CAN_AM30L			0xFFC02BF0	/* Mailbox 30 Low Acceptance Mask 	*/
#define CAN_AM30H			0xFFC02BF4	/* Mailbox 30 High Acceptance Mask	*/
#define CAN_AM31L			0xFFC02BF8	/* Mailbox 31 Low Acceptance Mask 	*/
#define CAN_AM31H			0xFFC02BFC	/* Mailbox 31 High Acceptance Mask	*/

/* CAN Acceptance Mask Macros				*/
#define CAN_AM_L(x)			(CAN_AM00L+((x)*0x8))
#define CAN_AM_H(x)			(CAN_AM00H+((x)*0x8))

/* Mailbox Registers																*/
#define CAN_MB00_DATA0		0xFFC02C00	/* Mailbox 0 Data Word 0 [15:0] Register	*/
#define CAN_MB00_DATA1		0xFFC02C04	/* Mailbox 0 Data Word 1 [31:16] Register	*/
#define CAN_MB00_DATA2		0xFFC02C08	/* Mailbox 0 Data Word 2 [47:32] Register	*/
#define CAN_MB00_DATA3		0xFFC02C0C	/* Mailbox 0 Data Word 3 [63:48] Register	*/
#define CAN_MB00_LENGTH		0xFFC02C10	/* Mailbox 0 Data Length Code Register		*/
#define CAN_MB00_TIMESTAMP	0xFFC02C14	/* Mailbox 0 Time Stamp Value Register		*/
#define CAN_MB00_ID0		0xFFC02C18	/* Mailbox 0 Identifier Low Register		*/
#define CAN_MB00_ID1		0xFFC02C1C	/* Mailbox 0 Identifier High Register		*/

#define CAN_MB01_DATA0		0xFFC02C20	/* Mailbox 1 Data Word 0 [15:0] Register	*/
#define CAN_MB01_DATA1		0xFFC02C24	/* Mailbox 1 Data Word 1 [31:16] Register	*/
#define CAN_MB01_DATA2		0xFFC02C28	/* Mailbox 1 Data Word 2 [47:32] Register	*/
#define CAN_MB01_DATA3		0xFFC02C2C	/* Mailbox 1 Data Word 3 [63:48] Register	*/
#define CAN_MB01_LENGTH		0xFFC02C30	/* Mailbox 1 Data Length Code Register		*/
#define CAN_MB01_TIMESTAMP	0xFFC02C34	/* Mailbox 1 Time Stamp Value Register		*/
#define CAN_MB01_ID0		0xFFC02C38	/* Mailbox 1 Identifier Low Register		*/
#define CAN_MB01_ID1		0xFFC02C3C	/* Mailbox 1 Identifier High Register		*/

#define CAN_MB02_DATA0		0xFFC02C40	/* Mailbox 2 Data Word 0 [15:0] Register	*/
#define CAN_MB02_DATA1		0xFFC02C44	/* Mailbox 2 Data Word 1 [31:16] Register	*/
#define CAN_MB02_DATA2		0xFFC02C48	/* Mailbox 2 Data Word 2 [47:32] Register	*/
#define CAN_MB02_DATA3		0xFFC02C4C	/* Mailbox 2 Data Word 3 [63:48] Register	*/
#define CAN_MB02_LENGTH		0xFFC02C50	/* Mailbox 2 Data Length Code Register		*/
#define CAN_MB02_TIMESTAMP	0xFFC02C54	/* Mailbox 2 Time Stamp Value Register		*/
#define CAN_MB02_ID0		0xFFC02C58	/* Mailbox 2 Identifier Low Register		*/
#define CAN_MB02_ID1		0xFFC02C5C	/* Mailbox 2 Identifier High Register		*/

#define CAN_MB03_DATA0		0xFFC02C60	/* Mailbox 3 Data Word 0 [15:0] Register	*/
#define CAN_MB03_DATA1		0xFFC02C64	/* Mailbox 3 Data Word 1 [31:16] Register	*/
#define CAN_MB03_DATA2		0xFFC02C68	/* Mailbox 3 Data Word 2 [47:32] Register	*/
#define CAN_MB03_DATA3		0xFFC02C6C	/* Mailbox 3 Data Word 3 [63:48] Register	*/
#define CAN_MB03_LENGTH		0xFFC02C70	/* Mailbox 3 Data Length Code Register		*/
#define CAN_MB03_TIMESTAMP	0xFFC02C74	/* Mailbox 3 Time Stamp Value Register		*/
#define CAN_MB03_ID0		0xFFC02C78	/* Mailbox 3 Identifier Low Register		*/
#define CAN_MB03_ID1		0xFFC02C7C	/* Mailbox 3 Identifier High Register		*/

#define CAN_MB04_DATA0		0xFFC02C80	/* Mailbox 4 Data Word 0 [15:0] Register	*/
#define CAN_MB04_DATA1		0xFFC02C84	/* Mailbox 4 Data Word 1 [31:16] Register	*/
#define CAN_MB04_DATA2		0xFFC02C88	/* Mailbox 4 Data Word 2 [47:32] Register	*/
#define CAN_MB04_DATA3		0xFFC02C8C	/* Mailbox 4 Data Word 3 [63:48] Register	*/
#define CAN_MB04_LENGTH		0xFFC02C90	/* Mailbox 4 Data Length Code Register		*/
#define CAN_MB04_TIMESTAMP	0xFFC02C94	/* Mailbox 4 Time Stamp Value Register		*/
#define CAN_MB04_ID0		0xFFC02C98	/* Mailbox 4 Identifier Low Register		*/
#define CAN_MB04_ID1		0xFFC02C9C	/* Mailbox 4 Identifier High Register		*/

#define CAN_MB05_DATA0		0xFFC02CA0	/* Mailbox 5 Data Word 0 [15:0] Register	*/
#define CAN_MB05_DATA1		0xFFC02CA4	/* Mailbox 5 Data Word 1 [31:16] Register	*/
#define CAN_MB05_DATA2		0xFFC02CA8	/* Mailbox 5 Data Word 2 [47:32] Register	*/
#define CAN_MB05_DATA3		0xFFC02CAC	/* Mailbox 5 Data Word 3 [63:48] Register	*/
#define CAN_MB05_LENGTH		0xFFC02CB0	/* Mailbox 5 Data Length Code Register		*/
#define CAN_MB05_TIMESTAMP	0xFFC02CB4	/* Mailbox 5 Time Stamp Value Register		*/
#define CAN_MB05_ID0		0xFFC02CB8	/* Mailbox 5 Identifier Low Register		*/
#define CAN_MB05_ID1		0xFFC02CBC	/* Mailbox 5 Identifier High Register		*/

#define CAN_MB06_DATA0		0xFFC02CC0	/* Mailbox 6 Data Word 0 [15:0] Register	*/
#define CAN_MB06_DATA1		0xFFC02CC4	/* Mailbox 6 Data Word 1 [31:16] Register	*/
#define CAN_MB06_DATA2		0xFFC02CC8	/* Mailbox 6 Data Word 2 [47:32] Register	*/
#define CAN_MB06_DATA3		0xFFC02CCC	/* Mailbox 6 Data Word 3 [63:48] Register	*/
#define CAN_MB06_LENGTH		0xFFC02CD0	/* Mailbox 6 Data Length Code Register		*/
#define CAN_MB06_TIMESTAMP	0xFFC02CD4	/* Mailbox 6 Time Stamp Value Register		*/
#define CAN_MB06_ID0		0xFFC02CD8	/* Mailbox 6 Identifier Low Register		*/
#define CAN_MB06_ID1		0xFFC02CDC	/* Mailbox 6 Identifier High Register		*/

#define CAN_MB07_DATA0		0xFFC02CE0	/* Mailbox 7 Data Word 0 [15:0] Register	*/
#define CAN_MB07_DATA1		0xFFC02CE4	/* Mailbox 7 Data Word 1 [31:16] Register	*/
#define CAN_MB07_DATA2		0xFFC02CE8	/* Mailbox 7 Data Word 2 [47:32] Register	*/
#define CAN_MB07_DATA3		0xFFC02CEC	/* Mailbox 7 Data Word 3 [63:48] Register	*/
#define CAN_MB07_LENGTH		0xFFC02CF0	/* Mailbox 7 Data Length Code Register		*/
#define CAN_MB07_TIMESTAMP	0xFFC02CF4	/* Mailbox 7 Time Stamp Value Register		*/
#define CAN_MB07_ID0		0xFFC02CF8	/* Mailbox 7 Identifier Low Register		*/
#define CAN_MB07_ID1		0xFFC02CFC	/* Mailbox 7 Identifier High Register		*/

#define CAN_MB08_DATA0		0xFFC02D00	/* Mailbox 8 Data Word 0 [15:0] Register	*/
#define CAN_MB08_DATA1		0xFFC02D04	/* Mailbox 8 Data Word 1 [31:16] Register	*/
#define CAN_MB08_DATA2		0xFFC02D08	/* Mailbox 8 Data Word 2 [47:32] Register	*/
#define CAN_MB08_DATA3		0xFFC02D0C	/* Mailbox 8 Data Word 3 [63:48] Register	*/
#define CAN_MB08_LENGTH		0xFFC02D10	/* Mailbox 8 Data Length Code Register		*/
#define CAN_MB08_TIMESTAMP	0xFFC02D14	/* Mailbox 8 Time Stamp Value Register		*/
#define CAN_MB08_ID0		0xFFC02D18	/* Mailbox 8 Identifier Low Register		*/
#define CAN_MB08_ID1		0xFFC02D1C	/* Mailbox 8 Identifier High Register		*/

#define CAN_MB09_DATA0		0xFFC02D20	/* Mailbox 9 Data Word 0 [15:0] Register	*/
#define CAN_MB09_DATA1		0xFFC02D24	/* Mailbox 9 Data Word 1 [31:16] Register	*/
#define CAN_MB09_DATA2		0xFFC02D28	/* Mailbox 9 Data Word 2 [47:32] Register	*/
#define CAN_MB09_DATA3		0xFFC02D2C	/* Mailbox 9 Data Word 3 [63:48] Register	*/
#define CAN_MB09_LENGTH		0xFFC02D30	/* Mailbox 9 Data Length Code Register		*/
#define CAN_MB09_TIMESTAMP	0xFFC02D34	/* Mailbox 9 Time Stamp Value Register		*/
#define CAN_MB09_ID0		0xFFC02D38	/* Mailbox 9 Identifier Low Register		*/
#define CAN_MB09_ID1		0xFFC02D3C	/* Mailbox 9 Identifier High Register		*/

#define CAN_MB10_DATA0		0xFFC02D40	/* Mailbox 10 Data Word 0 [15:0] Register	*/
#define CAN_MB10_DATA1		0xFFC02D44	/* Mailbox 10 Data Word 1 [31:16] Register	*/
#define CAN_MB10_DATA2		0xFFC02D48	/* Mailbox 10 Data Word 2 [47:32] Register	*/
#define CAN_MB10_DATA3		0xFFC02D4C	/* Mailbox 10 Data Word 3 [63:48] Register	*/
#define CAN_MB10_LENGTH		0xFFC02D50	/* Mailbox 10 Data Length Code Register		*/
#define CAN_MB10_TIMESTAMP	0xFFC02D54	/* Mailbox 10 Time Stamp Value Register		*/
#define CAN_MB10_ID0		0xFFC02D58	/* Mailbox 10 Identifier Low Register		*/
#define CAN_MB10_ID1		0xFFC02D5C	/* Mailbox 10 Identifier High Register		*/

#define CAN_MB11_DATA0		0xFFC02D60	/* Mailbox 11 Data Word 0 [15:0] Register	*/
#define CAN_MB11_DATA1		0xFFC02D64	/* Mailbox 11 Data Word 1 [31:16] Register	*/
#define CAN_MB11_DATA2		0xFFC02D68	/* Mailbox 11 Data Word 2 [47:32] Register	*/
#define CAN_MB11_DATA3		0xFFC02D6C	/* Mailbox 11 Data Word 3 [63:48] Register	*/
#define CAN_MB11_LENGTH		0xFFC02D70	/* Mailbox 11 Data Length Code Register		*/
#define CAN_MB11_TIMESTAMP	0xFFC02D74	/* Mailbox 11 Time Stamp Value Register		*/
#define CAN_MB11_ID0		0xFFC02D78	/* Mailbox 11 Identifier Low Register		*/
#define CAN_MB11_ID1		0xFFC02D7C	/* Mailbox 11 Identifier High Register		*/

#define CAN_MB12_DATA0		0xFFC02D80	/* Mailbox 12 Data Word 0 [15:0] Register	*/
#define CAN_MB12_DATA1		0xFFC02D84	/* Mailbox 12 Data Word 1 [31:16] Register	*/
#define CAN_MB12_DATA2		0xFFC02D88	/* Mailbox 12 Data Word 2 [47:32] Register	*/
#define CAN_MB12_DATA3		0xFFC02D8C	/* Mailbox 12 Data Word 3 [63:48] Register	*/
#define CAN_MB12_LENGTH		0xFFC02D90	/* Mailbox 12 Data Length Code Register		*/
#define CAN_MB12_TIMESTAMP	0xFFC02D94	/* Mailbox 12 Time Stamp Value Register		*/
#define CAN_MB12_ID0		0xFFC02D98	/* Mailbox 12 Identifier Low Register		*/
#define CAN_MB12_ID1		0xFFC02D9C	/* Mailbox 12 Identifier High Register		*/

#define CAN_MB13_DATA0		0xFFC02DA0	/* Mailbox 13 Data Word 0 [15:0] Register	*/
#define CAN_MB13_DATA1		0xFFC02DA4	/* Mailbox 13 Data Word 1 [31:16] Register	*/
#define CAN_MB13_DATA2		0xFFC02DA8	/* Mailbox 13 Data Word 2 [47:32] Register	*/
#define CAN_MB13_DATA3		0xFFC02DAC	/* Mailbox 13 Data Word 3 [63:48] Register	*/
#define CAN_MB13_LENGTH		0xFFC02DB0	/* Mailbox 13 Data Length Code Register		*/
#define CAN_MB13_TIMESTAMP	0xFFC02DB4	/* Mailbox 13 Time Stamp Value Register		*/
#define CAN_MB13_ID0		0xFFC02DB8	/* Mailbox 13 Identifier Low Register		*/
#define CAN_MB13_ID1		0xFFC02DBC	/* Mailbox 13 Identifier High Register		*/

#define CAN_MB14_DATA0		0xFFC02DC0	/* Mailbox 14 Data Word 0 [15:0] Register	*/
#define CAN_MB14_DATA1		0xFFC02DC4	/* Mailbox 14 Data Word 1 [31:16] Register	*/
#define CAN_MB14_DATA2		0xFFC02DC8	/* Mailbox 14 Data Word 2 [47:32] Register	*/
#define CAN_MB14_DATA3		0xFFC02DCC	/* Mailbox 14 Data Word 3 [63:48] Register	*/
#define CAN_MB14_LENGTH		0xFFC02DD0	/* Mailbox 14 Data Length Code Register		*/
#define CAN_MB14_TIMESTAMP	0xFFC02DD4	/* Mailbox 14 Time Stamp Value Register		*/
#define CAN_MB14_ID0		0xFFC02DD8	/* Mailbox 14 Identifier Low Register		*/
#define CAN_MB14_ID1		0xFFC02DDC	/* Mailbox 14 Identifier High Register		*/

#define CAN_MB15_DATA0		0xFFC02DE0	/* Mailbox 15 Data Word 0 [15:0] Register	*/
#define CAN_MB15_DATA1		0xFFC02DE4	/* Mailbox 15 Data Word 1 [31:16] Register	*/
#define CAN_MB15_DATA2		0xFFC02DE8	/* Mailbox 15 Data Word 2 [47:32] Register	*/
#define CAN_MB15_DATA3		0xFFC02DEC	/* Mailbox 15 Data Word 3 [63:48] Register	*/
#define CAN_MB15_LENGTH		0xFFC02DF0	/* Mailbox 15 Data Length Code Register		*/
#define CAN_MB15_TIMESTAMP	0xFFC02DF4	/* Mailbox 15 Time Stamp Value Register		*/
#define CAN_MB15_ID0		0xFFC02DF8	/* Mailbox 15 Identifier Low Register		*/
#define CAN_MB15_ID1		0xFFC02DFC	/* Mailbox 15 Identifier High Register		*/

#define CAN_MB16_DATA0		0xFFC02E00	/* Mailbox 16 Data Word 0 [15:0] Register	*/
#define CAN_MB16_DATA1		0xFFC02E04	/* Mailbox 16 Data Word 1 [31:16] Register	*/
#define CAN_MB16_DATA2		0xFFC02E08	/* Mailbox 16 Data Word 2 [47:32] Register	*/
#define CAN_MB16_DATA3		0xFFC02E0C	/* Mailbox 16 Data Word 3 [63:48] Register	*/
#define CAN_MB16_LENGTH		0xFFC02E10	/* Mailbox 16 Data Length Code Register		*/
#define CAN_MB16_TIMESTAMP	0xFFC02E14	/* Mailbox 16 Time Stamp Value Register		*/
#define CAN_MB16_ID0		0xFFC02E18	/* Mailbox 16 Identifier Low Register		*/
#define CAN_MB16_ID1		0xFFC02E1C	/* Mailbox 16 Identifier High Register		*/

#define CAN_MB17_DATA0		0xFFC02E20	/* Mailbox 17 Data Word 0 [15:0] Register	*/
#define CAN_MB17_DATA1		0xFFC02E24	/* Mailbox 17 Data Word 1 [31:16] Register	*/
#define CAN_MB17_DATA2		0xFFC02E28	/* Mailbox 17 Data Word 2 [47:32] Register	*/
#define CAN_MB17_DATA3		0xFFC02E2C	/* Mailbox 17 Data Word 3 [63:48] Register	*/
#define CAN_MB17_LENGTH		0xFFC02E30	/* Mailbox 17 Data Length Code Register		*/
#define CAN_MB17_TIMESTAMP	0xFFC02E34	/* Mailbox 17 Time Stamp Value Register		*/
#define CAN_MB17_ID0		0xFFC02E38	/* Mailbox 17 Identifier Low Register		*/
#define CAN_MB17_ID1		0xFFC02E3C	/* Mailbox 17 Identifier High Register		*/

#define CAN_MB18_DATA0		0xFFC02E40	/* Mailbox 18 Data Word 0 [15:0] Register	*/
#define CAN_MB18_DATA1		0xFFC02E44	/* Mailbox 18 Data Word 1 [31:16] Register	*/
#define CAN_MB18_DATA2		0xFFC02E48	/* Mailbox 18 Data Word 2 [47:32] Register	*/
#define CAN_MB18_DATA3		0xFFC02E4C	/* Mailbox 18 Data Word 3 [63:48] Register	*/
#define CAN_MB18_LENGTH		0xFFC02E50	/* Mailbox 18 Data Length Code Register		*/
#define CAN_MB18_TIMESTAMP	0xFFC02E54	/* Mailbox 18 Time Stamp Value Register		*/
#define CAN_MB18_ID0		0xFFC02E58	/* Mailbox 18 Identifier Low Register		*/
#define CAN_MB18_ID1		0xFFC02E5C	/* Mailbox 18 Identifier High Register		*/

#define CAN_MB19_DATA0		0xFFC02E60	/* Mailbox 19 Data Word 0 [15:0] Register	*/
#define CAN_MB19_DATA1		0xFFC02E64	/* Mailbox 19 Data Word 1 [31:16] Register	*/
#define CAN_MB19_DATA2		0xFFC02E68	/* Mailbox 19 Data Word 2 [47:32] Register	*/
#define CAN_MB19_DATA3		0xFFC02E6C	/* Mailbox 19 Data Word 3 [63:48] Register	*/
#define CAN_MB19_LENGTH		0xFFC02E70	/* Mailbox 19 Data Length Code Register		*/
#define CAN_MB19_TIMESTAMP	0xFFC02E74	/* Mailbox 19 Time Stamp Value Register		*/
#define CAN_MB19_ID0		0xFFC02E78	/* Mailbox 19 Identifier Low Register		*/
#define CAN_MB19_ID1		0xFFC02E7C	/* Mailbox 19 Identifier High Register		*/

#define CAN_MB20_DATA0		0xFFC02E80	/* Mailbox 20 Data Word 0 [15:0] Register	*/
#define CAN_MB20_DATA1		0xFFC02E84	/* Mailbox 20 Data Word 1 [31:16] Register	*/
#define CAN_MB20_DATA2		0xFFC02E88	/* Mailbox 20 Data Word 2 [47:32] Register	*/
#define CAN_MB20_DATA3		0xFFC02E8C	/* Mailbox 20 Data Word 3 [63:48] Register	*/
#define CAN_MB20_LENGTH		0xFFC02E90	/* Mailbox 20 Data Length Code Register		*/
#define CAN_MB20_TIMESTAMP	0xFFC02E94	/* Mailbox 20 Time Stamp Value Register		*/
#define CAN_MB20_ID0		0xFFC02E98	/* Mailbox 20 Identifier Low Register		*/
#define CAN_MB20_ID1		0xFFC02E9C	/* Mailbox 20 Identifier High Register		*/

#define CAN_MB21_DATA0		0xFFC02EA0	/* Mailbox 21 Data Word 0 [15:0] Register	*/
#define CAN_MB21_DATA1		0xFFC02EA4	/* Mailbox 21 Data Word 1 [31:16] Register	*/
#define CAN_MB21_DATA2		0xFFC02EA8	/* Mailbox 21 Data Word 2 [47:32] Register	*/
#define CAN_MB21_DATA3		0xFFC02EAC	/* Mailbox 21 Data Word 3 [63:48] Register	*/
#define CAN_MB21_LENGTH		0xFFC02EB0	/* Mailbox 21 Data Length Code Register		*/
#define CAN_MB21_TIMESTAMP	0xFFC02EB4	/* Mailbox 21 Time Stamp Value Register		*/
#define CAN_MB21_ID0		0xFFC02EB8	/* Mailbox 21 Identifier Low Register		*/
#define CAN_MB21_ID1		0xFFC02EBC	/* Mailbox 21 Identifier High Register		*/

#define CAN_MB22_DATA0		0xFFC02EC0	/* Mailbox 22 Data Word 0 [15:0] Register	*/
#define CAN_MB22_DATA1		0xFFC02EC4	/* Mailbox 22 Data Word 1 [31:16] Register	*/
#define CAN_MB22_DATA2		0xFFC02EC8	/* Mailbox 22 Data Word 2 [47:32] Register	*/
#define CAN_MB22_DATA3		0xFFC02ECC	/* Mailbox 22 Data Word 3 [63:48] Register	*/
#define CAN_MB22_LENGTH		0xFFC02ED0	/* Mailbox 22 Data Length Code Register		*/
#define CAN_MB22_TIMESTAMP	0xFFC02ED4	/* Mailbox 22 Time Stamp Value Register		*/
#define CAN_MB22_ID0		0xFFC02ED8	/* Mailbox 22 Identifier Low Register		*/
#define CAN_MB22_ID1		0xFFC02EDC	/* Mailbox 22 Identifier High Register		*/

#define CAN_MB23_DATA0		0xFFC02EE0	/* Mailbox 23 Data Word 0 [15:0] Register	*/
#define CAN_MB23_DATA1		0xFFC02EE4	/* Mailbox 23 Data Word 1 [31:16] Register	*/
#define CAN_MB23_DATA2		0xFFC02EE8	/* Mailbox 23 Data Word 2 [47:32] Register	*/
#define CAN_MB23_DATA3		0xFFC02EEC	/* Mailbox 23 Data Word 3 [63:48] Register	*/
#define CAN_MB23_LENGTH		0xFFC02EF0	/* Mailbox 23 Data Length Code Register		*/
#define CAN_MB23_TIMESTAMP	0xFFC02EF4	/* Mailbox 23 Time Stamp Value Register		*/
#define CAN_MB23_ID0		0xFFC02EF8	/* Mailbox 23 Identifier Low Register		*/
#define CAN_MB23_ID1		0xFFC02EFC	/* Mailbox 23 Identifier High Register		*/

#define CAN_MB24_DATA0		0xFFC02F00	/* Mailbox 24 Data Word 0 [15:0] Register	*/
#define CAN_MB24_DATA1		0xFFC02F04	/* Mailbox 24 Data Word 1 [31:16] Register	*/
#define CAN_MB24_DATA2		0xFFC02F08	/* Mailbox 24 Data Word 2 [47:32] Register	*/
#define CAN_MB24_DATA3		0xFFC02F0C	/* Mailbox 24 Data Word 3 [63:48] Register	*/
#define CAN_MB24_LENGTH		0xFFC02F10	/* Mailbox 24 Data Length Code Register		*/
#define CAN_MB24_TIMESTAMP	0xFFC02F14	/* Mailbox 24 Time Stamp Value Register		*/
#define CAN_MB24_ID0		0xFFC02F18	/* Mailbox 24 Identifier Low Register		*/
#define CAN_MB24_ID1		0xFFC02F1C	/* Mailbox 24 Identifier High Register		*/

#define CAN_MB25_DATA0		0xFFC02F20	/* Mailbox 25 Data Word 0 [15:0] Register	*/
#define CAN_MB25_DATA1		0xFFC02F24	/* Mailbox 25 Data Word 1 [31:16] Register	*/
#define CAN_MB25_DATA2		0xFFC02F28	/* Mailbox 25 Data Word 2 [47:32] Register	*/
#define CAN_MB25_DATA3		0xFFC02F2C	/* Mailbox 25 Data Word 3 [63:48] Register	*/
#define CAN_MB25_LENGTH		0xFFC02F30	/* Mailbox 25 Data Length Code Register		*/
#define CAN_MB25_TIMESTAMP	0xFFC02F34	/* Mailbox 25 Time Stamp Value Register		*/
#define CAN_MB25_ID0		0xFFC02F38	/* Mailbox 25 Identifier Low Register		*/
#define CAN_MB25_ID1		0xFFC02F3C	/* Mailbox 25 Identifier High Register		*/

#define CAN_MB26_DATA0		0xFFC02F40	/* Mailbox 26 Data Word 0 [15:0] Register	*/
#define CAN_MB26_DATA1		0xFFC02F44	/* Mailbox 26 Data Word 1 [31:16] Register	*/
#define CAN_MB26_DATA2		0xFFC02F48	/* Mailbox 26 Data Word 2 [47:32] Register	*/
#define CAN_MB26_DATA3		0xFFC02F4C	/* Mailbox 26 Data Word 3 [63:48] Register	*/
#define CAN_MB26_LENGTH		0xFFC02F50	/* Mailbox 26 Data Length Code Register		*/
#define CAN_MB26_TIMESTAMP	0xFFC02F54	/* Mailbox 26 Time Stamp Value Register		*/
#define CAN_MB26_ID0		0xFFC02F58	/* Mailbox 26 Identifier Low Register		*/
#define CAN_MB26_ID1		0xFFC02F5C	/* Mailbox 26 Identifier High Register		*/

#define CAN_MB27_DATA0		0xFFC02F60	/* Mailbox 27 Data Word 0 [15:0] Register	*/
#define CAN_MB27_DATA1		0xFFC02F64	/* Mailbox 27 Data Word 1 [31:16] Register	*/
#define CAN_MB27_DATA2		0xFFC02F68	/* Mailbox 27 Data Word 2 [47:32] Register	*/
#define CAN_MB27_DATA3		0xFFC02F6C	/* Mailbox 27 Data Word 3 [63:48] Register	*/
#define CAN_MB27_LENGTH		0xFFC02F70	/* Mailbox 27 Data Length Code Register		*/
#define CAN_MB27_TIMESTAMP	0xFFC02F74	/* Mailbox 27 Time Stamp Value Register		*/
#define CAN_MB27_ID0		0xFFC02F78	/* Mailbox 27 Identifier Low Register		*/
#define CAN_MB27_ID1		0xFFC02F7C	/* Mailbox 27 Identifier High Register		*/

#define CAN_MB28_DATA0		0xFFC02F80	/* Mailbox 28 Data Word 0 [15:0] Register	*/
#define CAN_MB28_DATA1		0xFFC02F84	/* Mailbox 28 Data Word 1 [31:16] Register	*/
#define CAN_MB28_DATA2		0xFFC02F88	/* Mailbox 28 Data Word 2 [47:32] Register	*/
#define CAN_MB28_DATA3		0xFFC02F8C	/* Mailbox 28 Data Word 3 [63:48] Register	*/
#define CAN_MB28_LENGTH		0xFFC02F90	/* Mailbox 28 Data Length Code Register		*/
#define CAN_MB28_TIMESTAMP	0xFFC02F94	/* Mailbox 28 Time Stamp Value Register		*/
#define CAN_MB28_ID0		0xFFC02F98	/* Mailbox 28 Identifier Low Register		*/
#define CAN_MB28_ID1		0xFFC02F9C	/* Mailbox 28 Identifier High Register		*/

#define CAN_MB29_DATA0		0xFFC02FA0	/* Mailbox 29 Data Word 0 [15:0] Register	*/
#define CAN_MB29_DATA1		0xFFC02FA4	/* Mailbox 29 Data Word 1 [31:16] Register	*/
#define CAN_MB29_DATA2		0xFFC02FA8	/* Mailbox 29 Data Word 2 [47:32] Register	*/
#define CAN_MB29_DATA3		0xFFC02FAC	/* Mailbox 29 Data Word 3 [63:48] Register	*/
#define CAN_MB29_LENGTH		0xFFC02FB0	/* Mailbox 29 Data Length Code Register		*/
#define CAN_MB29_TIMESTAMP	0xFFC02FB4	/* Mailbox 29 Time Stamp Value Register		*/
#define CAN_MB29_ID0		0xFFC02FB8	/* Mailbox 29 Identifier Low Register		*/
#define CAN_MB29_ID1		0xFFC02FBC	/* Mailbox 29 Identifier High Register		*/

#define CAN_MB30_DATA0		0xFFC02FC0	/* Mailbox 30 Data Word 0 [15:0] Register	*/
#define CAN_MB30_DATA1		0xFFC02FC4	/* Mailbox 30 Data Word 1 [31:16] Register	*/
#define CAN_MB30_DATA2		0xFFC02FC8	/* Mailbox 30 Data Word 2 [47:32] Register	*/
#define CAN_MB30_DATA3		0xFFC02FCC	/* Mailbox 30 Data Word 3 [63:48] Register	*/
#define CAN_MB30_LENGTH		0xFFC02FD0	/* Mailbox 30 Data Length Code Register		*/
#define CAN_MB30_TIMESTAMP	0xFFC02FD4	/* Mailbox 30 Time Stamp Value Register		*/
#define CAN_MB30_ID0		0xFFC02FD8	/* Mailbox 30 Identifier Low Register		*/
#define CAN_MB30_ID1		0xFFC02FDC	/* Mailbox 30 Identifier High Register		*/

#define CAN_MB31_DATA0		0xFFC02FE0	/* Mailbox 31 Data Word 0 [15:0] Register	*/
#define CAN_MB31_DATA1		0xFFC02FE4	/* Mailbox 31 Data Word 1 [31:16] Register	*/
#define CAN_MB31_DATA2		0xFFC02FE8	/* Mailbox 31 Data Word 2 [47:32] Register	*/
#define CAN_MB31_DATA3		0xFFC02FEC	/* Mailbox 31 Data Word 3 [63:48] Register	*/
#define CAN_MB31_LENGTH		0xFFC02FF0	/* Mailbox 31 Data Length Code Register		*/
#define CAN_MB31_TIMESTAMP	0xFFC02FF4	/* Mailbox 31 Time Stamp Value Register		*/
#define CAN_MB31_ID0		0xFFC02FF8	/* Mailbox 31 Identifier Low Register		*/
#define CAN_MB31_ID1		0xFFC02FFC	/* Mailbox 31 Identifier High Register		*/

/* CAN Mailbox Area Macros				*/
#define CAN_MB_ID1(x)		(CAN_MB00_ID1+((x)*0x20))
#define CAN_MB_ID0(x)		(CAN_MB00_ID0+((x)*0x20))
#define CAN_MB_TIMESTAMP(x)	(CAN_MB00_TIMESTAMP+((x)*0x20))
#define CAN_MB_LENGTH(x)	(CAN_MB00_LENGTH+((x)*0x20))
#define CAN_MB_DATA3(x)		(CAN_MB00_DATA3+((x)*0x20))
#define CAN_MB_DATA2(x)		(CAN_MB00_DATA2+((x)*0x20))
#define CAN_MB_DATA1(x)		(CAN_MB00_DATA1+((x)*0x20))
#define CAN_MB_DATA0(x)		(CAN_MB00_DATA0+((x)*0x20))


/************************************************************************************
** System MMR Register Bits And Macros
*************************************************************************************
**/

/* ********************* PLL AND RESET MASKS ****************************************/

/* PLL_CTL Masks (IN_DELAY and OUT_DELAY bit field definitions differ from BF533/BF532/BF531) */
#define IN_DELAY			0x0014  /* EBIU Input Delay Select */
#define OUT_DELAY			0x00C0  /* EBIU Output Delay Select */

#define SET_OUT_DELAY(x)	(((x)&0x03) << 0x6)
#define SET_IN_DELAY(x)		((((x)&0x02) << 0x3) | (((x)&0x01) << 0x2))

/* VR_CTL Masks	(Additional WakeUp Events) */
#define	CANWE				0x0200	/* Enable CAN Wakeup From Hibernate */
#define	GPWE				0x0400	/* Enable General-Purpose Wakeup From Hibernate */


/* **********************  SYSTEM INTERRUPT CONTROLLER MASKS **********************	*/
/* Peripheral Masks For SIC_ISR0, SIC_IWR0, SIC_IMASK0            */
#define DMAC0_ERR_IRQ		0x00000002	/* DMA Controller 0 Error Interrupt Request */
#define SPI0_ERR_IRQ		0x00000020	/* SPI0 Error Interrupt Request */
#define UART0_ERR_IRQ		0x00000040	/* UART0 Error Interrupt Request */
#define MDMA0_0_IRQ			0x00200000	/* MemDMA0 Stream 0 Interrupt Request */
#define MDMA0_1_IRQ			0x00400000	/* MemDMA0 Stream 1 Interrupt Request */
#define DMAC1_ERR_IRQ		0x01000000	/* DMA Controller 1 Error Interrupt Request */
#define SPORT2_ERR_IRQ		0x02000000	/* SPORT2 Error Interrupt Request */
#define SPORT3_ERR_IRQ		0x04000000	/* SPORT3 Error Interrupt Request */
#define SPI1_ERR_IRQ		0x10000000	/* SPI1 Error Interrupt Request */
#define SPI2_ERR_IRQ		0x20000000	/* SPI2 Error Interrupt Request */
#define UART1_ERR_IRQ		0x40000000	/* UART1 Error Interrupt Request */
#define UART2_ERR_IRQ		0x80000000	/* UART2 Error Interrupt Request */

#define DMA0_ERR_IRQ		DMAC0_ERR_IRQ	/* legacy */
#define DMA1_ERR_IRQ		DMAC1_ERR_IRQ	/* legacy */

/* Peripheral Masks For SIC_ISR1, SIC_IWR1, SIC_IMASK1								*/
#define CAN_ERR_IRQ			0x00000001	/* CAN Error Interrupt Request */
#define DMA8_IRQ			0x00000002	/* DMA Channel 8 (SPORT2 RX) Interrupt Request */
#define DMA9_IRQ			0x00000004	/* DMA Channel 9 (SPORT2 TX) Interrupt Request */
#define DMA10_IRQ			0x00000008	/* DMA Channel 10 (SPORT3 RX) Interrupt Request	*/
#define DMA11_IRQ			0x00000010	/* DMA Channel 11 (SPORT3 TX) Interrupt Request	*/
#define DMA12_IRQ			0x00000020	/* DMA Channel 12 Interrupt Request */
#define DMA13_IRQ			0x00000040	/* DMA Channel 13 Interrupt Request */
#define DMA14_IRQ			0x00000080	/* DMA Channel 14 (SPI1) Interrupt Request */
#define DMA15_IRQ			0x00000100	/* DMA Channel 15 (SPI2) Interrupt Request */
#define DMA16_IRQ			0x00000200	/* DMA Channel 16 (UART1 RX) Interrupt Request */
#define DMA17_IRQ			0x00000400	/* DMA Channel 17 (UART1 TX) Interrupt Request */
#define DMA18_IRQ			0x00000800	/* DMA Channel 18 (UART2 RX) Interrupt Request */
#define DMA19_IRQ			0x00001000	/* DMA Channel 19 (UART2 TX) Interrupt Request */
#define TWI0_IRQ			0x00002000	/* TWI0 Interrupt Request */
#define TWI1_IRQ			0x00004000	/* TWI1 Interrupt Request */
#define CAN_RX_IRQ			0x00008000	/* CAN Receive Interrupt Request */
#define CAN_TX_IRQ			0x00010000	/* CAN Transmit Interrupt Request */
#define MDMA1_0_IRQ			0x00020000	/* MemDMA1 Stream 0 Interrupt Request */
#define MDMA1_1_IRQ			0x00040000	/* MemDMA1 Stream 1 Interrupt Request */


/* SIC_IAR3 Macros															*/
#define P24_IVG(x)		(((x)-7)&0xF)			/* Peripheral #24 assigned IVG #x	*/
#define P25_IVG(x)		(((x)-7)&0xF) << 0x4	/* Peripheral #25 assigned IVG #x	*/
#define P26_IVG(x)		(((x)-7)&0xF) << 0x8	/* Peripheral #26 assigned IVG #x	*/
#define P27_IVG(x)		(((x)-7)&0xF) << 0xC	/* Peripheral #27 assigned IVG #x	*/
#define P28_IVG(x)		(((x)-7)&0xF) << 0x10	/* Peripheral #28 assigned IVG #x	*/
#define P29_IVG(x)		(((x)-7)&0xF) << 0x14	/* Peripheral #29 assigned IVG #x	*/
#define P30_IVG(x)		(((x)-7)&0xF) << 0x18	/* Peripheral #30 assigned IVG #x	*/
#define P31_IVG(x)		(((x)-7)&0xF) << 0x1C	/* Peripheral #31 assigned IVG #x	*/

/* SIC_IAR4 Macros															*/
#define P32_IVG(x)		(((x)-7)&0xF)			/* Peripheral #32 assigned IVG #x	*/
#define P33_IVG(x)		(((x)-7)&0xF) << 0x4	/* Peripheral #33 assigned IVG #x	*/
#define P34_IVG(x)		(((x)-7)&0xF) << 0x8	/* Peripheral #34 assigned IVG #x	*/
#define P35_IVG(x)		(((x)-7)&0xF) << 0xC	/* Peripheral #35 assigned IVG #x	*/
#define P36_IVG(x)		(((x)-7)&0xF) << 0x10	/* Peripheral #36 assigned IVG #x	*/
#define P37_IVG(x)		(((x)-7)&0xF) << 0x14	/* Peripheral #37 assigned IVG #x	*/
#define P38_IVG(x)		(((x)-7)&0xF) << 0x18	/* Peripheral #38 assigned IVG #x	*/
#define P39_IVG(x)		(((x)-7)&0xF) << 0x1C	/* Peripheral #39 assigned IVG #x	*/

/* SIC_IAR5 Macros															*/
#define P40_IVG(x)		(((x)-7)&0xF)			/* Peripheral #40 assigned IVG #x	*/
#define P41_IVG(x)		(((x)-7)&0xF) << 0x4	/* Peripheral #41 assigned IVG #x	*/
#define P42_IVG(x)		(((x)-7)&0xF) << 0x8	/* Peripheral #42 assigned IVG #x	*/
#define P43_IVG(x)		(((x)-7)&0xF) << 0xC	/* Peripheral #43 assigned IVG #x	*/
#define P44_IVG(x)		(((x)-7)&0xF) << 0x10	/* Peripheral #44 assigned IVG #x	*/
#define P45_IVG(x)		(((x)-7)&0xF) << 0x14	/* Peripheral #45 assigned IVG #x	*/
#define P46_IVG(x)		(((x)-7)&0xF) << 0x18	/* Peripheral #46 assigned IVG #x	*/
#define P47_IVG(x)		(((x)-7)&0xF) << 0x1C	/* Peripheral #47 assigned IVG #x	*/

/* SIC_IAR6 Macros															*/
#define P48_IVG(x)		(((x)-7)&0xF)			/* Peripheral #48 assigned IVG #x	*/
#define P49_IVG(x)		(((x)-7)&0xF) << 0x4	/* Peripheral #49 assigned IVG #x	*/
#define P50_IVG(x)		(((x)-7)&0xF) << 0x8	/* Peripheral #50 assigned IVG #x	*/
#define P51_IVG(x)		(((x)-7)&0xF) << 0xC	/* Peripheral #51 assigned IVG #x	*/
#define P52_IVG(x)		(((x)-7)&0xF) << 0x10	/* Peripheral #52 assigned IVG #x	*/
#define P53_IVG(x)		(((x)-7)&0xF) << 0x14	/* Peripheral #53 assigned IVG #x	*/
#define P54_IVG(x)		(((x)-7)&0xF) << 0x18	/* Peripheral #54 assigned IVG #x	*/
#define P55_IVG(x)		(((x)-7)&0xF) << 0x1C	/* Peripheral #55 assigned IVG #x	*/


/*******************   GPIO MASKS  *********************/
/* Port C Masks */
#define PC0		0x0001
#define PC1		0x0002
#define PC4		0x0010
#define PC5		0x0020
#define PC6		0x0040
#define PC7		0x0080
#define PC8		0x0100
#define PC9		0x0200
/* Port C Bit Positions */
#define PC0_P	0x0
#define PC1_P	0x1
#define PC4_P	0x4
#define PC5_P	0x5
#define PC6_P	0x6
#define PC7_P	0x7
#define PC8_P	0x8
#define PC9_P	0x9

/* Port D */
#define PD0		0x0001
#define PD1		0x0002
#define PD2		0x0004
#define PD3		0x0008
#define PD4		0x0010
#define PD5		0x0020
#define PD6		0x0040
#define PD7		0x0080
#define PD8		0x0100
#define PD9		0x0200
#define PD10	0x0400
#define PD11	0x0800
#define PD12	0x1000
#define PD13	0x2000
#define PD14	0x4000
#define PD15	0x8000
/* Port D Bit Positions */
#define PD0_P	0x0
#define PD1_P	0x1
#define PD2_P	0x2
#define PD3_P	0x3
#define PD4_P	0x4
#define PD5_P	0x5
#define PD6_P	0x6
#define PD7_P	0x7
#define PD8_P	0x8
#define PD9_P	0x9
#define PD10_P	0xA
#define PD11_P	0xB
#define PD12_P	0xC
#define PD13_P	0xD
#define PD14_P	0xE
#define PD15_P	0xF

/* Port E */
#define PE0		0x0001
#define PE1		0x0002
#define PE2		0x0004
#define PE3		0x0008
#define PE4		0x0010
#define PE5		0x0020
#define PE6		0x0040
#define PE7		0x0080
#define PE8		0x0100
#define PE9		0x0200
#define PE10	0x0400
#define PE11	0x0800
#define PE12	0x1000
#define PE13	0x2000
#define PE14	0x4000
#define PE15	0x8000
/* Port E Bit Positions */
#define PE0_P	0x0
#define PE1_P	0x1
#define PE2_P	0x2
#define PE3_P	0x3
#define PE4_P	0x4
#define PE5_P	0x5
#define PE6_P	0x6
#define PE7_P	0x7
#define PE8_P	0x8
#define PE9_P	0x9
#define PE10_P	0xA
#define PE11_P	0xB
#define PE12_P	0xC
#define PE13_P	0xD
#define PE14_P	0xE
#define PE15_P	0xF


/* ****************  DMA CONTROLLER 0 (DMAC0) MASKS  ***************************/
/* PMAP Encodings For DMA Controller 0 */
#define PMAP_SPI0           PMAP_SPI  /* PMAP SPI0 DMA */
#define PMAP_UART0RX        PMAP_UARTRX  /* PMAP UART0 Receive DMA */
#define PMAP_UART0TX        PMAP_UARTTX  /* PMAP UART0 Transmit DMA */

/* ****************  DMA CONTROLLER 1 (DMAC1) MASKS  ***************************/
/* PMAP Encodings For DMA Controller 1 */
#define PMAP_SPORT2RX       0x0000  /* PMAP SPORT2 Receive DMA */
#define	PMAP_SPORT2TX       0x1000  /* PMAP SPORT2 Transmit DMA */
#define PMAP_SPORT3RX       0x2000  /* PMAP SPORT3 Receive DMA */
#define PMAP_SPORT3TX       0x3000  /* PMAP SPORT3 Transmit DMA */
#define PMAP_SPI1           0x6000  /* PMAP SPI1 DMA */
#define PMAP_SPI2           0x7000  /* PMAP SPI2 DMA */
#define PMAP_UART1RX        0x8000  /* PMAP UART1 Receive DMA */
#define	PMAP_UART1TX        0x9000  /* PMAP UART1 Transmit DMA */
#define PMAP_UART2RX        0xA000  /* PMAP UART2 Receive DMA */
#define PMAP_UART2TX        0xB000  /* PMAP UART2 Transmit DMA */


/*  ********************  TWO-WIRE INTERFACE (TWIx) MASKS  ***********************/
/* TWIx_CLKDIV Macros (Use: *pTWIx_CLKDIV = CLKLOW(x)|CLKHI(y);  )				*/
#define	CLKLOW(x)	((x) & 0xFF)		/* Periods Clock Is Held Low			*/
#define CLKHI(y)	(((y)&0xFF)<<0x8)	/* Periods Before New Clock Low			*/

/* TWIx_PRESCALE Masks															*/
#define	PRESCALE	0x007F		/* SCLKs Per Internal Time Reference (10MHz)	*/
#define	TWI_ENA		0x0080		/* TWI Enable									*/
#define	SCCB		0x0200		/* SCCB Compatibility Enable					*/

/* TWIx_SLAVE_CTRL Masks															*/
#define	SEN			0x0001		/* Slave Enable									*/
#define	SADD_LEN	0x0002		/* Slave Address Length							*/
#define	STDVAL		0x0004		/* Slave Transmit Data Valid					*/
#define	NAK			0x0008		/* NAK/ACK* Generated At Conclusion Of Transfer */
#define	GEN			0x0010		/* General Call Adrress Matching Enabled		*/

/* TWIx_SLAVE_STAT Masks															*/
#define	SDIR		0x0001		/* Slave Transfer Direction (Transmit/Receive*)	*/
#define GCALL		0x0002		/* General Call Indicator						*/

/* TWIx_MASTER_CTRL Masks													*/
#define	MEN			0x0001		/* Master Mode Enable						*/
#define	MADD_LEN	0x0002		/* Master Address Length					*/
#define	MDIR		0x0004		/* Master Transmit Direction (RX/TX*)		*/
#define	FAST		0x0008		/* Use Fast Mode Timing Specs				*/
#define	STOP		0x0010		/* Issue Stop Condition						*/
#define	RSTART		0x0020		/* Repeat Start or Stop* At End Of Transfer	*/
#define	DCNT		0x3FC0		/* Data Bytes To Transfer					*/
#define	SDAOVR		0x4000		/* Serial Data Override						*/
#define	SCLOVR		0x8000		/* Serial Clock Override					*/

/* TWIx_MASTER_STAT Masks														*/
#define	MPROG		0x0001		/* Master Transfer In Progress					*/
#define	LOSTARB		0x0002		/* Lost Arbitration Indicator (Xfer Aborted)	*/
#define	ANAK		0x0004		/* Address Not Acknowledged						*/
#define	DNAK		0x0008		/* Data Not Acknowledged						*/
#define	BUFRDERR	0x0010		/* Buffer Read Error							*/
#define	BUFWRERR	0x0020		/* Buffer Write Error							*/
#define	SDASEN		0x0040		/* Serial Data Sense							*/
#define	SCLSEN		0x0080		/* Serial Clock Sense							*/
#define	BUSBUSY		0x0100		/* Bus Busy Indicator							*/

/* TWIx_INT_SRC and TWIx_INT_ENABLE Masks						*/
#define	SINIT		0x0001		/* Slave Transfer Initiated	*/
#define	SCOMP		0x0002		/* Slave Transfer Complete	*/
#define	SERR		0x0004		/* Slave Transfer Error		*/
#define	SOVF		0x0008		/* Slave Overflow			*/
#define	MCOMP		0x0010		/* Master Transfer Complete	*/
#define	MERR		0x0020		/* Master Transfer Error	*/
#define	XMTSERV		0x0040		/* Transmit FIFO Service	*/
#define	RCVSERV		0x0080		/* Receive FIFO Service		*/

/* TWIx_FIFO_CTRL Masks												*/
#define	XMTFLUSH	0x0001		/* Transmit Buffer Flush			*/
#define	RCVFLUSH	0x0002		/* Receive Buffer Flush				*/
#define	XMTINTLEN	0x0004		/* Transmit Buffer Interrupt Length	*/
#define	RCVINTLEN	0x0008		/* Receive Buffer Interrupt Length	*/

/* TWIx_FIFO_STAT Masks															*/
#define	XMTSTAT		0x0003		/* Transmit FIFO Status							*/
#define	XMT_EMPTY	0x0000		/* 		Transmit FIFO Empty						*/
#define	XMT_HALF	0x0001		/* 		Transmit FIFO Has 1 Byte To Write		*/
#define	XMT_FULL	0x0003		/* 		Transmit FIFO Full (2 Bytes To Write)	*/

#define	RCVSTAT		0x000C		/* Receive FIFO Status							*/
#define	RCV_EMPTY	0x0000		/* 		Receive FIFO Empty						*/
#define	RCV_HALF	0x0004		/* 		Receive FIFO Has 1 Byte To Read			*/
#define	RCV_FULL	0x000C		/* 		Receive FIFO Full (2 Bytes To Read)		*/


/* ************  CONTROLLER AREA NETWORK (CAN) MASKS  ***************/
/* CAN_CONTROL Masks												*/
#define	SRS			0x0001	/* Software Reset						*/
#define	DNM			0x0002	/* Device Net Mode						*/
#define	ABO			0x0004	/* Auto-Bus On Enable					*/
#define	WBA			0x0010	/* Wake-Up On CAN Bus Activity Enable	*/
#define	SMR			0x0020	/* Sleep Mode Request					*/
#define	CSR			0x0040	/* CAN Suspend Mode Request				*/
#define	CCR			0x0080	/* CAN Configuration Mode Request		*/

/* CAN_STATUS Masks												*/
#define	WT			0x0001	/* TX Warning Flag					*/
#define	WR			0x0002	/* RX Warning Flag					*/
#define	EP			0x0004	/* Error Passive Mode				*/
#define	EBO			0x0008	/* Error Bus Off Mode				*/
#define	CSA			0x0040	/* Suspend Mode Acknowledge			*/
#define	CCA			0x0080	/* Configuration Mode Acknowledge	*/
#define	MBPTR		0x1F00	/* Mailbox Pointer					*/
#define	TRM			0x4000	/* Transmit Mode					*/
#define	REC			0x8000	/* Receive Mode						*/

/* CAN_CLOCK Masks									*/
#define	BRP			0x03FF	/* Bit-Rate Pre-Scaler	*/

/* CAN_TIMING Masks											*/
#define	TSEG1		0x000F	/* Time Segment 1				*/
#define	TSEG2		0x0070	/* Time Segment 2				*/
#define	SAM			0x0080	/* Sampling						*/
#define	SJW			0x0300	/* Synchronization Jump Width	*/

/* CAN_DEBUG Masks											*/
#define	DEC			0x0001	/* Disable CAN Error Counters	*/
#define	DRI			0x0002	/* Disable CAN RX Input			*/
#define	DTO			0x0004	/* Disable CAN TX Output		*/
#define	DIL			0x0008	/* Disable CAN Internal Loop	*/
#define	MAA			0x0010	/* Mode Auto-Acknowledge Enable	*/
#define	MRB			0x0020	/* Mode Read Back Enable		*/
#define	CDE			0x8000	/* CAN Debug Enable				*/

/* CAN_CEC Masks										*/
#define	RXECNT		0x00FF	/* Receive Error Counter	*/
#define	TXECNT		0xFF00	/* Transmit Error Counter	*/

/* CAN_INTR Masks											*/
#define	MBRIRQ	0x0001	/* Mailbox Receive Interrupt	*/
#define	MBRIF		MBRIRQ	/* legacy */
#define	MBTIRQ	0x0002	/* Mailbox Transmit Interrupt	*/
#define	MBTIF		MBTIRQ	/* legacy */
#define	GIRQ		0x0004	/* Global Interrupt				*/
#define	SMACK		0x0008	/* Sleep Mode Acknowledge		*/
#define	CANTX		0x0040	/* CAN TX Bus Value				*/
#define	CANRX		0x0080	/* CAN RX Bus Value				*/

/* CAN_MBxx_ID1 and CAN_MBxx_ID0 Masks										*/
#define DFC			0xFFFF	/* Data Filtering Code (If Enabled) (ID0)		*/
#define	EXTID_LO	0xFFFF	/* Lower 16 Bits of Extended Identifier (ID0)	*/
#define	EXTID_HI	0x0003	/* Upper 2 Bits of Extended Identifier (ID1)	*/
#define	BASEID		0x1FFC	/* Base Identifier								*/
#define	IDE			0x2000	/* Identifier Extension							*/
#define	RTR			0x4000	/* Remote Frame Transmission Request			*/
#define	AME			0x8000	/* Acceptance Mask Enable						*/

/* CAN_MBxx_TIMESTAMP Masks					*/
#define TSV			0xFFFF	/* Timestamp	*/

/* CAN_MBxx_LENGTH Masks						*/
#define DLC			0x000F	/* Data Length Code	*/

/* CAN_AMxxH and CAN_AMxxL Masks												*/
#define DFM			0xFFFF	/* Data Field Mask (If Enabled) (CAN_AMxxL)			*/
#define	EXTID_LO	0xFFFF	/* Lower 16 Bits of Extended Identifier (CAN_AMxxL)	*/
#define	EXTID_HI	0x0003	/* Upper 2 Bits of Extended Identifier (CAN_AMxxH)	*/
#define	BASEID		0x1FFC	/* Base Identifier									*/
#define	AMIDE		0x2000	/* Acceptance Mask ID Extension Enable				*/
#define	FMD			0x4000	/* Full Mask Data Field Enable						*/
#define	FDF			0x8000	/* Filter On Data Field Enable						*/

/* CAN_MC1 Masks									*/
#define	MC0			0x0001	/* Enable Mailbox 0		*/
#define	MC1			0x0002	/* Enable Mailbox 1		*/
#define	MC2			0x0004	/* Enable Mailbox 2		*/
#define	MC3			0x0008	/* Enable Mailbox 3		*/
#define	MC4			0x0010	/* Enable Mailbox 4		*/
#define	MC5			0x0020	/* Enable Mailbox 5		*/
#define	MC6			0x0040	/* Enable Mailbox 6		*/
#define	MC7			0x0080	/* Enable Mailbox 7		*/
#define	MC8			0x0100	/* Enable Mailbox 8		*/
#define	MC9			0x0200	/* Enable Mailbox 9		*/
#define	MC10		0x0400	/* Enable Mailbox 10	*/
#define	MC11		0x0800	/* Enable Mailbox 11	*/
#define	MC12		0x1000	/* Enable Mailbox 12	*/
#define	MC13		0x2000	/* Enable Mailbox 13	*/
#define	MC14		0x4000	/* Enable Mailbox 14	*/
#define	MC15		0x8000	/* Enable Mailbox 15	*/

/* CAN_MC2 Masks									*/
#define	MC16		0x0001	/* Enable Mailbox 16	*/
#define	MC17		0x0002	/* Enable Mailbox 17	*/
#define	MC18		0x0004	/* Enable Mailbox 18	*/
#define	MC19		0x0008	/* Enable Mailbox 19	*/
#define	MC20		0x0010	/* Enable Mailbox 20	*/
#define	MC21		0x0020	/* Enable Mailbox 21	*/
#define	MC22		0x0040	/* Enable Mailbox 22	*/
#define	MC23		0x0080	/* Enable Mailbox 23	*/
#define	MC24		0x0100	/* Enable Mailbox 24	*/
#define	MC25		0x0200	/* Enable Mailbox 25	*/
#define	MC26		0x0400	/* Enable Mailbox 26	*/
#define	MC27		0x0800	/* Enable Mailbox 27	*/
#define	MC28		0x1000	/* Enable Mailbox 28	*/
#define	MC29		0x2000	/* Enable Mailbox 29	*/
#define	MC30		0x4000	/* Enable Mailbox 30	*/
#define	MC31		0x8000	/* Enable Mailbox 31	*/

/* CAN_MD1 Masks												*/
#define	MD0			0x0001	/* Enable Mailbox 0 For Receive		*/
#define	MD1			0x0002	/* Enable Mailbox 1 For Receive		*/
#define	MD2			0x0004	/* Enable Mailbox 2 For Receive		*/
#define	MD3			0x0008	/* Enable Mailbox 3 For Receive		*/
#define	MD4			0x0010	/* Enable Mailbox 4 For Receive		*/
#define	MD5			0x0020	/* Enable Mailbox 5 For Receive		*/
#define	MD6			0x0040	/* Enable Mailbox 6 For Receive		*/
#define	MD7			0x0080	/* Enable Mailbox 7 For Receive		*/
#define	MD8			0x0100	/* Enable Mailbox 8 For Receive		*/
#define	MD9			0x0200	/* Enable Mailbox 9 For Receive		*/
#define	MD10		0x0400	/* Enable Mailbox 10 For Receive	*/
#define	MD11		0x0800	/* Enable Mailbox 11 For Receive	*/
#define	MD12		0x1000	/* Enable Mailbox 12 For Receive	*/
#define	MD13		0x2000	/* Enable Mailbox 13 For Receive	*/
#define	MD14		0x4000	/* Enable Mailbox 14 For Receive	*/
#define	MD15		0x8000	/* Enable Mailbox 15 For Receive	*/

/* CAN_MD2 Masks												*/
#define	MD16		0x0001	/* Enable Mailbox 16 For Receive	*/
#define	MD17		0x0002	/* Enable Mailbox 17 For Receive	*/
#define	MD18		0x0004	/* Enable Mailbox 18 For Receive	*/
#define	MD19		0x0008	/* Enable Mailbox 19 For Receive	*/
#define	MD20		0x0010	/* Enable Mailbox 20 For Receive	*/
#define	MD21		0x0020	/* Enable Mailbox 21 For Receive	*/
#define	MD22		0x0040	/* Enable Mailbox 22 For Receive	*/
#define	MD23		0x0080	/* Enable Mailbox 23 For Receive	*/
#define	MD24		0x0100	/* Enable Mailbox 24 For Receive	*/
#define	MD25		0x0200	/* Enable Mailbox 25 For Receive	*/
#define	MD26		0x0400	/* Enable Mailbox 26 For Receive	*/
#define	MD27		0x0800	/* Enable Mailbox 27 For Receive	*/
#define	MD28		0x1000	/* Enable Mailbox 28 For Receive	*/
#define	MD29		0x2000	/* Enable Mailbox 29 For Receive	*/
#define	MD30		0x4000	/* Enable Mailbox 30 For Receive	*/
#define	MD31		0x8000	/* Enable Mailbox 31 For Receive	*/

/* CAN_RMP1 Masks												*/
#define	RMP0		0x0001	/* RX Message Pending In Mailbox 0	*/
#define	RMP1		0x0002	/* RX Message Pending In Mailbox 1	*/
#define	RMP2		0x0004	/* RX Message Pending In Mailbox 2	*/
#define	RMP3		0x0008	/* RX Message Pending In Mailbox 3	*/
#define	RMP4		0x0010	/* RX Message Pending In Mailbox 4	*/
#define	RMP5		0x0020	/* RX Message Pending In Mailbox 5	*/
#define	RMP6		0x0040	/* RX Message Pending In Mailbox 6	*/
#define	RMP7		0x0080	/* RX Message Pending In Mailbox 7	*/
#define	RMP8		0x0100	/* RX Message Pending In Mailbox 8	*/
#define	RMP9		0x0200	/* RX Message Pending In Mailbox 9	*/
#define	RMP10		0x0400	/* RX Message Pending In Mailbox 10	*/
#define	RMP11		0x0800	/* RX Message Pending In Mailbox 11	*/
#define	RMP12		0x1000	/* RX Message Pending In Mailbox 12	*/
#define	RMP13		0x2000	/* RX Message Pending In Mailbox 13	*/
#define	RMP14		0x4000	/* RX Message Pending In Mailbox 14	*/
#define	RMP15		0x8000	/* RX Message Pending In Mailbox 15	*/

/* CAN_RMP2 Masks												*/
#define	RMP16		0x0001	/* RX Message Pending In Mailbox 16	*/
#define	RMP17		0x0002	/* RX Message Pending In Mailbox 17	*/
#define	RMP18		0x0004	/* RX Message Pending In Mailbox 18	*/
#define	RMP19		0x0008	/* RX Message Pending In Mailbox 19	*/
#define	RMP20		0x0010	/* RX Message Pending In Mailbox 20	*/
#define	RMP21		0x0020	/* RX Message Pending In Mailbox 21	*/
#define	RMP22		0x0040	/* RX Message Pending In Mailbox 22	*/
#define	RMP23		0x0080	/* RX Message Pending In Mailbox 23	*/
#define	RMP24		0x0100	/* RX Message Pending In Mailbox 24	*/
#define	RMP25		0x0200	/* RX Message Pending In Mailbox 25	*/
#define	RMP26		0x0400	/* RX Message Pending In Mailbox 26	*/
#define	RMP27		0x0800	/* RX Message Pending In Mailbox 27	*/
#define	RMP28		0x1000	/* RX Message Pending In Mailbox 28	*/
#define	RMP29		0x2000	/* RX Message Pending In Mailbox 29	*/
#define	RMP30		0x4000	/* RX Message Pending In Mailbox 30	*/
#define	RMP31		0x8000	/* RX Message Pending In Mailbox 31	*/

/* CAN_RML1 Masks												*/
#define	RML0		0x0001	/* RX Message Lost In Mailbox 0		*/
#define	RML1		0x0002	/* RX Message Lost In Mailbox 1		*/
#define	RML2		0x0004	/* RX Message Lost In Mailbox 2		*/
#define	RML3		0x0008	/* RX Message Lost In Mailbox 3		*/
#define	RML4		0x0010	/* RX Message Lost In Mailbox 4		*/
#define	RML5		0x0020	/* RX Message Lost In Mailbox 5		*/
#define	RML6		0x0040	/* RX Message Lost In Mailbox 6		*/
#define	RML7		0x0080	/* RX Message Lost In Mailbox 7		*/
#define	RML8		0x0100	/* RX Message Lost In Mailbox 8		*/
#define	RML9		0x0200	/* RX Message Lost In Mailbox 9		*/
#define	RML10		0x0400	/* RX Message Lost In Mailbox 10	*/
#define	RML11		0x0800	/* RX Message Lost In Mailbox 11	*/
#define	RML12		0x1000	/* RX Message Lost In Mailbox 12	*/
#define	RML13		0x2000	/* RX Message Lost In Mailbox 13	*/
#define	RML14		0x4000	/* RX Message Lost In Mailbox 14	*/
#define	RML15		0x8000	/* RX Message Lost In Mailbox 15	*/

/* CAN_RML2 Masks												*/
#define	RML16		0x0001	/* RX Message Lost In Mailbox 16	*/
#define	RML17		0x0002	/* RX Message Lost In Mailbox 17	*/
#define	RML18		0x0004	/* RX Message Lost In Mailbox 18	*/
#define	RML19		0x0008	/* RX Message Lost In Mailbox 19	*/
#define	RML20		0x0010	/* RX Message Lost In Mailbox 20	*/
#define	RML21		0x0020	/* RX Message Lost In Mailbox 21	*/
#define	RML22		0x0040	/* RX Message Lost In Mailbox 22	*/
#define	RML23		0x0080	/* RX Message Lost In Mailbox 23	*/
#define	RML24		0x0100	/* RX Message Lost In Mailbox 24	*/
#define	RML25		0x0200	/* RX Message Lost In Mailbox 25	*/
#define	RML26		0x0400	/* RX Message Lost In Mailbox 26	*/
#define	RML27		0x0800	/* RX Message Lost In Mailbox 27	*/
#define	RML28		0x1000	/* RX Message Lost In Mailbox 28	*/
#define	RML29		0x2000	/* RX Message Lost In Mailbox 29	*/
#define	RML30		0x4000	/* RX Message Lost In Mailbox 30	*/
#define	RML31		0x8000	/* RX Message Lost In Mailbox 31	*/

/* CAN_OPSS1 Masks																				*/
#define	OPSS0		0x0001	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 0	*/
#define	OPSS1		0x0002	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 1	*/
#define	OPSS2		0x0004	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 2	*/
#define	OPSS3		0x0008	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 3	*/
#define	OPSS4		0x0010	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 4	*/
#define	OPSS5		0x0020	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 5	*/
#define	OPSS6		0x0040	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 6	*/
#define	OPSS7		0x0080	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 7	*/
#define	OPSS8		0x0100	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 8	*/
#define	OPSS9		0x0200	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 9	*/
#define	OPSS10		0x0400	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 10	*/
#define	OPSS11		0x0800	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 11	*/
#define	OPSS12		0x1000	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 12	*/
#define	OPSS13		0x2000	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 13	*/
#define	OPSS14		0x4000	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 14	*/
#define	OPSS15		0x8000	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 15	*/

/* CAN_OPSS2 Masks																				*/
#define	OPSS16		0x0001	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 16	*/
#define	OPSS17		0x0002	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 17	*/
#define	OPSS18		0x0004	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 18	*/
#define	OPSS19		0x0008	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 19	*/
#define	OPSS20		0x0010	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 20	*/
#define	OPSS21		0x0020	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 21	*/
#define	OPSS22		0x0040	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 22	*/
#define	OPSS23		0x0080	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 23	*/
#define	OPSS24		0x0100	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 24	*/
#define	OPSS25		0x0200	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 25	*/
#define	OPSS26		0x0400	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 26	*/
#define	OPSS27		0x0800	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 27	*/
#define	OPSS28		0x1000	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 28	*/
#define	OPSS29		0x2000	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 29	*/
#define	OPSS30		0x4000	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 30	*/
#define	OPSS31		0x8000	/* Enable RX Overwrite Protection or TX Single-Shot For Mailbox 31	*/

/* CAN_TRR1 Masks														*/
#define	TRR0		0x0001	/* Deny But Don't Lock Access To Mailbox 0	*/
#define	TRR1		0x0002	/* Deny But Don't Lock Access To Mailbox 1	*/
#define	TRR2		0x0004	/* Deny But Don't Lock Access To Mailbox 2	*/
#define	TRR3		0x0008	/* Deny But Don't Lock Access To Mailbox 3	*/
#define	TRR4		0x0010	/* Deny But Don't Lock Access To Mailbox 4	*/
#define	TRR5		0x0020	/* Deny But Don't Lock Access To Mailbox 5	*/
#define	TRR6		0x0040	/* Deny But Don't Lock Access To Mailbox 6	*/
#define	TRR7		0x0080	/* Deny But Don't Lock Access To Mailbox 7	*/
#define	TRR8		0x0100	/* Deny But Don't Lock Access To Mailbox 8	*/
#define	TRR9		0x0200	/* Deny But Don't Lock Access To Mailbox 9	*/
#define	TRR10		0x0400	/* Deny But Don't Lock Access To Mailbox 10	*/
#define	TRR11		0x0800	/* Deny But Don't Lock Access To Mailbox 11	*/
#define	TRR12		0x1000	/* Deny But Don't Lock Access To Mailbox 12	*/
#define	TRR13		0x2000	/* Deny But Don't Lock Access To Mailbox 13	*/
#define	TRR14		0x4000	/* Deny But Don't Lock Access To Mailbox 14	*/
#define	TRR15		0x8000	/* Deny But Don't Lock Access To Mailbox 15	*/

/* CAN_TRR2 Masks														*/
#define	TRR16		0x0001	/* Deny But Don't Lock Access To Mailbox 16	*/
#define	TRR17		0x0002	/* Deny But Don't Lock Access To Mailbox 17	*/
#define	TRR18		0x0004	/* Deny But Don't Lock Access To Mailbox 18	*/
#define	TRR19		0x0008	/* Deny But Don't Lock Access To Mailbox 19	*/
#define	TRR20		0x0010	/* Deny But Don't Lock Access To Mailbox 20	*/
#define	TRR21		0x0020	/* Deny But Don't Lock Access To Mailbox 21	*/
#define	TRR22		0x0040	/* Deny But Don't Lock Access To Mailbox 22	*/
#define	TRR23		0x0080	/* Deny But Don't Lock Access To Mailbox 23	*/
#define	TRR24		0x0100	/* Deny But Don't Lock Access To Mailbox 24	*/
#define	TRR25		0x0200	/* Deny But Don't Lock Access To Mailbox 25	*/
#define	TRR26		0x0400	/* Deny But Don't Lock Access To Mailbox 26	*/
#define	TRR27		0x0800	/* Deny But Don't Lock Access To Mailbox 27	*/
#define	TRR28		0x1000	/* Deny But Don't Lock Access To Mailbox 28	*/
#define	TRR29		0x2000	/* Deny But Don't Lock Access To Mailbox 29	*/
#define	TRR30		0x4000	/* Deny But Don't Lock Access To Mailbox 30	*/
#define	TRR31		0x8000	/* Deny But Don't Lock Access To Mailbox 31	*/

/* CAN_TRS1 Masks													*/
#define	TRS0		0x0001	/* Remote Frame Request For Mailbox 0	*/
#define	TRS1		0x0002	/* Remote Frame Request For Mailbox 1	*/
#define	TRS2		0x0004	/* Remote Frame Request For Mailbox 2	*/
#define	TRS3		0x0008	/* Remote Frame Request For Mailbox 3	*/
#define	TRS4		0x0010	/* Remote Frame Request For Mailbox 4	*/
#define	TRS5		0x0020	/* Remote Frame Request For Mailbox 5	*/
#define	TRS6		0x0040	/* Remote Frame Request For Mailbox 6	*/
#define	TRS7		0x0080	/* Remote Frame Request For Mailbox 7	*/
#define	TRS8		0x0100	/* Remote Frame Request For Mailbox 8	*/
#define	TRS9		0x0200	/* Remote Frame Request For Mailbox 9	*/
#define	TRS10		0x0400	/* Remote Frame Request For Mailbox 10	*/
#define	TRS11		0x0800	/* Remote Frame Request For Mailbox 11	*/
#define	TRS12		0x1000	/* Remote Frame Request For Mailbox 12	*/
#define	TRS13		0x2000	/* Remote Frame Request For Mailbox 13	*/
#define	TRS14		0x4000	/* Remote Frame Request For Mailbox 14	*/
#define	TRS15		0x8000	/* Remote Frame Request For Mailbox 15	*/

/* CAN_TRS2 Masks													*/
#define	TRS16		0x0001	/* Remote Frame Request For Mailbox 16	*/
#define	TRS17		0x0002	/* Remote Frame Request For Mailbox 17	*/
#define	TRS18		0x0004	/* Remote Frame Request For Mailbox 18	*/
#define	TRS19		0x0008	/* Remote Frame Request For Mailbox 19	*/
#define	TRS20		0x0010	/* Remote Frame Request For Mailbox 20	*/
#define	TRS21		0x0020	/* Remote Frame Request For Mailbox 21	*/
#define	TRS22		0x0040	/* Remote Frame Request For Mailbox 22	*/
#define	TRS23		0x0080	/* Remote Frame Request For Mailbox 23	*/
#define	TRS24		0x0100	/* Remote Frame Request For Mailbox 24	*/
#define	TRS25		0x0200	/* Remote Frame Request For Mailbox 25	*/
#define	TRS26		0x0400	/* Remote Frame Request For Mailbox 26	*/
#define	TRS27		0x0800	/* Remote Frame Request For Mailbox 27	*/
#define	TRS28		0x1000	/* Remote Frame Request For Mailbox 28	*/
#define	TRS29		0x2000	/* Remote Frame Request For Mailbox 29	*/
#define	TRS30		0x4000	/* Remote Frame Request For Mailbox 30	*/
#define	TRS31		0x8000	/* Remote Frame Request For Mailbox 31	*/

/* CAN_AA1 Masks												*/
#define	AA0			0x0001	/* Aborted Message In Mailbox 0		*/
#define	AA1			0x0002	/* Aborted Message In Mailbox 1		*/
#define	AA2			0x0004	/* Aborted Message In Mailbox 2		*/
#define	AA3			0x0008	/* Aborted Message In Mailbox 3		*/
#define	AA4			0x0010	/* Aborted Message In Mailbox 4		*/
#define	AA5			0x0020	/* Aborted Message In Mailbox 5		*/
#define	AA6			0x0040	/* Aborted Message In Mailbox 6		*/
#define	AA7			0x0080	/* Aborted Message In Mailbox 7		*/
#define	AA8			0x0100	/* Aborted Message In Mailbox 8		*/
#define	AA9			0x0200	/* Aborted Message In Mailbox 9		*/
#define	AA10		0x0400	/* Aborted Message In Mailbox 10	*/
#define	AA11		0x0800	/* Aborted Message In Mailbox 11	*/
#define	AA12		0x1000	/* Aborted Message In Mailbox 12	*/
#define	AA13		0x2000	/* Aborted Message In Mailbox 13	*/
#define	AA14		0x4000	/* Aborted Message In Mailbox 14	*/
#define	AA15		0x8000	/* Aborted Message In Mailbox 15	*/

/* CAN_AA2 Masks												*/
#define	AA16		0x0001	/* Aborted Message In Mailbox 16	*/
#define	AA17		0x0002	/* Aborted Message In Mailbox 17	*/
#define	AA18		0x0004	/* Aborted Message In Mailbox 18	*/
#define	AA19		0x0008	/* Aborted Message In Mailbox 19	*/
#define	AA20		0x0010	/* Aborted Message In Mailbox 20	*/
#define	AA21		0x0020	/* Aborted Message In Mailbox 21	*/
#define	AA22		0x0040	/* Aborted Message In Mailbox 22	*/
#define	AA23		0x0080	/* Aborted Message In Mailbox 23	*/
#define	AA24		0x0100	/* Aborted Message In Mailbox 24	*/
#define	AA25		0x0200	/* Aborted Message In Mailbox 25	*/
#define	AA26		0x0400	/* Aborted Message In Mailbox 26	*/
#define	AA27		0x0800	/* Aborted Message In Mailbox 27	*/
#define	AA28		0x1000	/* Aborted Message In Mailbox 28	*/
#define	AA29		0x2000	/* Aborted Message In Mailbox 29	*/
#define	AA30		0x4000	/* Aborted Message In Mailbox 30	*/
#define	AA31		0x8000	/* Aborted Message In Mailbox 31	*/

/* CAN_TA1 Masks													*/
#define	TA0			0x0001	/* Transmit Successful From Mailbox 0	*/
#define	TA1			0x0002	/* Transmit Successful From Mailbox 1	*/
#define	TA2			0x0004	/* Transmit Successful From Mailbox 2	*/
#define	TA3			0x0008	/* Transmit Successful From Mailbox 3	*/
#define	TA4			0x0010	/* Transmit Successful From Mailbox 4	*/
#define	TA5			0x0020	/* Transmit Successful From Mailbox 5	*/
#define	TA6			0x0040	/* Transmit Successful From Mailbox 6	*/
#define	TA7			0x0080	/* Transmit Successful From Mailbox 7	*/
#define	TA8			0x0100	/* Transmit Successful From Mailbox 8	*/
#define	TA9			0x0200	/* Transmit Successful From Mailbox 9	*/
#define	TA10		0x0400	/* Transmit Successful From Mailbox 10	*/
#define	TA11		0x0800	/* Transmit Successful From Mailbox 11	*/
#define	TA12		0x1000	/* Transmit Successful From Mailbox 12	*/
#define	TA13		0x2000	/* Transmit Successful From Mailbox 13	*/
#define	TA14		0x4000	/* Transmit Successful From Mailbox 14	*/
#define	TA15		0x8000	/* Transmit Successful From Mailbox 15	*/

/* CAN_TA2 Masks													*/
#define	TA16		0x0001	/* Transmit Successful From Mailbox 16	*/
#define	TA17		0x0002	/* Transmit Successful From Mailbox 17	*/
#define	TA18		0x0004	/* Transmit Successful From Mailbox 18	*/
#define	TA19		0x0008	/* Transmit Successful From Mailbox 19	*/
#define	TA20		0x0010	/* Transmit Successful From Mailbox 20	*/
#define	TA21		0x0020	/* Transmit Successful From Mailbox 21	*/
#define	TA22		0x0040	/* Transmit Successful From Mailbox 22	*/
#define	TA23		0x0080	/* Transmit Successful From Mailbox 23	*/
#define	TA24		0x0100	/* Transmit Successful From Mailbox 24	*/
#define	TA25		0x0200	/* Transmit Successful From Mailbox 25	*/
#define	TA26		0x0400	/* Transmit Successful From Mailbox 26	*/
#define	TA27		0x0800	/* Transmit Successful From Mailbox 27	*/
#define	TA28		0x1000	/* Transmit Successful From Mailbox 28	*/
#define	TA29		0x2000	/* Transmit Successful From Mailbox 29	*/
#define	TA30		0x4000	/* Transmit Successful From Mailbox 30	*/
#define	TA31		0x8000	/* Transmit Successful From Mailbox 31	*/

/* CAN_MBTD Masks												*/
#define TDPTR		0x001F	/* Mailbox To Temporarily Disable	*/
#define	TDA			0x0040	/* Temporary Disable Acknowledge	*/
#define	TDR			0x0080	/* Temporary Disable Request		*/

/* CAN_RFH1 Masks																		*/
#define	RFH0		0x0001	/* Enable Automatic Remote Frame Handling For Mailbox 0		*/
#define	RFH1		0x0002	/* Enable Automatic Remote Frame Handling For Mailbox 1		*/
#define	RFH2		0x0004	/* Enable Automatic Remote Frame Handling For Mailbox 2		*/
#define	RFH3		0x0008	/* Enable Automatic Remote Frame Handling For Mailbox 3		*/
#define	RFH4		0x0010	/* Enable Automatic Remote Frame Handling For Mailbox 4		*/
#define	RFH5		0x0020	/* Enable Automatic Remote Frame Handling For Mailbox 5		*/
#define	RFH6		0x0040	/* Enable Automatic Remote Frame Handling For Mailbox 6		*/
#define	RFH7		0x0080	/* Enable Automatic Remote Frame Handling For Mailbox 7		*/
#define	RFH8		0x0100	/* Enable Automatic Remote Frame Handling For Mailbox 8		*/
#define	RFH9		0x0200	/* Enable Automatic Remote Frame Handling For Mailbox 9		*/
#define	RFH10		0x0400	/* Enable Automatic Remote Frame Handling For Mailbox 10	*/
#define	RFH11		0x0800	/* Enable Automatic Remote Frame Handling For Mailbox 11	*/
#define	RFH12		0x1000	/* Enable Automatic Remote Frame Handling For Mailbox 12	*/
#define	RFH13		0x2000	/* Enable Automatic Remote Frame Handling For Mailbox 13	*/
#define	RFH14		0x4000	/* Enable Automatic Remote Frame Handling For Mailbox 14	*/
#define	RFH15		0x8000	/* Enable Automatic Remote Frame Handling For Mailbox 15	*/

/* CAN_RFH2 Masks																		*/
#define	RFH16		0x0001	/* Enable Automatic Remote Frame Handling For Mailbox 16	*/
#define	RFH17		0x0002	/* Enable Automatic Remote Frame Handling For Mailbox 17	*/
#define	RFH18		0x0004	/* Enable Automatic Remote Frame Handling For Mailbox 18	*/
#define	RFH19		0x0008	/* Enable Automatic Remote Frame Handling For Mailbox 19	*/
#define	RFH20		0x0010	/* Enable Automatic Remote Frame Handling For Mailbox 20	*/
#define	RFH21		0x0020	/* Enable Automatic Remote Frame Handling For Mailbox 21	*/
#define	RFH22		0x0040	/* Enable Automatic Remote Frame Handling For Mailbox 22	*/
#define	RFH23		0x0080	/* Enable Automatic Remote Frame Handling For Mailbox 23	*/
#define	RFH24		0x0100	/* Enable Automatic Remote Frame Handling For Mailbox 24	*/
#define	RFH25		0x0200	/* Enable Automatic Remote Frame Handling For Mailbox 25	*/
#define	RFH26		0x0400	/* Enable Automatic Remote Frame Handling For Mailbox 26	*/
#define	RFH27		0x0800	/* Enable Automatic Remote Frame Handling For Mailbox 27	*/
#define	RFH28		0x1000	/* Enable Automatic Remote Frame Handling For Mailbox 28	*/
#define	RFH29		0x2000	/* Enable Automatic Remote Frame Handling For Mailbox 29	*/
#define	RFH30		0x4000	/* Enable Automatic Remote Frame Handling For Mailbox 30	*/
#define	RFH31		0x8000	/* Enable Automatic Remote Frame Handling For Mailbox 31	*/

/* CAN_MBTIF1 Masks													*/
#define	MBTIF0		0x0001	/* TX Interrupt Active In Mailbox 0		*/
#define	MBTIF1		0x0002	/* TX Interrupt Active In Mailbox 1		*/
#define	MBTIF2		0x0004	/* TX Interrupt Active In Mailbox 2		*/
#define	MBTIF3		0x0008	/* TX Interrupt Active In Mailbox 3		*/
#define	MBTIF4		0x0010	/* TX Interrupt Active In Mailbox 4		*/
#define	MBTIF5		0x0020	/* TX Interrupt Active In Mailbox 5		*/
#define	MBTIF6		0x0040	/* TX Interrupt Active In Mailbox 6		*/
#define	MBTIF7		0x0080	/* TX Interrupt Active In Mailbox 7		*/
#define	MBTIF8		0x0100	/* TX Interrupt Active In Mailbox 8		*/
#define	MBTIF9		0x0200	/* TX Interrupt Active In Mailbox 9		*/
#define	MBTIF10		0x0400	/* TX Interrupt Active In Mailbox 10	*/
#define	MBTIF11		0x0800	/* TX Interrupt Active In Mailbox 11	*/
#define	MBTIF12		0x1000	/* TX Interrupt Active In Mailbox 12	*/
#define	MBTIF13		0x2000	/* TX Interrupt Active In Mailbox 13	*/
#define	MBTIF14		0x4000	/* TX Interrupt Active In Mailbox 14	*/
#define	MBTIF15		0x8000	/* TX Interrupt Active In Mailbox 15	*/

/* CAN_MBTIF2 Masks													*/
#define	MBTIF16		0x0001	/* TX Interrupt Active In Mailbox 16	*/
#define	MBTIF17		0x0002	/* TX Interrupt Active In Mailbox 17	*/
#define	MBTIF18		0x0004	/* TX Interrupt Active In Mailbox 18	*/
#define	MBTIF19		0x0008	/* TX Interrupt Active In Mailbox 19	*/
#define	MBTIF20		0x0010	/* TX Interrupt Active In Mailbox 20	*/
#define	MBTIF21		0x0020	/* TX Interrupt Active In Mailbox 21	*/
#define	MBTIF22		0x0040	/* TX Interrupt Active In Mailbox 22	*/
#define	MBTIF23		0x0080	/* TX Interrupt Active In Mailbox 23	*/
#define	MBTIF24		0x0100	/* TX Interrupt Active In Mailbox 24	*/
#define	MBTIF25		0x0200	/* TX Interrupt Active In Mailbox 25	*/
#define	MBTIF26		0x0400	/* TX Interrupt Active In Mailbox 26	*/
#define	MBTIF27		0x0800	/* TX Interrupt Active In Mailbox 27	*/
#define	MBTIF28		0x1000	/* TX Interrupt Active In Mailbox 28	*/
#define	MBTIF29		0x2000	/* TX Interrupt Active In Mailbox 29	*/
#define	MBTIF30		0x4000	/* TX Interrupt Active In Mailbox 30	*/
#define	MBTIF31		0x8000	/* TX Interrupt Active In Mailbox 31	*/

/* CAN_MBRIF1 Masks													*/
#define	MBRIF0		0x0001	/* RX Interrupt Active In Mailbox 0		*/
#define	MBRIF1		0x0002	/* RX Interrupt Active In Mailbox 1		*/
#define	MBRIF2		0x0004	/* RX Interrupt Active In Mailbox 2		*/
#define	MBRIF3		0x0008	/* RX Interrupt Active In Mailbox 3		*/
#define	MBRIF4		0x0010	/* RX Interrupt Active In Mailbox 4		*/
#define	MBRIF5		0x0020	/* RX Interrupt Active In Mailbox 5		*/
#define	MBRIF6		0x0040	/* RX Interrupt Active In Mailbox 6		*/
#define	MBRIF7		0x0080	/* RX Interrupt Active In Mailbox 7		*/
#define	MBRIF8		0x0100	/* RX Interrupt Active In Mailbox 8		*/
#define	MBRIF9		0x0200	/* RX Interrupt Active In Mailbox 9		*/
#define	MBRIF10		0x0400	/* RX Interrupt Active In Mailbox 10	*/
#define	MBRIF11		0x0800	/* RX Interrupt Active In Mailbox 11	*/
#define	MBRIF12		0x1000	/* RX Interrupt Active In Mailbox 12	*/
#define	MBRIF13		0x2000	/* RX Interrupt Active In Mailbox 13	*/
#define	MBRIF14		0x4000	/* RX Interrupt Active In Mailbox 14	*/
#define	MBRIF15		0x8000	/* RX Interrupt Active In Mailbox 15	*/

/* CAN_MBRIF2 Masks													*/
#define	MBRIF16		0x0001	/* RX Interrupt Active In Mailbox 16	*/
#define	MBRIF17		0x0002	/* RX Interrupt Active In Mailbox 17	*/
#define	MBRIF18		0x0004	/* RX Interrupt Active In Mailbox 18	*/
#define	MBRIF19		0x0008	/* RX Interrupt Active In Mailbox 19	*/
#define	MBRIF20		0x0010	/* RX Interrupt Active In Mailbox 20	*/
#define	MBRIF21		0x0020	/* RX Interrupt Active In Mailbox 21	*/
#define	MBRIF22		0x0040	/* RX Interrupt Active In Mailbox 22	*/
#define	MBRIF23		0x0080	/* RX Interrupt Active In Mailbox 23	*/
#define	MBRIF24		0x0100	/* RX Interrupt Active In Mailbox 24	*/
#define	MBRIF25		0x0200	/* RX Interrupt Active In Mailbox 25	*/
#define	MBRIF26		0x0400	/* RX Interrupt Active In Mailbox 26	*/
#define	MBRIF27		0x0800	/* RX Interrupt Active In Mailbox 27	*/
#define	MBRIF28		0x1000	/* RX Interrupt Active In Mailbox 28	*/
#define	MBRIF29		0x2000	/* RX Interrupt Active In Mailbox 29	*/
#define	MBRIF30		0x4000	/* RX Interrupt Active In Mailbox 30	*/
#define	MBRIF31		0x8000	/* RX Interrupt Active In Mailbox 31	*/

/* CAN_MBIM1 Masks												*/
#define	MBIM0		0x0001	/* Enable Interrupt For Mailbox 0	*/
#define	MBIM1		0x0002	/* Enable Interrupt For Mailbox 1	*/
#define	MBIM2		0x0004	/* Enable Interrupt For Mailbox 2	*/
#define	MBIM3		0x0008	/* Enable Interrupt For Mailbox 3	*/
#define	MBIM4		0x0010	/* Enable Interrupt For Mailbox 4	*/
#define	MBIM5		0x0020	/* Enable Interrupt For Mailbox 5	*/
#define	MBIM6		0x0040	/* Enable Interrupt For Mailbox 6	*/
#define	MBIM7		0x0080	/* Enable Interrupt For Mailbox 7	*/
#define	MBIM8		0x0100	/* Enable Interrupt For Mailbox 8	*/
#define	MBIM9		0x0200	/* Enable Interrupt For Mailbox 9	*/
#define	MBIM10		0x0400	/* Enable Interrupt For Mailbox 10	*/
#define	MBIM11		0x0800	/* Enable Interrupt For Mailbox 11	*/
#define	MBIM12		0x1000	/* Enable Interrupt For Mailbox 12	*/
#define	MBIM13		0x2000	/* Enable Interrupt For Mailbox 13	*/
#define	MBIM14		0x4000	/* Enable Interrupt For Mailbox 14	*/
#define	MBIM15		0x8000	/* Enable Interrupt For Mailbox 15	*/

/* CAN_MBIM2 Masks												*/
#define	MBIM16		0x0001	/* Enable Interrupt For Mailbox 16	*/
#define	MBIM17		0x0002	/* Enable Interrupt For Mailbox 17	*/
#define	MBIM18		0x0004	/* Enable Interrupt For Mailbox 18	*/
#define	MBIM19		0x0008	/* Enable Interrupt For Mailbox 19	*/
#define	MBIM20		0x0010	/* Enable Interrupt For Mailbox 20	*/
#define	MBIM21		0x0020	/* Enable Interrupt For Mailbox 21	*/
#define	MBIM22		0x0040	/* Enable Interrupt For Mailbox 22	*/
#define	MBIM23		0x0080	/* Enable Interrupt For Mailbox 23	*/
#define	MBIM24		0x0100	/* Enable Interrupt For Mailbox 24	*/
#define	MBIM25		0x0200	/* Enable Interrupt For Mailbox 25	*/
#define	MBIM26		0x0400	/* Enable Interrupt For Mailbox 26	*/
#define	MBIM27		0x0800	/* Enable Interrupt For Mailbox 27	*/
#define	MBIM28		0x1000	/* Enable Interrupt For Mailbox 28	*/
#define	MBIM29		0x2000	/* Enable Interrupt For Mailbox 29	*/
#define	MBIM30		0x4000	/* Enable Interrupt For Mailbox 30	*/
#define	MBIM31		0x8000	/* Enable Interrupt For Mailbox 31	*/

/* CAN_GIM Masks																*/
#define	EWTIM		0x0001	/* Enable TX Error Count Interrupt					*/
#define	EWRIM		0x0002	/* Enable RX Error Count Interrupt					*/
#define	EPIM		0x0004	/* Enable Error-Passive Mode Interrupt				*/
#define	BOIM		0x0008	/* Enable Bus Off Interrupt							*/
#define	WUIM		0x0010	/* Enable Wake-Up Interrupt							*/
#define	UIAIM		0x0020	/* Enable Access To Unimplemented Address Interrupt	*/
#define	AAIM		0x0040	/* Enable Abort Acknowledge Interrupt				*/
#define	RMLIM		0x0080	/* Enable RX Message Lost Interrupt					*/
#define	UCEIM		0x0100	/* Enable Universal Counter Overflow Interrupt		*/
#define	EXTIM		0x0200	/* Enable External Trigger Output Interrupt			*/
#define	ADIM		0x0400	/* Enable Access Denied Interrupt					*/

/* CAN_GIS Masks															*/
#define	EWTIS		0x0001	/* TX Error Count IRQ Status					*/
#define	EWRIS		0x0002	/* RX Error Count IRQ Status					*/
#define	EPIS		0x0004	/* Error-Passive Mode IRQ Status				*/
#define	BOIS		0x0008	/* Bus Off IRQ Status							*/
#define	WUIS		0x0010	/* Wake-Up IRQ Status							*/
#define	UIAIS		0x0020	/* Access To Unimplemented Address IRQ Status	*/
#define	AAIS		0x0040	/* Abort Acknowledge IRQ Status					*/
#define	RMLIS		0x0080	/* RX Message Lost IRQ Status					*/
#define	UCEIS		0x0100	/* Universal Counter Overflow IRQ Status		*/
#define	EXTIS		0x0200	/* External Trigger Output IRQ Status			*/
#define	ADIS		0x0400	/* Access Denied IRQ Status						*/

/* CAN_GIF Masks															*/
#define	EWTIF		0x0001	/* TX Error Count IRQ Flag						*/
#define	EWRIF		0x0002	/* RX Error Count IRQ Flag						*/
#define	EPIF		0x0004	/* Error-Passive Mode IRQ Flag					*/
#define	BOIF		0x0008	/* Bus Off IRQ Flag								*/
#define	WUIF		0x0010	/* Wake-Up IRQ Flag								*/
#define	UIAIF		0x0020	/* Access To Unimplemented Address IRQ Flag		*/
#define	AAIF		0x0040	/* Abort Acknowledge IRQ Flag					*/
#define	RMLIF		0x0080	/* RX Message Lost IRQ Flag						*/
#define	UCEIF		0x0100	/* Universal Counter Overflow IRQ Flag			*/
#define	EXTIF		0x0200	/* External Trigger Output IRQ Flag				*/
#define	ADIF		0x0400	/* Access Denied IRQ Flag						*/

/* CAN_UCCNF Masks															*/
#define	UCCNF		0x000F	/* Universal Counter Mode						*/
#define UC_STAMP	0x0001	/* 		Timestamp Mode							*/
#define UC_WDOG		0x0002	/* 		Watchdog Mode							*/
#define UC_AUTOTX	0x0003	/* 		Auto-Transmit Mode						*/
#define UC_ERROR	0x0006	/* 		CAN Error Frame Count					*/
#define UC_OVER		0x0007	/* 		CAN Overload Frame Count				*/
#define UC_LOST		0x0008	/* 		Arbitration Lost During TX Count		*/
#define UC_AA		0x0009	/* 		TX Abort Count							*/
#define UC_TA		0x000A	/* 		TX Successful Count						*/
#define UC_REJECT	0x000B	/* 		RX Message Rejected Count				*/
#define UC_RML		0x000C	/* 		RX Message Lost Count					*/
#define UC_RX		0x000D	/* 		Total Successful RX Messages Count		*/
#define UC_RMP		0x000E	/* 		Successful RX W/Matching ID Count		*/
#define UC_ALL		0x000F	/* 		Correct Message On CAN Bus Line Count	*/
#define	UCRC		0x0020	/* Universal Counter Reload/Clear				*/
#define	UCCT		0x0040	/* Universal Counter CAN Trigger				*/
#define	UCE			0x0080	/* Universal Counter Enable						*/

/* CAN_ESR Masks										*/
#define	ACKE		0x0004	/* Acknowledge Error		*/
#define	SER			0x0008	/* Stuff Error				*/
#define	CRCE		0x0010	/* CRC Error				*/
#define	SA0			0x0020	/* Stuck At Dominant Error	*/
#define	BEF			0x0040	/* Bit Error Flag			*/
#define	FER			0x0080	/* Form Error Flag			*/

/* CAN_EWR Masks												*/
#define	EWLREC		0x00FF	/* RX Error Count Limit (For EWRIS)	*/
#define	EWLTEC		0xFF00	/* TX Error Count Limit (For EWTIS)	*/

/* Alternate Deprecated Macros Provided For Backwards Code Compatibility */
#define CAN_CNF          CAN_DEBUG
#define TWI0_PRESCALE    TWI0_CONTROL
#define TWI0_INT_SRC     TWI0_INT_STAT
#define TWI0_INT_ENABLE  TWI0_INT_MASK
#define TWI1_PRESCALE     TWI1_CONTROL
#define TWI1_INT_SRC      TWI1_INT_STAT
#define TWI1_INT_ENABLE   TWI1_INT_MASK
#define TOVL_ERR0        TOVF_ERR0
#define TOVL_ERR1        TOVF_ERR1
#define TOVL_ERR2        TOVF_ERR2

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* _DEF_BF538_H */
