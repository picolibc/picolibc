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
** defBF59x_base.h
**
** Copyright (C) 2009-2010 Analog Devices Inc., All Rights Reserved.
**
************************************************************************************
**
** This include file contains a list of macro "defines" to enable the programmer
** to use symbolic names for the registers common to the ADSP-BF59x peripherals.
**
************************************************************************************
** System MMR Register Map
************************************************************************************/

#ifndef _DEF_BF59x_H
#define _DEF_BF59x_H

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_19_4)
#pragma diag(suppress:misra_rule_19_7)
#include <stdint.h>
#endif /* _MISRA_RULES */


/* ************************************************************************************************************** */
/*                           SYSTEM & MMR ADDRESS DEFINITIONS COMMON TO ALL ADSP-BF59x                            */
/* ************************************************************************************************************** */

/* Clock and System Control		(0xFFC00000 - 0xFFC000FF)									*/
#define PLL_CTL				0xFFC00000	/* PLL Control Register								*/
#define PLL_DIV				0xFFC00004	/* PLL Divide Register								*/
#define VR_CTL				0xFFC00008	/* Voltage Regulator Control Register					*/
#define PLL_STAT				0xFFC0000C	/* PLL Status Register								*/
#define PLL_LOCKCNT			0xFFC00010	/* PLL Lock Count Register							*/
#define CHIPID        			0xFFC00014  /* Device ID Register */
#define AUX_REVID      			0xFFC00108  /* Auxiliary Revision ID Register */

/* System Interrupt Controller 	(0xFFC00100 - 0xFFC001FF)									*/
#define SWRST				0xFFC00100	/* Software Reset Register							*/
#define SYSCR				0xFFC00104	/* System Configuration Register						*/

#define SIC_IMASK0			0xFFC0010C	/* Interrupt Mask Register							*/
/* legacy register name (below) provided for backwards code compatibility */
#define SIC_IMASK				SIC_IMASK0
#define SIC_IAR0				0xFFC00110	/* Interrupt Assignment Register 0						*/
#define SIC_IAR1				0xFFC00114	/* Interrupt Assignment Register 1						*/
#define SIC_IAR2				0xFFC00118	/* Interrupt Assignment Register 2						*/
#define SIC_IAR3				0xFFC0011C	/* Interrupt Assignment Register 3						*/
#define SIC_ISR0				0xFFC00120	/* Interrupt Status Register							*/
/* legacy register name (below) provided for backwards code compatibility */
#define SIC_ISR				SIC_ISR0
#define SIC_IWR0				0xFFC00124	/* Interrupt Wakeup Register							*/
/* legacy register name (below) provided for backwards code compatibility */
#define SIC_IWR				SIC_IWR0

/* Watchdog Timer				(0xFFC00200 - 0xFFC002FF) 									*/
#define WDOG_CTL				0xFFC00200	/* Watchdog Control Register							*/
#define WDOG_CNT				0xFFC00204	/* Watchdog Count Register							*/
#define WDOG_STAT				0xFFC00208	/* Watchdog Status Register							*/

/* UART0 Controller 			(0xFFC00400 - 0xFFC004FF) 									*/
#define UART0_THR				0xFFC00400	/* Transmit Holding register							*/
#define UART0_RBR				0xFFC00400	/* Receive Buffer register							*/
#define UART0_DLL				0xFFC00400	/* Divisor Latch (Low-Byte)							*/
#define UART0_IER				0xFFC00404	/* Interrupt Enable Register							*/
#define UART0_DLH				0xFFC00404	/* Divisor Latch (High-Byte)							*/
#define UART0_IIR				0xFFC00408	/* Interrupt Identification Register					*/
#define UART0_LCR				0xFFC0040C	/* Line Control Register							*/
#define UART0_MCR				0xFFC00410	/* Modem Control Register							*/
#define UART0_LSR				0xFFC00414	/* Line Status Register								*/
#define UART0_SCR				0xFFC0041C	/* SCR Scratch Register								*/
#define UART0_GCTL				0xFFC00424	/* Global Control Register							*/


/* SPI0 Controller				(0xFFC00500 - 0xFFC005FF) 									*/
#define SPI0_CTL				0xFFC00500	/* SPI0 Control Register								*/
#define SPI0_FLG				0xFFC00504	/* SPI0 Flag register								*/
#define SPI0_STAT				0xFFC00508	/* SPI0 Status register								*/
#define SPI0_TDBR				0xFFC0050C	/* SPI0 Transmit Data Buffer Register					*/
#define SPI0_RDBR				0xFFC00510	/* SPI0 Receive Data Buffer Register						*/
#define SPI0_BAUD				0xFFC00514	/* SPI0 Baud rate Register							*/
#define SPI0_SHADOW				0xFFC00518	/* SPI0_RDBR Shadow Register							*/

/* SPI1 Controller 				(0xFFC01300 - 0xFFC013FF)									*/
#define SPI1_CTL				0xFFC01300	/* SPI1 Control Register							*/
#define SPI1_FLG				0xFFC01304	/* SPI1 Flag register								*/
#define SPI1_STAT				0xFFC01308	/* SPI1 Status register								*/
#define SPI1_TDBR				0xFFC0130C	/* SPI1 Transmit Data Buffer Register					*/
#define SPI1_RDBR				0xFFC01310	/* SPI1 Receive Data Buffer Register					*/
#define SPI1_BAUD				0xFFC01314	/* SPI1 Baud rate Register							*/
#define SPI1_SHADOW				0xFFC01318	/* SPI1_RDBR Shadow Register							*/


/* TIMER0-2 Registers 			(0xFFC00600 - 0xFFC006FF)									*/
#define TIMER0_CONFIG			0xFFC00600	/* Timer 0 Configuration Register						*/
#define TIMER0_COUNTER			0xFFC00604	/* Timer 0 Counter Register							*/
#define TIMER0_PERIOD			0xFFC00608	/* Timer 0 Period Register							*/
#define TIMER0_WIDTH			0xFFC0060C	/* Timer 0 Width Register							*/

#define TIMER1_CONFIG			0xFFC00610	/* Timer 1 Configuration Register  						*/
#define TIMER1_COUNTER			0xFFC00614	/* Timer 1 Counter Register        						*/
#define TIMER1_PERIOD			0xFFC00618	/* Timer 1 Period Register         						*/
#define TIMER1_WIDTH			0xFFC0061C	/* Timer 1 Width Register          						*/

#define TIMER2_CONFIG			0xFFC00620	/* Timer 2 Configuration Register  						*/
#define TIMER2_COUNTER			0xFFC00624	/* Timer 2 Counter Register        						*/
#define TIMER2_PERIOD			0xFFC00628	/* Timer 2 Period Register         						*/
#define TIMER2_WIDTH			0xFFC0062C	/* Timer 2 Width Register          						*/

#define TIMER_ENABLE			0xFFC00640	/* Timer Enable Register							*/
#define TIMER_DISABLE			0xFFC00644	/* Timer Disable Register							*/
#define TIMER_STATUS			0xFFC00648	/* Timer Status Register							*/


/* General Purpose I/O Port F 	(0xFFC00700 - 0xFFC007FF) 									*/
#define PORTFIO				0xFFC00700	/* Port F I/O Pin State Specify Register					*/
#define PORTFIO_CLEAR			0xFFC00704	/* Port F I/O Peripheral Interrupt Clear Register			*/
#define PORTFIO_SET			0xFFC00708	/* Port F I/O Peripheral Interrupt Set Register				*/
#define PORTFIO_TOGGLE			0xFFC0070C	/* Port F I/O Pin State Toggle Register					*/
#define PORTFIO_MASKA			0xFFC00710	/* Port F I/O Mask State Specify Interrupt A Register			*/
#define PORTFIO_MASKA_CLEAR		0xFFC00714	/* Port F I/O Mask Disable Interrupt A Register				*/
#define PORTFIO_MASKA_SET		0xFFC00718	/* Port F I/O Mask Enable Interrupt A Register				*/
#define PORTFIO_MASKA_TOGGLE		0xFFC0071C	/* Port F I/O Mask Toggle Enable Interrupt A Register			*/
#define PORTFIO_MASKB			0xFFC00720	/* Port F I/O Mask State Specify Interrupt B Register			*/
#define PORTFIO_MASKB_CLEAR		0xFFC00724	/* Port F I/O Mask Disable Interrupt B Register				*/
#define PORTFIO_MASKB_SET		0xFFC00728	/* Port F I/O Mask Enable Interrupt B Register				*/
#define PORTFIO_MASKB_TOGGLE		0xFFC0072C	/* Port F I/O Mask Toggle Enable Interrupt B Register			*/
#define PORTFIO_DIR			0xFFC00730	/* Port F I/O Direction Register						*/
#define PORTFIO_POLAR			0xFFC00734	/* Port F I/O Source Polarity Register					*/
#define PORTFIO_EDGE			0xFFC00738	/* Port F I/O Source Sensitivity Register					*/
#define PORTFIO_BOTH			0xFFC0073C	/* Port F I/O Set on BOTH Edges Register					*/
#define PORTFIO_INEN			0xFFC00740	/* Port F I/O Input Enable Register 					*/

/* General Purpose I/O Port G 	(0xFFC01500 - 0xFFC015FF) 									*/
#define PORTGIO				0xFFC01500	/* Port G I/O Pin State Specify Register					*/
#define PORTGIO_CLEAR			0xFFC01504	/* Port G I/O Peripheral Interrupt Clear Register			*/
#define PORTGIO_SET			0xFFC01508	/* Port G I/O Peripheral Interrupt Set Register				*/
#define PORTGIO_TOGGLE			0xFFC0150C	/* Port G I/O Pin State Toggle Register					*/
#define PORTGIO_MASKA			0xFFC01510	/* Port G I/O Mask State Specify Interrupt A Register			*/
#define PORTGIO_MASKA_CLEAR		0xFFC01514	/* Port G I/O Mask Disable Interrupt A Register				*/
#define PORTGIO_MASKA_SET		0xFFC01518	/* Port G I/O Mask Enable Interrupt A Register				*/
#define PORTGIO_MASKA_TOGGLE		0xFFC0151C	/* Port G I/O Mask Toggle Enable Interrupt A Register			*/
#define PORTGIO_MASKB			0xFFC01520	/* Port G I/O Mask State Specify Interrupt B Register			*/
#define PORTGIO_MASKB_CLEAR		0xFFC01524	/* Port G I/O Mask Disable Interrupt B Register				*/
#define PORTGIO_MASKB_SET		0xFFC01528	/* Port G I/O Mask Enable Interrupt B Register				*/
#define PORTGIO_MASKB_TOGGLE		0xFFC0152C	/* Port G I/O Mask Toggle Enable Interrupt B Register			*/
#define PORTGIO_DIR			0xFFC01530	/* Port G I/O Direction Register						*/
#define PORTGIO_POLAR			0xFFC01534	/* Port G I/O Source Polarity Register					*/
#define PORTGIO_EDGE			0xFFC01538	/* Port G I/O Source Sensitivity Register					*/
#define PORTGIO_BOTH			0xFFC0153C	/* Port G I/O Set on BOTH Edges Register					*/
#define PORTGIO_INEN			0xFFC01540	/* Port G I/O Input Enable Register						*/


