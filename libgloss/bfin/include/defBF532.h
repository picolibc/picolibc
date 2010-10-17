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
 * defBF532.h
 *
 * (c) Copyright 2001-2009 Analog Devices, Inc.  All rights reserved.
 *
 ************************************************************************/

/* SYSTEM & MM REGISTER BIT & ADDRESS DEFINITIONS FOR ADSP-BF532 */

#ifndef _DEF_BF532_H
#define _DEF_BF532_H

#if !defined(__ADSPLPBLACKFIN__)
#warning defBF532.h should only be included for 532 compatible chips
#endif
/* include all Core registers and bit definitions */
#include <def_LPBlackfin.h>

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_19_4)
#pragma diag(suppress:misra_rule_19_7)
#include <stdint.h>
#endif /* _MISRA_RULES */

/*********************************************************************************** */
/* System MMR Register Map */
/*********************************************************************************** */
/*// Clock/Regulator Control (0xFFC00000 - 0xFFC000FF) */

#define PLL_CTL			0xFFC00000  /* PLL Control register (16-bit) */
#define PLL_DIV			0xFFC00004	/* PLL Divide Register (16-bit) */
#define VR_CTL			0xFFC00008	/* Voltage Regulator Control Register (16-bit) */
#define PLL_STAT		0xFFC0000C  /* PLL Status register (16-bit) */
#define PLL_LOCKCNT		0xFFC00010  /* PLL Lock Count register (16-bit) */
#define	CHIPID			0xFFC00014	/* Chip ID Register						*/


/* System Interrupt Controller (0xFFC00100 - 0xFFC001FF) */
#define SWRST			0xFFC00100  /* Software Reset Register (16-bit) */
#define SYSCR			0xFFC00104  /* System Configuration registe */
#define SIC_IMASK		0xFFC0010C  /* Interrupt Mask Register */
#define SIC_IAR0		0xFFC00110  /* Interrupt Assignment Register 0 */
#define SIC_IAR1		0xFFC00114  /* Interrupt Assignment Register 1 */
#define SIC_IAR2		0xFFC00118  /* Interrupt Assignment Register 2 */
#define SIC_ISR			0xFFC00120  /* Interrupt Status Register */
#define SIC_IWR			0xFFC00124  /* Interrupt Wakeup Register */


/*// Watchdog Timer (0xFFC00200 - 0xFFC002FF) */
#define WDOG_CTL        0xFFC00200  /* Watchdog Control Register */
#define WDOG_CNT        0xFFC00204  /* Watchdog Count Register */
#define WDOG_STAT       0xFFC00208  /* Watchdog Status Register */


/*// Real Time Clock (0xFFC00300 - 0xFFC003FF) */
#define RTC_STAT        0xFFC00300  /* RTC Status Register */
#define RTC_ICTL        0xFFC00304  /* RTC Interrupt Control Register */
#define RTC_ISTAT       0xFFC00308  /* RTC Interrupt Status Register */
#define RTC_SWCNT       0xFFC0030C  /* RTC Stopwatch Count Register */
#define RTC_ALARM       0xFFC00310  /* RTC Alarm Time Register */
#define RTC_FAST        0xFFC00314  /* RTC Prescaler Enable Register */
#define RTC_PREN		0xFFC00314  /* RTC Prescaler Enable Register (alternate macro) */


/* UART Controller (0xFFC00400 - 0xFFC004FF) */
#define UART_THR             0xFFC00400  /* Transmit Holding register */
#define UART_RBR             0xFFC00400  /* Receive Buffer register */
#define UART_DLL             0xFFC00400  /* Divisor Latch (Low-Byte) */
#define UART_IER             0xFFC00404  /* Interrupt Enable Register */
#define UART_DLH             0xFFC00404  /* Divisor Latch (High-Byte) */
#define UART_IIR             0xFFC00408  /* Interrupt Identification Register */
#define UART_LCR             0xFFC0040C  /* Line Control Register */
#define UART_MCR			 0xFFC00410  /* Modem Control Register */
#define UART_LSR             0xFFC00414  /* Line Status Register */
#define UART_SCR             0xFFC0041C  /* SCR Scratch Register */
#define UART_GCTL      	     0xFFC00424  /* Global Control Register */


/* SPI Controller (0xFFC00500 - 0xFFC005FF) */
#define SPI_CTL               		0xFFC00500  /* SPI Control Register */
#define SPI_FLG               		0xFFC00504  /* SPI Flag register */
#define SPI_STAT              		0xFFC00508  /* SPI Status register */
#define SPI_TDBR              		0xFFC0050C  /* SPI Transmit Data Buffer Register */
#define SPI_RDBR              		0xFFC00510  /* SPI Receive Data Buffer Register */
#define SPI_BAUD              		0xFFC00514  /* SPI Baud rate Register */
#define SPI_SHADOW            		0xFFC00518  /* SPI_RDBR Shadow Register */


/* TIMER 0, 1, 2 Registers (0xFFC00600 - 0xFFC006FF) */
#define TIMER0_CONFIG          		0xFFC00600     /* Timer 0 Configuration Register */
#define TIMER0_COUNTER				0xFFC00604     /* Timer 0 Counter Register */
#define TIMER0_PERIOD       		0xFFC00608     /* Timer 0 Period Register */
#define TIMER0_WIDTH        		0xFFC0060C     /* Timer 0 Width Register */

#define TIMER1_CONFIG          		0xFFC00610  	/*  Timer 1 Configuration Register   */
#define TIMER1_COUNTER         		0xFFC00614   	/*  Timer 1 Counter Register         */
#define TIMER1_PERIOD          		0xFFC00618      /*  Timer 1 Period Register          */
#define TIMER1_WIDTH           		0xFFC0061C      /*  Timer 1 Width Register           */

#define TIMER2_CONFIG          		0xFFC00620  	/* Timer 2 Configuration Register   */
#define TIMER2_COUNTER         		0xFFC00624      /* Timer 2 Counter Register         */
#define TIMER2_PERIOD          		0xFFC00628      /* Timer 2 Period Register          */
#define TIMER2_WIDTH           		0xFFC0062C      /* Timer 2 Width Register           */

#define TIMER_ENABLE				0xFFC00640   	/* Timer Enable Register */
#define TIMER_DISABLE				0xFFC00644   	/* Timer Disable Register */
#define TIMER_STATUS				0xFFC00648   	/* Timer Status Register */


/*// General Purpose IO (0xFFC00700 - 0xFFC007FF) */
#define FIO_FLAG_D	       			0xFFC00700  /* Flag Mask to directly specify state of pins */
#define FIO_FLAG_C             		0xFFC00704  /* Peripheral Interrupt Flag Register (clear) */
#define FIO_FLAG_S             		0xFFC00708  /* Peripheral Interrupt Flag Register (set) */
#define FIO_FLAG_T					0xFFC0070C  /* Flag Mask to directly toggle state of pins */
#define FIO_MASKA_D            		0xFFC00710  /* Flag Mask Interrupt A Register (set directly) */
#define FIO_MASKA_C            		0xFFC00714  /* Flag Mask Interrupt A Register (clear) */
#define FIO_MASKA_S            		0xFFC00718  /* Flag Mask Interrupt A Register (set) */
#define FIO_MASKA_T            		0xFFC0071C  /* Flag Mask Interrupt A Register (toggle) */
#define FIO_MASKB_D            		0xFFC00720  /* Flag Mask Interrupt B Register (set directly) */
#define FIO_MASKB_C            		0xFFC00724  /* Flag Mask Interrupt B Register (clear) */
#define FIO_MASKB_S            		0xFFC00728  /* Flag Mask Interrupt B Register (set) */
#define FIO_MASKB_T            		0xFFC0072C  /* Flag Mask Interrupt B Register (toggle) */
#define FIO_DIR                		0xFFC00730  /* Peripheral Flag Direction Register */
#define FIO_POLAR              		0xFFC00734  /* Flag Source Polarity Register */
#define FIO_EDGE               		0xFFC00738  /* Flag Source Sensitivity Register */
#define FIO_BOTH               		0xFFC0073C  /* Flag Set on BOTH Edges Register */
#define FIO_INEN					0xFFC00740  /* Flag Input Enable Register  */


/*// SPORT0 Controller (0xFFC00800 - 0xFFC008FF) */
#define SPORT0_TCR1     	 		0xFFC00800  /* SPORT0 Transmit Configuration 1 Register */
#define SPORT0_TCR2      	 		0xFFC00804  /* SPORT0 Transmit Configuration 2 Register */
#define SPORT0_TCLKDIV        		0xFFC00808  /* SPORT0 Transmit Clock Divider */
#define SPORT0_TFSDIV          		0xFFC0080C  /* SPORT0 Transmit Frame Sync Divider */
#define SPORT0_TX	             	0xFFC00810  /* SPORT0 TX Data Register */
#define SPORT0_RX	            	0xFFC00818  /* SPORT0 RX Data Register */
#define SPORT0_RCR1      	 		0xFFC00820  /* SPORT0 Transmit Configuration 1 Register */
#define SPORT0_RCR2      	 		0xFFC00824  /* SPORT0 Transmit Configuration 2 Register */
#define SPORT0_RCLKDIV        		0xFFC00828  /* SPORT0 Receive Clock Divider */
#define SPORT0_RFSDIV          		0xFFC0082C  /* SPORT0 Receive Frame Sync Divider */
#define SPORT0_STAT            		0xFFC00830  /* SPORT0 Status Register */
#define SPORT0_CHNL            		0xFFC00834  /* SPORT0 Current Channel Register */
#define SPORT0_MCMC1           		0xFFC00838  /* SPORT0 Multi-Channel Configuration Register 1 */
#define SPORT0_MCMC2           		0xFFC0083C  /* SPORT0 Multi-Channel Configuration Register 2 */
#define SPORT0_MTCS0           		0xFFC00840  /* SPORT0 Multi-Channel Transmit Select Register 0 */
#define SPORT0_MTCS1           		0xFFC00844  /* SPORT0 Multi-Channel Transmit Select Register 1 */
#define SPORT0_MTCS2           		0xFFC00848  /* SPORT0 Multi-Channel Transmit Select Register 2 */
#define SPORT0_MTCS3           		0xFFC0084C  /* SPORT0 Multi-Channel Transmit Select Register 3 */
#define SPORT0_MRCS0           		0xFFC00850  /* SPORT0 Multi-Channel Receive Select Register 0 */
#define SPORT0_MRCS1           		0xFFC00854  /* SPORT0 Multi-Channel Receive Select Register 1 */
#define SPORT0_MRCS2           		0xFFC00858  /* SPORT0 Multi-Channel Receive Select Register 2 */
#define SPORT0_MRCS3           		0xFFC0085C  /* SPORT0 Multi-Channel Receive Select Register 3 */


/*// SPORT1 Controller (0xFFC00900 - 0xFFC009FF) */
#define SPORT1_TCR1     	 		0xFFC00900  /* SPORT1 Transmit Configuration 1 Register */
#define SPORT1_TCR2      	 		0xFFC00904  /* SPORT1 Transmit Configuration 2 Register */
#define SPORT1_TCLKDIV        		0xFFC00908  /* SPORT1 Transmit Clock Divider */
#define SPORT1_TFSDIV          		0xFFC0090C  /* SPORT1 Transmit Frame Sync Divider */
#define SPORT1_TX	             	0xFFC00910  /* SPORT1 TX Data Register */
#define SPORT1_RX	            	0xFFC00918  /* SPORT1 RX Data Register */
#define SPORT1_RCR1      	 		0xFFC00920  /* SPORT1 Transmit Configuration 1 Register */
#define SPORT1_RCR2      	 		0xFFC00924  /* SPORT1 Transmit Configuration 2 Register */
#define SPORT1_RCLKDIV        		0xFFC00928  /* SPORT1 Receive Clock Divider */
#define SPORT1_RFSDIV          		0xFFC0092C  /* SPORT1 Receive Frame Sync Divider */
#define SPORT1_STAT            		0xFFC00930  /* SPORT1 Status Register */
#define SPORT1_CHNL            		0xFFC00934  /* SPORT1 Current Channel Register */
#define SPORT1_MCMC1           		0xFFC00938  /* SPORT1 Multi-Channel Configuration Register 1 */
#define SPORT1_MCMC2           		0xFFC0093C  /* SPORT1 Multi-Channel Configuration Register 2 */
#define SPORT1_MTCS0           		0xFFC00940  /* SPORT1 Multi-Channel Transmit Select Register 0 */
#define SPORT1_MTCS1           		0xFFC00944  /* SPORT1 Multi-Channel Transmit Select Register 1 */
#define SPORT1_MTCS2           		0xFFC00948  /* SPORT1 Multi-Channel Transmit Select Register 2 */
#define SPORT1_MTCS3           		0xFFC0094C  /* SPORT1 Multi-Channel Transmit Select Register 3 */
#define SPORT1_MRCS0           		0xFFC00950  /* SPORT1 Multi-Channel Receive Select Register 0 */
#define SPORT1_MRCS1           		0xFFC00954  /* SPORT1 Multi-Channel Receive Select Register 1 */
#define SPORT1_MRCS2           		0xFFC00958  /* SPORT1 Multi-Channel Receive Select Register 2 */
#define SPORT1_MRCS3           		0xFFC0095C  /* SPORT1 Multi-Channel Receive Select Register 3 */


/* External Bus Interface Unit (0xFFC00A00 - 0xFFC00AFF) */
/* Asynchronous Memory Controller  */
#define EBIU_AMGCTL			0xFFC00A00  /* Asynchronous Memory Global Control Register */
#define EBIU_AMBCTL0		0xFFC00A04  /* Asynchronous Memory Bank Control Register 0 */
#define EBIU_AMBCTL1		0xFFC00A08  /* Asynchronous Memory Bank Control Register 1 */