/* Pin Control Registers		(0xFFC01100 - 0xFFC01208)													*/
#define PORTF_FER				0xFFC01100	/* Port F Function Enable Register (Alternate/Flag*)			*/
#define PORTF_MUX               0xFFC01104  /* Port F mux control 											*/
#define PORTF_PADCTL        	0xFFC01108  /* Port F pad control 								*/
#define PORTG_FER				0xFFC01200	/* Port G Function Enable Register (Alternate/Flag*)			*/
#define PORTG_MUX               0xFFC01204  /* Port G mux control											*/
#define PORTG_PADCTL        	0xFFC01208  /* Port G pad control 								*/

/* SPORT Clock Gating			(0xFFC0120C)												*/
#define SPORT_GATECLK			0xFFC0120C	/* SPORT Clock Gating Control Register				*/


/* SPORT0 Controller 			(0xFFC00800 - 0xFFC008FF) 									*/
#define SPORT0_TCR1			0xFFC00800	/* SPORT0 Transmit Configuration 1 Register				*/
#define SPORT0_TCR2			0xFFC00804	/* SPORT0 Transmit Configuration 2 Register				*/
#define SPORT0_TCLKDIV			0xFFC00808	/* SPORT0 Transmit Clock Divider						*/
#define SPORT0_TFSDIV			0xFFC0080C	/* SPORT0 Transmit Frame Sync Divider					*/
#define SPORT0_TX				0xFFC00810	/* SPORT0 TX Data Register							*/
#define SPORT0_RX				0xFFC00818	/* SPORT0 RX Data Register							*/
#define SPORT0_RCR1			0xFFC00820	/* SPORT0 Transmit Configuration 1 Register				*/
#define SPORT0_RCR2			0xFFC00824	/* SPORT0 Transmit Configuration 2 Register				*/
#define SPORT0_RCLKDIV			0xFFC00828	/* SPORT0 Receive Clock Divider						*/
#define SPORT0_RFSDIV			0xFFC0082C	/* SPORT0 Receive Frame Sync Divider					*/
#define SPORT0_STAT			0xFFC00830	/* SPORT0 Status Register							*/
#define SPORT0_CHNL			0xFFC00834	/* SPORT0 Current Channel Register						*/
#define SPORT0_MCMC1			0xFFC00838	/* SPORT0 Multi-Channel Configuration Register 1			*/
#define SPORT0_MCMC2			0xFFC0083C	/* SPORT0 Multi-Channel Configuration Register 2			*/
#define SPORT0_MTCS0			0xFFC00840	/* SPORT0 Multi-Channel Transmit Select Register 0			*/
#define SPORT0_MTCS1			0xFFC00844	/* SPORT0 Multi-Channel Transmit Select Register 1			*/
#define SPORT0_MTCS2			0xFFC00848	/* SPORT0 Multi-Channel Transmit Select Register 2			*/
#define SPORT0_MTCS3			0xFFC0084C	/* SPORT0 Multi-Channel Transmit Select Register 3			*/
#define SPORT0_MRCS0			0xFFC00850	/* SPORT0 Multi-Channel Receive Select Register 0			*/
#define SPORT0_MRCS1			0xFFC00854	/* SPORT0 Multi-Channel Receive Select Register 1			*/
#define SPORT0_MRCS2			0xFFC00858	/* SPORT0 Multi-Channel Receive Select Register 2			*/
#define SPORT0_MRCS3			0xFFC0085C	/* SPORT0 Multi-Channel Receive Select Register 3			*/


/* SPORT1 Controller 			(0xFFC00900 - 0xFFC009FF) 									*/
#define SPORT1_TCR1			0xFFC00900	/* SPORT1 Transmit Configuration 1 Register				*/
#define SPORT1_TCR2			0xFFC00904	/* SPORT1 Transmit Configuration 2 Register				*/
#define SPORT1_TCLKDIV			0xFFC00908	/* SPORT1 Transmit Clock Divider						*/
#define SPORT1_TFSDIV			0xFFC0090C	/* SPORT1 Transmit Frame Sync Divider					*/
#define SPORT1_TX				0xFFC00910	/* SPORT1 TX Data Register							*/
#define SPORT1_RX				0xFFC00918	/* SPORT1 RX Data Register							*/
#define SPORT1_RCR1			0xFFC00920	/* SPORT1 Transmit Configuration 1 Register				*/
#define SPORT1_RCR2			0xFFC00924	/* SPORT1 Transmit Configuration 2 Register				*/
#define SPORT1_RCLKDIV			0xFFC00928	/* SPORT1 Receive Clock Divider						*/
#define SPORT1_RFSDIV			0xFFC0092C	/* SPORT1 Receive Frame Sync Divider					*/
#define SPORT1_STAT			0xFFC00930	/* SPORT1 Status Register							*/
#define SPORT1_CHNL			0xFFC00934	/* SPORT1 Current Channel Register						*/
#define SPORT1_MCMC1			0xFFC00938	/* SPORT1 Multi-Channel Configuration Register 1			*/
#define SPORT1_MCMC2			0xFFC0093C	/* SPORT1 Multi-Channel Configuration Register 2			*/
#define SPORT1_MTCS0			0xFFC00940	/* SPORT1 Multi-Channel Transmit Select Register 0			*/
#define SPORT1_MTCS1			0xFFC00944	/* SPORT1 Multi-Channel Transmit Select Register 1			*/
#define SPORT1_MTCS2			0xFFC00948	/* SPORT1 Multi-Channel Transmit Select Register 2			*/
#define SPORT1_MTCS3			0xFFC0094C	/* SPORT1 Multi-Channel Transmit Select Register 3			*/
#define SPORT1_MRCS0			0xFFC00950	/* SPORT1 Multi-Channel Receive Select Register 0			*/
#define SPORT1_MRCS1			0xFFC00954	/* SPORT1 Multi-Channel Receive Select Register 1			*/
#define SPORT1_MRCS2			0xFFC00958	/* SPORT1 Multi-Channel Receive Select Register 2			*/
#define SPORT1_MRCS3			0xFFC0095C	/* SPORT1 Multi-Channel Receive Select Register 3			*/

/* DMA Traffic Control Registers 	(0xFFC00B00 - 0xFFC00BFF)									*/
#define DMA_TC_PER			0xFFC00B0C	/* Traffic Control Periods Register						*/
#define DMA_TC_CNT			0xFFC00B10	/* Traffic Control Current Counts Register				*/

/* Alternate deprecated register names (below) provided for backwards code compatibility 					*/
#define DMA_TCPER				0xFFC00B0C	/* Traffic Control Periods Register						*/
#define DMA_TCCNT				0xFFC00B10	/* Traffic Control Current Counts Register				*/

/* DMA Controller 			(0xFFC00C00 - 0xFFC00FFF) 									*/
#define DMA0_NEXT_DESC_PTR		0xFFC00C00	/* DMA Channel 0 Next Descriptor Pointer Register			*/
#define DMA0_START_ADDR			0xFFC00C04	/* DMA Channel 0 Start Address Register					*/
#define DMA0_CONFIG			0xFFC00C08	/* DMA Channel 0 Configuration Register					*/
#define DMA0_X_COUNT			0xFFC00C10	/* DMA Channel 0 X Count Register						*/
#define DMA0_X_MODIFY			0xFFC00C14	/* DMA Channel 0 X Modify Register						*/
#define DMA0_Y_COUNT			0xFFC00C18	/* DMA Channel 0 Y Count Register						*/
#define DMA0_Y_MODIFY			0xFFC00C1C	/* DMA Channel 0 Y Modify Register						*/
#define DMA0_CURR_DESC_PTR		0xFFC00C20	/* DMA Channel 0 Current Descriptor Pointer Register			*/
#define DMA0_CURR_ADDR			0xFFC00C24	/* DMA Channel 0 Current Address Register					*/
#define DMA0_IRQ_STATUS			0xFFC00C28	/* DMA Channel 0 Interrupt/Status Register				*/
#define DMA0_PERIPHERAL_MAP		0xFFC00C2C	/* DMA Channel 0 Peripheral Map Register					*/
#define DMA0_CURR_X_COUNT		0xFFC00C30	/* DMA Channel 0 Current X Count Register					*/
#define DMA0_CURR_Y_COUNT		0xFFC00C38	/* DMA Channel 0 Current Y Count Register					*/

#define DMA1_NEXT_DESC_PTR		0xFFC00C40	/* DMA Channel 1 Next Descriptor Pointer Register			*/
#define DMA1_START_ADDR			0xFFC00C44	/* DMA Channel 1 Start Address Register					*/
#define DMA1_CONFIG			0xFFC00C48	/* DMA Channel 1 Configuration Register					*/
#define DMA1_X_COUNT			0xFFC00C50	/* DMA Channel 1 X Count Register						*/
#define DMA1_X_MODIFY			0xFFC00C54	/* DMA Channel 1 X Modify Register						*/
#define DMA1_Y_COUNT			0xFFC00C58	/* DMA Channel 1 Y Count Register						*/
#define DMA1_Y_MODIFY			0xFFC00C5C	/* DMA Channel 1 Y Modify Register						*/
#define DMA1_CURR_DESC_PTR		0xFFC00C60	/* DMA Channel 1 Current Descriptor Pointer Register			*/
#define DMA1_CURR_ADDR			0xFFC00C64	/* DMA Channel 1 Current Address Register					*/
#define DMA1_IRQ_STATUS			0xFFC00C68	/* DMA Channel 1 Interrupt/Status Register				*/
#define DMA1_PERIPHERAL_MAP		0xFFC00C6C	/* DMA Channel 1 Peripheral Map Register					*/
#define DMA1_CURR_X_COUNT		0xFFC00C70	/* DMA Channel 1 Current X Count Register					*/
#define DMA1_CURR_Y_COUNT		0xFFC00C78	/* DMA Channel 1 Current Y Count Register					*/

#define DMA2_NEXT_DESC_PTR		0xFFC00C80	/* DMA Channel 2 Next Descriptor Pointer Register			*/
#define DMA2_START_ADDR			0xFFC00C84	/* DMA Channel 2 Start Address Register					*/
#define DMA2_CONFIG			0xFFC00C88	/* DMA Channel 2 Configuration Register					*/
#define DMA2_X_COUNT			0xFFC00C90	/* DMA Channel 2 X Count Register						*/
#define DMA2_X_MODIFY			0xFFC00C94	/* DMA Channel 2 X Modify Register						*/
#define DMA2_Y_COUNT			0xFFC00C98	/* DMA Channel 2 Y Count Register						*/
#define DMA2_Y_MODIFY			0xFFC00C9C	/* DMA Channel 2 Y Modify Register						*/
#define DMA2_CURR_DESC_PTR		0xFFC00CA0	/* DMA Channel 2 Current Descriptor Pointer Register			*/
#define DMA2_CURR_ADDR			0xFFC00CA4	/* DMA Channel 2 Current Address Register					*/
#define DMA2_IRQ_STATUS			0xFFC00CA8	/* DMA Channel 2 Interrupt/Status Register				*/
#define DMA2_PERIPHERAL_MAP		0xFFC00CAC	/* DMA Channel 2 Peripheral Map Register					*/
#define DMA2_CURR_X_COUNT		0xFFC00CB0	/* DMA Channel 2 Current X Count Register					*/
#define DMA2_CURR_Y_COUNT		0xFFC00CB8	/* DMA Channel 2 Current Y Count Register					*/

#define DMA3_NEXT_DESC_PTR		0xFFC00CC0	/* DMA Channel 3 Next Descriptor Pointer Register			*/
#define DMA3_START_ADDR			0xFFC00CC4	/* DMA Channel 3 Start Address Register					*/
#define DMA3_CONFIG			0xFFC00CC8	/* DMA Channel 3 Configuration Register					*/
#define DMA3_X_COUNT			0xFFC00CD0	/* DMA Channel 3 X Count Register						*/
#define DMA3_X_MODIFY			0xFFC00CD4	/* DMA Channel 3 X Modify Register						*/
#define DMA3_Y_COUNT			0xFFC00CD8	/* DMA Channel 3 Y Count Register						*/
#define DMA3_Y_MODIFY			0xFFC00CDC	/* DMA Channel 3 Y Modify Register						*/
#define DMA3_CURR_DESC_PTR		0xFFC00CE0	/* DMA Channel 3 Current Descriptor Pointer Register			*/
#define DMA3_CURR_ADDR			0xFFC00CE4	/* DMA Channel 3 Current Address Register					*/
#define DMA3_IRQ_STATUS			0xFFC00CE8	/* DMA Channel 3 Interrupt/Status Register				*/
#define DMA3_PERIPHERAL_MAP		0xFFC00CEC	/* DMA Channel 3 Peripheral Map Register					*/
#define DMA3_CURR_X_COUNT		0xFFC00CF0	/* DMA Channel 3 Current X Count Register					*/
#define DMA3_CURR_Y_COUNT		0xFFC00CF8	/* DMA Channel 3 Current Y Count Register					*/

#define DMA4_NEXT_DESC_PTR		0xFFC00D00	/* DMA Channel 4 Next Descriptor Pointer Register			*/
#define DMA4_START_ADDR			0xFFC00D04	/* DMA Channel 4 Start Address Register					*/
#define DMA4_CONFIG			0xFFC00D08	/* DMA Channel 4 Configuration Register					*/
#define DMA4_X_COUNT			0xFFC00D10	/* DMA Channel 4 X Count Register						*/
#define DMA4_X_MODIFY			0xFFC00D14	/* DMA Channel 4 X Modify Register						*/
#define DMA4_Y_COUNT			0xFFC00D18	/* DMA Channel 4 Y Count Register						*/
#define DMA4_Y_MODIFY			0xFFC00D1C	/* DMA Channel 4 Y Modify Register						*/
#define DMA4_CURR_DESC_PTR		0xFFC00D20	/* DMA Channel 4 Current Descriptor Pointer Register			*/
#define DMA4_CURR_ADDR			0xFFC00D24	/* DMA Channel 4 Current Address Register					*/
#define DMA4_IRQ_STATUS			0xFFC00D28	/* DMA Channel 4 Interrupt/Status Register				*/
#define DMA4_PERIPHERAL_MAP		0xFFC00D2C	/* DMA Channel 4 Peripheral Map Register					*/
#define DMA4_CURR_X_COUNT		0xFFC00D30	/* DMA Channel 4 Current X Count Register					*/
#define DMA4_CURR_Y_COUNT		0xFFC00D38	/* DMA Channel 4 Current Y Count Register					*/

#define DMA5_NEXT_DESC_PTR		0xFFC00D40	/* DMA Channel 5 Next Descriptor Pointer Register			*/
#define DMA5_START_ADDR			0xFFC00D44	/* DMA Channel 5 Start Address Register					*/
#define DMA5_CONFIG			0xFFC00D48	/* DMA Channel 5 Configuration Register					*/
#define DMA5_X_COUNT			0xFFC00D50	/* DMA Channel 5 X Count Register						*/
#define DMA5_X_MODIFY			0xFFC00D54	/* DMA Channel 5 X Modify Register						*/
#define DMA5_Y_COUNT			0xFFC00D58	/* DMA Channel 5 Y Count Register						*/
#define DMA5_Y_MODIFY			0xFFC00D5C	/* DMA Channel 5 Y Modify Register						*/
#define DMA5_CURR_DESC_PTR		0xFFC00D60	/* DMA Channel 5 Current Descriptor Pointer Register			*/
#define DMA5_CURR_ADDR			0xFFC00D64	/* DMA Channel 5 Current Address Register					*/
#define DMA5_IRQ_STATUS			0xFFC00D68	/* DMA Channel 5 Interrupt/Status Register				*/
#define DMA5_PERIPHERAL_MAP		0xFFC00D6C	/* DMA Channel 5 Peripheral Map Register					*/
#define DMA5_CURR_X_COUNT		0xFFC00D70	/* DMA Channel 5 Current X Count Register					*/
#define DMA5_CURR_Y_COUNT		0xFFC00D78	/* DMA Channel 5 Current Y Count Register					*/

#define DMA6_NEXT_DESC_PTR		0xFFC00D80	/* DMA Channel 6 Next Descriptor Pointer Register			*/
#define DMA6_START_ADDR			0xFFC00D84	/* DMA Channel 6 Start Address Register					*/
#define DMA6_CONFIG			0xFFC00D88	/* DMA Channel 6 Configuration Register					*/
#define DMA6_X_COUNT			0xFFC00D90	/* DMA Channel 6 X Count Register						*/
#define DMA6_X_MODIFY			0xFFC00D94	/* DMA Channel 6 X Modify Register						*/
#define DMA6_Y_COUNT			0xFFC00D98	/* DMA Channel 6 Y Count Register						*/
#define DMA6_Y_MODIFY			0xFFC00D9C	/* DMA Channel 6 Y Modify Register						*/
#define DMA6_CURR_DESC_PTR		0xFFC00DA0	/* DMA Channel 6 Current Descriptor Pointer Register			*/
#define DMA6_CURR_ADDR			0xFFC00DA4	/* DMA Channel 6 Current Address Register					*/
#define DMA6_IRQ_STATUS			0xFFC00DA8	/* DMA Channel 6 Interrupt/Status Register				*/
#define DMA6_PERIPHERAL_MAP		0xFFC00DAC	/* DMA Channel 6 Peripheral Map Register					*/
#define DMA6_CURR_X_COUNT		0xFFC00DB0	/* DMA Channel 6 Current X Count Register					*/
#define DMA6_CURR_Y_COUNT		0xFFC00DB8	/* DMA Channel 6 Current Y Count Register					*/

#define DMA7_NEXT_DESC_PTR		0xFFC00DC0	/* DMA Channel 7 Next Descriptor Pointer Register			*/
#define DMA7_START_ADDR			0xFFC00DC4	/* DMA Channel 7 Start Address Register					*/
#define DMA7_CONFIG			0xFFC00DC8	/* DMA Channel 7 Configuration Register					*/
#define DMA7_X_COUNT			0xFFC00DD0	/* DMA Channel 7 X Count Register						*/
#define DMA7_X_MODIFY			0xFFC00DD4	/* DMA Channel 7 X Modify Register						*/
#define DMA7_Y_COUNT			0xFFC00DD8	/* DMA Channel 7 Y Count Register						*/
#define DMA7_Y_MODIFY			0xFFC00DDC	/* DMA Channel 7 Y Modify Register						*/
#define DMA7_CURR_DESC_PTR		0xFFC00DE0	/* DMA Channel 7 Current Descriptor Pointer Register			*/
#define DMA7_CURR_ADDR			0xFFC00DE4	/* DMA Channel 7 Current Address Register					*/
#define DMA7_IRQ_STATUS			0xFFC00DE8	/* DMA Channel 7 Interrupt/Status Register				*/
#define DMA7_PERIPHERAL_MAP		0xFFC00DEC	/* DMA Channel 7 Peripheral Map Register					*/
#define DMA7_CURR_X_COUNT		0xFFC00DF0	/* DMA Channel 7 Current X Count Register					*/
#define DMA7_CURR_Y_COUNT		0xFFC00DF8	/* DMA Channel 7 Current Y Count Register					*/

#define DMA8_NEXT_DESC_PTR		0xFFC00E00	/* DMA Channel 8 Next Descriptor Pointer Register			*/
#define DMA8_START_ADDR			0xFFC00E04	/* DMA Channel 8 Start Address Register					*/
#define DMA8_CONFIG			0xFFC00E08	/* DMA Channel 8 Configuration Register					*/
#define DMA8_X_COUNT			0xFFC00E10	/* DMA Channel 8 X Count Register						*/
#define DMA8_X_MODIFY			0xFFC00E14	/* DMA Channel 8 X Modify Register						*/
#define DMA8_Y_COUNT			0xFFC00E18	/* DMA Channel 8 Y Count Register						*/
#define DMA8_Y_MODIFY			0xFFC00E1C	/* DMA Channel 8 Y Modify Register						*/
#define DMA8_CURR_DESC_PTR		0xFFC00E20	/* DMA Channel 8 Current Descriptor Pointer Register			*/
#define DMA8_CURR_ADDR			0xFFC00E24	/* DMA Channel 8 Current Address Register					*/
#define DMA8_IRQ_STATUS			0xFFC00E28	/* DMA Channel 8 Interrupt/Status Register				*/
#define DMA8_PERIPHERAL_MAP		0xFFC00E2C	/* DMA Channel 8 Peripheral Map Register					*/
#define DMA8_CURR_X_COUNT		0xFFC00E30	/* DMA Channel 8 Current X Count Register					*/
#define DMA8_CURR_Y_COUNT		0xFFC00E38	/* DMA Channel 8 Current Y Count Register					*/