/* SDRAM Controller */
#define EBIU_SDGCTL			0xFFC00A10  /* SDRAM Global Control Register */
#define EBIU_SDBCTL			0xFFC00A14  /* SDRAM Bank Control Register */
#define EBIU_SDRRC 			0xFFC00A18  /* SDRAM Refresh Rate Control Register */
#define EBIU_SDSTAT			0xFFC00A1C  /* SDRAM Status Register */


/* DMA Traffic controls */
#define DMA_TC_PER 			0xFFC00B0C	/* Traffic Control Periods Register */
#define DMA_TC_CNT 			0xFFC00B10	/* Traffic Control Current Counts Register */

/* Alternate deprecated register names (below) provided for backwards code compatibility */
#define DMA_TCPER 			0xFFC00B0C	/* Traffic Control Periods Register */
#define DMA_TCCNT 			0xFFC00B10	/* Traffic Control Current Counts Register */


/* DMA Controller (0xFFC00C00 - 0xFFC00FFF) */
#define DMA0_CONFIG			0xFFC00C08	/* DMA Channel 0 Configuration Register */
#define DMA0_NEXT_DESC_PTR	0xFFC00C00	/* DMA Channel 0 Next Descriptor Pointer Register */
#define DMA0_START_ADDR		0xFFC00C04	/* DMA Channel 0 Start Address Register */
#define DMA0_X_COUNT		0xFFC00C10	/* DMA Channel 0 X Count Register */
#define DMA0_Y_COUNT		0xFFC00C18	/* DMA Channel 0 Y Count Register */
#define DMA0_X_MODIFY		0xFFC00C14	/* DMA Channel 0 X Modify Register */
#define DMA0_Y_MODIFY		0xFFC00C1C	/* DMA Channel 0 Y Modify Register */
#define DMA0_CURR_DESC_PTR	0xFFC00C20	/* DMA Channel 0 Current Descriptor Pointer Register */
#define DMA0_CURR_ADDR		0xFFC00C24	/* DMA Channel 0 Current Address Register */
#define DMA0_CURR_X_COUNT	0xFFC00C30	/* DMA Channel 0 Current X Count Register */
#define DMA0_CURR_Y_COUNT	0xFFC00C38	/* DMA Channel 0 Current Y Count Register */
#define DMA0_IRQ_STATUS		0xFFC00C28	/* DMA Channel 0 Interrupt/Status Register */
#define DMA0_PERIPHERAL_MAP	0xFFC00C2C	/* DMA Channel 0 Peripheral Map Register */

#define DMA1_CONFIG			0xFFC00C48	/* DMA Channel 1 Configuration Register */
#define DMA1_NEXT_DESC_PTR	0xFFC00C40	/* DMA Channel 1 Next Descriptor Pointer Register */
#define DMA1_START_ADDR		0xFFC00C44	/* DMA Channel 1 Start Address Register */
#define DMA1_X_COUNT		0xFFC00C50	/* DMA Channel 1 X Count Register */
#define DMA1_Y_COUNT		0xFFC00C58	/* DMA Channel 1 Y Count Register */
#define DMA1_X_MODIFY		0xFFC00C54	/* DMA Channel 1 X Modify Register */
#define DMA1_Y_MODIFY		0xFFC00C5C	/* DMA Channel 1 Y Modify Register */
#define DMA1_CURR_DESC_PTR	0xFFC00C60	/* DMA Channel 1 Current Descriptor Pointer Register */
#define DMA1_CURR_ADDR		0xFFC00C64	/* DMA Channel 1 Current Address Register */
#define DMA1_CURR_X_COUNT	0xFFC00C70	/* DMA Channel 1 Current X Count Register */
#define DMA1_CURR_Y_COUNT	0xFFC00C78	/* DMA Channel 1 Current Y Count Register */
#define DMA1_IRQ_STATUS		0xFFC00C68	/* DMA Channel 1 Interrupt/Status Register */
#define DMA1_PERIPHERAL_MAP	0xFFC00C6C	/* DMA Channel 1 Peripheral Map Register */

#define DMA2_CONFIG			0xFFC00C88	/* DMA Channel 2 Configuration Register */
#define DMA2_NEXT_DESC_PTR	0xFFC00C80	/* DMA Channel 2 Next Descriptor Pointer Register */
#define DMA2_START_ADDR		0xFFC00C84	/* DMA Channel 2 Start Address Register */
#define DMA2_X_COUNT		0xFFC00C90	/* DMA Channel 2 X Count Register */
#define DMA2_Y_COUNT		0xFFC00C98	/* DMA Channel 2 Y Count Register */
#define DMA2_X_MODIFY		0xFFC00C94	/* DMA Channel 2 X Modify Register */
#define DMA2_Y_MODIFY		0xFFC00C9C	/* DMA Channel 2 Y Modify Register */
#define DMA2_CURR_DESC_PTR	0xFFC00CA0	/* DMA Channel 2 Current Descriptor Pointer Register */
#define DMA2_CURR_ADDR		0xFFC00CA4	/* DMA Channel 2 Current Address Register */
#define DMA2_CURR_X_COUNT	0xFFC00CB0	/* DMA Channel 2 Current X Count Register */
#define DMA2_CURR_Y_COUNT	0xFFC00CB8	/* DMA Channel 2 Current Y Count Register */
#define DMA2_IRQ_STATUS		0xFFC00CA8	/* DMA Channel 2 Interrupt/Status Register */
#define DMA2_PERIPHERAL_MAP	0xFFC00CAC	/* DMA Channel 2 Peripheral Map Register */

#define DMA3_CONFIG			0xFFC00CC8	/* DMA Channel 3 Configuration Register */
#define DMA3_NEXT_DESC_PTR	0xFFC00CC0	/* DMA Channel 3 Next Descriptor Pointer Register */
#define DMA3_START_ADDR		0xFFC00CC4	/* DMA Channel 3 Start Address Register */
#define DMA3_X_COUNT		0xFFC00CD0	/* DMA Channel 3 X Count Register */
#define DMA3_Y_COUNT		0xFFC00CD8	/* DMA Channel 3 Y Count Register */
#define DMA3_X_MODIFY		0xFFC00CD4	/* DMA Channel 3 X Modify Register */
#define DMA3_Y_MODIFY		0xFFC00CDC	/* DMA Channel 3 Y Modify Register */
#define DMA3_CURR_DESC_PTR	0xFFC00CE0	/* DMA Channel 3 Current Descriptor Pointer Register */
#define DMA3_CURR_ADDR		0xFFC00CE4	/* DMA Channel 3 Current Address Register */
#define DMA3_CURR_X_COUNT	0xFFC00CF0	/* DMA Channel 3 Current X Count Register */
#define DMA3_CURR_Y_COUNT	0xFFC00CF8	/* DMA Channel 3 Current Y Count Register */
#define DMA3_IRQ_STATUS		0xFFC00CE8	/* DMA Channel 3 Interrupt/Status Register */
#define DMA3_PERIPHERAL_MAP	0xFFC00CEC	/* DMA Channel 3 Peripheral Map Register */

#define DMA4_CONFIG			0xFFC00D08	/* DMA Channel 4 Configuration Register */
#define DMA4_NEXT_DESC_PTR	0xFFC00D00	/* DMA Channel 4 Next Descriptor Pointer Register */
#define DMA4_START_ADDR		0xFFC00D04	/* DMA Channel 4 Start Address Register */
#define DMA4_X_COUNT		0xFFC00D10	/* DMA Channel 4 X Count Register */
#define DMA4_Y_COUNT		0xFFC00D18	/* DMA Channel 4 Y Count Register */
#define DMA4_X_MODIFY		0xFFC00D14	/* DMA Channel 4 X Modify Register */
#define DMA4_Y_MODIFY		0xFFC00D1C	/* DMA Channel 4 Y Modify Register */
#define DMA4_CURR_DESC_PTR	0xFFC00D20	/* DMA Channel 4 Current Descriptor Pointer Register */
#define DMA4_CURR_ADDR		0xFFC00D24	/* DMA Channel 4 Current Address Register */
#define DMA4_CURR_X_COUNT	0xFFC00D30	/* DMA Channel 4 Current X Count Register */
#define DMA4_CURR_Y_COUNT	0xFFC00D38	/* DMA Channel 4 Current Y Count Register */
#define DMA4_IRQ_STATUS		0xFFC00D28	/* DMA Channel 4 Interrupt/Status Register */
#define DMA4_PERIPHERAL_MAP	0xFFC00D2C	/* DMA Channel 4 Peripheral Map Register */

#define DMA5_CONFIG			0xFFC00D48	/* DMA Channel 5 Configuration Register */
#define DMA5_NEXT_DESC_PTR	0xFFC00D40	/* DMA Channel 5 Next Descriptor Pointer Register */
#define DMA5_START_ADDR		0xFFC00D44	/* DMA Channel 5 Start Address Register */
#define DMA5_X_COUNT		0xFFC00D50	/* DMA Channel 5 X Count Register */
#define DMA5_Y_COUNT		0xFFC00D58	/* DMA Channel 5 Y Count Register */
#define DMA5_X_MODIFY		0xFFC00D54	/* DMA Channel 5 X Modify Register */
#define DMA5_Y_MODIFY		0xFFC00D5C	/* DMA Channel 5 Y Modify Register */
#define DMA5_CURR_DESC_PTR	0xFFC00D60	/* DMA Channel 5 Current Descriptor Pointer Register */
#define DMA5_CURR_ADDR		0xFFC00D64	/* DMA Channel 5 Current Address Register */
#define DMA5_CURR_X_COUNT	0xFFC00D70	/* DMA Channel 5 Current X Count Register */
#define DMA5_CURR_Y_COUNT	0xFFC00D78	/* DMA Channel 5 Current Y Count Register */
#define DMA5_IRQ_STATUS		0xFFC00D68	/* DMA Channel 5 Interrupt/Status Register */
#define DMA5_PERIPHERAL_MAP	0xFFC00D6C	/* DMA Channel 5 Peripheral Map Register */

#define DMA6_CONFIG			0xFFC00D88	/* DMA Channel 6 Configuration Register */
#define DMA6_NEXT_DESC_PTR	0xFFC00D80	/* DMA Channel 6 Next Descriptor Pointer Register */
#define DMA6_START_ADDR		0xFFC00D84	/* DMA Channel 6 Start Address Register */
#define DMA6_X_COUNT		0xFFC00D90	/* DMA Channel 6 X Count Register */
#define DMA6_Y_COUNT		0xFFC00D98	/* DMA Channel 6 Y Count Register */
#define DMA6_X_MODIFY		0xFFC00D94	/* DMA Channel 6 X Modify Register */
#define DMA6_Y_MODIFY		0xFFC00D9C	/* DMA Channel 6 Y Modify Register */
#define DMA6_CURR_DESC_PTR	0xFFC00DA0	/* DMA Channel 6 Current Descriptor Pointer Register */
#define DMA6_CURR_ADDR		0xFFC00DA4	/* DMA Channel 6 Current Address Register */
#define DMA6_CURR_X_COUNT	0xFFC00DB0	/* DMA Channel 6 Current X Count Register */
#define DMA6_CURR_Y_COUNT	0xFFC00DB8	/* DMA Channel 6 Current Y Count Register */
#define DMA6_IRQ_STATUS		0xFFC00DA8	/* DMA Channel 6 Interrupt/Status Register */
#define DMA6_PERIPHERAL_MAP	0xFFC00DAC	/* DMA Channel 6 Peripheral Map Register */

#define DMA7_CONFIG			0xFFC00DC8	/* DMA Channel 7 Configuration Register */
#define DMA7_NEXT_DESC_PTR	0xFFC00DC0	/* DMA Channel 7 Next Descriptor Pointer Register */
#define DMA7_START_ADDR		0xFFC00DC4	/* DMA Channel 7 Start Address Register */
#define DMA7_X_COUNT		0xFFC00DD0	/* DMA Channel 7 X Count Register */
#define DMA7_Y_COUNT		0xFFC00DD8	/* DMA Channel 7 Y Count Register */
#define DMA7_X_MODIFY		0xFFC00DD4	/* DMA Channel 7 X Modify Register */
#define DMA7_Y_MODIFY		0xFFC00DDC	/* DMA Channel 7 Y Modify Register */
#define DMA7_CURR_DESC_PTR	0xFFC00DE0	/* DMA Channel 7 Current Descriptor Pointer Register */
#define DMA7_CURR_ADDR		0xFFC00DE4	/* DMA Channel 7 Current Address Register */
#define DMA7_CURR_X_COUNT	0xFFC00DF0	/* DMA Channel 7 Current X Count Register */
#define DMA7_CURR_Y_COUNT	0xFFC00DF8	/* DMA Channel 7 Current Y Count Register */
#define DMA7_IRQ_STATUS		0xFFC00DE8	/* DMA Channel 7 Interrupt/Status Register */
#define DMA7_PERIPHERAL_MAP	0xFFC00DEC	/* DMA Channel 7 Peripheral Map Register */

#define MDMA_D1_CONFIG			0xFFC00E88	/* MemDMA Stream 1 Destination Configuration Register */
#define MDMA_D1_NEXT_DESC_PTR	0xFFC00E80	/* MemDMA Stream 1 Destination Next Descriptor Pointer Register */
#define MDMA_D1_START_ADDR		0xFFC00E84	/* MemDMA Stream 1 Destination Start Address Register */
#define MDMA_D1_X_COUNT			0xFFC00E90	/* MemDMA Stream 1 Destination X Count Register */
#define MDMA_D1_Y_COUNT			0xFFC00E98	/* MemDMA Stream 1 Destination Y Count Register */
#define MDMA_D1_X_MODIFY		0xFFC00E94	/* MemDMA Stream 1 Destination X Modify Register */
#define MDMA_D1_Y_MODIFY		0xFFC00E9C	/* MemDMA Stream 1 Destination Y Modify Register */
#define MDMA_D1_CURR_DESC_PTR	0xFFC00EA0	/* MemDMA Stream 1 Destination Current Descriptor Pointer Register */
#define MDMA_D1_CURR_ADDR		0xFFC00EA4	/* MemDMA Stream 1 Destination Current Address Register */
#define MDMA_D1_CURR_X_COUNT	0xFFC00EB0	/* MemDMA Stream 1 Destination Current X Count Register */
#define MDMA_D1_CURR_Y_COUNT	0xFFC00EB8	/* MemDMA Stream 1 Destination Current Y Count Register */
#define MDMA_D1_IRQ_STATUS		0xFFC00EA8	/* MemDMA Stream 1 Destination Interrupt/Status Register */
#define MDMA_D1_PERIPHERAL_MAP	0xFFC00EAC	/* MemDMA Stream 1 Destination Peripheral Map Register */

#define MDMA_S1_CONFIG			0xFFC00EC8	/* MemDMA Stream 1 Source Configuration Register */
#define MDMA_S1_NEXT_DESC_PTR	0xFFC00EC0	/* MemDMA Stream 1 Source Next Descriptor Pointer Register */
#define MDMA_S1_START_ADDR		0xFFC00EC4	/* MemDMA Stream 1 Source Start Address Register */
#define MDMA_S1_X_COUNT			0xFFC00ED0	/* MemDMA Stream 1 Source X Count Register */
#define MDMA_S1_Y_COUNT			0xFFC00ED8	/* MemDMA Stream 1 Source Y Count Register */
#define MDMA_S1_X_MODIFY		0xFFC00ED4	/* MemDMA Stream 1 Source X Modify Register */
#define MDMA_S1_Y_MODIFY		0xFFC00EDC	/* MemDMA Stream 1 Source Y Modify Register */
#define MDMA_S1_CURR_DESC_PTR	0xFFC00EE0	/* MemDMA Stream 1 Source Current Descriptor Pointer Register */
#define MDMA_S1_CURR_ADDR		0xFFC00EE4	/* MemDMA Stream 1 Source Current Address Register */
#define MDMA_S1_CURR_X_COUNT	0xFFC00EF0	/* MemDMA Stream 1 Source Current X Count Register */
#define MDMA_S1_CURR_Y_COUNT	0xFFC00EF8	/* MemDMA Stream 1 Source Current Y Count Register */
#define MDMA_S1_IRQ_STATUS		0xFFC00EE8	/* MemDMA Stream 1 Source Interrupt/Status Register */
#define MDMA_S1_PERIPHERAL_MAP	0xFFC00EEC	/* MemDMA Stream 1 Source Peripheral Map Register */

#define MDMA_D0_CONFIG			0xFFC00E08	/* MemDMA Stream 0 Destination Configuration Register */
#define MDMA_D0_NEXT_DESC_PTR	0xFFC00E00	/* MemDMA Stream 0 Destination Next Descriptor Pointer Register */
#define MDMA_D0_START_ADDR		0xFFC00E04	/* MemDMA Stream 0 Destination Start Address Register */
#define MDMA_D0_X_COUNT			0xFFC00E10	/* MemDMA Stream 0 Destination X Count Register */
#define MDMA_D0_Y_COUNT			0xFFC00E18	/* MemDMA Stream 0 Destination Y Count Register */
#define MDMA_D0_X_MODIFY		0xFFC00E14	/* MemDMA Stream 0 Destination X Modify Register */
#define MDMA_D0_Y_MODIFY		0xFFC00E1C	/* MemDMA Stream 0 Destination Y Modify Register */
#define MDMA_D0_CURR_DESC_PTR	0xFFC00E20	/* MemDMA Stream 0 Destination Current Descriptor Pointer Register */
#define MDMA_D0_CURR_ADDR		0xFFC00E24	/* MemDMA Stream 0 Destination Current Address Register */
#define MDMA_D0_CURR_X_COUNT	0xFFC00E30	/* MemDMA Stream 0 Destination Current X Count Register */
#define MDMA_D0_CURR_Y_COUNT	0xFFC00E38	/* MemDMA Stream 0 Destination Current Y Count Register */
#define MDMA_D0_IRQ_STATUS		0xFFC00E28	/* MemDMA Stream 0 Destination Interrupt/Status Register */
#define MDMA_D0_PERIPHERAL_MAP	0xFFC00E2C	/* MemDMA Stream 0 Destination Peripheral Map Register */

#define MDMA_S0_CONFIG			0xFFC00E48	/* MemDMA Stream 0 Source Configuration Register */
#define MDMA_S0_NEXT_DESC_PTR	0xFFC00E40	/* MemDMA Stream 0 Source Next Descriptor Pointer Register */
#define MDMA_S0_START_ADDR		0xFFC00E44	/* MemDMA Stream 0 Source Start Address Register */
#define MDMA_S0_X_COUNT			0xFFC00E50	/* MemDMA Stream 0 Source X Count Register */
#define MDMA_S0_Y_COUNT			0xFFC00E58	/* MemDMA Stream 0 Source Y Count Register */
#define MDMA_S0_X_MODIFY		0xFFC00E54	/* MemDMA Stream 0 Source X Modify Register */
#define MDMA_S0_Y_MODIFY		0xFFC00E5C	/* MemDMA Stream 0 Source Y Modify Register */
#define MDMA_S0_CURR_DESC_PTR	0xFFC00E60	/* MemDMA Stream 0 Source Current Descriptor Pointer Register */
#define MDMA_S0_CURR_ADDR		0xFFC00E64	/* MemDMA Stream 0 Source Current Address Register */
#define MDMA_S0_CURR_X_COUNT	0xFFC00E70	/* MemDMA Stream 0 Source Current X Count Register */
#define MDMA_S0_CURR_Y_COUNT	0xFFC00E78	/* MemDMA Stream 0 Source Current Y Count Register */
#define MDMA_S0_IRQ_STATUS		0xFFC00E68	/* MemDMA Stream 0 Source Interrupt/Status Register */
#define MDMA_S0_PERIPHERAL_MAP	0xFFC00E6C	/* MemDMA Stream 0 Source Peripheral Map Register */


/*// Parallel Peripheral Interface (PPI) (0xFFC01000 - 0xFFC010FF) */
#define PPI_CONTROL			0xFFC01000	/* PPI Control Register */
#define PPI_STATUS			0xFFC01004	/* PPI Status Register */
#define PPI_COUNT			0xFFC01008	/* PPI Transfer Count Register */
#define PPI_DELAY			0xFFC0100C	/* PPI Delay Count Register */
#define PPI_FRAME			0xFFC01010	/* PPI Frame Length Register */

/*********************************************************************************** */
/* System MMR Register Bits */
/******************************************************************************* */

/* ********************* PLL AND RESET MASKS ************************ */
/*// PLL_CTL Masks */
#define MSEL                    0x7E00              /* Multiplier Select For CCLK/VCO Factors */
#define SPORT_HYS               0x8000              /* Add 250mV of Hysteresis to SPORT Inputs */

/* PLL_STAT Masks */
#define VSTAT         0x0080  /* Voltage Regulator Status: Regulator at programmed voltage */
#define CORE_IDLE     0x0040  /* processor is in the IDLE operating mode */
#define SLEEP         0x0010  /* processor is in the Sleep operating mode */
#define DEEP_SLEEP    0x0008  /* processor is in the Deep Sleep operating mode */

#define PLL_CLKIN			0x0000  /* Pass CLKIN to PLL */
#define PLL_CLKIN_DIV2		0x0001  /* Pass CLKIN/2 to PLL */
#define DF					0x0001	 /* 0: PLL = CLKIN, 1: PLL = CLKIN/2					*/
#define PLL_OFF				0x0002  /* Shut off PLL clocks */
#define STOPCK_OFF			0x0008  /* Core clock off */
#define STOPCK				0x0008	/* Core Clock Off									*/
#define PDWN				0x0020  /* Put the PLL in a Deep Sleep state */

#if !defined(__ADSPBF538__)
/* this file is included in defBF538.h but IN_DELAY/OUT_DELAY are different */
# define IN_DELAY        0x0040  /* Add 200ps Delay To EBIU Input Latches */
# define OUT_DELAY       0x0080  /* Add 200ps Delay To EBIU Output Signals */
#endif

#define BYPASS				0x0100  /* Bypass the PLL */
/* PLL_CTL Macros											*/
#ifdef _MISRA_RULES
#define	SET_MSEL(x)		(((x)&0x3Fu) << 0x9)	/* Set MSEL = 0-63 --> VCO = CLKIN*MSEL		*/
#else
#define	SET_MSEL(x)		(((x)&0x3F) << 0x9)	/* Set MSEL = 0-63 --> VCO = CLKIN*MSEL		*/
#endif /* _MISRA_RULES */

/* PLL_DIV Masks */
#define SSEL				0x000F	/* System Select						*/
#define	CSEL				0x0030	/* Core Select							*/

#define SCLK_DIV(x)			(x)		/* SCLK = VCO / x */

#define CCLK_DIV1			0x0000  /* CCLK = VCO / 1 */
#define CCLK_DIV2			0x0010  /* CCLK = VCO / 2 */
#define CCLK_DIV4			0x0020  /* CCLK = VCO / 4 */
#define CCLK_DIV8			0x0030  /* CCLK = VCO / 8 */
/* PLL_DIV Macros														*/
#ifdef _MISRA_RULES
#define SET_SSEL(x)			((x)&0xFu)	/* Set SSEL = 0-15 --> SCLK = VCO/SSEL	*/
#else
#define SET_SSEL(x)			((x)&0xF)	/* Set SSEL = 0-15 --> SCLK = VCO/SSEL	*/
#endif /* _MISRA_RULES */

/* PLL_STAT Masks																	*/
#define ACTIVE_PLLENABLED	0x0001	/* Processor In Active Mode With PLL Enabled	*/
#define	FULL_ON				0x0002	/* Processor In Full On Mode					*/
#define ACTIVE_PLLDISABLED	0x0004	/* Processor In Active Mode With PLL Disabled	*/
#define	PLL_LOCKED			0x0020	/* PLL_LOCKCNT Has Been Reached					*/

/* VR_CTL Masks																	*/
#define	FREQ			0x0003	/* Switching Oscillator Frequency For Regulator	*/
#define	HIBERNATE		0x0000	/* 	Powerdown/Bypass On-Board Regulation	*/
#define	FREQ_333		0x0001	/* 	Switching Frequency Is 333 kHz			*/
#define	FREQ_667		0x0002	/* 	Switching Frequency Is 667 kHz			*/
#define	FREQ_1000		0x0003	/* 	Switching Frequency Is 1 MHz			*/

#define GAIN			0x000C	/* Voltage Level Gain	*/
#define	GAIN_5			0x0000	/* 	GAIN = 5		*/
#define	GAIN_10			0x0004	/* 	GAIN = 10		*/
#define	GAIN_20			0x0008	/* 	GAIN = 20		*/
#define	GAIN_50			0x000C	/* 	GAIN = 50		*/

#define	VLEV			0x00F0	/* Internal Voltage Level - Only Program Values Within Specifications	*/
#define	VLEV_085 		0x0060	/* 	VLEV = 0.85 V (See Datasheet for Regulator Tolerance)	*/
#define	VLEV_090		0x0070	/* 	VLEV = 0.90 V (See Datasheet for Regulator Tolerance)	*/
#define	VLEV_095		0x0080	/* 	VLEV = 0.95 V (See Datasheet for Regulator Tolerance)	*/
#define	VLEV_100		0x0090	/* 	VLEV = 1.00 V (See Datasheet for Regulator Tolerance)	*/
#define	VLEV_105		0x00A0	/* 	VLEV = 1.05 V (See Datasheet for Regulator Tolerance)	*/
#define	VLEV_110		0x00B0	/* 	VLEV = 1.10 V (See Datasheet for Regulator Tolerance)	*/
#define	VLEV_115		0x00C0	/* 	VLEV = 1.15 V (See Datasheet for Regulator Tolerance)	*/
#define	VLEV_120		0x00D0	/* 	VLEV = 1.20 V (See Datasheet for Regulator Tolerance)	*/
#define	VLEV_125		0x00E0	/* 	VLEV = 1.25 V (See Datasheet for Regulator Tolerance)	*/
#define	VLEV_130		0x00F0	/* 	VLEV = 1.30 V (See Datasheet for Regulator Tolerance)	*/

#define	WAKE			0x0100	/* Enable RTC/Reset Wakeup From Hibernate	*/
#define	SCKELOW			0x8000	/* Do Not Drive SCKE High During Reset After Hibernate */

/* SWRST Mask */
#define SYSTEM_RESET	0x0007	/* Initiates A System Software Reset			*/
#define	DOUBLE_FAULT	0x0008	/* Core Double Fault Causes Reset				*/
#define RESET_DOUBLE	0x2000	/* SW Reset Generated By Core Double-Fault		*/
#define RESET_WDOG		0x4000	/* SW Reset Generated By Watchdog Timer			*/
#define RESET_SOFTWARE	0x8000	/* SW Reset Occurred Since Last Read Of SWRST	*/

/* SYSCR Masks																				*/
#define BMODE_BYPASS  0x0000  /* Bypass boot ROM, execute from 16-bit external memory */
#define BMODE_FLASH   0x0002  /* Use Boot ROM to load from 8-bit or 16-bit flash */
#define BMODE_SPIHOST 0x0004  /* Boot from SPI0 host (slave mode) */
#define BMODE_SPIMEM  0x0006  /* Boot from serial SPI memory */
#define BMODE			0x0006	/* Boot Mode - Latched During HW Reset From Mode Pins	*/
#define	NOBOOT			0x0010	/* Execute From L1 or ASYNC Bank 0 When BMODE = 0		*/