#define MDMA_D0_NEXT_DESC_PTR		0xFFC00F00	/* MemDMA Stream 0 Destination Next Descriptor Pointer Register 	*/
#define MDMA_D0_START_ADDR		0xFFC00F04	/* MemDMA Stream 0 Destination Start Address Register			*/
#define MDMA_D0_CONFIG			0xFFC00F08	/* MemDMA Stream 0 Destination Configuration Register			*/
#define MDMA_D0_X_COUNT			0xFFC00F10	/* MemDMA Stream 0 Destination X Count Register				*/
#define MDMA_D0_X_MODIFY		0xFFC00F14	/* MemDMA Stream 0 Destination X Modify Register			*/
#define MDMA_D0_Y_COUNT			0xFFC00F18	/* MemDMA Stream 0 Destination Y Count Register				*/
#define MDMA_D0_Y_MODIFY		0xFFC00F1C	/* MemDMA Stream 0 Destination Y Modify Register			*/
#define MDMA_D0_CURR_DESC_PTR		0xFFC00F20	/* MemDMA Stream 0 Destination Current Descriptor Pointer Register*/
#define MDMA_D0_CURR_ADDR		0xFFC00F24	/* MemDMA Stream 0 Destination Current Address Register 		*/
#define MDMA_D0_IRQ_STATUS		0xFFC00F28	/* MemDMA Stream 0 Destination Interrupt/Status Register		*/
#define MDMA_D0_PERIPHERAL_MAP	0xFFC00F2C	/* MemDMA Stream 0 Destination Peripheral Map Register  		*/
#define MDMA_D0_CURR_X_COUNT		0xFFC00F30	/* MemDMA Stream 0 Destination Current X Count Register		*/
#define MDMA_D0_CURR_Y_COUNT		0xFFC00F38	/* MemDMA Stream 0 Destination Current Y Count Register		*/

#define MDMA_S0_NEXT_DESC_PTR		0xFFC00F40	/* MemDMA Stream 0 Source Next Descriptor Pointer Register		*/
#define MDMA_S0_START_ADDR		0xFFC00F44	/* MemDMA Stream 0 Source Start Address Register			*/
#define MDMA_S0_CONFIG			0xFFC00F48	/* MemDMA Stream 0 Source Configuration Register			*/
#define MDMA_S0_X_COUNT			0xFFC00F50	/* MemDMA Stream 0 Source X Count Register				*/
#define MDMA_S0_X_MODIFY		0xFFC00F54	/* MemDMA Stream 0 Source X Modify Register				*/
#define MDMA_S0_Y_COUNT			0xFFC00F58	/* MemDMA Stream 0 Source Y Count Register				*/
#define MDMA_S0_Y_MODIFY		0xFFC00F5C	/* MemDMA Stream 0 Source Y Modify Register				*/
#define MDMA_S0_CURR_DESC_PTR		0xFFC00F60	/* MemDMA Stream 0 Source Current Descriptor Pointer Register	*/
#define MDMA_S0_CURR_ADDR		0xFFC00F64	/* MemDMA Stream 0 Source Current Address Register			*/
#define MDMA_S0_IRQ_STATUS		0xFFC00F68	/* MemDMA Stream 0 Source Interrupt/Status Register			*/
#define MDMA_S0_PERIPHERAL_MAP	0xFFC00F6C	/* MemDMA Stream 0 Source Peripheral Map Register			*/
#define MDMA_S0_CURR_X_COUNT		0xFFC00F70	/* MemDMA Stream 0 Source Current X Count Register			*/
#define MDMA_S0_CURR_Y_COUNT		0xFFC00F78	/* MemDMA Stream 0 Source Current Y Count Register			*/

#define MDMA_D1_NEXT_DESC_PTR		0xFFC00F80	/* MemDMA Stream 1 Destination Next Descriptor Pointer Register	*/
#define MDMA_D1_START_ADDR		0xFFC00F84	/* MemDMA Stream 1 Destination Start Address Register			*/
#define MDMA_D1_CONFIG			0xFFC00F88	/* MemDMA Stream 1 Destination Configuration Register			*/
#define MDMA_D1_X_COUNT			0xFFC00F90	/* MemDMA Stream 1 Destination X Count Register				*/
#define MDMA_D1_X_MODIFY		0xFFC00F94	/* MemDMA Stream 1 Destination X Modify Register			*/
#define MDMA_D1_Y_COUNT			0xFFC00F98	/* MemDMA Stream 1 Destination Y Count Register				*/
#define MDMA_D1_Y_MODIFY		0xFFC00F9C	/* MemDMA Stream 1 Destination Y Modify Register			*/
#define MDMA_D1_CURR_DESC_PTR		0xFFC00FA0	/* MemDMA Stream 1 Destination Current Descriptor Pointer Register*/
#define MDMA_D1_CURR_ADDR		0xFFC00FA4	/* MemDMA Stream 1 Destination Current Address Register		*/
#define MDMA_D1_IRQ_STATUS		0xFFC00FA8	/* MemDMA Stream 1 Destination Interrupt/Status Register		*/
#define MDMA_D1_PERIPHERAL_MAP	0xFFC00FAC	/* MemDMA Stream 1 Destination Peripheral Map Register		*/
#define MDMA_D1_CURR_X_COUNT		0xFFC00FB0	/* MemDMA Stream 1 Destination Current X Count Register		*/
#define MDMA_D1_CURR_Y_COUNT		0xFFC00FB8	/* MemDMA Stream 1 Destination Current Y Count Register		*/

#define MDMA_S1_NEXT_DESC_PTR		0xFFC00FC0	/* MemDMA Stream 1 Source Next Descriptor Pointer Register		*/
#define MDMA_S1_START_ADDR		0xFFC00FC4	/* MemDMA Stream 1 Source Start Address Register			*/
#define MDMA_S1_CONFIG			0xFFC00FC8	/* MemDMA Stream 1 Source Configuration Register			*/
#define MDMA_S1_X_COUNT			0xFFC00FD0	/* MemDMA Stream 1 Source X Count Register				*/
#define MDMA_S1_X_MODIFY		0xFFC00FD4	/* MemDMA Stream 1 Source X Modify Register				*/
#define MDMA_S1_Y_COUNT			0xFFC00FD8	/* MemDMA Stream 1 Source Y Count Register				*/
#define MDMA_S1_Y_MODIFY		0xFFC00FDC	/* MemDMA Stream 1 Source Y Modify Register				*/
#define MDMA_S1_CURR_DESC_PTR		0xFFC00FE0	/* MemDMA Stream 1 Source Current Descriptor Pointer Register	*/
#define MDMA_S1_CURR_ADDR		0xFFC00FE4	/* MemDMA Stream 1 Source Current Address Register			*/
#define MDMA_S1_IRQ_STATUS		0xFFC00FE8	/* MemDMA Stream 1 Source Interrupt/Status Register			*/
#define MDMA_S1_PERIPHERAL_MAP	0xFFC00FEC	/* MemDMA Stream 1 Source Peripheral Map Register			*/
#define MDMA_S1_CURR_X_COUNT		0xFFC00FF0	/* MemDMA Stream 1 Source Current X Count Register			*/
#define MDMA_S1_CURR_Y_COUNT		0xFFC00FF8	/* MemDMA Stream 1 Source Current Y Count Register			*/


/* Parallel Peripheral Interface 	(0xFFC01000 - 0xFFC010FF) 									*/
#define PPI_CONTROL			0xFFC01000	/* PPI Control Register								*/
#define PPI_STATUS			0xFFC01004	/* PPI Status Register								*/
#define PPI_COUNT				0xFFC01008	/* PPI Transfer Count Register						*/
#define PPI_DELAY				0xFFC0100C	/* PPI Delay Count Register							*/
#define PPI_FRAME				0xFFC01010	/* PPI Frame Length Register							*/


/* Two-Wire Interface 			(0xFFC01400 - 0xFFC014FF)									*/
#define TWI_CLKDIV			0xFFC01400	/* Serial Clock Divider Register						*/
#define TWI_CONTROL			0xFFC01404	/* TWI Control Register								*/
#define TWI_SLAVE_CTL			0xFFC01408	/* Slave Mode Control Register						*/
#define TWI_SLAVE_STAT			0xFFC0140C	/* Slave Mode Status Register							*/
#define TWI_SLAVE_ADDR			0xFFC01410	/* Slave Mode Address Register						*/
#define TWI_MASTER_CTL			0xFFC01414	/* Master Mode Control Register						*/
#define TWI_MASTER_STAT			0xFFC01418	/* Master Mode Status Register						*/
#define TWI_MASTER_ADDR			0xFFC0141C	/* Master Mode Address Register						*/
#define TWI_INT_STAT			0xFFC01420	/* TWI Interrupt Status Register						*/
#define TWI_INT_MASK			0xFFC01424	/* TWI Master Interrupt Mask Register					*/
#define TWI_FIFO_CTL			0xFFC01428	/* FIFO Control Register							*/
#define TWI_FIFO_STAT			0xFFC0142C	/* FIFO Status Register								*/
#define TWI_XMT_DATA8			0xFFC01480	/* FIFO Transmit Data Single Byte Register				*/
#define TWI_XMT_DATA16			0xFFC01484	/* FIFO Transmit Data Double Byte Register				*/
#define TWI_RCV_DATA8			0xFFC01488	/* FIFO Receive Data Single Byte Register					*/
#define TWI_RCV_DATA16			0xFFC0148C	/* FIFO Receive Data Double Byte Register					*/


/******************************************************************************************************************
** System MMR Register Bits And Macros
**
** Disclaimer:	All macros are intended to make C and Assembly code more readable.
**			Use these macros carefully, as any that do left shifts for field
**			depositing will result in the lower order bits being destroyed.  Any
**			macro that shifts left to properly position the bit-field should be
**			used as part of an OR to initialize a register and NOT as a dynamic
**			modifier UNLESS the lower order bits are saved and ORed back in when
**			the macro is used.
*******************************************************************************************************************/

/************************************** PLL AND RESET MASKS *******************************************************/

/* PLL_CTL Masks */
#define DF					0x0001	/* 0: PLL = CLKIN, 1: PLL = CLKIN/2						*/
#define PLL_OFF				0x0002	/* PLL Not Powered								*/
#define STOPCK				0x0008	/* Core Clock Off									*/
#define PDWN				0x0020	/* Enter Deep Sleep Mode							*/
#define BYPASS				0x0100	/* Bypass the PLL									*/
#define MSEL				0x7E00	/* Multiplier Select For CCLK/VCO Factors					*/

/* PLL_CTL Macros (Only Use With Logic OR While Setting Lower Order Bits) */
#ifdef _MISRA_RULES
#define SET_MSEL(x)	(((x)&0x3Fu) << 0x9)	/* Set MSEL = 0-63 --> VCO = CLKIN*MSEL					*/
#else
#define SET_MSEL(x)	(((x)&0x3F) << 0x9)	/* Set MSEL = 0-63 --> VCO = CLKIN*MSEL					*/
#endif /* _MISRA_RULES */