/* *************  SYSTEM INTERRUPT CONTROLLER MASKS ***************** */

/* Peripheral Masks For SIC_ISR, SIC_IWR, SIC_IMASK */
#define PLL_WAKEUP_IRQ	0x00000001      /* PLL Wakeup Interrupt Request */
#define DMA_ERR_IRQ		0x00000002      /* DMA Controller Error Interrupt Request */
#define PPI_ERR_IRQ		0x00000004      /* PPI Error Interrupt Request */
#define SPORT0_ERR_IRQ	0x00000008      /* SPORT0 Error Interrupt Request */
#define SPORT1_ERR_IRQ	0x00000010      /* SPORT1 Error Interrupt Request */
#define SPI_ERR_IRQ		0x00000020      /* SPI Error Interrupt Request */
#define UART_ERR_IRQ	0x00000040      /* UART Error Interrupt Request */
#define RTC_IRQ			0x00000080      /* Real-Time Clock Interrupt Request */
#define DMA0_IRQ		0x00000100      /* DMA Channel 0 (PPI) Interrupt Request */
#define DMA1_IRQ		0x00000200      /* DMA Channel 1 (SPORT0 RX) Interrupt Request */
#define DMA2_IRQ		0x00000400      /* DMA Channel 2 (SPORT0 TX) Interrupt Request */
#define DMA3_IRQ		0x00000800      /* DMA Channel 3 (SPORT1 RX) Interrupt Request */
#define DMA4_IRQ		0x00001000      /* DMA Channel 4 (SPORT1 TX) Interrupt Request */
#define DMA5_IRQ		0x00002000      /* DMA Channel 5 (SPI) Interrupt Request */
#define DMA6_IRQ		0x00004000      /* DMA Channel 6 (UART RX) Interrupt Request */
#define DMA7_IRQ		0x00008000      /* DMA Channel 7 (UART TX) Interrupt Request */
#define TIMER0_IRQ		0x00010000      /* Timer 0 Interrupt Request */
#define TIMER1_IRQ		0x00020000      /* Timer 1 Interrupt Request */
#define TIMER2_IRQ		0x00040000      /* Timer 2 Interrupt Request */
#define PFA_IRQ			0x00080000      /* Programmable Flag Interrupt Request A */
#define PFB_IRQ			0x00100000      /* Programmable Flag Interrupt Request B */
#define MDMA0_IRQ		0x00200000      /* MemDMA Stream 0 Interrupt Request */
#define MDMA1_IRQ		0x00400000      /* MemDMA Stream 1 Interrupt Request */
#define WDOG_IRQ		0x00800000      /* Software Watchdog Timer Interrupt Request */

#ifdef _MISRA_RULES
#define _MF15 0xFu
#define _MF7 7u
#else
#define _MF15 0xF
#define _MF7 7
#endif /* _MISRA_RULES */

/* SIC_IAR0 Macros															*/
#define P0_IVG(x)		(((x)-_MF7)&_MF15)			/* Peripheral #0 assigned IVG #x	*/
#define P1_IVG(x)		(((x)-_MF7)&_MF15) << 0x4	/* Peripheral #1 assigned IVG #x	*/
#define P2_IVG(x)		(((x)-_MF7)&_MF15) << 0x8	/* Peripheral #2 assigned IVG #x	*/
#define P3_IVG(x)		(((x)-_MF7)&_MF15) << 0xC	/* Peripheral #3 assigned IVG #x	*/
#define P4_IVG(x)		(((x)-_MF7)&_MF15) << 0x10	/* Peripheral #4 assigned IVG #x	*/
#define P5_IVG(x)		(((x)-_MF7)&_MF15) << 0x14	/* Peripheral #5 assigned IVG #x	*/
#define P6_IVG(x)		(((x)-_MF7)&_MF15) << 0x18	/* Peripheral #6 assigned IVG #x	*/
#define P7_IVG(x)		(((x)-_MF7)&_MF15) << 0x1C	/* Peripheral #7 assigned IVG #x	*/

/* SIC_IAR1 Macros															*/
#define P8_IVG(x)		(((x)-_MF7)&_MF15)			/* Peripheral #8 assigned IVG #x	*/
#define P9_IVG(x)		(((x)-_MF7)&_MF15) << 0x4	/* Peripheral #9 assigned IVG #x	*/
#define P10_IVG(x)		(((x)-_MF7)&_MF15) << 0x8	/* Peripheral #10 assigned IVG #x	*/
#define P11_IVG(x)		(((x)-_MF7)&_MF15) << 0xC	/* Peripheral #11 assigned IVG #x	*/
#define P12_IVG(x)		(((x)-_MF7)&_MF15) << 0x10	/* Peripheral #12 assigned IVG #x	*/
#define P13_IVG(x)		(((x)-_MF7)&_MF15) << 0x14	/* Peripheral #13 assigned IVG #x	*/
#define P14_IVG(x)		(((x)-_MF7)&_MF15) << 0x18	/* Peripheral #14 assigned IVG #x	*/
#define P15_IVG(x)		(((x)-_MF7)&_MF15) << 0x1C	/* Peripheral #15 assigned IVG #x	*/

/* SIC_IAR2 Macros															*/
#define P16_IVG(x)		(((x)-_MF7)&_MF15)			/* Peripheral #16 assigned IVG #x	*/
#define P17_IVG(x)		(((x)-_MF7)&_MF15) << 0x4	/* Peripheral #17 assigned IVG #x	*/
#define P18_IVG(x)		(((x)-_MF7)&_MF15) << 0x8	/* Peripheral #18 assigned IVG #x	*/
#define P19_IVG(x)		(((x)-_MF7)&_MF15) << 0xC	/* Peripheral #19 assigned IVG #x	*/
#define P20_IVG(x)		(((x)-_MF7)&_MF15) << 0x10	/* Peripheral #20 assigned IVG #x	*/
#define P21_IVG(x)		(((x)-_MF7)&_MF15) << 0x14	/* Peripheral #21 assigned IVG #x	*/
#define P22_IVG(x)		(((x)-_MF7)&_MF15) << 0x18	/* Peripheral #22 assigned IVG #x	*/
#define P23_IVG(x)		(((x)-_MF7)&_MF15) << 0x1C	/* Peripheral #23 assigned IVG #x	*/

/* SIC_IARx Macros */
#ifdef _MISRA_RULES
#define PX_IVG_CLR(x)  (0xFFFFFFFFu ^ (0xFu << (((x)%8)*4)))  /* Clear IVG Select for Peripheral #x */
/* Usage: *pSIC_IAR1 &= PX_IVG_CLR(11);	// Clears IVG Level of Peripheral #11 */
#define PX_IVG(x,y)     ((((y)-7u)&0xFu) << (((x)%8)*4))      /* Set IVG Select to #y for Peripheral #x */
/* Usage: *pSIC_IAR1 |= PX_IVG(11, 8);	// Sets Peripheral #11 to IVG8 */
#else
#define PX_IVG_CLR(x)  (0xFFFFFFFF ^ (0xF << (((x)%8)*4)))  /* Clear IVG Select for Peripheral #x */
/* Usage: *pSIC_IAR1 &= PX_IVG_CLR(11);	// Clears IVG Level of Peripheral #11 */
#define PX_IVG(x,y)     ((((y)-7)&0xF) << (((x)%8)*4))      /* Set IVG Select to #y for Peripheral #x */
/* Usage: *pSIC_IAR1 |= PX_IVG(11, 8);	// Sets Peripheral #11 to IVG8 */
#endif /* _MISRA_RULES */

/* SIC_IMASK Masks*/
#define SIC_UNMASK_ALL 0x00000000         /* Unmask all peripheral interrupts */
#define SIC_MASK_ALL   0xFFFFFFFF         /* Mask all peripheral interrupts   */
#ifdef _MISRA_RULES
#define SIC_MASK(x) ((int32_t)1 << ((x)&0x1Fu))  /* Mask Peripheral #x interrupt  */
#define SIC_UNMASK(x)  (0xFFFFFFFFu ^ ((uint32_t)1 << ((x)&0x1Fu)))  /*Unmask Peripheral #x interrupt*/
#else
#define SIC_MASK(x) (1 << ((x)&0x1F))     /* Mask Peripheral #x interrupt  */
#define SIC_UNMASK(x)  (0xFFFFFFFF ^ (1 << ((x)&0x1F)))  /* Unmask Peripheral #x interrupt */
#endif /* _MISRA_RULES */

/* SIC_IWR Masks*/
#define IWR_DISABLE_ALL 0x00000000        /* Wakeup Disable all peripherals   */
#define IWR_ENABLE_ALL  0xFFFFFFFF        /* Wakeup Enable all peripherals    */
#ifdef _MISRA_RULES
#define IWR_ENABLE(x)   ((int32_t)1 << ((x)&0x1Fu))  /* Wakeup Enable Peripheral #x   */
#define IWR_DISABLE(x)  (0xFFFFFFFFu ^ ((uint32_t)1 << ((x)&0x1Fu)))  /*Wakeup Disable Peripheral #x */
#else
#define IWR_ENABLE(x)   (1 << ((x)&0x1F)) /* Wakeup Enable Peripheral #x   */
#define IWR_DISABLE(x)  (0xFFFFFFFF ^ (1 << ((x)&0x1F)))  /* Wakeup Disable Peripheral #x  */
#endif /* _MISRA_RULES */


/* ********* WATCHDOG TIMER MASKS ******************** */

/* Watchdog Timer WDOG_CTL Register Masks */

#ifdef _MISRA_RULES
#define WDEV(x)			(((x)<<1) & 0x0006u) 	/* event generated on roll over */
#else
#define WDEV(x)			(((x)<<1) & 0x0006) 	/* event generated on roll over */
#endif /* _MISRA_RULES */
#define WDEV_RESET		0x0000 				/* generate reset event on roll over */
#define WDEV_NMI		0x0002 				/* generate NMI event on roll over */
#define WDEV_GPI		0x0004 				/* generate GP IRQ on roll over */
#define WDEV_NONE		0x0006 				/* no event on roll over */
#define WDEN			0x0FF0 				/* enable watchdog */
#define WDDIS			0x0AD0 				/* disable watchdog */
#define WDRO			0x8000 				/* watchdog rolled over latch */

/* depreciated WDOG_CTL Register Masks for legacy code */
#define ICTL WDEV
#define ENABLE_RESET 	WDEV_RESET
#define WDOG_RESET 		WDEV_RESET
#define ENABLE_NMI 		WDEV_NMI
#define WDOG_NMI 		WDEV_NMI
#define ENABLE_GPI 		WDEV_GPI
#define WDOG_GPI 		WDEV_GPI
#define DISABLE_EVT 	WDEV_NONE
#define WDOG_NONE 		WDEV_NONE

#define TMR_EN 			WDEN
#define WDOG_DISABLE	WDDIS
#define TRO 			WDRO

#define ICTL_P0			0x01
#define ICTL_P1			0x02
#define TRO_P			0x0F


/* ***************  REAL TIME CLOCK MASKS  **************************/
/* RTC_STAT and RTC_ALARM register */
#define	RTSEC		0x0000003F	/* Real-Time Clock Seconds */
#define	RTMIN		0x00000FC0	/* Real-Time Clock Minutes */
#define	RTHR		0x0001F000	/* Real-Time Clock Hours */
#define	RTDAY		0xFFFE0000	/* Real-Time Clock Days */

/* RTC_ICTL register */
#define	SWIE		0x0001		/* Stopwatch Interrupt Enable */
#define	AIE			0x0002		/* Alarm Interrupt Enable */
#define	SIE			0x0004		/* Seconds (1 Hz) Interrupt Enable */
#define	MIE			0x0008		/* Minutes Interrupt Enable */
#define	HIE			0x0010		/* Hours Interrupt Enable */
#define	DIE			0x0020		/* 24 Hours (Days) Interrupt Enable */
#define	DAIE		0x0040		/* Day Alarm (Day, Hour, Minute, Second) Interrupt Enable */
#define	WCIE		0x8000		/* Write Complete Interrupt Enable */

/* RTC_ISTAT register */
#define	SWEF		0x0001		/* Stopwatch Event Flag */
#define	AEF			0x0002		/* Alarm Event Flag */
#define	SEF			0x0004		/* Seconds (1 Hz) Event Flag */
#define	MEF			0x0008		/* Minutes Event Flag */
#define	HEF			0x0010		/* Hours Event Flag */
#define	DEF			0x0020		/* 24 Hours (Days) Event Flag */
#define	DAEF		0x0040		/* Day Alarm (Day, Hour, Minute, Second) Event Flag */
#define	WPS			0x4000		/* Write Pending Status (RO) */
#define	WCOM		0x8000		/* Write Complete */

/*// RTC_FAST Mask (RTC_PREN Mask) */
#define ENABLE_PRESCALE      0x00000001  /* Enable prescaler so RTC runs at 1 Hz */
#define PREN                 0x00000001
         	/* ** Must be set after power-up for proper operation of RTC */

/* RTC_ALARM Macro			z=day		y=hr	x=min	w=sec		*/
#ifdef _MISRA_RULES
#define SET_ALARM(z,y,x,w)	((((z)&0x7FFFu)<<0x11)|(((y)&0x1Fu)<<0xC)|(((x)&0x3Fu)<<0x6)|((w)&0x3Fu))
#else
#define SET_ALARM(z,y,x,w)	((((z)&0x7FFF)<<0x11)|(((y)&0x1F)<<0xC)|(((x)&0x3F)<<0x6)|((w)&0x3F))
#endif /* _MISRA_RULES */

/* Deprecated RTC_STAT and RTC_ALARM Masks										*/
#define	RTC_SEC			RTSEC	/* Real-Time Clock Seconds	*/
#define	RTC_MIN			RTMIN	/* Real-Time Clock Minutes	*/
#define	RTC_HR			RTHR	/* Real-Time Clock Hours	*/
#define	RTC_DAY			RTDAY	/* Real-Time Clock Days		*/

/* Deprecated RTC_ICTL/RTC_ISTAT Masks																		*/
#define	STOPWATCH		SWIE		/* Stopwatch Interrupt Enable								*/
#define	ALARM			AIE		/* Alarm Interrupt Enable									*/
#define	SECOND			SIE		/* Seconds (1 Hz) Interrupt Enable							*/
#define	MINUTE			MIE		/* Minutes Interrupt Enable									*/
#define	HOUR			HIE		/* Hours Interrupt Enable									*/
#define	DAY				DIE		/* 24 Hours (Days) Interrupt Enable							*/
#define	DAY_ALARM		DAIE		/* Day Alarm (Day, Hour, Minute, Second) Interrupt Enable	*/
#define	WRITE_COMPLETE	WCIE		/* Write Complete Interrupt Enable							*/


/* ***************************** UART CONTROLLER MASKS ********************** */
/* UART_LCR Register */

#ifdef _MISRA_RULES
#define WLS(x)		(((x)-5u) & 0x03u)	/* Word Length Select */
#else
#define WLS(x)		(((x)-5) & 0x03)	/* Word Length Select */
#endif /* _MISRA_RULES */
#define STB			0x04				/* Stop Bits			*/
#define PEN			0x08				/* Parity Enable		*/
#define EPS			0x10				/* Even Parity Select	*/
#define STP			0x20				/* Stick Parity			*/
#define SB			0x40				/* Set Break			*/
#define DLAB		0x80				/* Divisor Latch Access	*/

#define DLAB_P		0x07
#define SB_P		0x06
#define STP_P		0x05
#define EPS_P		0x04
#define PEN_P		0x03
#define STB_P		0x02
#define WLS_P1		0x01
#define WLS_P0		0x00

/* UART_MCR Register */
#define LOOP_ENA	0x10		/* Loopback Mode Enable    */
#define LOOP_ENA_P	0x04
/* Deprecated UARTx_MCR Mask										*/

/* UART_LSR Register */
#define DR			0x01	/* Data Ready				*/
#define OE			0x02	/* Overrun Error			*/
#define PE			0x04	/* Parity Error				*/
#define FE			0x08	/* Framing Error			*/
#define BI			0x10	/* Break Interrupt			*/
#define THRE		0x20	/* THR Empty				*/
#define TEMT		0x40	/* TSR and UART_THR Empty	*/

#define TEMP_P		0x06
#define THRE_P		0x05
#define BI_P		0x04
#define FE_P		0x03
#define PE_P		0x02
#define OE_P		0x01
#define DR_P		0x00

/* UART_IER Register */
#define ERBFI		0x01		/* Enable Receive Buffer Full Interrupt		*/
#define ETBEI		0x02		/* Enable Transmit Buffer Empty Interrupt	*/
#define ELSI		0x04		/* Enable RX Status Interrupt				*/

#define ELSI_P		0x02
#define ETBEI_P		0x01
#define ERBFI_P		0x00

/* UART_IIR Register */
#ifdef _MISRA_RULES
#define STATUS(x)	(((x) << 1) & 0x06u)
#else
#define STATUS(x)	(((x) << 1) & 0x06)
#endif /* _MISRA_RULES */
#define NINT		0x01
#define STATUS_P1	0x02
#define STATUS_P0	0x01
#define NINT_P		0x00

/* UART_GCTL Register */
#define UCEN		0x01		/* Enable UARTx Clocks				*/
#define IREN		0x02		/* Enable IrDA Mode					*/
#define TPOLC		0x04		/* IrDA TX Polarity Change			*/
#define RPOLC		0x08		/* IrDA RX Polarity Change			*/
#define FPE			0x10		/* Force Parity Error On Transmit	*/
#define FFE			0x20		/* Force Framing Error On Transmit	*/

#define FFE_P		0x05
#define FPE_P		0x04
#define RPOLC_P		0x03
#define TPOLC_P		0x02
#define IREN_P		0x01
#define UCEN_P		0x00


/* **********  SERIAL PORT MASKS  ********************** */
/* SPORTx_TCR1 Masks */
#define TSPEN		0x0001  /* TX enable  */
#define ITCLK		0x0002  /* Internal TX Clock Select  */
#define TDTYPE		0x000C  /* TX Data Formatting Select */
#define DTYPE_NORM	0x0000		/* Data Format Normal							*/
#define DTYPE_ULAW	0x0008		/* Compand Using u-Law							*/
#define DTYPE_ALAW	0x000C		/* Compand Using A-Law							*/
#define TLSBIT		0x0010  /* TX Bit Order */
#define ITFS		0x0200  /* Internal TX Frame Sync Select  */
#define TFSR		0x0400  /* TX Frame Sync Required Select  */
#define DITFS		0x0800  /* Data Independent TX Frame Sync Select  */
#define LTFS		0x1000  /* Low TX Frame Sync Select  */
#define LATFS		0x2000  /* Late TX Frame Sync Select  */
#define TCKFE		0x4000  /* TX Clock Falling Edge Select  */
/* SPORTx_RCR1 Deprecated Masks															*/
#define TULAW		DTYPE_ULAW		/* Compand Using u-Law							*/
#define TALAW		DTYPE_ALAW			/* Compand Using A-Law							*/

/* SPORTx_TCR2 Masks */
#if defined(__ADSPBF531__) || defined(__ADSPBF532__) || \
    defined(__ADSPBF533__)
# define SLEN		0x001F
#else
#ifdef _MISRA_RULES
# define SLEN(x)		((x)&0x1Fu)	/* SPORT TX Word Length (2 - 31)				*/
#else
# define SLEN(x)		((x)&0x1F)	/* SPORT TX Word Length (2 - 31)				*/
#endif /* _MISRA_RULES */
#endif
#define TXSE		0x0100  /*TX Secondary Enable */
#define TSFSE		0x0200  /*TX Stereo Frame Sync Enable */
#define TRFST		0x0400  /*TX Right-First Data Order  */

/* SPORTx_RCR1 Masks */
#define RSPEN		0x0001  /* RX enable  */
#define IRCLK		0x0002  /* Internal RX Clock Select  */
#define RDTYPE		0x000C  /* RX Data Formatting Select */
#define DTYPE_NORM	0x0000		/* no companding							*/
#define DTYPE_ULAW	0x0008		/* Compand Using u-Law							*/
#define DTYPE_ALAW	0x000C		/* Compand Using A-Law							*/
#define RLSBIT		0x0010  /* RX Bit Order */
#define IRFS		0x0200  /* Internal RX Frame Sync Select  */
#define RFSR		0x0400  /* RX Frame Sync Required Select  */
#define LRFS		0x1000  /* Low RX Frame Sync Select  */
#define LARFS		0x2000  /* Late RX Frame Sync Select  */
#define RCKFE		0x4000  /* RX Clock Falling Edge Select  */
/* SPORTx_RCR1 Deprecated Masks															*/
#define RULAW		DTYPE_ULAW		/* Compand Using u-Law							*/
#define RALAW		DTYPE_ALAW			/* Compand Using A-Law							*/

/* SPORTx_RCR2 Masks */
/* SLEN defined above */
#define RXSE		0x0100  /*RX Secondary Enable */
#define RSFSE		0x0200  /*RX Stereo Frame Sync Enable */
#define RRFST		0x0400  /*Right-First Data Order  */

/*SPORTx_STAT Masks */
#define RXNE		0x0001		/*RX FIFO Not Empty Status */
#define RUVF		0x0002  	/*RX Underflow Status */
#define ROVF		0x0004		/*RX Overflow Status */
#define TXF			0x0008		/*TX FIFO Full Status */
#define TUVF		0x0010  	/*TX Underflow Status */
#define TOVF		0x0020  	/*TX Overflow Status */
#define TXHRE		0x0040  	/*TX Hold Register Empty */

/*SPORTx_MCMC1 Masks */
#define WSIZE			0x0000F000 	/*Multichannel Window Size Field */
#define WOFF			0x000003FF 	/*Multichannel Window Offset Field */
/* SPORTx_MCMC1 Macros															*/
#ifdef _MISRA_RULES
#define SET_WOFF(x)		((x) & 0x3FFu) 	/* Multichannel Window Offset Field			*/
/* Only use SET_WSIZE Macro With Logic OR While Setting Lower Order Bits						*/
#define SET_WSIZE(x)	(((((x)>>0x3)-1u)&0xFu) << 0xC)	/* Multichannel Window Size = (x/8)-1	*/
#else
#define SET_WOFF(x)		((x) & 0x3FF) 	/* Multichannel Window Offset Field			*/
/* Only use SET_WSIZE Macro With Logic OR While Setting Lower Order Bits						*/
#define SET_WSIZE(x)	(((((x)>>0x3)-1)&0xF) << 0xC)	/* Multichannel Window Size = (x/8)-1	*/
#endif /* _MISRA_RULES */

/*SPORTx_MCMC2 Masks */
#define MCCRM		0x0003 	/*Multichannel Clock Recovery Mode */
#define REC_BYPASS	0x0000		/* Bypass Mode (No Clock Recovery)				*/
#define REC_2FROM4	0x0002		/* Recover 2 MHz Clock from 4 MHz Clock			*/
#define REC_8FROM16	0x0003		/* Recover 8 MHz Clock from 16 MHz Clock		*/
#define MCDTXPE		0x0004 	/*Multichannel DMA Transmit Packing */
#define MCDRXPE		0x0008 	/*Multichannel DMA Receive Packing */
#define MCMEN		0x0010 	/*Multichannel Frame Mode Enable */
#define FSDR		0x0080 	/*Multichannel Frame Sync to Data Relationship */
#define MFD			0xF000 	/*Multichannel Frame Delay    */
#define MFD_0		0x0000		/* Multichannel Frame Delay = 0					*/
#define MFD_1		0x1000		/* Multichannel Frame Delay = 1					*/
#define MFD_2		0x2000		/* Multichannel Frame Delay = 2					*/
#define MFD_3		0x3000		/* Multichannel Frame Delay = 3					*/
#define MFD_4		0x4000		/* Multichannel Frame Delay = 4					*/
#define MFD_5		0x5000		/* Multichannel Frame Delay = 5					*/
#define MFD_6		0x6000		/* Multichannel Frame Delay = 6					*/
#define MFD_7		0x7000		/* Multichannel Frame Delay = 7					*/
#define MFD_8		0x8000		/* Multichannel Frame Delay = 8					*/
#define MFD_9		0x9000		/* Multichannel Frame Delay = 9					*/
#define MFD_10		0xA000		/* Multichannel Frame Delay = 10				*/
#define MFD_11		0xB000		/* Multichannel Frame Delay = 11				*/
#define MFD_12		0xC000		/* Multichannel Frame Delay = 12				*/
#define MFD_13		0xD000		/* Multichannel Frame Delay = 13				*/
#define MFD_14		0xE000		/* Multichannel Frame Delay = 14				*/
#define MFD_15		0xF000		/* Multichannel Frame Delay = 15				*/


/*  *********  PARALLEL PERIPHERAL INTERFACE (PPI) MASKS ****************   */
/*//  PPI_CONTROL Masks         */
#define PORT_EN		0x0001  /* PPI Port Enable  */
#define PORT_DIR	0x0002  /* PPI Port Direction       */
#define XFR_TYPE	0x000C  /* PPI Transfer Type  */
#define PORT_CFG	0x0030  /* PPI Port Configuration */
#define FLD_SEL		0x0040  /* PPI Active Field Select */
#define PACK_EN		0x0080  /* PPI Packing Mode */
/* previous versions of defBF532.h erroneously included DMA32 (PPI 32-bit DMA Enable) */
#define ALT_TIMING	0x0100  /* Enable Alternate PPI Timing (0.5 Silicon And Beyond) */
#define SKIP_EN		0x0200  /* PPI Skip Element Enable */
#define SKIP_EO		0x0400  /* PPI Skip Even/Odd Elements */
#define DLENGTH		0x3800  /* PPI Data Length  */
#define DLEN_8		0x0	     /* PPI Data Length mask for DLEN=8 */
#define DLEN_10		0x0800		/* Data Length = 10 Bits			*/
#define DLEN_11		0x1000		/* Data Length = 11 Bits			*/
#define DLEN_12		0x1800		/* Data Length = 12 Bits			*/
#define DLEN_13		0x2000		/* Data Length = 13 Bits			*/
#define DLEN_14		0x2800		/* Data Length = 14 Bits			*/
#define DLEN_15		0x3000		/* Data Length = 15 Bits			*/
#define DLEN_16		0x3800		/* Data Length = 16 Bits			*/
#ifdef _MISRA_RULES
#define DLEN(x)		((((x)-9u) & 0x07u) << 11)  /* PPI Data Length (only works for x=10-->x=16) */
#else
#define DLEN(x)		((((x)-9) & 0x07) << 11)  /* PPI Data Length (only works for x=10-->x=16) */
#endif /* _MISRA_RULES */
#define POL			0xC000  /* PPI Signal Polarities       */
#define POLC		0x4000		/* PPI Clock Polarity				*/
#define POLS		0x8000		/* PPI Frame Sync Polarity			*/


/*// PPI_STATUS Masks                                          */
#define FLD			0x0400  /* Field Indicator   */
#define FT_ERR		0x0800  /* Frame Track Error */
#define OVR			0x1000  /* FIFO Overflow Error */
#define UNDR		0x2000  /* FIFO Underrun Error */
#define ERR_DET		0x4000  /* Error Detected Indicator */
#define ERR_NCOR	0x8000  /* Error Not Corrected Indicator */