/* PLL_DIV Masks */
#define SSEL				0x000F	/* System Select									*/
#define CSEL				0x0030	/* Core Select									*/
#define CSEL_DIV1				0x0000	/* CCLK = VCO / 1									*/
#define CSEL_DIV2				0x0010	/* CCLK = VCO / 2									*/
#define CSEL_DIV4				0x0020	/* CCLK = VCO / 4									*/
#define CSEL_DIV8				0x0030	/* CCLK = VCO / 8									*/

/* PLL_DIV Macros	*/
#ifdef _MISRA_RULES
#define SET_SSEL(x)			((x)&0xFu)	/* Set SSEL = 0-15 --> SCLK = VCO/SSEL					*/
#else
#define SET_SSEL(x)			((x)&0xF)	/* Set SSEL = 0-15 --> SCLK = VCO/SSEL					*/
#endif /* _MISRA_RULES */

/* VR_CTL Masks */
#define WAKE_EN0				0x0100	/* Enable Wakeup From Hibernate/Deep Sleep on the WAKEN0 signal		*/
#define WAKE_EN1				0x0200	/* Enable Wakeup From Hibernate/Deep Sleep on the WAKEN1 signal		*/
#define WAKE_EN2				0x0400	/* Enable Wakeup From Hibernate/Deep Sleep on the WAKEN2 signal		*/
#define WAKE_EN3				0x0800	/* Enable Wakeup From Hibernate/Deep Sleep on the WAKEN3 signal		*/
#define HIBERNATEB				0x1000	/* Bit mask for HIBERNATEB								*/
#define HIBERNATE				0x0000	/* Deasserts EXT_WAKE in order to enter hibernate mode			*/
#define EXTCLK_SEL				0x2000	/* Selects SCLK for the EXTCLK signal						*/
#define EXTCLK_OE				0x4000	/* Output enable for the EXTCLK signal						*/
#define WAKE_POLARITY			0x8000	/* Make wakeups active-high								*/

/* PLL_STAT Masks	*/
#define ACTIVE_PLLENABLED		0x0001	/* Processor In Active Mode With PLL Enabled				*/
#define FULL_ON					0x0002	/* Processor In Full On Mode							*/
#define ACTIVE_PLLDISABLED		0x0004	/* Processor In Active Mode With PLL Disabled				*/
#define PLL_LOCKED				0x0020	/* PLL_LOCKCNT Has Been Reached						*/

/* SWRST Masks */
#define SYSTEM_RESET			0x0007	/* Initiates A System Software Reset					*/
#define DOUBLE_FAULT			0x0008	/* Core Double Fault Causes Reset						*/
#define RESET_DOUBLE			0x2000	/* SW Reset Generated By Core Double-Fault				*/
#define RESET_WDOG				0x4000	/* SW Reset Generated By Watchdog Timer					*/
#define RESET_SOFTWARE			0x8000	/* SW Reset Occurred Since Last Read Of SWRST				*/

/* SYSCR Masks */
#define BMODE_IDLE        		0x0000	/* Bypass boot ROM, go to idle 		*/
#define BMODE_SPI1MEM           0x0002	/* Boot from serial SPI1 memory 						*/
#define BMODE_SPI1HOST          0x0003	/* Boot from SPI1 host (slave mode) 					*/
#define BMODE_SPI0MEM           0x0004	/* Boot from serial SPI0 memory 						*/
#define BMODE_PPI        		0x0005	/* Boot from PPI Port 							*/
#define BMODE_L1ROM         	0x0006	/* Boot from internal L1 ROM					*/
#define BMODE_UART0HOST        	0x0007	/* Boot from UART0 host 							*/
#define BMODE 					0x0007 	/* Boot Mode. Mirror of BMODE Mode Pins 					*/

#define BCODE 					0x00F0
#define BCODE_NORMAL 			0x0000 	/* normal boot, update PLL/VR, quickboot as by WURESET		*/
#define BCODE_NOBOOT 			0x0010 	/* bypass boot, don't update PLL/VR 					*/
#define BCODE_QUICKBOOT 		0x0020 	/* quick boot, overrule WURESET, don't update PLL/VR 			*/
#define BCODE_ALLBOOT			0x0040 	/* no quick boot, overrule WURESET, don't update PLL/VR 		*/
#define BCODE_FULLBOOT 			0x0060 	/* no quick boot, overrule WURESET, update PLL/VR 			*/

#define WURESET 				0x1000 	/* wakeup event since last hardware reset			 		*/
#define DFRESET 				0x2000 	/* recent reset was due to a double fault event 			*/
#define WDRESET 				0x4000 	/* recent reset was due to a watchdog event 				*/
#define SWRESET 				0x8000 	/* recent reset was issued by software 					*/

/*********************************  SYSTEM INTERRUPT CONTROLLER MASKS *********************************************/
/* Peripheral Masks For SIC_ISR0, SIC_IWR0, SIC_IMASK0 */
#define IRQ_PLL_WAKEUP			0x00000001	/* PLL Wakeup Interrupt			 					*/
#define IRQ_DMA_ERR0   			0x00000002  /* Error Interrupt (DMA error 0 interrupt (generic)) 			*/
#define IRQ_PPI_ERR    			0x00000004  /* Error Interrupt (PPI error interrupt) 					*/
#define IRQ_SPORT0_ERR 			0x00000008  /* Error Interrupt (SPORT0 status interrupt) 				*/
#define IRQ_SPORT1_ERR 			0x00000010  /* Error Interrupt (SPORT1 status interrupt) 				*/
#define IRQ_SPI0_ERR     		0x00000020  /* Error Interrupt (SPI0 status interrupt) 				*/
#define IRQ_SPI1_ERR			0x00000040	/* Error Interrupt (SPI1 status interrupt)				*/
#define IRQ_UART0_ERR 			0x00000080  /* Error Interrupt (UART0 status interrupt) 				*/
#define IRQ_DMA0	   			0x00000100  /* DMA channel 0 (PPI) Interrupt 					*/
#define IRQ_DMA1	   			0x00000200  /* DMA Channel 1 (SPORT0 RX) Interrupt				*/
#define IRQ_DMA2	   			0x00000400  /* DMA Channel 2 (SPORT0 TX) Interrupt 				*/
#define IRQ_DMA3	   			0x00000800  /* DMA Channel 3 (SPORT1 RX) Interrupt 					*/
#define IRQ_DMA4	   			0x00001000  /* DMA Channel 4 (SPORT1 TX) Interrupt 					*/
#define IRQ_DMA5	   			0x00002000  /* DMA Channel 5 (SPI0) Interrupt 					*/
#define IRQ_DMA6	   			0x00004000  /* DMA Channel 6 (SPI1) Interrupt 		 			*/
#define IRQ_DMA7	   			0x00008000  /* DMA Channel 7 (UART0 RX) Interrupt 						*/
#define IRQ_DMA8	   			0x00010000  /* DMA Channel 8 (UART0 TX) Interrupt 					*/
#define IRQ_PFA_PORTF			0x00020000	/* PF Port F Interrupt A		 					*/
#define IRQ_PFB_PORTF			0x00040000	/* PF Port F Interrupt B 							*/
#define IRQ_TIMER0				0x00080000	/* Timer 0 Interrupt								*/
#define IRQ_TIMER1				0x00100000	/* Timer 1 Interrupt 								*/
#define IRQ_TIMER2				0x00200000	/* Timer 2 Interrupt 								*/
#define IRQ_PFA_PORTG			0x00400000	/* PF Port G Interrupt A 							*/
#define IRQ_PFB_PORTG			0x00800000	/* PF Port G Interrupt B		 					*/
#define IRQ_TWI			   		0x01000000  /* TWI Interrupt									*/
#define IRQ_DMA12				0x20000000	/* DMA Channels 12 (MDMA0 Destination) TX Interrupt			*/
#define IRQ_DMA13				0x20000000	/* DMA Channels 13 (MDMA0 Source) RX Interrupt				*/
#define IRQ_DMA14				0x40000000	/* DMA Channels 14 (MDMA1 Destination) TX Interrupt			*/
#define IRQ_DMA15				0x40000000	/* DMA Channels 15 (MDMA1 Source) RX Interrupt 				*/
#define IRQ_WDOG				0x80000000	/* Software Watchdog Timer Interrupt 					*/

/* Peripheral Masks For SIC_ISR, SIC_IWR, SIC_IMASK */
#define IWR_DISABLE_ALL        	0x00000000	/* Wakeup Disable all peripherals 						*/
#define IWR_ENABLE_ALL          	0xFFFFFFFF	/* Wakeup Enable all peripherals 						*/
/* x = pos 0 to 31, for 32-63 use value-32 */
#define IWR_ENABLE(x)           	(1 << (x))	/* Wakeup Enable Peripheral #x 						*/
#define IWR_DISABLE(x)	 (0xFFFFFFFF^(1<<(x)))	/* Wakeup Disable Peripheral #x 						*/


#ifdef _MISRA_RULES
#define _MF15 	0xFu
#define _MF7 	7u
#else
#define _MF15 	0xF
#define _MF7 	7
#endif /* _MISRA_RULES */


/* SIC_IAR0 Macros*/
#define P0_IVG(x)		(((x)&_MF15)-_MF7)			/* Peripheral #0 assigned IVG #x 	*/
#define P1_IVG(x)		(((x)&_MF15)-_MF7) << 0x4	/* Peripheral #1 assigned IVG #x 	*/
#define P2_IVG(x)		(((x)&_MF15)-_MF7) << 0x8	/* Peripheral #2 assigned IVG #x 	*/
#define P3_IVG(x)		(((x)&_MF15)-_MF7) << 0xC	/* Peripheral #3 assigned IVG #x	*/
#define P4_IVG(x)		(((x)&_MF15)-_MF7) << 0x10	/* Peripheral #4 assigned IVG #x	*/
#define P5_IVG(x)		(((x)&_MF15)-_MF7) << 0x14	/* Peripheral #5 assigned IVG #x	*/
#define P6_IVG(x)		(((x)&_MF15)-_MF7) << 0x18	/* Peripheral #6 assigned IVG #x	*/
#define P7_IVG(x)		(((x)&_MF15)-_MF7) << 0x1C	/* Peripheral #7 assigned IVG #x	*/

/* SIC_IAR1 Macros*/
#define P8_IVG(x)		(((x)&_MF15)-_MF7)			/* Peripheral #8 assigned IVG #x 	*/
#define P9_IVG(x)		(((x)&_MF15)-_MF7) << 0x4	/* Peripheral #9 assigned IVG #x 	*/
#define P10_IVG(x)		(((x)&_MF15)-_MF7) << 0x8	/* Peripheral #10 assigned IVG #x 	*/
#define P11_IVG(x)		(((x)&_MF15)-_MF7) << 0xC	/* Peripheral #11 assigned IVG #x	*/
#define P12_IVG(x)		(((x)&_MF15)-_MF7) << 0x10	/* Peripheral #12 assigned IVG #x	*/
#define P13_IVG(x)		(((x)&_MF15)-_MF7) << 0x14	/* Peripheral #13 assigned IVG #x	*/
#define P14_IVG(x)		(((x)&_MF15)-_MF7) << 0x18	/* Peripheral #14 assigned IVG #x	*/
#define P15_IVG(x)		(((x)&_MF15)-_MF7) << 0x1C	/* Peripheral #15 assigned IVG #x	*/

/* SIC_IAR2 Macros*/
#define P16_IVG(x)		(((x)&_MF15)-_MF7)			/* Peripheral #16 assigned IVG #x 	*/
#define P17_IVG(x)		(((x)&_MF15)-_MF7) << 0x4	/* Peripheral #17 assigned IVG #x 	*/
#define P18_IVG(x)		(((x)&_MF15)-_MF7) << 0x8	/* Peripheral #18 assigned IVG #x 	*/
#define P19_IVG(x)		(((x)&_MF15)-_MF7) << 0xC	/* Peripheral #19 assigned IVG #x	*/
#define P20_IVG(x)		(((x)&_MF15)-_MF7) << 0x10	/* Peripheral #20 assigned IVG #x	*/
#define P21_IVG(x)		(((x)&_MF15)-_MF7) << 0x14	/* Peripheral #21 assigned IVG #x	*/
#define P22_IVG(x)		(((x)&_MF15)-_MF7) << 0x18	/* Peripheral #22 assigned IVG #x	*/
#define P23_IVG(x)		(((x)&_MF15)-_MF7) << 0x1C	/* Peripheral #23 assigned IVG #x	*/

/* SIC_IAR3 Macros*/
#define P24_IVG(x)		(((x)&_MF15)-_MF7)			/* Peripheral #24 assigned IVG #x 	*/
#define P29_IVG(x)		(((x)&_MF15)-_MF7) << 0x14	/* Peripheral #29 assigned IVG #x	*/
#define P30_IVG(x)		(((x)&_MF15)-_MF7) << 0x18	/* Peripheral #30 assigned IVG #x	*/
#define P31_IVG(x)		(((x)&_MF15)-_MF7) << 0x1C	/* Peripheral #31 assigned IVG #x	*/


/* SIC_IMASK0 Masks*/
#define SIC_UNMASK0_ALL 0x00000000        /* Unmask all peripheral interrupts */
#define SIC_MASK0_ALL   0xE1FFFFFF        /* Mask all peripheral interrupts   */

/* SIC_IWR0 Masks*/
#define IWR0_DISABLE_ALL 0x00000000       /* Wakeup Disable all peripherals   */
#define IWR0_ENABLE_ALL  0xE1FFFFFF       /* Wakeup Enable all peripherals    */


/* ************************************** WATCHDOG TIMER MASKS ****************************************************/

/* Watchdog Timer WDOG_CTL Register Masks */
#ifdef _MISRA_RULES
#define WDEV(x)	     (((x)<<1) & 0x0006u)	/* event generated on roll over 						*/
#else
#define WDEV(x) 		(((x)<<1) & 0x0006)	/* event generated on roll over 						*/
#endif /* _MISRA_RULES */

#define WDEV_RESET			0x0000	/* generate reset event on roll over					*/
#define WDEV_NMI 				0x0002 	/* generate NMI event on roll over 						*/
#define WDEV_GPI 				0x0004 	/* generate GP IRQ on roll over 						*/
#define WDEV_NONE 			0x0006	/* no event on roll over 							*/
#define WDEN 				0x0FF0 	/* enable watchdog 								*/
#define WDDIS 				0x0AD0 	/* disable watchdog 								*/
#define WDRO 				0x8000 	/* watchdog rolled over latch 						*/

/* depreciated WDOG_CTL Register Masks for legacy code */
#define ICTL 				WDEV
#define ENABLE_RESET 			WDEV_RESET
#define WDOG_RESET 			WDEV_RESET
#define ENABLE_NMI 			WDEV_NMI
#define WDOG_NMI 				WDEV_NMI
#define ENABLE_GPI 			WDEV_GPI
#define WDOG_GPI 				WDEV_GPI
#define DISABLE_EVT 			WDEV_NONE
#define WDOG_NONE 			WDEV_NONE

#define TMR_EN 				WDEN
#define TMR_DIS 				WDDIS
#define TRO 				WDRO
#define ICTL_P0 				0x01
#define ICTL_P1 				0x02
#define TRO_P 				0x0F


/* ************************************ UART CONTROLLER MASKS *****************************************************/

/* UARTx_LCR Masks*/
#ifdef _MISRA_RULES
#define WLS(x)		(((x)-5u) & 0x03u)	/* Word Length Select 								*/
#else
#define WLS(x)		  (((x)-5) & 0x03)	/* Word Length Select 								*/
#endif /* _MISRA_RULES */

#define STB					0x04		/* Stop Bits									*/
#define PEN					0x08		/* Parity Enable									*/
#define EPS					0x10		/* Even Parity Select								*/
#define STP					0x20		/* Stick Parity									*/
#define SB					0x40		/* Set Break									*/
#define DLAB				0x80		/* Divisor Latch Access								*/

/* UARTx_MCR Mask	*/
#define LOOP_ENA				0x10		/* Loopback Mode Enable 							*/

/* UARTx_LSR Masks */
#define DR					0x01		/* Data Ready									*/
#define OE					0x02		/* Overrun Error									*/
#define PE					0x04		/* Parity Error									*/
#define FE					0x08		/* Framing Error									*/
#define BI					0x10		/* Break Interrupt								*/
#define THRE				0x20		/* THR Empty									*/
#define TEMT				0x40		/* TSR and UART_THR Empty							*/

/* UARTx_IER Masks*/
#define ERBFI				0x01		/* Enable Receive Buffer Full Interrupt					*/
#define ETBEI				0x02		/* Enable Transmit Buffer Empty Interrupt					*/
#define ELSI				0x04		/* Enable RX Status Interrupt							*/

/* UARTx_IIR Masks*/
#define NINT				0x01		/* Pending Interrupt								*/
#define STATUS				0x06		/* Highest Priority Pending Interrupt					*/

/* UARTx_GCTL Masks*/
#define UCEN				0x01		/* Enable UARTx Clocks								*/
#define IREN				0x02		/* Enable IrDA Mode								*/
#define TPOLC				0x04		/* IrDA TX Polarity Change							*/
#define RPOLC				0x08		/* IrDA RX Polarity Change							*/
#define FPE					0x10		/* Force Parity Error On Transmit						*/
#define FFE					0x20		/* Force Framing Error On Transmit						*/

/* Bit masks for UART Divisor Latch Registers: UARTx_DLL & UARTx_DLH */
#define UARTDLL                 	0x00FF	/* Divisor Latch Low Byte 							*/
#define UARTDLH                	0xFF00	/* Divisor Latch High Byte 							*/


/********************************  SERIAL PERIPHERAL INTERFACE (SPI) MASKS  ***************************************/

/* SPIx_CTL Masks*/
#define TIMOD				0x0003	/* Transfer Initiate Mode							*/
#define RDBR_CORE				0x0000	/* RDBR Read Initiates, IRQ When RDBR Full				*/
#define TDBR_CORE				0x0001	/* TDBR Write Initiates, IRQ When TDBR Empty				*/
#define RDBR_DMA				0x0002	/* DMA Read, DMA Until FIFO Empty						*/
#define TDBR_DMA				0x0003	/* DMA Write, DMA Until FIFO Full						*/
#define SZ					0x0004	/* Send Zero (When TDBR Empty, Send Zero/Last*)				*/
#define GM					0x0008	/* Get More (When RDBR Full, Overwrite/Discard*)			*/
#define PSSE				0x0010	/* Slave-Select Input Enable							*/
#define EMISO				0x0020	/* Enable MISO As Output							*/
#define SIZE				0x0100	/* Size of Words (16/8* Bits)							*/
#define LSBF				0x0200	/* LSB First									*/
#define CPHA				0x0400	/* Clock Phase									*/
#define CPOL				0x0800	/* Clock Polarity									*/
#define MSTR				0x1000	/* Master/Slave*									*/
#define WOM					0x2000	/* Write Open Drain Master							*/
#define SPE					0x4000	/* SPI Enable									*/

/* SPIx_FLG Masks*/
#define FLS1				0x0002	/* Enables SPI_FLOUT1 as SPI Slave-Select Output			*/
#define FLS2				0x0004	/* Enables SPI_FLOUT2 as SPI Slave-Select Output			*/
#define FLS3				0x0008	/* Enables SPI_FLOUT3 as SPI Slave-Select Output			*/
#define FLS4				0x0010	/* Enables SPI_FLOUT4 as SPI Slave-Select Output			*/
#define FLS5				0x0020	/* Enables SPI_FLOUT5 as SPI Slave-Select Output			*/
#define FLS6				0x0040	/* Enables SPI_FLOUT6 as SPI Slave-Select Output			*/
#define FLS7				0x0080	/* Enables SPI_FLOUT7 as SPI Slave-Select Output			*/
#define FLG1				0xFDFF	/* Activates SPI_FLOUT1 							*/
#define FLG2				0xFBFF	/* Activates SPI_FLOUT2								*/
#define FLG3				0xF7FF	/* Activates SPI_FLOUT3								*/
#define FLG4				0xEFFF	/* Activates SPI_FLOUT4								*/
#define FLG5				0xDFFF	/* Activates SPI_FLOUT5								*/
#define FLG6				0xBFFF	/* Activates SPI_FLOUT6								*/
#define FLG7				0x7FFF	/* Activates SPI_FLOUT7								*/

/* SPIx_STAT Masks*/
#define SPIF				0x0001	/* SPI Finished (Single-Word Transfer Complete)				*/
#define MODF				0x0002	/* Mode Fault Error (Another Device Tried To Become Master)		*/
#define TXE					0x0004	/* Transmission Error (Data Sent With No New Data In TDBR)		*/
#define TXS					0x0008	/* SPI_TDBR Data Buffer Status (Full/Empty*)				*/
#define RBSY				0x0010	/* Receive Error (Data Received With RDBR Full)				*/
#define RXS					0x0020	/* SPI_RDBR Data Buffer Status (Full/Empty*)				*/
#define TXCOL				0x0040	/* Transmit Collision Error (Corrupt Data May Have Been Sent)	*/