/* **********  DMA CONTROLLER MASKS  ***********************/
/*//DMAx_CONFIG, MDMA_yy_CONFIG Masks */
#define DMAEN		0x0001  /* Channel Enable */
#define WNR			0x0002  /* Channel Direction (W/R*) */
#define WDSIZE_8	0x0000  /* Word Size 8 bits */
#define WDSIZE_16	0x0004  /* Word Size 16 bits */
#define WDSIZE_32	0x0008  /* Word Size 32 bits */
#define DMA2D		0x0010  /* 2D/1D* Mode */
#define RESTART		0x0020  /* Restart */
#define DI_SEL		0x0040  /* Data Interrupt Select */
#define DI_EN		0x0080  /* Data Interrupt Enable */
#define NDSIZE		0x0900  /* Next Descriptor Size */
#define NDSIZE_0	0x0000	/* Next Descriptor Size = 0 (Stop/Autobuffer)	*/
#define NDSIZE_1	0x0100	/* Next Descriptor Size = 1						*/
#define NDSIZE_2	0x0200	/* Next Descriptor Size = 2						*/
#define NDSIZE_3	0x0300	/* Next Descriptor Size = 3						*/
#define NDSIZE_4	0x0400	/* Next Descriptor Size = 4						*/
#define NDSIZE_5	0x0500	/* Next Descriptor Size = 5						*/
#define NDSIZE_6	0x0600	/* Next Descriptor Size = 6						*/
#define NDSIZE_7	0x0700	/* Next Descriptor Size = 7						*/
#define NDSIZE_8	0x0800	/* Next Descriptor Size = 8						*/
#define NDSIZE_9	0x0900	/* Next Descriptor Size = 9						*/
#define FLOW		0x7000  /* Flow Control */
#define FLOW_STOP	0x0000	/* Stop Mode									*/
#define FLOW_AUTO	0x1000	/* Autobuffer Mode								*/
#define FLOW_ARRAY	0x4000	/* Descriptor Array Mode						*/
#define FLOW_SMALL	0x6000	/* Small Model Descriptor List Mode				*/
#define FLOW_LARGE	0x7000	/* Large Model Descriptor List Mode				*/

#define DMAEN_P		0x0		/* Channel Enable */
#define WNR_P		0x1		/* Channel Direction (W/R*) */
#define DMA2D_P		0x4		/* 2D/1D* Mode */
#define RESTART_P	0x5		/* Restart */
#define DI_SEL_P	0x6		/* Data Interrupt Select */
#define DI_EN_P		0x7		/* Data Interrupt Enable */

/*//DMAx_IRQ_STATUS, MDMA_yy_IRQ_STATUS Masks */
#define DMA_DONE	0x0001	/* DMA Done Indicator */
#define DMA_ERR		0x0002	/* DMA Error Indicator */
#define DFETCH		0x0004	/* Descriptor Fetch Indicator */
#define DMA_RUN		0x0008	/* DMA Running Indicator */

#define DMA_DONE_P	0x0		/* DMA Done Indicator */
#define DMA_ERR_P	0x1		/* DMA Error Indicator */
#define DFETCH_P	0x2		/* Descriptor Fetch Indicator */
#define DMA_RUN_P	0x3		/* DMA Running Indicator */

/*//DMAx_PERIPHERAL_MAP, MDMA_yy_PERIPHERAL_MAP Masks */

#define CTYPE			0x0040	/* DMA Channel Type Indicator */
#define CTYPE_P			0x6		/* DMA Channel Type Indicator BIT POSITION */
#define PCAP8			0x0080	/* DMA 8-bit Operation Indicator   */
#define PCAP16			0x0100	/* DMA 16-bit Operation Indicator */
#define PCAP32			0x0200	/* DMA 32-bit Operation Indicator */
#define PCAPWR			0x0400	/* DMA Write Operation Indicator */
#define PCAPRD			0x0800	/* DMA Read Operation Indicator */
#define PMAP			0xF000	/* DMA Peripheral Map Field */

#define PMAP_PPI		0x0000	/* PMAP PPI Port DMA */
#define	PMAP_SPORT0RX	0x1000	/* PMAP SPORT0 Receive DMA */
#define PMAP_SPORT0TX	0x2000	/* PMAP SPORT0 Transmit DMA */
#define	PMAP_SPORT1RX	0x3000	/* PMAP SPORT1 Receive DMA */
#define PMAP_SPORT1TX	0x4000	/* PMAP SPORT1 Transmit DMA */
#define PMAP_SPI		0x5000	/* PMAP SPI DMA */
#define PMAP_UARTRX		0x6000	/* PMAP UART Receive DMA */
#define PMAP_UARTTX		0x7000	/* PMAP UART Transmit DMA */


/*  *************  GENERAL PURPOSE TIMER MASKS  ******************** */
/* PWM Timer bit definitions */
/* TIMER_ENABLE Register */
#define TIMEN0			0x0001	/* Enable Timer 0	*/
#define TIMEN1			0x0002	/* Enable Timer 1	*/
#define TIMEN2			0x0004	/* Enable Timer 2	*/

#define TIMEN0_P		0x00
#define TIMEN1_P		0x01
#define TIMEN2_P		0x02

/* TIMER_DISABLE Register */
#define TIMDIS0			0x0001	/* Disable Timer 0	*/
#define TIMDIS1			0x0002	/* Disable Timer 1	*/
#define TIMDIS2			0x0004	/* Disable Timer 2	*/

#define TIMDIS0_P		0x00
#define TIMDIS1_P		0x01
#define TIMDIS2_P		0x02

/* TIMER_STATUS Register */
#define TIMIL0			0x0001	/* Timer 0 Interrupt			*/
#define TIMIL1			0x0002	/* Timer 1 Interrupt			*/
#define TIMIL2			0x0004	/* Timer 2 Interrupt			*/
#define TOVF_ERR0		0x0010	/* Timer 0 Counter Overflow		*/
#define TOVF_ERR1		0x0020	/* Timer 1 Counter Overflow		*/
#define TOVF_ERR2		0x0040	/* Timer 2 Counter Overflow		*/
#define TRUN0			0x1000	/* Timer 0 Slave Enable Status	*/
#define TRUN1			0x2000	/* Timer 1 Slave Enable Status	*/
#define TRUN2			0x4000	/* Timer 2 Slave Enable Status	*/

#define TIMIL0_P		0x00
#define TIMIL1_P		0x01
#define TIMIL2_P		0x02
#define TOVF_ERR0_P		0x04
#define TOVF_ERR1_P		0x05
#define TOVF_ERR2_P		0x06
#define TRUN0_P			0x0C
#define TRUN1_P			0x0D
#define TRUN2_P			0x0E

/* Alternate Deprecated Macros Provided For Backwards Code Compatibility */
#define TOVL_ERR0 		TOVF_ERR0
#define TOVL_ERR1 		TOVF_ERR1
#define TOVL_ERR2 		TOVF_ERR2
#define TOVL_ERR0_P		TOVF_ERR0_P
#define TOVL_ERR1_P 	TOVF_ERR1_P
#define TOVL_ERR2_P 	TOVF_ERR2_P

/* TIMERx_CONFIG Registers */
#define PWM_OUT			0x0001
#define WDTH_CAP		0x0002
#define EXT_CLK			0x0003
#define PULSE_HI		0x0004
#define PERIOD_CNT		0x0008
#define IRQ_ENA			0x0010
#define TIN_SEL			0x0020
#define OUT_DIS			0x0040
#define CLK_SEL			0x0080
#define TOGGLE_HI		0x0100
#define EMU_RUN			0x0200
#ifdef _MISRA_RULES
#define ERR_TYP(x)		(((x) & 0x03u) << 14)
#else
#define ERR_TYP(x)		(((x) & 0x03) << 14)
#endif /* _MISRA_RULES */

#define TMODE_P0		0x00
#define TMODE_P1		0x01
#define PULSE_HI_P		0x02
#define PERIOD_CNT_P	0x03
#define IRQ_ENA_P		0x04
#define TIN_SEL_P		0x05
#define OUT_DIS_P		0x06
#define CLK_SEL_P		0x07
#define TOGGLE_HI_P		0x08
#define EMU_RUN_P		0x09
#define ERR_TYP_P0		0x0E
#define ERR_TYP_P1		0x0F


/*/ ******************   GENERAL-PURPOSE I/O  ********************* */
/*  Port F (Previously Flag I/O_ Masks */
#define PF0			0x0001
#define PF1			0x0002
#define PF2			0x0004
#define PF3			0x0008
#define PF4			0x0010
#define PF5			0x0020
#define PF6			0x0040
#define PF7			0x0080
#define PF8			0x0100
#define PF9			0x0200
#define PF10		0x0400
#define PF11		0x0800
#define PF12		0x1000
#define PF13		0x2000
#define PF14		0x4000
#define PF15		0x8000

/*  PORT F BIT POSITIONS */
#define PF0_P		0x0
#define PF1_P		0x1
#define PF2_P		0x2
#define PF3_P		0x3
#define PF4_P		0x4
#define PF5_P		0x5
#define PF6_P		0x6
#define PF7_P		0x7
#define PF8_P		0x8
#define PF9_P		0x9
#define PF10_P		0xA
#define PF11_P		0xB
#define PF12_P		0xC
#define PF13_P		0xD
#define PF14_P		0xE
#define PF15_P		0xF


/* ***********  SERIAL PERIPHERAL INTERFACE (SPI) MASKS  **************** */
/* SPI_CTL Masks */
#define	TIMOD		0x0003		/* Transfer Initiate Mode							*/
#define RDBR_CORE	0x0000		/* 		RDBR Read Initiates, IRQ When RDBR Full		*/
#define	TDBR_CORE	0x0001		/* 		TDBR Write Initiates, IRQ When TDBR Empty	*/
#define RDBR_DMA	0x0002		/* 		DMA Read, DMA Until FIFO Empty				*/
#define TDBR_DMA	0x0003		/* 		DMA Write, DMA Until FIFO Full				*/
#define SZ			0x0004		/* Send Zero (When TDBR Empty, Send Zero/Last*)		*/
#define GM			0x0008		/* Get More (When RDBR Full, Overwrite/Discard*)	*/
#define PSSE		0x0010		/* Slave-Select Input Enable						*/
#define EMISO		0x0020		/* Enable MISO As Output							*/
#define SIZE		0x0100		/* Size of Words (16/8* Bits)						*/
#define LSBF		0x0200		/* LSB First										*/
#define CPHA		0x0400		/* Clock Phase										*/
#define CPOL		0x0800		/* Clock Polarity									*/
#define MSTR		0x1000		/* Master/Slave*									*/
#define WOM			0x2000		/* Write Open Drain Master							*/
#define SPE			0x4000		/* SPI Enable										*/

/* SPI_FLG Masks */
#define FLS1	0x0002  /* Enables (=1) SPI_FLOUT1 as flag output for SPI Slave-select */
#define FLS2	0x0004  /* Enables (=1) SPI_FLOUT2 as flag output for SPI Slave-select */
#define FLS3	0x0008  /* Enables (=1) SPI_FLOUT3 as flag output for SPI Slave-select */
#define FLS4	0x0010  /* Enables (=1) SPI_FLOUT4 as flag output for SPI Slave-select */
#define FLS5	0x0020  /* Enables (=1) SPI_FLOUT5 as flag output for SPI Slave-select */
#define FLS6	0x0040  /* Enables (=1) SPI_FLOUT6 as flag output for SPI Slave-select */
#define FLS7	0x0080  /* Enables (=1) SPI_FLOUT7 as flag output for SPI Slave-select */

#define FLG1	0x0200  /* Activates (=0) SPI_FLOUT1 as flag output for SPI Slave-select  */
#define FLG2	0x0400  /* Activates (=0) SPI_FLOUT2 as flag output for SPI Slave-select */
#define FLG3	0x0800  /* Activates (=0) SPI_FLOUT3 as flag output for SPI Slave-select  */
#define FLG4	0x1000  /* Activates (=0) SPI_FLOUT4 as flag output for SPI Slave-select  */
#define FLG5	0x2000  /* Activates (=0) SPI_FLOUT5 as flag output for SPI Slave-select  */
#define FLG6	0x4000  /* Activates (=0) SPI_FLOUT6 as flag output for SPI Slave-select  */
#define FLG7	0x8000  /* Activates (=0) SPI_FLOUT7 as flag output for SPI Slave-select */

/* SPI_FLG Bit Positions */
#define FLS1_P	0x0001  /* Enables (=1) SPI_FLOUT1 as flag output for SPI Slave-select */
#define FLS2_P	0x0002  /* Enables (=1) SPI_FLOUT2 as flag output for SPI Slave-select */
#define FLS3_P	0x0003  /* Enables (=1) SPI_FLOUT3 as flag output for SPI Slave-select */
#define FLS4_P	0x0004  /* Enables (=1) SPI_FLOUT4 as flag output for SPI Slave-select */
#define FLS5_P	0x0005  /* Enables (=1) SPI_FLOUT5 as flag output for SPI Slave-select */
#define FLS6_P	0x0006  /* Enables (=1) SPI_FLOUT6 as flag output for SPI Slave-select */
#define FLS7_P	0x0007  /* Enables (=1) SPI_FLOUT7 as flag output for SPI Slave-select */
#define FLG1_P	0x0009  /* Activates (=0) SPI_FLOUT1 as flag output for SPI Slave-select  */
#define FLG2_P	0x000A  /* Activates (=0) SPI_FLOUT2 as flag output for SPI Slave-select */
#define FLG3_P	0x000B  /* Activates (=0) SPI_FLOUT3 as flag output for SPI Slave-select  */
#define FLG4_P	0x000C  /* Activates (=0) SPI_FLOUT4 as flag output for SPI Slave-select  */
#define FLG5_P	0x000D  /* Activates (=0) SPI_FLOUT5 as flag output for SPI Slave-select  */
#define FLG6_P	0x000E  /* Activates (=0) SPI_FLOUT6 as flag output for SPI Slave-select  */
#define FLG7_P	0x000F  /* Activates (=0) SPI_FLOUT7 as flag output for SPI Slave-select */