/***********************************  GENERAL PURPOSE TIMER MASKS  ************************************************/
/* TIMER_ENABLE Masks*/
#define TIMEN0				0x0001	/* Enable Timer 0									*/
#define TIMEN1				0x0002	/* Enable Timer 1									*/
#define TIMEN2				0x0004	/* Enable Timer 2									*/

/* TIMER_DISABLE Masks*/
#define TIMDIS0				TIMEN0	/* Disable Timer 0								*/
#define TIMDIS1				TIMEN1	/* Disable Timer 1								*/
#define TIMDIS2				TIMEN2	/* Disable Timer 2								*/

/* TIMER_STATUS Masks*/
#define TIMIL0				0x00000001	/* Timer 0 Interrupt								*/
#define TIMIL1				0x00000002	/* Timer 1 Interrupt								*/
#define TIMIL2				0x00000004	/* Timer 2 Interrupt								*/
#define TOVF_ERR0				0x00000010	/* Timer 0 Counter Overflow							*/
#define TOVF_ERR1				0x00000020	/* Timer 1 Counter Overflow							*/
#define TOVF_ERR2				0x00000040	/* Timer 2 Counter Overflow							*/
#define TRUN0				0x00001000	/* Timer 0 Slave Enable Status						*/
#define TRUN1				0x00002000	/* Timer 1 Slave Enable Status						*/
#define TRUN2				0x00004000	/* Timer 2 Slave Enable Status						*/

/* Alternate Deprecated Macros Provided For Backwards Code Compatibility */
#define TOVL_ERR0 			TOVF_ERR0
#define TOVL_ERR1				TOVF_ERR1
#define TOVL_ERR2 			TOVF_ERR2

/* TIMERx_CONFIG Masks */
#define PWM_OUT				0x0001	/* Pulse-Width Modulation Output Mode					*/
#define WDTH_CAP				0x0002	/* Width Capture Input Mode							*/
#define EXT_CLK				0x0003	/* External Clock Mode								*/
#define PULSE_HI				0x0004	/* Action Pulse (Positive/Negative*)					*/
#define PERIOD_CNT			0x0008	/* Period Count									*/
#define IRQ_ENA				0x0010	/* Interrupt Request Enable							*/
#define TIN_SEL				0x0020	/* Timer Input Select								*/
#define OUT_DIS				0x0040	/* Output Pad Disable								*/
#define CLK_SEL				0x0080	/* Timer Clock Select								*/
#define TOGGLE_HI				0x0100	/* PWM_OUT PULSE_HI Toggle Mode						*/
#define EMU_RUN				0x0200	/* Emulation Behavior Select							*/
#define ERR_TYP				0xC000	/* Error Type									*/


/* *************************************   GPIO PORTS F, G MASKS  **********************************************/

/*  General Purpose IO Masks */
/* Port F Masks */
#define PF0					0x0001
#define PF1					0x0002
#define PF2					0x0004
#define PF3					0x0008
#define PF4					0x0010
#define PF5					0x0020
#define PF6					0x0040
#define PF7					0x0080
#define PF8					0x0100
#define PF9					0x0200
#define PF10				0x0400
#define PF11				0x0800
#define PF12				0x1000
#define PF13				0x2000
#define PF14				0x4000
#define PF15				0x8000

/* Port G Masks */
#define PG0					0x0001
#define PG1					0x0002
#define PG2					0x0004
#define PG3					0x0008
#define PG4					0x0010
#define PG5					0x0020
#define PG6					0x0040
#define PG7					0x0080
#define PG8					0x0100
#define PG9					0x0200
#define PG10				0x0400
#define PG11				0x0800
#define PG12				0x1000
#define PG13				0x2000
#define PG14				0x4000
#define PG15				0x8000

/* **************************************  SERIAL PORT MASKS  *****************************************************/
/* SPORT_GATECLK Masks */
#define SPORT0_GATECLK_EN				0x0001	/* SPORT0 Clock Gating Enable							*/
#define SPORT0_GATECLK_MODE				0x0002	/* SPORT0 Clock Gating Mode						*/
#define SPORT0_GATECLK_STATE			0x0004	/* SPORT0 Clock Gating State								*/
#define SPORT1_GATECLK_EN				0x0010	/* SPORT1 Clock Gating Enable							*/
#define SPORT1_GATECLK_MODE				0x0020	/* SPORT1 Clock Gating Mode						*/
#define SPORT1_GATECLK_STATE			0x0040	/* SPORT1 Clock Gating State								*/

/* SPORTx_TCR1 Masks */
#define TSPEN				0x0001	/* Transmit Enable								*/
#define ITCLK				0x0002	/* Internal Transmit Clock Select						*/
#define DTYPE_NORM			0x0004	/* Data Format Normal								*/
#define DTYPE_ULAW			0x0008	/* Compand Using u-Law								*/
#define DTYPE_ALAW			0x000C	/* Compand Using A-Law								*/
#define TLSBIT				0x0010	/* Transmit Bit Order								*/
#define ITFS				0x0200	/* Internal Transmit Frame Sync Select					*/
#define TFSR				0x0400	/* Transmit Frame Sync Required Select					*/
#define DITFS				0x0800	/* Data-Independent Transmit Frame Sync Select				*/
#define LTFS				0x1000	/* Low Transmit Frame Sync Select						*/
#define LATFS				0x2000	/* Late Transmit Frame Sync Select						*/
#define TCKFE				0x4000	/* Clock Falling Edge Select							*/

/* SPORTx_TCR2 Masks and Macro */
#ifdef _MISRA_RULES
#define SLEN(x)				((x)&0x1Fu)	/* SPORT TX Word Length (2 - 31)						*/
#else
#define SLEN(x)				((x)&0x1F)	/* SPORT TX Word Length (2 - 31)						*/
#endif /* _MISRA_RULES */

#define TXSE				0x0100	/* TX Secondary Enable								*/
#define TSFSE				0x0200	/* Transmit Stereo Frame Sync Enable					*/
#define TRFST				0x0400	/* Left/Right Order (1 = Right Channel 1st)				*/

/* SPORTx_RCR1 Masks */
#define RSPEN				0x0001	/* Receive Enable 								*/
#define IRCLK				0x0002	/* Internal Receive Clock Select 						*/
#define DTYPE_NORM			0x0004	/* Data Format Normal								*/
#define DTYPE_ULAW			0x0008	/* Compand Using u-Law								*/
#define DTYPE_ALAW			0x000C	/* Compand Using A-Law								*/
#define RLSBIT				0x0010	/* Receive Bit Order								*/
#define IRFS				0x0200	/* Internal Receive Frame Sync Select 					*/
#define RFSR				0x0400	/* Receive Frame Sync Required Select 					*/
#define LRFS				0x1000	/* Low Receive Frame Sync Select 						*/
#define LARFS				0x2000	/* Late Receive Frame Sync Select 						*/
#define RCKFE				0x4000	/* Clock Falling Edge Select 							*/

/* SPORTx_RCR2 Masks */
#ifdef _MISRA_RULES
#define SLEN(x)				((x)&0x1Fu)	/* SPORT RX Word Length (2 - 31)						*/
#else
#define SLEN(x)				((x)&0x1F)	/* SPORT RX Word Length (2 - 31)						*/
#endif /* _MISRA_RULES */

#define RXSE				0x0100	/* RX Secondary Enable								*/
#define RSFSE				0x0200	/* RX Stereo Frame Sync Enable						*/
#define RRFST				0x0400	/* Right-First Data Order 							*/

/* SPORTx_STAT Masks */
#define RXNE				0x0001	/* Receive FIFO Not Empty Status						*/
#define RUVF				0x0002	/* Sticky Receive Underflow Status						*/
#define ROVF				0x0004	/* Sticky Receive Overflow Status						*/
#define TXF					0x0008	/* Transmit FIFO Full Status							*/
#define TUVF				0x0010	/* Sticky Transmit Underflow Status						*/
#define TOVF				0x0020	/* Sticky Transmit Overflow Status						*/
#define TXHRE				0x0040	/* Transmit Hold Register Empty						*/

/* SPORTx_MCMC1 Macros */
#ifdef _MISRA_RULES
#define WOFF(x)		((x) & 0x3FFu) 		/* Multichannel Window Offset Field						*/
/* Only use WSIZE Macro With Logic OR While Setting Lower Order Bits*/
#define WSIZE(x)		(((((x)>>0x3)-1u)&0xFu) << 0xC) /* Multichannel Window Size = (x/8)-1				*/
#else
#define WOFF(x)		((x) & 0x3FF) 		/* Multichannel Window Offset Field						*/
/* Only use WSIZE Macro With Logic OR While Setting Lower Order Bits								*/
#define WSIZE(x)	(((((x)>>0x3)-1)&0xF) << 0xC)	/* Multichannel Window Size = (x/8)-1					*/
#endif /* _MISRA_RULES */

/* SPORTx_MCMC2 Masks */
#define REC_BYPASS			0x0000	/* Bypass Mode (No Clock Recovery)						*/
#define REC_2FROM4			0x0002	/* Recover 2 MHz Clock from 4 MHz Clock					*/
#define REC_8FROM16			0x0003	/* Recover 8 MHz Clock from 16 MHz Clock					*/
#define MCDTXPE				0x0004 	/* Multichannel DMA Transmit Packing					*/
#define MCDRXPE				0x0008 	/* Multichannel DMA Receive Packing						*/
#define MCMEN				0x0010 	/* Multichannel Frame Mode Enable						*/
#define FSDR				0x0080 	/* Multichannel Frame Sync to Data Relationship				*/
#define MFD_0				0x0000	/* Multichannel Frame Delay = 0						*/
#define MFD_1				0x1000	/* Multichannel Frame Delay = 1						*/
#define MFD_2				0x2000	/* Multichannel Frame Delay = 2						*/
#define MFD_3				0x3000	/* Multichannel Frame Delay = 3						*/
#define MFD_4				0x4000	/* Multichannel Frame Delay = 4						*/
#define MFD_5				0x5000	/* Multichannel Frame Delay = 5						*/
#define MFD_6				0x6000	/* Multichannel Frame Delay = 6						*/
#define MFD_7				0x7000	/* Multichannel Frame Delay = 7						*/
#define MFD_8				0x8000	/* Multichannel Frame Delay = 8						*/
#define MFD_9				0x9000	/* Multichannel Frame Delay = 9						*/
#define MFD_10				0xA000	/* Multichannel Frame Delay = 10						*/
#define MFD_11				0xB000	/* Multichannel Frame Delay = 11						*/
#define MFD_12				0xC000	/* Multichannel Frame Delay = 12						*/
#define MFD_13				0xD000	/* Multichannel Frame Delay = 13						*/
#define MFD_14				0xE000	/* Multichannel Frame Delay = 14						*/
#define MFD_15				0xF000	/* Multichannel Frame Delay = 15						*/


/****************************************  DMA CONTROLLER MASKS  **************************************************/

/* DMAx_CONFIG, MDMA_yy_CONFIG Masks */
#define DMAEN				0x0001	/* DMA Channel Enable								*/
#define WNR					0x0002	/* Channel Direction (W/R*)							*/
#define WDSIZE_8				0x0000	/* Transfer Word Size = 8							*/
#define WDSIZE_16				0x0004	/* Transfer Word Size = 16							*/
#define WDSIZE_32				0x0008	/* Transfer Word Size = 32							*/
#define DMA2D				0x0010	/* DMA Mode (2D/1D*)								*/
#define SYNC				0x0020	/* DMA Buffer Clear								*/
#define DI_SEL				0x0040	/* Data Interrupt Timing Select						*/
#define DI_EN				0x0080	/* Data Interrupt Enable							*/
#define NDSIZE_0				0x0000	/* Next Descriptor Size = 0 (Stop/Autobuffer)				*/
#define NDSIZE_1				0x0100	/* Next Descriptor Size = 1							*/
#define NDSIZE_2				0x0200	/* Next Descriptor Size = 2							*/
#define NDSIZE_3				0x0300	/* Next Descriptor Size = 3							*/
#define NDSIZE_4				0x0400	/* Next Descriptor Size = 4							*/
#define NDSIZE_5				0x0500	/* Next Descriptor Size = 5							*/
#define NDSIZE_6				0x0600	/* Next Descriptor Size = 6							*/
#define NDSIZE_7				0x0700	/* Next Descriptor Size = 7							*/
#define NDSIZE_8				0x0800	/* Next Descriptor Size = 8							*/
#define NDSIZE_9				0x0900	/* Next Descriptor Size = 9							*/
#define FLOW_STOP				0x0000	/* Stop Mode									*/
#define FLOW_AUTO				0x1000	/* Autobuffer Mode								*/
#define FLOW_ARRAY			0x4000	/* Descriptor Array Mode							*/
#define FLOW_SMALL			0x6000	/* Small Model Descriptor List Mode						*/
#define FLOW_LARGE			0x7000	/* Large Model Descriptor List Mode						*/

/* DMAx_PERIPHERAL_MAP, MDMA_yy_PERIPHERAL_MAP Masks */
#define CTYPE				0x0040	/* DMA Channel Type Indicator (Memory/Peripheral*)			*/
#define PMAP				0xF000	/* Peripheral Mapped To This Channel					*/
#define PMAP_PPI				0x0000	/* PPI Port DMA									*/
#define PMAP_SPORT0RX			0x1000	/* SPORT0 Receive DMA								*/
#define PMAP_SPORT0TX			0x2000	/* SPORT0 Transmit DMA								*/
#define PMAP_SPORT1RX			0x3000	/* SPORT1 Receive DMA								*/
#define PMAP_SPORT1TX			0x4000	/* SPORT1 Transmit DMA								*/
#define PMAP_SPI0				0x5000	/* SPI0 Transmit/Receive DMA								*/
#define PMAP_SPI1				0x6000	/* SPI1 Transmit/Receive DMA							*/
#define PMAP_UART0RX			0x7000	/* UART0 Port Receive DMA							*/
#define PMAP_UART0TX			0x8000	/* UART0 Port Transmit DMA							*/

/* DMAx_IRQ_STATUS, MDMA_yy_IRQ_STATUS Masks */
#define DMA_DONE				0x0001	/* DMA Completion Interrupt Status						*/
#define DMA_ERR				0x0002	/* DMA Error Interrupt Status							*/
#define DFETCH				0x0004	/* DMA Descriptor Fetch Indicator						*/
#define DMA_RUN				0x0008	/* DMA Channel Running Indicator						*/


/*********************************  PARALLEL PERIPHERAL INTERFACE (PPI) MASKS *************************************/

/*  PPI_CONTROL Masks */
#define PORT_EN				0x0001	/* PPI Port Enable								*/
#define PORT_DIR				0x0002	/* PPI Port Direction								*/
#define XFR_TYPE				0x000C	/* PPI Transfer Type								*/
#define PORT_CFG				0x0030	/* PPI Port Configuration							*/
#define FLD_SEL				0x0040	/* PPI Active Field Select							*/
#define PACK_EN				0x0080	/* PPI Packing Mode								*/ /* previous versions of defBF532.h erroneously included DMA32 (PPI 32-bit DMA Enable) */
#define SKIP_EN				0x0200	/* PPI Skip Element Enable							*/
#define SKIP_EO				0x0400	/* PPI Skip Even/Odd Elements							*/
#define DLEN_8				0x0000	/* Data Length = 8 Bits								*/
#define DLEN_10				0x0800	/* Data Length = 10 Bits							*/
#define DLEN_11				0x1000	/* Data Length = 11 Bits							*/
#define DLEN_12				0x1800	/* Data Length = 12 Bits							*/
#define DLEN_13				0x2000	/* Data Length = 13 Bits							*/
#define DLEN_14				0x2800	/* Data Length = 14 Bits							*/
#define DLEN_15				0x3000	/* Data Length = 15 Bits							*/
#define DLEN_16				0x3800	/* Data Length = 16 Bits							*/
#define POLC				0x4000	/* PPI Clock Polarity								*/
#define POLS				0x8000	/* PPI Frame Sync Polarity							*/

/* PPI_STATUS Masks */
#define LT_ERR_OVR			0x0100	/* Line Track Overflow Error							*/
#define LT_ERR_UNDR			0x0200	/* Line Track Underflow Error							*/
#define FLD					0x0400	/* Field Indicator								*/
#define FT_ERR				0x0800	/* Frame Track Error								*/
#define OVR					0x1000	/* FIFO Overflow Error								*/
#define UNDR				0x2000	/* FIFO Underrun Error								*/
#define ERR_DET				0x4000	/* Error Detected Indicator							*/
#define ERR_NCOR				0x8000	/* Error Not Corrected Indicator						*/


/***************************************  TWO-WIRE INTERFACE (TWI) MASKS  *****************************************/

/* TWI_CLKDIV Macros (Use: *pTWI_CLKDIV = CLKLOW(x)|CLKHI(y);  ) */
#ifdef _MISRA_RULES
#define CLKLOW(x)				((x) & 0xFFu)/* Periods Clock Is Held Low							*/
#define CLKHI(y)				(((y)&0xFFu)<<0x8)/* Periods Before New Clock Low					*/
#else
#define CLKLOW(x)				((x) & 0xFF)/* Periods Clock Is Held Low							*/
#define CLKHI(y)				(((y)&0xFF)<<0x8)	/* Periods Before New Clock Low					*/
#endif /* _MISRA_RULES */

/* TWI_PRESCALE Masks */
#define PRESCALE				0x007F	/* SCLKs Per Internal Time Reference (10MHz)				*/
#define TWI_ENA				0x0080	/* TWI Enable									*/
#define SCCB				0x0200	/* SCCB Compatibility Enable							*/

/* TWI_SLAVE_CTRL Masks */
#define SEN					0x0001	/* Slave Enable									*/
#define SADD_LEN				0x0002	/* Slave Address Length								*/
#define STDVAL				0x0004	/* Slave Transmit Data Valid							*/
#define NAK					0x0008	/* NAK/ACK* Generated At Conclusion Of Transfer 			*/
#define GEN					0x0010	/* General Call Adrress Matching Enabled					*/

/* TWI_SLAVE_STAT Masks	*/
#define SDIR				0x0001	/* Slave Transfer Direction (Transmit/Receive*)				*/
#define GCALL				0x0002	/* General Call Indicator							*/

/* TWI_MASTER_CTRL Masks */
#define MEN					0x0001	/* Master Mode Enable								*/
#define MADD_LEN				0x0002	/* Master Address Length							*/
#define MDIR				0x0004	/* Master Transmit Direction (RX/TX*)					*/
#define FAST				0x0008	/* Use Fast Mode Timing Specs							*/
#define STOP				0x0010	/* Issue Stop Condition								*/
#define RSTART				0x0020	/* Repeat Start or Stop* At End Of Transfer				*/
#define DCNT				0x3FC0	/* Data Bytes To Transfer							*/
#define SDAOVR				0x4000	/* Serial Data Override								*/
#define SCLOVR				0x8000	/* Serial Clock Override							*/

/* TWI_MASTER_STAT Masks */
#define MPROG				0x0001	/* Master Transfer In Progress						*/
#define LOSTARB				0x0002	/* Lost Arbitration Indicator (Xfer Aborted)				*/
#define ANAK				0x0004	/* Address Not Acknowledged							*/
#define DNAK				0x0008	/* Data Not Acknowledged							*/
#define BUFRDERR				0x0010	/* Buffer Read Error								*/
#define BUFWRERR				0x0020	/* Buffer Write Error								*/
#define SDASEN				0x0040	/* Serial Data Sense								*/
#define SCLSEN				0x0080	/* Serial Clock Sense								*/
#define BUSBUSY				0x0100	/* Bus Busy Indicator								*/

/* TWI_INT_SRC and TWI_INT_ENABLE Masks */
#define SINIT				0x0001	/* Slave Transfer Initiated							*/
#define SCOMP				0x0002	/* Slave Transfer Complete							*/
#define SERR				0x0004	/* Slave Transfer Error								*/
#define SOVF				0x0008	/* Slave Overflow									*/
#define MCOMP				0x0010	/* Master Transfer Complete							*/
#define MERR				0x0020	/* Master Transfer Error							*/
#define XMTSERV				0x0040	/* Transmit FIFO Service							*/
#define RCVSERV				0x0080	/* Receive FIFO Service								*/

/* TWI_FIFO_CTRL Masks */
#define XMTFLUSH				0x0001	/* Transmit Buffer Flush							*/
#define RCVFLUSH				0x0002	/* Receive Buffer Flush								*/
#define XMTINTLEN				0x0004	/* Transmit Buffer Interrupt Length						*/
#define RCVINTLEN				0x0008	/* Receive Buffer Interrupt Length						*/

/* TWI_FIFO_STAT Masks */
#define XMTSTAT				0x0003	/* Transmit FIFO Status								*/
#define XMT_EMPTY				0x0000	/* Transmit FIFO Empty								*/
#define XMT_HALF				0x0001	/* Transmit FIFO Has 1 Byte To Write					*/
#define XMT_FULL				0x0003	/* Transmit FIFO Full (2 Bytes To Write)					*/

#define RCVSTAT				0x000C	/* Receive FIFO Status								*/
#define RCV_EMPTY				0x0000	/* Receive FIFO Empty								*/
#define RCV_HALF				0x0004	/* Receive FIFO Has 1 Byte To Read						*/
#define RCV_FULL				0x000C	/* Receive FIFO Full (2 Bytes To Read)					*/

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* _DEF_BF59x_H */