/* SPI_STAT Masks */
#define SPIF	0x0001  /* Set (=1) when SPI single-word transfer complete */
#define MODF	0x0002  /* Set (=1) in a master device when some other device tries to become master */
#define TXE		0x0004  /* Set (=1) when transmission occurs with no new data in SPI_TDBR */
#define TXS		0x0008  /* SPI_TDBR Data Buffer Status (0=Empty, 1=Full) */
#define RBSY	0x0010  /* Set (=1) when data is received with RDBR full */
#define RXS		0x0020  /* SPI_RDBR Data Buffer Status (0=Empty, 1=Full)  */
#define TXCOL	0x0040  /* When set (=1), corrupt data may have been transmitted  */

/* SPIx_FLG Masks																	*/
#define FLG1E	0xFDFF		/* Activates SPI_FLOUT1 							*/
#define FLG2E	0xFBFF		/* Activates SPI_FLOUT2								*/
#define FLG3E	0xF7FF		/* Activates SPI_FLOUT3								*/
#define FLG4E	0xEFFF		/* Activates SPI_FLOUT4								*/
#define FLG5E	0xDFFF		/* Activates SPI_FLOUT5								*/
#define FLG6E	0xBFFF		/* Activates SPI_FLOUT6								*/
#define FLG7E	0x7FFF		/* Activates SPI_FLOUT7								*/


/* *********************  ASYNCHRONOUS MEMORY CONTROLLER MASKS  ************* */
/* EBIU_AMGCTL Masks */
#define AMCKEN			0x0001	/* Enable CLKOUT */
#define	AMBEN_NONE		0x0000	/* All Banks Disabled								*/
#define AMBEN_B0		0x0002	/* Enable Asynchronous Memory Bank 0 only */
#define AMBEN_B0_B1		0x0004	/* Enable Asynchronous Memory Banks 0 & 1 only */
#define AMBEN_B0_B1_B2		0x0006	/* Enable Asynchronous Memory Banks 0, 1, and 2 */
#define AMBEN_ALL		0x0008	/* Enable Asynchronous Memory Banks (all) 0, 1, 2, and 3 */
#define CDPRIO                  0x0100      /* DMA has priority over core for for external accesses */

/* EBIU_AMGCTL Bit Positions */
#define AMCKEN_P		0x0000	/* Enable CLKOUT */
#define AMBEN_P0		0x0001	/* Asynchronous Memory Enable, 000 - banks 0-3 disabled, 001 - Bank 0 enabled */
#define AMBEN_P1		0x0002	/* Asynchronous Memory Enable, 010 - banks 0&1 enabled,  011 - banks 0-3 enabled */
#define AMBEN_P2		0x0003	/* Asynchronous Memory Enable, 1xx - All banks (bank 0, 1, 2, and 3) enabled */

/* EBIU_AMBCTL0 Masks */
#define B0RDYEN			0x00000001  /* Bank 0 RDY Enable, 0=disable, 1=enable */
#define B0RDYPOL 		0x00000002  /* Bank 0 RDY Active high, 0=active low, 1=active high */
#define B0TT_1			0x00000004  /* Bank 0 Transition Time from Read to Write = 1 cycle */
#define B0TT_2			0x00000008  /* Bank 0 Transition Time from Read to Write = 2 cycles */
#define B0TT_3			0x0000000C  /* Bank 0 Transition Time from Read to Write = 3 cycles */
#define B0TT_4			0x00000000  /* Bank 0 Transition Time from Read to Write = 4 cycles */
#define B0ST_1			0x00000010  /* Bank 0 Setup Time from AOE asserted to Read/Write asserted=1 cycle */
#define B0ST_2			0x00000020  /* Bank 0 Setup Time from AOE asserted to Read/Write asserted=2 cycles */
#define B0ST_3			0x00000030  /* Bank 0 Setup Time from AOE asserted to Read/Write asserted=3 cycles */
#define B0ST_4			0x00000000  /* Bank 0 Setup Time from AOE asserted to Read/Write asserted=4 cycles */
#define B0HT_1			0x00000040  /* Bank 0 Hold Time from Read/Write deasserted to AOE deasserted = 1 cycle */
#define B0HT_2			0x00000080  /* Bank 0 Hold Time from Read/Write deasserted to AOE deasserted = 2 cycles */
#define B0HT_3			0x000000C0  /* Bank 0 Hold Time from Read/Write deasserted to AOE deasserted = 3 cycles */
#define B0HT_0			0x00000000  /* Bank 0 Hold Time from Read/Write deasserted to AOE deasserted = 0 cycles */
#define B0RAT_1			0x00000100  /* Bank 0 Read Access Time = 1 cycle */
#define B0RAT_2			0x00000200  /* Bank 0 Read Access Time = 2 cycles */
#define B0RAT_3			0x00000300  /* Bank 0 Read Access Time = 3 cycles */
#define B0RAT_4			0x00000400  /* Bank 0 Read Access Time = 4 cycles */
#define B0RAT_5			0x00000500  /* Bank 0 Read Access Time = 5 cycles */
#define B0RAT_6			0x00000600  /* Bank 0 Read Access Time = 6 cycles */
#define B0RAT_7			0x00000700  /* Bank 0 Read Access Time = 7 cycles */
#define B0RAT_8			0x00000800  /* Bank 0 Read Access Time = 8 cycles */
#define B0RAT_9			0x00000900  /* Bank 0 Read Access Time = 9 cycles */
#define B0RAT_10		0x00000A00  /* Bank 0 Read Access Time = 10 cycles */
#define B0RAT_11		0x00000B00  /* Bank 0 Read Access Time = 11 cycles */
#define B0RAT_12		0x00000C00  /* Bank 0 Read Access Time = 12 cycles */
#define B0RAT_13		0x00000D00  /* Bank 0 Read Access Time = 13 cycles */
#define B0RAT_14		0x00000E00  /* Bank 0 Read Access Time = 14 cycles */
#define B0RAT_15		0x00000F00  /* Bank 0 Read Access Time = 15 cycles */
#define B0WAT_1			0x00001000  /* Bank 0 Write Access Time = 1 cycle */
#define B0WAT_2			0x00002000  /* Bank 0 Write Access Time = 2 cycles */
#define B0WAT_3			0x00003000  /* Bank 0 Write Access Time = 3 cycles */
#define B0WAT_4			0x00004000  /* Bank 0 Write Access Time = 4 cycles */
#define B0WAT_5			0x00005000  /* Bank 0 Write Access Time = 5 cycles */
#define B0WAT_6			0x00006000  /* Bank 0 Write Access Time = 6 cycles */
#define B0WAT_7			0x00007000  /* Bank 0 Write Access Time = 7 cycles */
#define B0WAT_8			0x00008000  /* Bank 0 Write Access Time = 8 cycles */
#define B0WAT_9			0x00009000  /* Bank 0 Write Access Time = 9 cycles */
#define B0WAT_10		0x0000A000  /* Bank 0 Write Access Time = 10 cycles */
#define B0WAT_11		0x0000B000  /* Bank 0 Write Access Time = 11 cycles */
#define B0WAT_12		0x0000C000  /* Bank 0 Write Access Time = 12 cycles */
#define B0WAT_13		0x0000D000  /* Bank 0 Write Access Time = 13 cycles */
#define B0WAT_14		0x0000E000  /* Bank 0 Write Access Time = 14 cycles */
#define B0WAT_15		0x0000F000  /* Bank 0 Write Access Time = 15 cycles */
#define B1RDYEN			0x00010000  /* Bank 1 RDY enable, 0=disable, 1=enable */
#define B1RDYPOL		0x00020000  /* Bank 1 RDY Active high, 0=active low, 1=active high */
#define B1TT_1			0x00040000  /* Bank 1 Transition Time from Read to Write = 1 cycle */
#define B1TT_2			0x00080000  /* Bank 1 Transition Time from Read to Write = 2 cycles */
#define B1TT_3			0x000C0000  /* Bank 1 Transition Time from Read to Write = 3 cycles */
#define B1TT_4			0x00000000  /* Bank 1 Transition Time from Read to Write = 4 cycles */
#define B1ST_1			0x00100000  /* Bank 1 Setup Time from AOE asserted to Read or Write asserted = 1 cycle */
#define B1ST_2			0x00200000  /* Bank 1 Setup Time from AOE asserted to Read or Write asserted = 2 cycles */
#define B1ST_3			0x00300000  /* Bank 1 Setup Time from AOE asserted to Read or Write asserted = 3 cycles */
#define B1ST_4			0x00000000  /* Bank 1 Setup Time from AOE asserted to Read or Write asserted = 4 cycles */
#define B1HT_1			0x00400000  /* Bank 1 Hold Time from Read or Write deasserted to AOE deasserted = 1 cycle */
#define B1HT_2			0x00800000  /* Bank 1 Hold Time from Read or Write deasserted to AOE deasserted = 2 cycles */
#define B1HT_3			0x00C00000  /* Bank 1 Hold Time from Read or Write deasserted to AOE deasserted = 3 cycles */
#define B1HT_0			0x00000000  /* Bank 1 Hold Time from Read or Write deasserted to AOE deasserted = 0 cycles */
#define B1RAT_1			0x01000000  /* Bank 1 Read Access Time = 1 cycle */
#define B1RAT_2			0x02000000  /* Bank 1 Read Access Time = 2 cycles */
#define B1RAT_3			0x03000000  /* Bank 1 Read Access Time = 3 cycles */
#define B1RAT_4			0x04000000  /* Bank 1 Read Access Time = 4 cycles */
#define B1RAT_5			0x05000000  /* Bank 1 Read Access Time = 5 cycles */
#define B1RAT_6			0x06000000  /* Bank 1 Read Access Time = 6 cycles */
#define B1RAT_7			0x07000000  /* Bank 1 Read Access Time = 7 cycles */
#define B1RAT_8			0x08000000  /* Bank 1 Read Access Time = 8 cycles */
#define B1RAT_9			0x09000000  /* Bank 1 Read Access Time = 9 cycles */
#define B1RAT_10		0x0A000000  /* Bank 1 Read Access Time = 10 cycles */
#define B1RAT_11		0x0B000000  /* Bank 1 Read Access Time = 11 cycles */
#define B1RAT_12		0x0C000000  /* Bank 1 Read Access Time = 12 cycles */
#define B1RAT_13		0x0D000000  /* Bank 1 Read Access Time = 13 cycles */
#define B1RAT_14		0x0E000000  /* Bank 1 Read Access Time = 14 cycles */
#define B1RAT_15		0x0F000000  /* Bank 1 Read Access Time = 15 cycles */
#define B1WAT_1			0x10000000 /* Bank 1 Write Access Time = 1 cycle */
#define B1WAT_2			0x20000000  /* Bank 1 Write Access Time = 2 cycles */
#define B1WAT_3			0x30000000  /* Bank 1 Write Access Time = 3 cycles */
#define B1WAT_4			0x40000000  /* Bank 1 Write Access Time = 4 cycles */
#define B1WAT_5			0x50000000  /* Bank 1 Write Access Time = 5 cycles */
#define B1WAT_6			0x60000000  /* Bank 1 Write Access Time = 6 cycles */
#define B1WAT_7			0x70000000  /* Bank 1 Write Access Time = 7 cycles */
#define B1WAT_8			0x80000000  /* Bank 1 Write Access Time = 8 cycles */
#define B1WAT_9			0x90000000  /* Bank 1 Write Access Time = 9 cycles */
#define B1WAT_10		0xA0000000  /* Bank 1 Write Access Time = 10 cycles */
#define B1WAT_11		0xB0000000  /* Bank 1 Write Access Time = 11 cycles */
#define B1WAT_12		0xC0000000  /* Bank 1 Write Access Time = 12 cycles */
#define B1WAT_13		0xD0000000  /* Bank 1 Write Access Time = 13 cycles */
#define B1WAT_14		0xE0000000  /* Bank 1 Write Access Time = 14 cycles */
#define B1WAT_15		0xF0000000  /* Bank 1 Write Access Time = 15 cycles */

/* EBIU_AMBCTL1 Masks */
#define B2RDYEN			0x00000001  /* Bank 2 RDY Enable, 0=disable, 1=enable */
#define B2RDYPOL		0x00000002  /* Bank 2 RDY Active high, 0=active low, 1=active high */
#define B2TT_1			0x00000004  /* Bank 2 Transition Time from Read to Write = 1 cycle */
#define B2TT_2			0x00000008  /* Bank 2 Transition Time from Read to Write = 2 cycles */
#define B2TT_3			0x0000000C  /* Bank 2 Transition Time from Read to Write = 3 cycles */
#define B2TT_4			0x00000000  /* Bank 2 Transition Time from Read to Write = 4 cycles */
#define B2ST_1			0x00000010  /* Bank 2 Setup Time from AOE asserted to Read or Write asserted = 1 cycle */
#define B2ST_2			0x00000020  /* Bank 2 Setup Time from AOE asserted to Read or Write asserted = 2 cycles */
#define B2ST_3			0x00000030  /* Bank 2 Setup Time from AOE asserted to Read or Write asserted = 3 cycles */
#define B2ST_4			0x00000000  /* Bank 2 Setup Time from AOE asserted to Read or Write asserted = 4 cycles */
#define B2HT_1			0x00000040  /* Bank 2 Hold Time from Read or Write deasserted to AOE deasserted = 1 cycle */
#define B2HT_2			0x00000080  /* Bank 2 Hold Time from Read or Write deasserted to AOE deasserted = 2 cycles */
#define B2HT_3			0x000000C0  /* Bank 2 Hold Time from Read or Write deasserted to AOE deasserted = 3 cycles */
#define B2HT_0			0x00000000  /* Bank 2 Hold Time from Read or Write deasserted to AOE deasserted = 0 cycles */
#define B2RAT_1			0x00000100  /* Bank 2 Read Access Time = 1 cycle */
#define B2RAT_2			0x00000200  /* Bank 2 Read Access Time = 2 cycles */
#define B2RAT_3			0x00000300  /* Bank 2 Read Access Time = 3 cycles */
#define B2RAT_4			0x00000400  /* Bank 2 Read Access Time = 4 cycles */
#define B2RAT_5			0x00000500  /* Bank 2 Read Access Time = 5 cycles */
#define B2RAT_6			0x00000600  /* Bank 2 Read Access Time = 6 cycles */
#define B2RAT_7			0x00000700  /* Bank 2 Read Access Time = 7 cycles */
#define B2RAT_8			0x00000800  /* Bank 2 Read Access Time = 8 cycles */
#define B2RAT_9			0x00000900  /* Bank 2 Read Access Time = 9 cycles */
#define B2RAT_10		0x00000A00  /* Bank 2 Read Access Time = 10 cycles */
#define B2RAT_11		0x00000B00  /* Bank 2 Read Access Time = 11 cycles */
#define B2RAT_12		0x00000C00  /* Bank 2 Read Access Time = 12 cycles */
#define B2RAT_13		0x00000D00  /* Bank 2 Read Access Time = 13 cycles */
#define B2RAT_14		0x00000E00  /* Bank 2 Read Access Time = 14 cycles */
#define B2RAT_15		0x00000F00  /* Bank 2 Read Access Time = 15 cycles */
#define B2WAT_1			0x00001000  /* Bank 2 Write Access Time = 1 cycle */
#define B2WAT_2			0x00002000  /* Bank 2 Write Access Time = 2 cycles */
#define B2WAT_3			0x00003000  /* Bank 2 Write Access Time = 3 cycles */
#define B2WAT_4			0x00004000  /* Bank 2 Write Access Time = 4 cycles */
#define B2WAT_5			0x00005000  /* Bank 2 Write Access Time = 5 cycles */
#define B2WAT_6			0x00006000  /* Bank 2 Write Access Time = 6 cycles */
#define B2WAT_7			0x00007000  /* Bank 2 Write Access Time = 7 cycles */
#define B2WAT_8			0x00008000  /* Bank 2 Write Access Time = 8 cycles */
#define B2WAT_9			0x00009000  /* Bank 2 Write Access Time = 9 cycles */
#define B2WAT_10		0x0000A000  /* Bank 2 Write Access Time = 10 cycles */
#define B2WAT_11		0x0000B000  /* Bank 2 Write Access Time = 11 cycles */
#define B2WAT_12		0x0000C000  /* Bank 2 Write Access Time = 12 cycles */
#define B2WAT_13		0x0000D000  /* Bank 2 Write Access Time = 13 cycles */
#define B2WAT_14		0x0000E000  /* Bank 2 Write Access Time = 14 cycles */
#define B2WAT_15		0x0000F000  /* Bank 2 Write Access Time = 15 cycles */
#define B3RDYEN			0x00010000  /* Bank 3 RDY enable, 0=disable, 1=enable */
#define B3RDYPOL		0x00020000  /* Bank 3 RDY Active high, 0=active low, 1=active high */
#define B3TT_1			0x00040000  /* Bank 3 Transition Time from Read to Write = 1 cycle */
#define B3TT_2			0x00080000  /* Bank 3 Transition Time from Read to Write = 2 cycles */
#define B3TT_3			0x000C0000  /* Bank 3 Transition Time from Read to Write = 3 cycles */
#define B3TT_4			0x00000000  /* Bank 3 Transition Time from Read to Write = 4 cycles */
#define B3ST_1			0x00100000  /* Bank 3 Setup Time from AOE asserted to Read or Write asserted = 1 cycle */
#define B3ST_2			0x00200000  /* Bank 3 Setup Time from AOE asserted to Read or Write asserted = 2 cycles */
#define B3ST_3			0x00300000  /* Bank 3 Setup Time from AOE asserted to Read or Write asserted = 3 cycles */
#define B3ST_4			0x00000000  /* Bank 3 Setup Time from AOE asserted to Read or Write asserted = 4 cycles */
#define B3HT_1			0x00400000  /* Bank 3 Hold Time from Read or Write deasserted to AOE deasserted = 1 cycle */
#define B3HT_2			0x00800000  /* Bank 3 Hold Time from Read or Write deasserted to AOE deasserted = 2 cycles */
#define B3HT_3			0x00C00000  /* Bank 3 Hold Time from Read or Write deasserted to AOE deasserted = 3 cycles */
#define B3HT_0			0x00000000  /* Bank 3 Hold Time from Read or Write deasserted to AOE deasserted = 0 cycles */
#define B3RAT_1			0x01000000 /* Bank 3 Read Access Time = 1 cycle */
#define B3RAT_2			0x02000000  /* Bank 3 Read Access Time = 2 cycles */
#define B3RAT_3			0x03000000  /* Bank 3 Read Access Time = 3 cycles */
#define B3RAT_4			0x04000000  /* Bank 3 Read Access Time = 4 cycles */
#define B3RAT_5			0x05000000  /* Bank 3 Read Access Time = 5 cycles */
#define B3RAT_6			0x06000000  /* Bank 3 Read Access Time = 6 cycles */
#define B3RAT_7			0x07000000  /* Bank 3 Read Access Time = 7 cycles */
#define B3RAT_8			0x08000000  /* Bank 3 Read Access Time = 8 cycles */
#define B3RAT_9			0x09000000  /* Bank 3 Read Access Time = 9 cycles */
#define B3RAT_10		0x0A000000  /* Bank 3 Read Access Time = 10 cycles */
#define B3RAT_11		0x0B000000  /* Bank 3 Read Access Time = 11 cycles */
#define B3RAT_12		0x0C000000  /* Bank 3 Read Access Time = 12 cycles */
#define B3RAT_13		0x0D000000  /* Bank 3 Read Access Time = 13 cycles */
#define B3RAT_14		0x0E000000  /* Bank 3 Read Access Time = 14 cycles */
#define B3RAT_15		0x0F000000  /* Bank 3 Read Access Time = 15 cycles */
#define B3WAT_1			0x10000000 /* Bank 3 Write Access Time = 1 cycle */
#define B3WAT_2			0x20000000  /* Bank 3 Write Access Time = 2 cycles */
#define B3WAT_3			0x30000000  /* Bank 3 Write Access Time = 3 cycles */
#define B3WAT_4			0x40000000  /* Bank 3 Write Access Time = 4 cycles */
#define B3WAT_5			0x50000000  /* Bank 3 Write Access Time = 5 cycles */
#define B3WAT_6			0x60000000  /* Bank 3 Write Access Time = 6 cycles */
#define B3WAT_7			0x70000000  /* Bank 3 Write Access Time = 7 cycles */
#define B3WAT_8			0x80000000  /* Bank 3 Write Access Time = 8 cycles */
#define B3WAT_9			0x90000000  /* Bank 3 Write Access Time = 9 cycles */
#define B3WAT_10		0xA0000000  /* Bank 3 Write Access Time = 10 cycles */
#define B3WAT_11		0xB0000000  /* Bank 3 Write Access Time = 11 cycles */
#define B3WAT_12		0xC0000000  /* Bank 3 Write Access Time = 12 cycles */
#define B3WAT_13		0xD0000000  /* Bank 3 Write Access Time = 13 cycles */
#define B3WAT_14		0xE0000000  /* Bank 3 Write Access Time = 14 cycles */
#define B3WAT_15		0xF0000000  /* Bank 3 Write Access Time = 15 cycles */

/* **********************  SDRAM CONTROLLER MASKS  *************************** */
/* EBIU_SDGCTL Masks */


#define SCTLE			0x00000001 /* Enable SCLK[0], /SRAS, /SCAS, /SWE, SDQM[3:0] */
#define CL_2			0x00000008 /* SDRAM CAS latency = 2 cycles */
#define CL_3			0x0000000C /* SDRAM CAS latency = 3 cycles */
#define CL           0x0000000C  /* SDRAM CAS latency */
#define PFE				0x00000010 /* Enable SDRAM prefetch */
#define PFP				0x00000020 /* Prefetch has priority over AMC requests */
#define PASR_ALL		0x00000000	/* All 4 SDRAM Banks Refreshed In Self-Refresh				*/
#define PASR_B0_B1		0x00000010	/* SDRAM Banks 0 and 1 Are Refreshed In Self-Refresh		*/
#define PASR_B0			0x00000020	/* Only SDRAM Bank 0 Is Refreshed In Self-Refresh			*/
#define PASR         0x00000030  /* SDRAM partial array self-refresh */
#define TRAS_1			0x00000040 /* SDRAM tRAS = 1 cycle */
#define TRAS_2			0x00000080 /* SDRAM tRAS = 2 cycles */
#define TRAS_3			0x000000C0 /* SDRAM tRAS = 3 cycles */
#define TRAS_4			0x00000100 /* SDRAM tRAS = 4 cycles */
#define TRAS_5			0x00000140 /* SDRAM tRAS = 5 cycles */
#define TRAS_6			0x00000180 /* SDRAM tRAS = 6 cycles */
#define TRAS_7			0x000001C0 /* SDRAM tRAS = 7 cycles */
#define TRAS_8			0x00000200 /* SDRAM tRAS = 8 cycles */
#define TRAS_9			0x00000240 /* SDRAM tRAS = 9 cycles */
#define TRAS_10			0x00000280 /* SDRAM tRAS = 10 cycles */
#define TRAS_11			0x000002C0 /* SDRAM tRAS = 11 cycles */
#define TRAS_12			0x00000300 /* SDRAM tRAS = 12 cycles */
#define TRAS_13			0x00000340 /* SDRAM tRAS = 13 cycles */
#define TRAS_14			0x00000380 /* SDRAM tRAS = 14 cycles */
#define TRAS_15			0x000003C0 /* SDRAM tRAS = 15 cycles */
#define TRAS            0x000003C0 /* SDRAM tRAS in SCLK cycles */
#define TRP_1			0x00000800 /* SDRAM tRP = 1 cycle */
#define TRP_2			0x00001000 /* SDRAM tRP = 2 cycles */
#define TRP_3			0x00001800 /* SDRAM tRP = 3 cycles */
#define TRP_4			0x00002000 /* SDRAM tRP = 4 cycles */
#define TRP_5			0x00002800 /* SDRAM tRP = 5 cycles */
#define TRP_6			0x00003000 /* SDRAM tRP = 6 cycles */
#define TRP_7			0x00003800 /* SDRAM tRP = 7 cycles */
#define TRP          0x00003800 /* SDRAM tRP in SCLK cycles */
#define TRCD_1			0x00008000 /* SDRAM tRCD = 1 cycle */
#define TRCD_2			0x00010000 /* SDRAM tRCD = 2 cycles */
#define TRCD_3			0x00018000 /* SDRAM tRCD = 3 cycles */
#define TRCD_4			0x00020000 /* SDRAM tRCD = 4 cycles */
#define TRCD_5			0x00028000 /* SDRAM tRCD = 5 cycles */
#define TRCD_6			0x00030000 /* SDRAM tRCD = 6 cycles */
#define TRCD_7			0x00038000 /* SDRAM tRCD = 7 cycles */
#define TRCD         0x00030000 /* SDRAM tRCD in SCLK cycles */
#define TWR_1			0x00080000 /* SDRAM tWR = 1 cycle */
#define TWR_2			0x00100000 /* SDRAM tWR = 2 cycles */
#define TWR_3			0x00180000 /* SDRAM tWR = 3 cycles */
#define TWR          0x00180000  /* SDRAM tWR in SCLK cycles */
#define PUPSD			0x00200000 /*Power-up start delay */
#define PSM				0x00400000 /* SDRAM power-up sequence = Precharge, mode register set, 8 CBR refresh cycles */
#define PSS				0x00800000 /* enable SDRAM power-up sequence on next SDRAM access */
#define SRFS			0x01000000 /* Start SDRAM self-refresh mode */
#define EBUFE			0x02000000 /* Enable external buffering timing */
#define FBBRW			0x04000000 /* Fast back-to-back read write enable */
#define EMREN			0x10000000 /* Extended mode register enable */
#define TCSR			0x20000000 /* Temp compensated self refresh value 85 deg C */
#define CDDBG			0x40000000 /* Tristate SDRAM controls during bus grant */

/* EBIU_SDBCTL Masks */
#define EBE				0x00000001 /* Enable SDRAM external bank */
#define EBSZ_16		0x00000000 /* SDRAM external bank size = 16MB */
#define EBSZ_32		0x00000002 /* SDRAM external bank size = 32MB */
#define EBSZ_64		0x00000004 /* SDRAM external bank size = 64MB */
#define EBSZ_128		0x00000006 /* SDRAM external bank size = 128MB */
#define EBCAW_8		0x00000000 /* SDRAM external bank column address width = 8 bits */
#define EBSZ         0x0006 /* SDRAM external bank size */
#define EBCAW_9		0x00000010 /* SDRAM external bank column address width = 9 bits */
#define EBCAW_10		0x00000020 /* SDRAM external bank column address width = 9 bits */
#define EBCAW_11		0x00000030 /* SDRAM external bank column address width = 9 bits */
#define EBCAW        0x0030 /* SDRAM external bank column address width */

/* EBIU_SDSTAT Masks */
#define SDCI			0x00000001 /* SDRAM controller is idle  */
#define SDSRA			0x00000002 /* SDRAM SDRAM self refresh is active */
#define SDPUA			0x00000004 /* SDRAM power up active  */
#define SDRS			0x00000008 /* SDRAM is in reset state */
#define SDEASE			0x00000010 /* SDRAM EAB sticky error status - W1C */
#define BGSTAT			0x00000020 /* Bus granted */

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* _DEF_BF532_H */
