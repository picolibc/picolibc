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
 * defBF535.h
 *
 * (c) Copyright 2001-2008 Analog Devices, Inc.  All rights reserved.
 *
 ************************************************************************/

/* SYSTEM & MM REGISTER BIT & ADDRESS DEFINITIONS FOR ADSP-BF535 */

#ifndef _DEF_BF535_H
#define _DEF_BF535_H

#if defined(__ADSPLPBLACKFIN__)
#warning defBF535.h should only be included for 535 compatible chips.
#endif
/* include all Core registers and bit definitions */
#include <defblackfin.h>

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_19_4:"some macro definitions not MISRA compliant")
#endif /* _MISRA_RULES */

/*********************************************************************************** */
/* Memory Map */
/*********************************************************************************** */

/* Core MMRs */
#define COREMMR_BASE           0xFFE00000     /* Core MMRs */
#define COREMMR_SIZE           0x200000       /* 2MB */

/* System MMRs */
#define SYSMMR_BASE            0xFFC00000     /* System MMRs */
#define SYSMMR_SIZE            0x200000       /* 2MB */

/* L1 cache/SRAM internal memory */
#define L1_DATA_A		0xFF800000	/* L1 Data Bank A */
#define L1_DATA_B		0xFF900000	/* L1 Data Bank B */
#define L1_DATA_SIZE		0x4000		/*  16K */
#define L1_CODE			0xFFA00000	/* L1 Code SRAM */
#define L1_CODE_SIZE		0x4000		/*  16K */
#define L1_SCRATCH		0xFFB00000	/* L1 Scratch SRAM */
#define L1_SCRATCH_SIZE		0x1000		/*  4K */

/* L2 SRAM external memory */
#define L2_BASE			0xF0000000	/* L2 SRAM */
#define L2_SIZE			0x40000		/*  256K */

/* PCI Spaces */
#define PCI_CONFIG_SPACE_PORT	0xEEFFFFFC	/* PCI config space reg */
#define PCI_CONFIG_BASE		0xEEFFFF00	/* PCI config region */
#define PCI_CONFIG_SIZE		0x10000		/*  64K */
#define PCI_IO_BASE		0xEEFE0000	/* PCI I/O space */
#define PCI_IO_SIZE		0x10000		/*  64K */
#define PCI_MEM_BASE		0xE0000000	/* PCI Mem space */
#define PCI_MEM_SIZE		0x8000000	/*  64K */

/* Async Memory Banks */
#define ASYNC_BANK3_BASE	0x2C000000	/* Async Bank 3 */
#define ASYNC_BANK3_SIZE	0x4000000	/*  64 MB */
#define ASYNC_BANK2_BASE	0x28000000	/* Async Bank 2 */
#define ASYNC_BANK2_SIZE	0x4000000	/*  64 MB */
#define ASYNC_BANK1_BASE	0x24000000	/* Async Bank 1 */
#define ASYNC_BANK1_SIZE	0x4000000	/*  64 MB */
#define ASYNC_BANK0_BASE	0x20000000	/* Async Bank 0 */
#define ASYNC_BANK0_SIZE	0x4000000	/*  64 MB */

/* Sync DRAM Banks */
#define SDRAM_BANK3_BASE	0x18000000	/* Sync Bank 3 */
#define SDRAM_BANK2_BASE	0x10000000	/* Sync Bank 2 */
#define SDRAM_BANK1_BASE	0x08000000	/* Sync Bank 1 */
#define SDRAM_BANK0_BASE	0x00000000	/* Sync Bank 0 */


/*********************************************************************************** */
/* System MMR Register Map */
/*********************************************************************************** */

/* L2 MISR MMRs (0xFFC0 0000-0xFFC0 03FF) */
#define MISR_CTL               0xFFC00000     /* Control Register */
#define MISR_RMISR0            0xFFC00004     /* coreL2[31:0] read bus */
#define MISR_RMISR1            0xFFC00008     /* coreL2[63:32] read bus */
#define MISR_RMISR2            0xFFC0000C     /* sysL2[31:0] read bus */
#define MISR_WMISR0            0xFFC00010     /* coreL2[31:0] write bus */
#define MISR_WMISR1            0xFFC00014     /* coreL2[63:32] write bus */
#define MISR_WMISR2            0xFFC00018     /* sysL2[31:0] write bus */

/* Clock and System Control (0xFFC0 0400-0xFFC0 07FF) */
#define PLL_CTL                0xFFC00400      /* PLL Control register (32-bit) */
#define PLL_STAT               0xFFC00404      /* PLL Status register */
#define PLL_LOCKCNT            0xFFC00406      /* PLL Lock Counter register */
#define PLL_IOCKR              0xFFC00408      /* Peripheral Clock Enable register (32-bit) */
#define PLL_IOCK               0xFFC00408      /* Peripheral Clock Enable register (32-bit) - alternate spelling */
#define SWRST                  0xFFC00410      /* Software Reset Register */

#define PLLCTL			PLL_CTL
#define PLLSTAT			PLL_STAT
#define LOCKCNT			PLL_LOCKCNT
#define IOCKR			PLL_IOCKR

#define SYSCR                  0xFFC00414      /* System Configuration register (RCSR) */

/* JTAG/Debug Communication Channel (0xFFC0 0800-0xFFC0 0BFF) */
#define CHIPID                 0xFFC048C0      /* Device ID Register */

/* System Interrupt Controller (0xFFC0 0C00-0xFFC0 0FFF) */
#define SIC_IAR0               0xFFC00C04  /* Interrupt Assignment Register 0 */
#define SIC_IAR1               0xFFC00C08  /* Interrupt Assignment Register 1 */
#define SIC_IAR2               0xFFC00C0C  /* Interrupt Assignment Register 2 */
#define SIC_IMASK              0xFFC00C10  /* Interrupt Mask Register */
#define SIC_ISR                0xFFC00C14  /* Interrupt Status Register */
#define SIC_IWR                0xFFC00C18  /* Interrupt Wakeup Register */

/* Watchdog Timer (0xFFC0 1000-0xFFC0 13FF) */
#define WDOGCTL                0xFFC01000  /* Watchdog Control Register */
#define WDOGCNT                0xFFC01004  /* Watchdog Count Register */
#define WDOGSTAT               0xFFC01008  /* Watchdog Status Register */

#define WDOG_CTL		WDOGCTL
#define WDOG_CNT		WDOGCNT
#define WDOG_STAT		WDOGSTAT

/* Real Time Clock (0xFFC0 1400-0xFFC0 17FF) */
#define RTCSTAT                0xFFC01400  /* RTC Status Register */
#define RTCICTL                0xFFC01404  /* RTC Interrupt Control Register */
#define RTCISTAT               0xFFC01408  /* RTC Interrupt Status Register */
#define RTCSWCNT               0xFFC0140C  /* RTC Stopwatch Count Register */
#define RTCALARM               0xFFC01410  /* RTC Alarm Time Register */
#define RTCFAST                0xFFC01414  /* RTC Prescaler Control Register */

#define RTC_STAT		RTCSTAT
#define RTC_ICTL		RTCICTL
#define RTC_ISTAT		RTCISTAT
#define RTC_SWCNT		RTCSWCNT
#define RTC_ALARM		RTCALARM
#define RTC_FAST		RTCFAST

/* UART 0 Controller (0xFFC0 1800-0xFFC0 1BFF) */
#define UART0_THR              0xFFC01800  /* Transmit Holding register */
#define UART0_RBR              0xFFC01800  /* Receive Buffer register */
#define UART0_DLL              0xFFC01800  /* Divisor Latch (Low-Byte) */
#define UART0_IER              0xFFC01802  /* Interrupt Enable Register */
#define UART0_DLH              0xFFC01802  /* Divisor Latch (High-Byte) */
#define UART0_IIR              0xFFC01804  /* Interrupt Identification Register */
#define UART0_LCR              0xFFC01806  /* Line Control Register */
#define UART0_MCR              0xFFC01808  /* Module Control Register */
#define UART0_LSR              0xFFC0180A  /* Line Status Register */
#define UART0_MSR              0xFFC0180C  /* MSR Modem Status Register */
#define UART0_SCR              0xFFC0180E  /* SCR Scratch Register */
#define UART0_IRCR             0xFFC01810  /* IRCR IrDA Control Register */
#define UART0_CURR_PTR_RX      0xFFC01A00  /* UART -DMA RCV Current Pointer register */
#define UART0_CONFIG_RX        0xFFC01A02  /* UART -RCV DMA Configuration register */
#define UART0_START_ADDR_HI_RX 0xFFC01A04  /* UART -RCV DMA Start Page register */
#define UART0_START_ADDR_LO_RX 0xFFC01A06  /* UART -RCV DMA Start Address register */
#define UART0_COUNT_RX         0xFFC01A08  /* UART -RCV DMA Count register */
#define UART0_NEXT_DESCR_RX    0xFFC01A0A  /* UART -RCV DMA Next Descriptor Pointer register */
#define UART0_DESCR_RDY_RX     0xFFC01A0C  /* UART -RCV DMA Descriptor Ready */
#define UART0_IRQSTAT_RX       0xFFC01A0E  /* UART -RCV DMA Interrupt Register */
#define UART0_CURR_PTR_TX      0xFFC01B00  /* UART -XMT DMA Current Pointer register */
#define UART0_CONFIG_TX        0xFFC01B02  /* UART -XMT DMA Configuration register */
#define UART0_START_ADDR_HI_TX 0xFFC01B04  /* UART -XMT DMA Start Page register */
#define UART0_START_ADDR_LO_TX 0xFFC01B06  /* UART -XMT DMA Start Address register */
#define UART0_COUNT_TX         0xFFC01B08  /* UART -XMT DMA Count register */
#define UART0_NEXT_DESCR_TX    0xFFC01B0A  /* UART -XMT DMA Next Descriptor Pointer register */
#define UART0_DESCR_RDY_TX     0xFFC01B0C  /* UART -XMT DMA Descriptor Ready */
#define UART0_IRQSTAT_TX       0xFFC01B0E  /* UART -XMT DMA Interrupt register */

/* UART 1 Controller (0xFFC0 1C00-0xFFC0 1FFF) */
#define UART1_THR              0xFFC01C00  /* Transmit Holding register */
#define UART1_RBR              0xFFC01C00  /* Receive Buffer register */
#define UART1_DLL              0xFFC01C00  /* Divisor Latch (Low-Byte) */
#define UART1_IER              0xFFC01C02  /* Interrupt Enable Register */
#define UART1_DLH              0xFFC01C02  /* Divisor Latch (High-Byte) */
#define UART1_IIR              0xFFC01C04  /* Interrupt Identification Register */
#define UART1_LCR              0xFFC01C06  /* Line Control Register */
#define UART1_MCR              0xFFC01C08  /* Module Control Register */
#define UART1_LSR              0xFFC01C0A  /* Line Status Register */
#define UART1_MSR              0xFFC01C0C  /* MSR Modem Status Register */
#define UART1_SCR              0xFFC01C0E  /* SCR Scratch Register */
#define UART1_CURR_PTR_RX      0xFFC01E00  /* UART -DMA RCV Current Pointer register */
#define UART1_CONFIG_RX        0xFFC01E02  /* UART -RCV DMA Configuration register */
#define UART1_START_ADDR_HI_RX 0xFFC01E04  /* UART -RCV DMA Start Page register */
#define UART1_START_ADDR_LO_RX 0xFFC01E06  /* UART -RCV DMA Start Address register */
#define UART1_COUNT_RX         0xFFC01E08  /* UART -RCV DMA Count register */
#define UART1_NEXT_DESCR_RX    0xFFC01E0A  /* UART -RCV DMA Next Descriptor Pointer register */
#define UART1_DESCR_RDY_RX     0xFFC01E0C  /* UART -RCV DMA Descriptor Ready */
#define UART1_IRQSTAT_RX       0xFFC01E0E  /* UART -RCV DMA Interrupt Register */
#define UART1_CURR_PTR_TX      0xFFC01F00  /* UART -XMT DMA Current Pointer register */
#define UART1_CONFIG_TX        0xFFC01F02  /* UART -XMT DMA Configuration register */
#define UART1_START_ADDR_HI_TX 0xFFC01F04  /* UART -XMT DMA Start Page register */
#define UART1_START_ADDR_LO_TX 0xFFC01F06  /* UART -XMT DMA Start Address register */
#define UART1_COUNT_TX         0xFFC01F08  /* UART -XMT DMA Count register */
#define UART1_NEXT_DESCR_TX    0xFFC01F0A  /* UART -XMT DMA Next Descriptor Pointer register */
#define UART1_DESCR_RDY_TX     0xFFC01F0C  /* UART -XMT DMA Descriptor Ready */
#define UART1_IRQSTAT_TX       0xFFC01F0E  /* UART -XMT DMA Interrupt register */

/* TIMER 0, 1, 2 Registers (0xFFC0 2000-0xFFC0 23FF) */
#define TIMER0_STATUS          0xFFC02000  /* Timer 0 Global Status and Sticky Register */
#define TIMER0_CONFIG          0xFFC02002  /* Timer 0 configuration Register */
#define TIMER0_COUNTER_LO      0xFFC02004  /* Timer 0 Counter Register (low word) */
#define TIMER0_COUNTER_HI      0xFFC02006  /* Timer 0 Counter Register (high word) */
#define TIMER0_PERIOD_LO       0xFFC02008  /* Timer 0 Period Register (low word) */
#define TIMER0_PERIOD_HI       0xFFC0200A  /* Timer 0 Period Register (high word) */
#define TIMER0_WIDTH_LO        0xFFC0200C  /* Timer 0 Width Register (low word) */
#define TIMER0_WIDTH_HI        0xFFC0200E  /* Timer 0 Width Register (high word) */
#define TIMER1_STATUS          0xFFC02010  /* Timer 1 Global Status and Sticky Register */
#define TIMER1_CONFIG          0xFFC02012  /* Timer 1 configuration register */
#define TIMER1_COUNTER_LO      0xFFC02014  /* Timer 1 Counter Register (low word) */
#define TIMER1_COUNTER_HI      0xFFC02016  /* Timer 1 Counter Register (high word) */
#define TIMER1_PERIOD_LO       0xFFC02018  /* Timer 1 Period Register (low word) */
#define TIMER1_PERIOD_HI       0xFFC0201A  /* Timer 1 Period Register (high word) */
#define TIMER1_WIDTH_LO        0xFFC0201C  /* Timer 1 Width Register (low word) */
#define TIMER1_WIDTH_HI        0xFFC0201E  /* Timer 1 Width Register (high word) */
#define TIMER2_STATUS          0xFFC02020  /* Timer 2 Global Status and Sticky Register */
#define TIMER2_CONFIG          0xFFC02022  /* Timer 2 configuration register */
#define TIMER2_COUNTER_LO      0xFFC02024  /* Timer 2 Counter Register (low word) */
#define TIMER2_COUNTER_HI      0xFFC02026  /* Timer 2 Counter Register (high word) */
#define TIMER2_PERIOD_LO       0xFFC02028  /* Timer 2 Period Register (low word) */
#define TIMER2_PERIOD_HI       0xFFC0202A  /* Timer 2 Period Register (high word) */
#define TIMER2_WIDTH_LO        0xFFC0202C  /* Timer 2 Width Register (low word) */
#define TIMER2_WIDTH_HI        0xFFC0202E  /* Timer 2 Width Register (high word) */

/* General Purpose IO (0xFFC0 2400-0xFFC0 27FF) */
#define FIO_DIR                0xFFC02400  /* Peripheral Flag Direction Register */
#define FIO_FLAG_C             0xFFC02404  /* Peripheral Interrupt Flag Register (clear) */
#define FIO_FLAG_S             0xFFC02406  /* Peripheral Interrupt Flag Register (set) */
#define FIO_MASKA_C            0xFFC02408  /* Flag Mask Interrupt A Register (clear) */
#define FIO_MASKA_S            0xFFC0240A  /* Flag Mask Interrupt A Register (set) */
#define FIO_MASKB_C            0xFFC0240C  /* Flag Mask Interrupt B Register (clear) */
#define FIO_MASKB_S            0xFFC0240E  /* Flag Mask Interrupt B Register (set) */
#define FIO_POLAR              0xFFC02410  /* Flag Source Polarity Register */
#define FIO_EDGE               0xFFC02414  /* Flag Source Sensitivity Register */
#define FIO_BOTH               0xFFC02418  /* Flag Set on BOTH Edges Register */

/* SPORT0 Controller (0xFFC0 2800-0xFFC0 2BFF) */
#define SPORT0_TX_CONFIG       0xFFC02800  /* SPORT0 Transmit Configuration Register */
#define SPORT0_RX_CONFIG       0xFFC02802  /* SPORT0 Receive Configuration Register */
#define SPORT0_TX              0xFFC02804  /* SPORT0 TX transmit Register */
#define SPORT0_RX              0xFFC02806  /* SPORT0 RX Receive register */
#define SPORT0_TSCLKDIV        0xFFC02808  /* SPORT0 Transmit Serial Clock Divider */
#define SPORT0_RSCLKDIV        0xFFC0280A  /* SPORT0 Receive Serial Clock Divider */
#define SPORT0_TFSDIV          0xFFC0280C  /* SPORT0 Transmit Frame Sync Divider */
#define SPORT0_RFSDIV          0xFFC0280E  /* SPORT0 Receive Frame Sync Divider */
#define SPORT0_STAT            0xFFC02810  /* SPORT0 Status Register */
#define SPORT0_MTCS0           0xFFC02812  /* SPORT0 Multi-Channel Transmit Select Register */
#define SPORT0_MTCS1           0xFFC02814  /* SPORT0 Multi-Channel Transmit Select Register */
#define SPORT0_MTCS2           0xFFC02816  /* SPORT0 Multi-Channel Transmit Select Register */
#define SPORT0_MTCS3           0xFFC02818  /* SPORT0 Multi-Channel Transmit Select Register */
#define SPORT0_MTCS4           0xFFC0281A  /* SPORT0 Multi-Channel Transmit Select Register */
#define SPORT0_MTCS5           0xFFC0281C  /* SPORT0 Multi-Channel Transmit Select Register */
#define SPORT0_MTCS6           0xFFC0281E  /* SPORT0 Multi-Channel Transmit Select Register */
#define SPORT0_MTCS7           0xFFC02820  /* SPORT0 Multi-Channel Transmit Select Register */
#define SPORT0_MRCS0           0xFFC02822  /* SPORT0 Multi-Channel Receive Select Register */
#define SPORT0_MRCS1           0xFFC02824  /* SPORT0 Multi-Channel Receive Select Register */
#define SPORT0_MRCS2           0xFFC02826  /* SPORT0 Multi-Channel Receive Select Register */
#define SPORT0_MRCS3           0xFFC02828  /* SPORT0 Multi-Channel Receive Select Register */
#define SPORT0_MRCS4           0xFFC0282A  /* SPORT0 Multi-Channel Receive Select Register */
#define SPORT0_MRCS5           0xFFC0282C  /* SPORT0 Multi-Channel Receive Select Register */
#define SPORT0_MRCS6           0xFFC0282E  /* SPORT0 Multi-Channel Receive Select Register */
#define SPORT0_MRCS7           0xFFC02830  /* SPORT0 Multi-Channel Receive Select Register */
#define SPORT0_MCMC1           0xFFC02832  /* SPORT0 Multi-Channel Configuration Register 1 */
#define SPORT0_MCMC2           0xFFC02834  /* SPORT0 Multi-Channel Configuration Register 2 */
#define SPORT0_CURR_PTR_RX     0xFFC02A00  /* SPORT0 -RCV DMA Current Pointer */
#define SPORT0_CONFIG_DMA_RX   0xFFC02A02  /* SPORT0 -RCV DMA Configuration */
#define SPORT0_START_ADDR_HI_RX 0xFFC02A04 /* SPORT0 -RCV DMA Start Page */
#define SPORT0_START_ADDR_LO_RX 0xFFC02A06 /* SPORT0 -RCV DMA Start Address */
#define SPORT0_COUNT_RX        0xFFC02A08  /* SPORT0 -RCV DMA Count */
#define SPORT0_NEXT_DESCR_RX   0xFFC02A0A  /* SPORT0 -RCV DMA Next Descriptor Pointer */
#define SPORT0_DESCR_RDY_RX    0xFFC02A0C  /* SPORT0 -RCV DMA Descriptor Ready */
#define SPORT0_IRQSTAT_RX      0xFFC02A0E  /* SPORT0 -RCV DMA Interrupt Register */
#define SPORT0_CURR_PTR_TX     0xFFC02B00  /* SPORT0 -XMT DMA Current Pointer */
#define SPORT0_CONFIG_DMA_TX   0xFFC02B02  /* SPORT0 -XMT DMA Configuration */
#define SPORT0_START_ADDR_HI_TX 0xFFC02B04 /* SPORT0 -XMT DMA Start Page */
#define SPORT0_START_ADDR_LO_TX 0xFFC02B06 /* SPORT0 -XMT DMA Start Address */
#define SPORT0_COUNT_TX        0xFFC02B08  /* SPORT0 -XMT DMA Count */
#define SPORT0_NEXT_DESCR_TX   0xFFC02B0A  /* SPORT0 -XMT DMA Next Descriptor Pointer */
#define SPORT0_DESCR_RDY_TX    0xFFC02B0C  /* SPORT0 -XMT DMA Descriptor Ready */
#define SPORT0_IRQSTAT_TX      0xFFC02B0E  /* SPORT0 -XMT DMA Interrupt Register */

/* SPORT1 Controller (0xFFC0 2C00-0xFFC0 2FFF) */
#define SPORT1_TX_CONFIG       0xFFC02C00  /* SPORT1 Transmit Configuration Register */
#define SPORT1_RX_CONFIG       0xFFC02C02  /* SPORT1 Receive Configuration Register */
#define SPORT1_TX              0xFFC02C04  /* SPORT1 TX transmit Register */
#define SPORT1_RX              0xFFC02C06  /* SPORT1 RX Receive register */
#define SPORT1_TSCLKDIV        0xFFC02C08  /* SPORT1 Transmit Serial Clock Divider */
#define SPORT1_RSCLKDIV        0xFFC02C0A  /* SPORT1 Receive Serial Clock Divider */
#define SPORT1_TFSDIV          0xFFC02C0C  /* SPORT1 Transmit Frame Sync Divider */
#define SPORT1_RFSDIV          0xFFC02C0E  /* SPORT1 Receive Frame Sync Divider */
#define SPORT1_STAT            0xFFC02C10  /* SPORT1 Status Register */
#define SPORT1_MTCS0           0xFFC02C12  /* SPORT1 Multi-Channel Transmit Select Register */
#define SPORT1_MTCS1           0xFFC02C14  /* SPORT1 Multi-Channel Transmit Select Register */
#define SPORT1_MTCS2           0xFFC02C16  /* SPORT1 Multi-Channel Transmit Select Register */
#define SPORT1_MTCS3           0xFFC02C18  /* SPORT1 Multi-Channel Transmit Select Register */
#define SPORT1_MTCS4           0xFFC02C1A  /* SPORT1 Multi-Channel Transmit Select Register */
#define SPORT1_MTCS5           0xFFC02C1C  /* SPORT1 Multi-Channel Transmit Select Register */
#define SPORT1_MTCS6           0xFFC02C1E  /* SPORT1 Multi-Channel Transmit Select Register */
#define SPORT1_MTCS7           0xFFC02C20  /* SPORT1 Multi-Channel Transmit Select Register */
#define SPORT1_MRCS0           0xFFC02C22  /* SPORT1 Multi-Channel Receive Select Register */
#define SPORT1_MRCS1           0xFFC02C24  /* SPORT1 Multi-Channel Receive Select Register */
#define SPORT1_MRCS2           0xFFC02C26  /* SPORT1 Multi-Channel Receive Select Register */
#define SPORT1_MRCS3           0xFFC02C28  /* SPORT1 Multi-Channel Receive Select Register */
#define SPORT1_MRCS4           0xFFC02C2A  /* SPORT1 Multi-Channel Receive Select Register */
#define SPORT1_MRCS5           0xFFC02C2C  /* SPORT1 Multi-Channel Receive Select Register */
#define SPORT1_MRCS6           0xFFC02C2E  /* SPORT1 Multi-Channel Receive Select Register */
#define SPORT1_MRCS7           0xFFC02C30  /* SPORT1 Multi-Channel Receive Select Register */
#define SPORT1_MCMC1           0xFFC02C32  /* SPORT1 Multi-Channel Configuration Register 1 */
#define SPORT1_MCMC2           0xFFC02C34  /* SPORT1 Multi-Channel Configuration Register 2 */
#define SPORT1_CURR_PTR_RX     0xFFC02E00  /* SPORT1 -RCV DMA Current Pointer */
#define SPORT1_CONFIG_DMA_RX   0xFFC02E02  /* SPORT1 -RCV DMA Configuration */
#define SPORT1_START_ADDR_HI_RX 0xFFC02E04 /* SPORT1 -RCV DMA Start Page */
#define SPORT1_START_ADDR_LO_RX 0xFFC02E06 /* SPORT1 -RCV DMA Start Address */
#define SPORT1_COUNT_RX        0xFFC02E08  /* SPORT1 -RCV DMA Count */
#define SPORT1_NEXT_DESCR_RX   0xFFC02E0A  /* SPORT1 -RCV DMA Next Descriptor Pointer */
#define SPORT1_DESCR_RDY_RX    0xFFC02E0C  /* SPORT1 -RCV DMA Descriptor Ready */
#define SPORT1_IRQSTAT_RX      0xFFC02E0E  /* SPORT1 -RCV DMA Interrupt Register */
#define SPORT1_CURR_PTR_TX     0xFFC02F00  /* SPORT1 -XMT DMA Current Pointer */
#define SPORT1_CONFIG_DMA_TX   0xFFC02F02  /* SPORT1 -XMT DMA Configuration */
#define SPORT1_START_ADDR_HI_TX 0xFFC02F04 /* SPORT1 -XMT DMA Start Page */
#define SPORT1_START_ADDR_LO_TX 0xFFC02F06 /* SPORT1 -XMT DMA Start Address */
#define SPORT1_COUNT_TX        0xFFC02F08  /* SPORT1 -XMT DMA Count */
#define SPORT1_NEXT_DESCR_TX   0xFFC02F0A  /* SPORT1 -XMT DMA Next Descriptor Pointer */
#define SPORT1_DESCR_RDY_TX    0xFFC02F0C  /* SPORT1 -XMT DMA Descriptor Ready */
#define SPORT1_IRQSTAT_TX      0xFFC02F0E  /* SPORT1 -XMT DMA Interrupt Register */

/* SPI 0 Controller (0xFFC0 3000-0xFFC0 33FF) */
#define SPI0_CTL               0xFFC03000  /* SPI0 Control Register */
#define SPI0_FLG               0xFFC03002  /* SPI0 Flag register */
#define SPI0_ST                0xFFC03004  /* SPI0 Status register */
#define SPI0_TDBR              0xFFC03006  /* SPI0 Transmit Data Buffer Register */
#define SPI0_RDBR              0xFFC03008  /* SPI0 Receive Data Buffer Register */
#define SPI0_BAUD              0xFFC0300A  /* SPI0 Baud rate Register */
#define SPI0_SHADOW            0xFFC0300C
#define SPI0_CURR_PTR          0xFFC03200  /* SPI0 -DMA Current Pointer register */
#define SPI0_CONFIG            0xFFC03202  /* SPI0 -DMA Configuration register */
#define SPI0_START_ADDR_HI     0xFFC03204  /* SPI0 -DMA Start Page register */
#define SPI0_START_ADDR_LO     0xFFC03206  /* SPI0 -DMA Start Address register */
#define SPI0_COUNT             0xFFC03208  /* SPI0 -DMA Count register */
#define SPI0_NEXT_DESCR        0xFFC0320A  /* SPI0 -DMA Next Descriptor Pointer */
#define SPI0_DESCR_RDY         0xFFC0320C  /* SPI0 -DMA Descriptor Ready */
#define SPI0_DMA_INT           0xFFC0320E  /* SPI0 -DMA Interrupt register */

/* SPI 1 Controller (0xFFC0 3400-0xFFC0 37FF) */
#define SPI1_CTL               0xFFC03400  /* SPI1 Control Register */
#define SPI1_FLG               0xFFC03402  /* SPI1 Flag register */
#define SPI1_ST                0xFFC03404  /* SPI1 Status register */
#define SPI1_TDBR              0xFFC03406  /* SPI1 Transmit Data Buffer Register */
#define SPI1_RDBR              0xFFC03408  /* SPI1 Receive Data Buffer Register */
#define SPI1_BAUD              0xFFC0340A  /* SPI1 Baud rate Register */
#define SPI1_SHADOW            0xFFC0340C
#define SPI1_CURR_PTR          0xFFC03600  /* SPI1 -DMA Current Pointer register */
#define SPI1_CONFIG            0xFFC03602  /* SPI1 -DMA Configuration register */
#define SPI1_START_ADDR_HI     0xFFC03604  /* SPI1 -DMA Start Page register */
#define SPI1_START_ADDR_LO     0xFFC03606  /* SPI1 -DMA Start Address register */
#define SPI1_COUNT             0xFFC03608  /* SPI1 -DMA Count register */
#define SPI1_NEXT_DESCR        0xFFC0360A  /* SPI1 -DMA Next Descriptor Pointer */
#define SPI1_DESCR_RDY         0xFFC0360C  /* SPI1 -DMA Descriptor Ready */
#define SPI1_DMA_INT           0xFFC0360E  /* SPI1 -DMA Interrupt register */

/* Memory DMA Controller (0xFFC0 3800-0xFFC0 3BFF) */
#define MDD_DCP                0xFFC03800  /* Current Pointer - Write Channel */
#define MDD_DCFG               0xFFC03802  /* DMA Configuration - Write Channel */
#define MDD_DSAH               0xFFC03804  /* Start Address Hi - Write Channel */
#define MDD_DSAL               0xFFC03806  /* Start Address Lo - Write Channel */
#define MDD_DCT                0xFFC03808  /* DMA Count - Write Channel */
#define MDD_DND                0xFFC0380A  /* Next Descriptor Pointer - Write Channel */
#define MDD_DDR                0xFFC0380C  /* Descriptor Ready - Write Channel */
#define MDD_DI                 0xFFC0380E  /* DMA Interrupt - Write Channel */
#define MDS_DCP                0xFFC03900  /* Current Pointer - Read Channel */
#define MDS_DCFG               0xFFC03902  /* DMA Configuration - Read Channel */
#define MDS_DSAH               0xFFC03904  /* Start Address Hi - Read Channel */
#define MDS_DSAL               0xFFC03906  /* Start Address Lo - Read Channel */
#define MDS_DCT                0xFFC03908  /* DMA Count - Read Channel */
#define MDS_DND                0xFFC0390A  /* Next Descriptor Pointer - Read Channel */
#define MDS_DDR                0xFFC0390C  /* Descriptor Ready - Read Channel */
#define MDS_DI                 0xFFC0390E  /* DMA Interrupt - Read Channel */

/* For backwards-compatibility with VDSP++3.0 and earlier code... */
#define MDW_DCP                MDD_DCP
#define MDW_DCFG               MDD_DCFG
#define MDW_DSAH               MDD_DSAH
#define MDW_DSAL               MDD_DSAL
#define MDW_DCT                MDD_DCT
#define MDW_DND                MDD_DND
#define MDW_DDR                MDD_DDR
#define MDW_DI                 MDD_DI
#define MDR_DCP                MDS_DCP
#define MDR_DCFG               MDS_DCFG
#define MDR_DSAH               MDS_DSAH
#define MDR_DSAL               MDS_DSAL
#define MDR_DCT                MDS_DCT
#define MDR_DND                MDS_DND
#define MDR_DDR                MDS_DDR
#define MDR_DI                 MDS_DI

/* Aysnchronous Memory Controller - External Bus Interface Unit (0xFFC0 3C00-0xFFC0 3FFF) */
#define EBIU_AMGCTL            0xFFC03C00  /* Asynchronous Memory Global Control Register */
#define EBIU_AMBCTL0           0xFFC03C04  /* Asynchronous Memory Bank Control Register 0 */
#define EBIU_AMBCTL1           0xFFC03C08  /* Asynchronous Memory Bank Control Register 1 */

/* PCI Bridge PAB Registers (0xFFC0 4000-0xFFC0 43FF) */
#define PCI_CTL                0xFFC04000  /* PCI Bridge Control */
#define  PCI_CTL_HOST	 	 0x01
#define  PCI_CTL_ENABPCI	 0x02
#define  PCI_CTL_FASTBCK2BCK	 0x04
#define  PCI_CTL_ENABINTA	 0x08
#define  PCI_CTL_OUTPUTINTA	 0x10
#define  PCI_CTL_ENABRST	 0x20
#define  PCI_CTL_OUTPUTRST	 0x40


#define PCI_STAT               0xFFC04004  /* PCI Bridge Status */
#define   PCI_STAT_INTA	         0x0001
#define   PCI_STAT_INTB	         0x0002
#define   PCI_STAT_INTC	         0x0004
#define   PCI_STAT_INTD	         0x0008
#define   PCI_STAT_PARERR	 0x0010
#define   PCI_STAT_FATERR	 0x0020
#define   PCI_STAT_RESET	 0x0040
#define   PCI_STAT_TXEMPTY	 0x0080
#define   PCI_STAT_TXFULL	 0x0100
#define   PCI_STAT_QUEFULL	 0x0200
#define   PCI_STAT_MEMWRINV	 0x0400
#define   PCI_STAT_INRDERR	 0x0800
#define   PCI_STAT_INWRERR	 0x1000
#define   PCI_STAT_INVEABACC	 0x2000
#define   PCI_STAT_SYSERR	 0x4000

#define PCI_ICTL               0xFFC04008  /* PCI Bridge Interrupt Control */
#define   PCI_ICTL_INTA	         0x0001
#define   PCI_ICTL_INTB	         0x0002
#define   PCI_ICTL_INTC	         0x0004
#define   PCI_ICTL_INTD	         0x0008
#define   PCI_ICTL_PARERR	 0x0010
#define   PCI_ICTL_FATERR	 0x0020
#define   PCI_ICTL_RESET	 0x0040
#define   PCI_ICTL_TXFULL	 0x0080
#define   PCI_ICTL_MEMWRINV	 0x0400
#define   PCI_ICTL_INRDERR	 0x0800
#define   PCI_ICTL_INWRERR	 0x1000
#define   PCI_ICTL_INVEABACC	 0x2000
#define   PCI_ICTL_SYSERR	 0x4000

#define PCI_MBAP               0xFFC0400C  /* PCI Memory Space Base Address Pointer [31:27] */
#define PCI_IBAP               0xFFC04010  /* PCI IO Space Base Address Pointer */
#define PCI_CBAP               0xFFC04014  /* PCI Config Space Base Address Port */
#define PCI_TMBAP              0xFFC04018  /* PCI to BF535 Memory Base Address Pointer */
#define PCI_TIBAP              0xFFC0401C  /* PCI to BF535 IO Base Address Pointer */

/* PCI Bridge External Access Bus Registers (0xEEFF FF00-0xEEFF FFFF) */
#define PCI_DMBARM             0xEEFFFF00  /* PCI Device Memory Bar Mask */
#define PCI_DIBARM             0xEEFFFF04  /* PCI Device IO Bar Mask */
#define PCI_CFG_DIC            0xEEFFFF08  /* PCI Config Device ID */
#define PCI_CFG_VIC            0xEEFFFF0C  /* PCI Config Vendor ID */
#define PCI_CFG_STAT           0xEEFFFF10  /* PCI Config Status (Read-only) */
#define PCI_CFG_CMD            0xEEFFFF14  /* PCI Config Command */
#define PCI_CFG_CC             0xEEFFFF18  /* PCI Config Class Code */
#define PCI_CFG_RID            0xEEFFFF1C  /* PCI Config Revision ID */
#define PCI_CFG_BIST           0xEEFFFF20  /* PCI Config BIST */
#define PCI_CFG_HT             0xEEFFFF24  /* PCI Config Header Type */
#define PCI_CFG_MLT            0xEEFFFF28  /* PCI Config Memory Latency Timer */
#define PCI_CFG_CLS            0xEEFFFF2C  /* PCI Config Cache Line Size */
#define PCI_CFG_MBAR           0xEEFFFF30  /* PCI Config Memory Base Address Register */
#define PCI_CFG_IBAR           0xEEFFFF34  /* PCI Config IO Base Address Register */
#define PCI_CFG_SID            0xEEFFFF38  /* PCI Config Sub-system ID */
#define PCI_CFG_SVID           0xEEFFFF3C  /* PCI Config Sub-system Vendor ID */
#define PCI_CFG_MAXL           0xEEFFFF40  /* PCI Config Maximum Latency Cycles */
#define PCI_CFG_MING           0xEEFFFF44  /* PCI Config Minimum Grant Cycles */
#define PCI_CFG_IP             0xEEFFFF48  /* PCI Config Interrupt Pin */
#define PCI_CFG_IL             0xEEFFFF4C  /* PCI Config Interrupt Line */
#define PCI_HMCTL              0xEEFFFF50  /* PCI Blocking BAR Host Mode Control */

#define  PCI_HMCTL_SYSMMRENAB	 0x1
#define  PCI_HMCTL_L2ENAB	 0x2
#define  PCI_HMCTL_ASYNCENAB	 0x4
#define  PCI_HMCTL_ASYNCSIZE	 0x18	/* 00-64MB, 01-128MB, 10-192MB, 11-256MB */
#define  PCI_HMCTL_SDRAMENAB	 0x20
#define  PCI_HMCTL_SDRAMSIZE	 0x7C0	/* 0-32MB, 1-64MB, 2-96MB, 128MB, 160MB */

/* USB Registers (0xFFC0 4400 - 0xFFC0 47FF) */
#define USBD_ID                0xFFC04400  /* USB Device ID Register */
#define USBD_FRM               0xFFC04402  /* Current USB Frame Number */
#define USBD_FRMAT             0xFFC04404  /* Match value for USB frame number. */
#define USBD_EPBUF             0xFFC04406  /* Enables Download of Configuration Into UDC Core */
#define USBD_STAT              0xFFC04408  /* Returns USBD Module Status */
#define USBD_CTRL              0xFFC0440A  /* Allows Configuration and Control of USBD Module. */
#define USBD_GINTR             0xFFC0440C  /* Global Interrupt Register */
#define USBD_GMASK             0xFFC0440E  /* Global Interrupt Register Mask */
#define USBD_DMACFG            0xFFC04440  /* DMA Master Channel Configuration Register */
#define USBD_DMABL             0xFFC04442  /* DMA Master Channel Base Address, Low */
#define USBD_DMABH             0xFFC04444  /* DMA Master Channel Base Address, High */
#define USBD_DMACT             0xFFC04446  /* DMA Master Channel Count Register */
#define USBD_DMAIRQ            0xFFC04448  /* DMA Master Channel DMA Count Register */
#define USBD_INTR0             0xFFC04480  /* USB Endpoint 0 Interrupt Register */
#define USBD_MASK0             0xFFC04482  /* USB Endpoint 0 Mask Register */
#define USBD_EPCFG0            0xFFC04484  /* USB Endpoint 0 Control Register */
#define USBD_EPADR0            0xFFC04486  /* USB Endpoint 0 Address Offset Register */
#define USBD_EPLEN0            0xFFC04488  /* USB Endpoint 0 Buffer Length Register */
#define USBD_INTR1             0xFFC0448A  /* USB Endpoint 1 Interrupt Register */
#define USBD_MASK1             0xFFC0448C  /* USB Endpoint 1 Mask Register */
#define USBD_EPCFG1            0xFFC0448E  /* USB Endpoint 1 Control Register */
#define USBD_EPADR1            0xFFC04490  /* USB Endpoint 1 Address Offset Register */
#define USBD_EPLEN1            0xFFC04492  /* USB Endpoint 1 Buffer Length Register */
#define USBD_INTR2             0xFFC04494  /* USB Endpoint 2 Interrupt Register */
#define USBD_MASK2             0xFFC04496  /* USB Endpoint 2 Mask Register */
#define USBD_EPCFG2            0xFFC04498  /* USB Endpoint 2 Control Register */
#define USBD_EPADR2            0xFFC0449A  /* USB Endpoint 2 Address Offset Register */
#define USBD_EPLEN2            0xFFC0449C  /* USB Endpoint 2 Buffer Length Register */
#define USBD_INTR3             0xFFC0449E  /* USB Endpoint 3 Interrupt Register */
#define USBD_MASK3             0xFFC044A0  /* USB Endpoint 3 Mask Register */
#define USBD_EPCFG3            0xFFC044A2  /* USB Endpoint 3 Control Register */
#define USBD_EPADR3            0xFFC044A4  /* USB Endpoint 3 Address Offset Register */
#define USBD_EPLEN3            0xFFC044A6  /* USB Endpoint 3 Buffer Length Register */
#define USBD_INTR4             0xFFC044A8  /* USB Endpoint 4 Interrupt Register */
#define USBD_MASK4             0xFFC044AA  /* USB Endpoint 4 Mask Register */
#define USBD_EPCFG4            0xFFC044AC  /* USB Endpoint 4 Control Register */
#define USBD_EPADR4            0xFFC044AE  /* USB Endpoint 4 Address Offset Register */
#define USBD_EPLEN4            0xFFC044B0  /* USB Endpoint 4 Buffer Length Register */
#define USBD_INTR5             0xFFC044B2  /* USB Endpoint 5 Interrupt Register */
#define USBD_MASK5             0xFFC044B4  /* USB Endpoint 5 Mask Register */
#define USBD_EPCFG5            0xFFC044B6  /* USB Endpoint 5 Control Register */
#define USBD_EPADR5            0xFFC044B8  /* USB Endpoint 5 Address Offset Register */
#define USBD_EPLEN5            0xFFC044BA  /* USB Endpoint 5 Buffer Length Register */
#define USBD_INTR6             0xFFC044BC  /* USB Endpoint 6 Interrupt Register */
#define USBD_MASK6             0xFFC044BE  /* USB Endpoint 6 Mask Register */
#define USBD_EPCFG6            0xFFC044C0  /* USB Endpoint 6 Control Register */
#define USBD_EPADR6            0xFFC044C2  /* USB Endpoint 6 Address Offset Register */
#define USBD_EPLEN6            0xFFC044C4  /* USB Endpoint 6 Buffer Length Register */
#define USBD_INTR7             0xFFC044C6  /* USB Endpoint 7 Interrupt Register */
#define USBD_MASK7             0xFFC044C8  /* USB Endpoint 7 Mask Register */
#define USBD_EPCFG7            0xFFC044CA  /* USB Endpoint 7 Control Register */
#define USBD_EPADR7            0xFFC044CC  /* USB Endpoint 7 Address Offset Register */
#define USBD_EPLEN7            0xFFC044CE  /* USB Endpoint 7 Buffer Length Register */

/* System Bus Interface Unit (0xFFC0 4800-0xFFC0 4FFF) */
#define L1SBAR                 0xFFC04840  /* L1 SRAM Base Address Register */
#define L1CSR                  0xFFC04844  /* L1 SRAM Control Initialization Register */
#define DMA_DBP                0xFFC04880  /* Next Descriptor Base Pointer */
#define DB_ACOMP               0xFFC04884  /* DMA Bus Address Comparator */
#define DB_CCOMP               0xFFC04888  /* DMA Bus Control Comparator */

#define DB_NDBP                DMA_DBP     /* Backward compatibility */

#define L1_SBAR			L1SBAR
#define L1_CSR			L1CSR

/* SDRAM Controller External Bus Interface Unit (0xFFC0 4C00-0xFFC0 4FFF) */
#define EBIU_SDGCTL            0xFFC04C00  /* SDRAM Global Control Register */
#define EBIU_SDBCTL            0xFFC04C04  /* SDRAM Bank Control Register */
#define EBIU_SDRRC             0xFFC04C0A  /* SDRAM Refresh Rate Control Register */
#define EBIU_SDSTAT            0xFFC04C0E  /* SDRAM Status Register */

/* PAB Reserved (0xFFC0 5000-0xFFDF FFFF) (**Reserved**) */

/*********************************************************************************** */
/* System MMR Register Bits */
/*********************************************************************************** */

/* PLLCTL Masks */
#define PLL_CLKIN              0x00000000  /* Pass CLKIN to PLL */
#define PLL_CLKIN_DIV2         0x00000001  /* Pass CLKIN/2 to PLL */
#define PLL_OFF                0x00000002  /* Shut off PLL clocks */
#define STOPCK_OFF             0x00000008  /* Core clock off */
#define PDWN                   0x00000020  /* Put the PLL in a Deep Sleep state */
#define BYPASS                 0x00000100  /* Bypass the PLL */
#define CCLK_DIV2              0x00000000  /* SCLK = CCLK / 2 */
#define CCLK_DIV2_5            0x00010000  /* SCLK = CCLK / 2.5 */
#define CCLK_DIV3              0x00020000  /* SCLK = CCLK / 3 */
#define CCLK_DIV4              0x00030000  /* SCLK = CCLK / 4 */

/* IOCKR Masks */
#define IOCK_PCI               0x00000001  /* Enable PCI peripheral clock */
#define IOCK_L2                0x00000002  /* Enable L2 memory peripheral clock */
#define IOCK_EBIU              0x00000004  /* Enable EBIU controller peripheral clock */
#define IOCK_GPIO              0x00000008  /* Enable GPIO peripheral clock */
#define IOCK_MEMDMA            0x00000010  /* Enable MemDMA controller peripheral clock */
#define IOCK_SPORT0            0x00000020  /* Enable SPORT0 controller peripheral clock */
#define IOCK_SPORT1            0x00000040  /* Enable SPORT1 controller peripheral clock */
#define IOCK_SPI0              0x00000080  /* Enable SPI0 controller peripheral clock */
#define IOCK_SPI1              0x00000100  /* Enable SPI1 controller peripheral clock */
#define IOCK_UART0             0x00000200  /* Enable UART0 controller peripheral clock */
#define IOCK_UART1             0x00000400  /* Enable UART1 controller peripheral clock */
#define IOCK_TIMER0            0x00000800  /* Enable TIMER0 peripheral clock */
#define IOCK_TIMER1            0x00001000  /* Enable TIMER1 peripheral clock */
#define IOCK_TIMER2            0x00002000  /* Enable TIMER2 peripheral clock */
#define IOCK_USB               0x00004000  /* Enable USB peripheral clock */

/* SWRST Mask */
#define SYSTEM_RESET           0x00000007  /* Initiates a system software reset */

/* System Interrupt Controller Masks (SIC_IAR0, SIC_IAR1, SIC_IAR2, SIC_IMASK, SIC_IWR) */
/* SIC_IAR0 Masks */

/* */
#define P0_IVG7                0x00000000  /* Peripheral #0 assigned IVG7 */
#define P0_IVG8                0x00000001  /* Peripheral #0 assigned IVG8 */
#define P0_IVG9                0x00000002  /* Peripheral #0 assigned IVG9 */
#define P0_IVG10               0x00000003  /* Peripheral #0 assigned IVG10 */
#define P0_IVG11               0x00000004  /* Peripheral #0 assigned IVG11 */
#define P0_IVG12               0x00000005  /* Peripheral #0 assigned IVG12 */
#define P0_IVG13               0x00000006  /* Peripheral #0 assigned IVG13 */
#define P0_IVG14               0x00000007  /* Peripheral #0 assigned IVG14 */
#define P0_IVG15               0x00000008  /* Peripheral #0 assigned IVG15 */
#define P1_IVG7                0x00000000  /* Peripheral #1 assigned IVG7 */
#define P1_IVG8                0x00000010  /* Peripheral #1 assigned IVG8 */
#define P1_IVG9                0x00000020  /* Peripheral #1 assigned IVG9 */
#define P1_IVG10               0x00000030  /* Peripheral #1 assigned IVG10 */
#define P1_IVG11               0x00000040  /* Peripheral #1 assigned IVG11 */
#define P1_IVG12               0x00000050  /* Peripheral #1 assigned IVG12 */
#define P1_IVG13               0x00000060  /* Peripheral #1 assigned IVG13 */
#define P1_IVG14               0x00000070  /* Peripheral #1 assigned IVG14 */
#define P1_IVG15               0x00000080  /* Peripheral #1 assigned IVG15 */
#define P2_IVG7                0x00000000  /* Peripheral #2 assigned IVG7 */
#define P2_IVG8                0x00000100  /* Peripheral #2 assigned IVG8 */
#define P2_IVG9                0x00000200  /* Peripheral #2 assigned IVG9 */
#define P2_IVG10               0x00000300  /* Peripheral #2 assigned IVG10 */
#define P2_IVG11               0x00000400  /* Peripheral #2 assigned IVG11 */
#define P2_IVG12               0x00000500  /* Peripheral #2 assigned IVG12 */
#define P2_IVG13               0x00000600  /* Peripheral #2 assigned IVG13 */
#define P2_IVG14               0x00000700  /* Peripheral #2 assigned IVG14 */
#define P2_IVG15               0x00000800  /* Peripheral #2 assigned IVG15 */
#define P3_IVG7                0x00000000  /* Peripheral #3 assigned IVG7 */
#define P3_IVG8                0x00001000  /* Peripheral #3 assigned IVG8 */
#define P3_IVG9                0x00002000  /* Peripheral #3 assigned IVG9 */
#define P3_IVG10               0x00003000  /* Peripheral #3 assigned IVG10 */
#define P3_IVG11               0x00004000  /* Peripheral #3 assigned IVG11 */
#define P3_IVG12               0x00005000  /* Peripheral #3 assigned IVG12 */
#define P3_IVG13               0x00006000  /* Peripheral #3 assigned IVG13 */
#define P3_IVG14               0x00007000  /* Peripheral #3 assigned IVG14 */
#define P3_IVG15               0x00008000  /* Peripheral #3 assigned IVG15 */
#define P4_IVG7                0x00000000  /* Peripheral #4 assigned IVG7 */
#define P4_IVG8                0x00010000  /* Peripheral #4 assigned IVG8 */
#define P4_IVG9                0x00020000  /* Peripheral #4 assigned IVG9 */
#define P4_IVG10               0x00030000  /* Peripheral #4 assigned IVG10 */
#define P4_IVG11               0x00040000  /* Peripheral #4 assigned IVG11 */
#define P4_IVG12               0x00050000  /* Peripheral #4 assigned IVG12 */
#define P4_IVG13               0x00060000  /* Peripheral #4 assigned IVG13 */
#define P4_IVG14               0x00070000  /* Peripheral #4 assigned IVG14 */
#define P4_IVG15               0x00080000  /* Peripheral #4 assigned IVG15 */
#define P5_IVG7                0x00000000  /* Peripheral #5 assigned IVG7 */
#define P5_IVG8                0x00100000  /* Peripheral #5 assigned IVG8 */
#define P5_IVG9                0x00200000  /* Peripheral #5 assigned IVG9 */
#define P5_IVG10               0x00300000  /* Peripheral #5 assigned IVG10 */
#define P5_IVG11               0x00400000  /* Peripheral #5 assigned IVG11 */
#define P5_IVG12               0x00500000  /* Peripheral #5 assigned IVG12 */
#define P5_IVG13               0x00600000  /* Peripheral #5 assigned IVG13 */
#define P5_IVG14               0x00700000  /* Peripheral #5 assigned IVG14 */
#define P5_IVG15               0x00800000  /* Peripheral #5 assigned IVG15 */
#define P6_IVG7                0x00000000  /* Peripheral #6 assigned IVG7 */
#define P6_IVG8                0x01000000  /* Peripheral #6 assigned IVG8 */
#define P6_IVG9                0x02000000  /* Peripheral #6 assigned IVG9 */
#define P6_IVG10               0x03000000  /* Peripheral #6 assigned IVG10 */
#define P6_IVG11               0x04000000  /* Peripheral #6 assigned IVG11 */
#define P6_IVG12               0x05000000  /* Peripheral #6 assigned IVG12 */
#define P6_IVG13               0x06000000  /* Peripheral #6 assigned IVG13 */
#define P6_IVG14               0x07000000  /* Peripheral #6 assigned IVG14 */
#define P6_IVG15               0x08000000  /* Peripheral #6 assigned IVG15 */
#define P7_IVG7                0x00000000  /* Peripheral #7 assigned IVG7 */
#define P7_IVG8                0x10000000  /* Peripheral #7 assigned IVG8 */
#define P7_IVG9                0x20000000  /* Peripheral #7 assigned IVG9 */
#define P7_IVG10               0x30000000  /* Peripheral #7 assigned IVG10 */
#define P7_IVG11               0x40000000  /* Peripheral #7 assigned IVG11 */
#define P7_IVG12               0x50000000  /* Peripheral #7 assigned IVG12 */
#define P7_IVG13               0x60000000  /* Peripheral #7 assigned IVG13 */
#define P7_IVG14               0x70000000  /* Peripheral #7 assigned IVG14 */
#define P7_IVG15               0x80000000  /* Peripheral #7 assigned IVG15 */

/* SIC_IAR1 Masks */
#define P8_IVG7                0x00000000  /* Peripheral #8 assigned IVG7 */
#define P8_IVG8                0x00000001  /* Peripheral #8 assigned IVG8 */
#define P8_IVG9                0x00000002  /* Peripheral #8 assigned IVG9 */
#define P8_IVG10               0x00000003  /* Peripheral #8 assigned IVG10 */
#define P8_IVG11               0x00000004  /* Peripheral #8 assigned IVG11 */
#define P8_IVG12               0x00000005  /* Peripheral #8 assigned IVG12 */
#define P8_IVG13               0x00000006  /* Peripheral #8 assigned IVG13 */
#define P8_IVG14               0x00000007  /* Peripheral #8 assigned IVG14 */
#define P8_IVG15               0x00000008  /* Peripheral #8 assigned IVG15 */
#define P9_IVG7                0x00000000  /* Peripheral #9 assigned IVG7 */
#define P9_IVG8                0x00000010  /* Peripheral #9 assigned IVG8 */
#define P9_IVG9                0x00000020  /* Peripheral #9 assigned IVG9 */
#define P9_IVG10               0x00000030  /* Peripheral #9 assigned IVG10 */
#define P9_IVG11               0x00000040  /* Peripheral #9 assigned IVG11 */
#define P9_IVG12               0x00000050  /* Peripheral #9 assigned IVG12 */
#define P9_IVG13               0x00000060  /* Peripheral #9 assigned IVG13 */
#define P9_IVG14               0x00000070  /* Peripheral #9 assigned IVG14 */
#define P9_IVG15               0x00000080  /* Peripheral #9 assigned IVG15 */
#define P10_IVG7               0x00000000  /* Peripheral #10 assigned IVG7 */
#define P10_IVG8               0x00000100  /* Peripheral #10 assigned IVG8 */
#define P10_IVG9               0x00000200  /* Peripheral #10 assigned IVG9 */
#define P10_IVG10              0x00000300  /* Peripheral #10 assigned IVG10 */
#define P10_IVG11              0x00000400  /* Peripheral #10 assigned IVG11 */
#define P10_IVG12              0x00000500  /* Peripheral #10 assigned IVG12 */
#define P10_IVG13              0x00000600  /* Peripheral #10 assigned IVG13 */
#define P10_IVG14              0x00000700  /* Peripheral #10 assigned IVG14 */
#define P10_IVG15              0x00000800  /* Peripheral #10 assigned IVG15 */
#define P11_IVG7               0x00000000  /* Peripheral #11 assigned IVG7 */
#define P11_IVG8               0x00001000  /* Peripheral #11 assigned IVG8 */
#define P11_IVG9               0x00002000  /* Peripheral #11 assigned IVG9 */
#define P11_IVG10              0x00003000  /* Peripheral #11 assigned IVG10 */
#define P11_IVG11              0x00004000  /* Peripheral #11 assigned IVG11 */
#define P11_IVG12              0x00005000  /* Peripheral #11 assigned IVG12 */
#define P11_IVG13              0x00006000  /* Peripheral #11 assigned IVG13 */
#define P11_IVG14              0x00007000  /* Peripheral #11 assigned IVG14 */
#define P11_IVG15              0x00008000  /* Peripheral #11 assigned IVG15 */
#define P12_IVG7               0x00000000  /* Peripheral #12 assigned IVG7 */
#define P12_IVG8               0x00010000  /* Peripheral #12 assigned IVG8 */
#define P12_IVG9               0x00020000  /* Peripheral #12 assigned IVG9 */
#define P12_IVG10              0x00030000  /* Peripheral #12 assigned IVG10 */
#define P12_IVG11              0x00040000  /* Peripheral #12 assigned IVG11 */
#define P12_IVG12              0x00050000  /* Peripheral #12 assigned IVG12 */
#define P12_IVG13              0x00060000  /* Peripheral #12 assigned IVG13 */
#define P12_IVG14              0x00070000  /* Peripheral #12 assigned IVG14 */
#define P12_IVG15              0x00080000  /* Peripheral #12 assigned IVG15 */
#define P13_IVG7               0x00000000  /* Peripheral #13 assigned IVG7 */
#define P13_IVG8               0x00100000  /* Peripheral #13 assigned IVG8 */
#define P13_IVG9               0x00200000  /* Peripheral #13 assigned IVG9 */
#define P13_IVG10              0x00300000  /* Peripheral #13 assigned IVG10 */
#define P13_IVG11              0x00400000  /* Peripheral #13 assigned IVG11 */
#define P13_IVG12              0x00500000  /* Peripheral #13 assigned IVG12 */
#define P13_IVG13              0x00600000  /* Peripheral #13 assigned IVG13 */
#define P13_IVG14              0x00700000  /* Peripheral #14 assigned IVG14 */
#define P13_IVG15              0x00800000  /* Peripheral #14 assigned IVG15 */
#define P14_IVG7               0x00000000  /* Peripheral #14 assigned IVG7 */
#define P14_IVG8               0x01000000  /* Peripheral #14 assigned IVG8 */
#define P14_IVG9               0x02000000  /* Peripheral #14 assigned IVG9 */
#define P14_IVG10              0x03000000  /* Peripheral #14 assigned IVG10 */
#define P14_IVG11              0x04000000  /* Peripheral #14 assigned IVG11 */
#define P14_IVG12              0x05000000  /* Peripheral #14 assigned IVG12 */
#define P14_IVG13              0x06000000  /* Peripheral #14 assigned IVG13 */
#define P14_IVG14              0x07000000  /* Peripheral #14 assigned IVG14 */
#define P14_IVG15              0x08000000  /* Peripheral #14 assigned IVG15 */
#define P15_IVG7               0x00000000  /* Peripheral #15 assigned IVG7 */
#define P15_IVG8               0x10000000  /* Peripheral #15 assigned IVG8 */
#define P15_IVG9               0x20000000  /* Peripheral #15 assigned IVG9 */
#define P15_IVG10              0x30000000  /* Peripheral #15 assigned IVG10 */
#define P15_IVG11              0x40000000  /* Peripheral #15 assigned IVG11 */
#define P15_IVG12              0x50000000  /* Peripheral #15 assigned IVG12 */
#define P15_IVG13              0x60000000  /* Peripheral #15 assigned IVG13 */
#define P15_IVG14              0x70000000  /* Peripheral #15 assigned IVG14 */
#define P15_IVG15              0x80000000  /* Peripheral #15 assigned IVG15 */

/* SIC_IAR2 Masks */
#define P16_IVG7               0x00000000  /* Peripheral #16 assigned IVG7 */
#define P16_IVG8               0x00000001  /* Peripheral #16 assigned IVG8 */
#define P16_IVG9               0x00000002  /* Peripheral #16 assigned IVG9 */
#define P16_IVG10              0x00000003  /* Peripheral #16 assigned IVG10 */
#define P16_IVG11              0x00000004  /* Peripheral #16 assigned IVG11 */
#define P16_IVG12              0x00000005  /* Peripheral #16 assigned IVG12 */
#define P16_IVG13              0x00000006  /* Peripheral #16 assigned IVG13 */
#define P16_IVG14              0x00000007  /* Peripheral #16 assigned IVG14 */
#define P16_IVG15              0x00000008  /* Peripheral #16 assigned IVG15 */
#define P17_IVG7               0x00000000  /* Peripheral #17 assigned IVG7 */
#define P17_IVG8               0x00000010  /* Peripheral #17 assigned IVG8 */
#define P17_IVG9               0x00000020  /* Peripheral #17 assigned IVG9 */
#define P17_IVG10              0x00000030  /* Peripheral #17 assigned IVG10 */
#define P17_IVG11              0x00000040  /* Peripheral #17 assigned IVG11 */
#define P17_IVG12              0x00000050  /* Peripheral #17 assigned IVG12 */
#define P17_IVG13              0x00000060  /* Peripheral #17 assigned IVG13 */
#define P17_IVG14              0x00000070  /* Peripheral #17 assigned IVG14 */
#define P17_IVG15              0x00000080  /* Peripheral #17 assigned IVG15 */
#define P18_IVG7               0x00000000  /* Peripheral #18 assigned IVG7 */
#define P18_IVG8               0x00000100  /* Peripheral #18 assigned IVG8 */
#define P18_IVG9               0x00000200  /* Peripheral #18 assigned IVG9 */
#define P18_IVG10              0x00000300  /* Peripheral #18 assigned IVG10 */
#define P18_IVG11              0x00000400  /* Peripheral #18 assigned IVG11 */
#define P18_IVG12              0x00000500  /* Peripheral #18 assigned IVG12 */
#define P18_IVG13              0x00000600  /* Peripheral #18 assigned IVG13 */
#define P18_IVG14              0x00000700  /* Peripheral #18 assigned IVG14 */
#define P18_IVG15              0x00000800  /* Peripheral #18 assigned IVG15 */
#define P19_IVG7               0x00000000  /* Peripheral #19 assigned IVG7 */
#define P19_IVG8               0x00001000  /* Peripheral #19 assigned IVG8 */
#define P19_IVG9               0x00002000  /* Peripheral #19 assigned IVG9 */
#define P19_IVG10              0x00003000  /* Peripheral #19 assigned IVG10 */
#define P19_IVG11              0x00004000  /* Peripheral #19 assigned IVG11 */
#define P19_IVG12              0x00005000  /* Peripheral #19 assigned IVG12 */
#define P19_IVG13              0x00006000  /* Peripheral #19 assigned IVG13 */
#define P19_IVG14              0x00007000  /* Peripheral #19 assigned IVG14 */
#define P19_IVG15              0x00008000  /* Peripheral #19 assigned IVG15 */
#define P20_IVG7               0x00000000  /* Peripheral #20 assigned IVG7 */
#define P20_IVG8               0x00010000  /* Peripheral #20 assigned IVG8 */
#define P20_IVG9               0x00020000  /* Peripheral #20 assigned IVG9 */
#define P20_IVG10              0x00030000  /* Peripheral #20 assigned IVG10 */
#define P20_IVG11              0x00040000  /* Peripheral #20 assigned IVG11 */
#define P20_IVG12              0x00050000  /* Peripheral #20 assigned IVG12 */
#define P20_IVG13              0x00060000  /* Peripheral #20 assigned IVG13 */
#define P20_IVG14              0x00070000  /* Peripheral #20 assigned IVG14 */
#define P20_IVG15              0x00080000  /* Peripheral #20 assigned IVG15 */
/* */
/* SIC_IMASK Masks */
#define SIC_UNMASK_ALL         0x00000000  /* Unmask all peripheral interrupts */
#define SIC_MASK_ALL           0xFFFFFFFF  /* Mask all peripheral interrupts */
#define SIC_MASK0              0x00000001  /* Mask Peripheral #0 interrupt  */
#define SIC_MASK1              0x00000002  /* Mask Peripheral #1 interrupt  */
#define SIC_MASK2              0x00000004  /* Mask Peripheral #2 interrupt  */
#define SIC_MASK3              0x00000008  /* Mask Peripheral #3 interrupt  */
#define SIC_MASK4              0x00000010  /* Mask Peripheral #4 interrupt  */
#define SIC_MASK5              0x00000020  /* Mask Peripheral #5 interrupt  */
#define SIC_MASK6              0x00000040  /* Mask Peripheral #6 interrupt  */
#define SIC_MASK7              0x00000080  /* Mask Peripheral #7 interrupt  */
#define SIC_MASK8              0x00000100  /* Mask Peripheral #8 interrupt  */
#define SIC_MASK9              0x00000200  /* Mask Peripheral #9 interrupt  */
#define SIC_MASK10             0x00000400  /* Mask Peripheral #10 interrupt  */
#define SIC_MASK11             0x00000800  /* Mask Peripheral #11 interrupt  */
#define SIC_MASK12             0x00001000  /* Mask Peripheral #12 interrupt  */
#define SIC_MASK13             0x00002000  /* Mask Peripheral #13 interrupt  */
#define SIC_MASK14             0x00004000  /* Mask Peripheral #14 interrupt  */
#define SIC_MASK15             0x00008000  /* Mask Peripheral #15 interrupt  */
#define SIC_MASK16             0x00010000  /* Mask Peripheral #16 interrupt  */
#define SIC_MASK17             0x00020000  /* Mask Peripheral #17 interrupt  */
#define SIC_MASK18             0x00040000  /* Mask Peripheral #18 interrupt */
#define SIC_MASK19             0x00080000  /* Mask Peripheral #19 interrupt */
#define SIC_MASK20             0x00100000  /* Mask Peripheral #20 interrupt */
#define SIC_MASK_DFR           0x80000000  /* Mask Core Double Fault Reset */
#define SIC_UNMASK0            0xFFFFFFFE  /* Unmask Peripheral #0 interrupt  */
#define SIC_UNMASK1            0xFFFFFFFD  /* Unmask Peripheral #1 interrupt  */
#define SIC_UNMASK2            0xFFFFFFFB  /* Unmask Peripheral #2 interrupt  */
#define SIC_UNMASK3            0xFFFFFFF7  /* Unmask Peripheral #3 interrupt  */
#define SIC_UNMASK4            0xFFFFFFEF  /* Unmask Peripheral #4 interrupt  */
#define SIC_UNMASK5            0xFFFFFFDF  /* Unmask Peripheral #5 interrupt  */
#define SIC_UNMASK6            0xFFFFFFBF  /* Unmask Peripheral #6 interrupt  */
#define SIC_UNMASK7            0xFFFFFF7F  /* Unmask Peripheral #7 interrupt  */
#define SIC_UNMASK8            0xFFFFFEFF  /* Unmask Peripheral #8 interrupt  */
#define SIC_UNMASK9            0xFFFFFDFF  /* Unmask Peripheral #9 interrupt  */
#define SIC_UNMASK10           0xFFFFFBFF  /* Unmask Peripheral #10 interrupt  */
#define SIC_UNMASK11           0xFFFFF7FF  /* Unmask Peripheral #11 interrupt  */
#define SIC_UNMASK12           0xFFFFEFFF  /* Unmask Peripheral #12 interrupt  */
#define SIC_UNMASK13           0xFFFFDFFF  /* Unmask Peripheral #13 interrupt  */
#define SIC_UNMASK14           0xFFFFBFFF  /* Unmask Peripheral #14 interrupt  */
#define SIC_UNMASK15           0xFFFF7FFF  /* Unmask Peripheral #15 interrupt  */
#define SIC_UNMASK16           0xFFFEFFFF  /* Unmask Peripheral #16 interrupt  */
#define SIC_UNMASK17           0xFFFDFFFF  /* Unmask Peripheral #17 interrupt  */
#define SIC_UNMASK18           0xFFFBFFFF  /* Unmask Peripheral #18 interrupt */
#define SIC_UNMASK19           0xFFF7FFFF  /* Unmask Peripheral #19 interrupt */
#define SIC_UNMASK20           0xFFEFFFFF  /* Unmask Peripheral #20 interrupt */
#define SIC_UNMASK_DFR         0x7FFFFFFF  /* Unmask Core Double Fault Reset */

/* SIC_IWR Masks */
#define IWR_DISABLE_ALL        0x00000000  /* Wakeup Disable all peripherals */
#define IWR_ENABLE_ALL         0xFFFFFFFF  /* Wakeup Enable all peripherals */
#define IWR_ENABLE0            0x00000001  /* Wakeup Enable Peripheral #0 */
#define IWR_ENABLE1            0x00000002  /* Wakeup Enable Peripheral #1 */
#define IWR_ENABLE2            0x00000004  /* Wakeup Enable Peripheral #2 */
#define IWR_ENABLE3            0x00000008  /* Wakeup Enable Peripheral #3 */
#define IWR_ENABLE4            0x00000010  /* Wakeup Enable Peripheral #4 */
#define IWR_ENABLE5            0x00000020  /* Wakeup Enable Peripheral #5 */
#define IWR_ENABLE6            0x00000040  /* Wakeup Enable Peripheral #6 */
#define IWR_ENABLE7            0x00000080  /* Wakeup Enable Peripheral #7 */
#define IWR_ENABLE8            0x00000100  /* Wakeup Enable Peripheral #8 */
#define IWR_ENABLE9            0x00000200  /* Wakeup Enable Peripheral #9 */
#define IWR_ENABLE10           0x00000400  /* Wakeup Enable Peripheral #10 */
#define IWR_ENABLE11           0x00000800  /* Wakeup Enable Peripheral #11 */
#define IWR_ENABLE12           0x00001000  /* Wakeup Enable Peripheral #12 */
#define IWR_ENABLE13           0x00002000  /* Wakeup Enable Peripheral #13 */
#define IWR_ENABLE14           0x00004000  /* Wakeup Enable Peripheral #14 */
#define IWR_ENABLE15           0x00008000  /* Wakeup Enable Peripheral #15 */
#define IWR_ENABLE16           0x00010000  /* Wakeup Enable Peripheral #16 */
#define IWR_ENABLE17           0x00020000  /* Wakeup Enable Peripheral #17 */
#define IWR_ENABLE18           0x00040000  /* Wakeup Enable Peripheral #18 */
#define IWR_ENABLE19           0x00080000  /* Wakeup Enable Peripheral #19 */
#define IWR_ENABLE20           0x00100000  /* Wakeup Enable Peripheral #20 */
#define IWR_DISABLE0           0xFFFFFFFE  /* Wakeup Disable Peripheral #0 */
#define IWR_DISABLE1           0xFFFFFFFD  /* Wakeup Disable Peripheral #1 */
#define IWR_DISABLE2           0xFFFFFFFB  /* Wakeup Disable Peripheral #2 */
#define IWR_DISABLE3           0xFFFFFFF7  /* Wakeup Disable Peripheral #3 */
#define IWR_DISABLE4           0xFFFFFFEF  /* Wakeup Disable Peripheral #4 */
#define IWR_DISABLE5           0xFFFFFFDF  /* Wakeup Disable Peripheral #5 */
#define IWR_DISABLE6           0xFFFFFFBF  /* Wakeup Disable Peripheral #6 */
#define IWR_DISABLE7           0xFFFFFF7F  /* Wakeup Disable Peripheral #7 */
#define IWR_DISABLE8           0xFFFFFEFF  /* Wakeup Disable Peripheral #8 */
#define IWR_DISABLE9           0xFFFFFDFF  /* Wakeup Disable Peripheral #9 */
#define IWR_DISABLE10          0xFFFFFBFF  /* Wakeup Disable Peripheral #10 */
#define IWR_DISABLE11          0xFFFFF7FF  /* Wakeup Disable Peripheral #11 */
#define IWR_DISABLE12          0xFFFFEFFF  /* Wakeup Disable Peripheral #12 */
#define IWR_DISABLE13          0xFFFFDFFF  /* Wakeup Disable Peripheral #13 */
#define IWR_DISABLE14          0xFFFFBFFF  /* Wakeup Disable Peripheral #14 */
#define IWR_DISABLE15          0xFFFF7FFF  /* Wakeup Disable Peripheral #15 */
#define IWR_DISABLE16          0xFFFEFFFF  /* Wakeup Disable Peripheral #16 */
#define IWR_DISABLE17          0xFFFDFFFF  /* Wakeup Disable Peripheral #17 */
#define IWR_DISABLE18          0xFFFBFFFF  /* Wakeup Disable Peripheral #18 */
#define IWR_DISABLE19          0xFFF7FFFF  /* Wakeup Disable Peripheral #19 */
#define IWR_DISABLE20          0xFFEFFFFF  /* Wakeup Disable Peripheral #20 */

/* WDOGCTL Masks */
#define ENABLE_RESET           0x00000000  /* Set Watchdog Timer to generate reset */
#define ENABLE_NMI             0x00000002  /* Set Watchdog Timer to generate non-maskable interrupt */
#define ENABLE_GPI             0x00000004  /* Set Watchdog Timer to generate general-purpose interrupt */
#define DISABLE_EVT            0x00000006  /* Disable Watchdog Timer interrupts */

/* RTCFAST Mask */
#define ENABLE_PRESCALE        0x00000001  /* Enable prescaler so RTC runs at 1 Hz */
          /* Must be set after power-up for proper operation of RTC */

/* SPICTLx Masks */
#define TIMOD                  0x00000003  /* Transfer initiation mode and interrupt generation */
#define SZ                     0x00000004  /* Send Zero (=0) or last (=1) word when TDBR empty. */
#define GM                     0x00000008  /* When RDBR full, get more (=1) data or discard (=0) incoming Data */
#define PSSE                   0x00000010  /* Enable (=1) Slave-Select input for Master. */
#define EMISO                  0x00000020  /* Enable (=1) MISO pin as an output. */
#define SIZE                   0x00000100  /* Word length (0 => 8 bits, 1 => 16 bits) */
#define LSBF                   0x00000200  /* Data format (0 => MSB sent/received first 1 => LSB sent/received first) */
#define CPHA                   0x00000400  /* Clock phase (0 => SPICLK starts toggling in middle of xfer, 1 => SPICLK toggles at the beginning of xfer. */
#define CPOL                   0x00000800  /* Clock polarity (0 => active-high, 1 => active-low) */
#define MSTR                   0x00001000  /* Configures SPI as master (=1) or slave (=0) */
#define WOM                    0x00002000  /* Open drain (=1) data output enable (for MOSI and MISO) */
#define SPE                    0x00004000  /* SPI module enable (=1), disable (=0) */

/* SPIFLGx Masks */
#define FLS1                   0x00000002  /* Enables (=1) SPI_FLOUT1 as flag output for SPI Slave-select */
#define FLS2                   0x00000004  /* Enables (=1) SPI_FLOUT2 as flag output for SPI Slave-select */
#define FLS3                   0x00000008  /* Enables (=1) SPI_FLOUT3 as flag output for SPI Slave-select */
#define FLS4                   0x00000010  /* Enables (=1) SPI_FLOUT4 as flag output for SPI Slave-select */
#define FLS5                   0x00000020  /* Enables (=1) SPI_FLOUT5 as flag output for SPI Slave-select */
#define FLS6                   0x00000040  /* Enables (=1) SPI_FLOUT6 as flag output for SPI Slave-select */
#define FLS7                   0x00000080  /* Enables (=1) SPI_FLOUT7 as flag output for SPI Slave-select */
#define FLG1                   0x00000200  /* Activates (=0) SPI_FLOUT1 as flag output for SPI Slave-select  */
#define FLG2                   0x00000400  /* Activates (=0) SPI_FLOUT2 as flag output for SPI Slave-select */
#define FLG3                   0x00000800  /* Activates (=0) SPI_FLOUT3 as flag output for SPI Slave-select  */
#define FLG4                   0x00001000  /* Activates (=0) SPI_FLOUT4 as flag output for SPI Slave-select  */
#define FLG5                   0x00002000  /* Activates (=0) SPI_FLOUT5 as flag output for SPI Slave-select  */
#define FLG6                   0x00004000  /* Activates (=0) SPI_FLOUT6 as flag output for SPI Slave-select  */
#define FLG7                   0x00008000  /* Activates (=0) SPI_FLOUT7 as flag output for SPI Slave-select */

/* SPIFLGx Bit Positions */
#define FLS1_P                 0x00000001  /* Enables (=1) SPI_FLOUT1 as flag output for SPI Slave-select */
#define FLS2_P                 0x00000002  /* Enables (=1) SPI_FLOUT2 as flag output for SPI Slave-select */
#define FLS3_P                 0x00000003  /* Enables (=1) SPI_FLOUT3 as flag output for SPI Slave-select */
#define FLS4_P                 0x00000004  /* Enables (=1) SPI_FLOUT4 as flag output for SPI Slave-select */
#define FLS5_P                 0x00000005  /* Enables (=1) SPI_FLOUT5 as flag output for SPI Slave-select */
#define FLS6_P                 0x00000006  /* Enables (=1) SPI_FLOUT6 as flag output for SPI Slave-select */
#define FLS7_P                 0x00000007  /* Enables (=1) SPI_FLOUT7 as flag output for SPI Slave-select */
#define FLG1_P                 0x00000009  /* Activates (=0) SPI_FLOUT1 as flag output for SPI Slave-select  */
#define FLG2_P                 0x0000000A  /* Activates (=0) SPI_FLOUT2 as flag output for SPI Slave-select */
#define FLG3_P                 0x0000000B  /* Activates (=0) SPI_FLOUT3 as flag output for SPI Slave-select  */
#define FLG4_P                 0x0000000C  /* Activates (=0) SPI_FLOUT4 as flag output for SPI Slave-select  */
#define FLG5_P                 0x0000000D  /* Activates (=0) SPI_FLOUT5 as flag output for SPI Slave-select  */
#define FLG6_P                 0x0000000E  /* Activates (=0) SPI_FLOUT6 as flag output for SPI Slave-select  */
#define FLG7_P                 0x0000000F  /* Activates (=0) SPI_FLOUT7 as flag output for SPI Slave-select */

/* AMGCTL Masks */
#define AMCKEN                 0x00000001  /* Enable CLKOUT */
#define AMBEN_B4               0x00000002  /* Enable Asynchronous Memory Bank 6 only */
#define AMBEN_B4_B5            0x00000004  /* Enable Asynchronous Memory Banks 4 & 5 only */
#define AMBEN_ALL              0x00000006  /* Enable Asynchronous Memory Banks (all) 4, 5, 6, and 7 */
#define B4PEN                  0x00000010  /* Enable 16-bit packing for Asynchronous Memory Bank 4 */
#define B5PEN                  0x00000020  /* Enable 16-bit packing for Asynchronous Memory Bank 5 */
#define B6PEN                  0x00000040  /* Enable 16-bit packing for Asynchronous Memory Bank 6 */
#define B7PEN                  0x00000080  /* Enable 16-bit packing for Asynchronous Memory Bank 7 */

/* AMGCTL Bit Positions */
#define AMCKEN_P               0x00000000  /* Enable CLKOUT */
#define AMBEN_P0               0x00000001  /* Asynchronous Memory Enable, 00 - banks 4-7 disabled, 01 - bank 4 enabled */
#define AMBEN_P1               0x00000002  /* Asynchronous Memory Enable, 10 - banks 4&5 enabled,  11 - banks 4-7 enabled */
#define B4PEN_P                0x00000004  /* Enable 16-bit packing for Asynchronous Memory Bank 4 */
#define B5PEN_P                0x00000005  /* Enable 16-bit packing for Asynchronous Memory Bank 5 */
#define B6PEN_P                0x00000006  /* Enable 16-bit packing for Asynchronous Memory Bank 6 */
#define B7PEN_P                0x00000007  /* Enable 16-bit packing for Asynchronous Memory Bank 7 */

/* AMBCTL0 Masks */
#define B4RDYEN                0x00000001  /* Bank 4 RDY Enable, 0=disable, 1=enable */
#define B4RDYPOL               0x00000002  /* Bank 4 RDY Active high, 0=active low, 1=active high */
#define B4TT_1                 0x00000004  /* Bank 4 Transition Time from Read to Write = 1 cycle */
#define B4TT_2                 0x00000008  /* Bank 4 Transition Time from Read to Write = 2 cycles */
#define B4TT_3                 0x0000000C  /* Bank 4 Transition Time from Read to Write = 3 cycles */
#define B4TT_4                 0x00000000  /* Bank 4 Transition Time from Read to Write = 4 cycles */
#define B4ST_1                 0x00000010  /* Bank 4 Setup Time from AOE asserted to Read or Write asserted = 1 cycle */
#define B4ST_2                 0x00000020  /* Bank 4 Setup Time from AOE asserted to Read or Write asserted = 2 cycles */
#define B4ST_3                 0x00000030  /* Bank 4 Setup Time from AOE asserted to Read or Write asserted = 3 cycles */
#define B4ST_4                 0x00000000  /* Bank 4 Setup Time from AOE asserted to Read or Write asserted = 4 cycles */
#define B4HT_1                 0x00000040  /* Bank 4 Hold Time from Read or Write deasserted to AOE deasserted = 1 cycle */
#define B4HT_2                 0x00000080  /* Bank 4 Hold Time from Read or Write deasserted to AOE deasserted = 2 cycles */
#define B4HT_3                 0x000000C0  /* Bank 4 Hold Time from Read or Write deasserted to AOE deasserted = 3 cycles */
#define B4HT_4                 0x00000000  /* Bank 4 Hold Time from Read or Write deasserted to AOE deasserted = 4 cycles */
#define B4RAT_1                0x00000100  /* Bank 4 Read Access Time = 1 cycle */
#define B4RAT_2                0x00000200  /* Bank 4 Read Access Time = 2 cycles */
#define B4RAT_3                0x00000300  /* Bank 4 Read Access Time = 3 cycles */
#define B4RAT_4                0x00000400  /* Bank 4 Read Access Time = 4 cycles */
#define B4RAT_5                0x00000500  /* Bank 4 Read Access Time = 5 cycles */
#define B4RAT_6                0x00000600  /* Bank 4 Read Access Time = 6 cycles */
#define B4RAT_7                0x00000700  /* Bank 4 Read Access Time = 7 cycles */
#define B4RAT_8                0x00000800  /* Bank 4 Read Access Time = 8 cycles */
#define B4RAT_9                0x00000900  /* Bank 4 Read Access Time = 9 cycles */
#define B4RAT_10               0x00000A00  /* Bank 4 Read Access Time = 10 cycles */
#define B4RAT_11               0x00000B00  /* Bank 4 Read Access Time = 11 cycles */
#define B4RAT_12               0x00000C00  /* Bank 4 Read Access Time = 12 cycles */
#define B4RAT_13               0x00000D00  /* Bank 4 Read Access Time = 13 cycles */
#define B4RAT_14               0x00000E00  /* Bank 4 Read Access Time = 14 cycles */
#define B4RAT_15               0x00000F00  /* Bank 4 Read Access Time = 15 cycles */
#define B4WAT_1                0x00001000  /* Bank 4 Write Access Time = 1 cycle */
#define B4WAT_2                0x00002000  /* Bank 4 Write Access Time = 2 cycles */
#define B4WAT_3                0x00003000  /* Bank 4 Write Access Time = 3 cycles */
#define B4WAT_4                0x00004000  /* Bank 4 Write Access Time = 4 cycles */
#define B4WAT_5                0x00005000  /* Bank 4 Write Access Time = 5 cycles */
#define B4WAT_6                0x00006000  /* Bank 4 Write Access Time = 6 cycles */
#define B4WAT_7                0x00007000  /* Bank 4 Write Access Time = 7 cycles */
#define B4WAT_8                0x00008000  /* Bank 4 Write Access Time = 8 cycles */
#define B4WAT_9                0x00009000  /* Bank 4 Write Access Time = 9 cycles */
#define B4WAT_10               0x0000A000  /* Bank 4 Write Access Time = 10 cycles */
#define B4WAT_11               0x0000B000  /* Bank 4 Write Access Time = 11 cycles */
#define B4WAT_12               0x0000C000  /* Bank 4 Write Access Time = 12 cycles */
#define B4WAT_13               0x0000D000  /* Bank 4 Write Access Time = 13 cycles */
#define B4WAT_14               0x0000E000  /* Bank 4 Write Access Time = 14 cycles */
#define B4WAT_15               0x0000F000  /* Bank 4 Write Access Time = 15 cycles */
#define B5RDYEN                0x00000001  /* Bank 5 RDY enable, 0=disable, 1=enable */
#define B5RDYPOL               0x00000002  /* Bank 5 RDY Active high, 0=active low, 1=active high */
#define B5TT_1                 0x00000004  /* Bank 5 Transition Time from Read to Write = 1 cycle */
#define B5TT_2                 0x00000008  /* Bank 5 Transition Time from Read to Write = 2 cycles */
#define B5TT_3                 0x0000000C  /* Bank 5 Transition Time from Read to Write = 3 cycles */
#define B5TT_4                 0x00000000  /* Bank 5 Transition Time from Read to Write = 4 cycles */
#define B5ST_1                 0x00000010  /* Bank 5 Setup Time from AOE asserted to Read or Write asserted = 1 cycle */
#define B5ST_2                 0x00000020  /* Bank 5 Setup Time from AOE asserted to Read or Write asserted = 2 cycles */
#define B5ST_3                 0x00000030  /* Bank 5 Setup Time from AOE asserted to Read or Write asserted = 3 cycles */
#define B5ST_4                 0x00000000  /* Bank 5 Setup Time from AOE asserted to Read or Write asserted = 4 cycles */
#define B5HT_1                 0x00000040  /* Bank 5 Hold Time from Read or Write deasserted to AOE deasserted = 1 cycle */
#define B5HT_2                 0x00000080  /* Bank 5 Hold Time from Read or Write deasserted to AOE deasserted = 2 cycles */
#define B5HT_3                 0x000000C0  /* Bank 5 Hold Time from Read or Write deasserted to AOE deasserted = 3 cycles */
#define B5HT_4                 0x00000000  /* Bank 5 Hold Time from Read or Write deasserted to AOE deasserted = 4 cycles */
#define B5RAT_1                0x00000100  /* Bank 5 Read Access Time = 1 cycle */
#define B5RAT_2                0x00000200  /* Bank 5 Read Access Time = 2 cycles */
#define B5RAT_3                0x00000300  /* Bank 5 Read Access Time = 3 cycles */
#define B5RAT_4                0x00000400  /* Bank 5 Read Access Time = 4 cycles */
#define B5RAT_5                0x00000500  /* Bank 5 Read Access Time = 5 cycles */
#define B5RAT_6                0x00000600  /* Bank 5 Read Access Time = 6 cycles */
#define B5RAT_7                0x00000700  /* Bank 5 Read Access Time = 7 cycles */
#define B5RAT_8                0x00000800  /* Bank 5 Read Access Time = 8 cycles */
#define B5RAT_9                0x00000900  /* Bank 5 Read Access Time = 9 cycles */
#define B5RAT_10               0x00000A00  /* Bank 5 Read Access Time = 10 cycles */
#define B5RAT_11               0x00000B00  /* Bank 5 Read Access Time = 11 cycles */
#define B5RAT_12               0x00000C00  /* Bank 5 Read Access Time = 12 cycles */
#define B5RAT_13               0x00000D00  /* Bank 5 Read Access Time = 13 cycles */
#define B5RAT_14               0x00000E00  /* Bank 5 Read Access Time = 14 cycles */
#define B5RAT_15               0x00000F00  /* Bank 5 Read Access Time = 15 cycles */
#define B5WAT_1                0x00001000  /* Bank 5 Write Access Time = 1 cycle */
#define B5WAT_2                0x00002000  /* Bank 5 Write Access Time = 2 cycles */
#define B5WAT_3                0x00003000  /* Bank 5 Write Access Time = 3 cycles */
#define B5WAT_4                0x00004000  /* Bank 5 Write Access Time = 4 cycles */
#define B5WAT_5                0x00005000  /* Bank 5 Write Access Time = 5 cycles */
#define B5WAT_6                0x00006000  /* Bank 5 Write Access Time = 6 cycles */
#define B5WAT_7                0x00007000  /* Bank 5 Write Access Time = 7 cycles */
#define B5WAT_8                0x00008000  /* Bank 5 Write Access Time = 8 cycles */
#define B5WAT_9                0x00009000  /* Bank 5 Write Access Time = 9 cycles */
#define B5WAT_10               0x0000A000  /* Bank 5 Write Access Time = 10 cycles */
#define B5WAT_11               0x0000B000  /* Bank 5 Write Access Time = 11 cycles */
#define B5WAT_12               0x0000C000  /* Bank 5 Write Access Time = 12 cycles */
#define B5WAT_13               0x0000D000  /* Bank 5 Write Access Time = 13 cycles */
#define B5WAT_14               0x0000E000  /* Bank 5 Write Access Time = 14 cycles */
#define B5WAT_15               0x0000F000  /* Bank 5 Write Access Time = 15 cycles */

/* AMBCTL1 Masks */
#define B6RDYEN                0x00000001  /* Bank 6 RDY Enable, 0=disable, 1=enable */
#define B6RDYPOL               0x00000002  /* Bank 6 RDY Active high, 0=active low, 1=active high */
#define B6TT_1                 0x00000004  /* Bank 6 Transition Time from Read to Write = 1 cycle */
#define B6TT_2                 0x00000008  /* Bank 6 Transition Time from Read to Write = 2 cycles */
#define B6TT_3                 0x0000000C  /* Bank 6 Transition Time from Read to Write = 3 cycles */
#define B6TT_4                 0x00000000  /* Bank 6 Transition Time from Read to Write = 4 cycles */
#define B6ST_1                 0x00000010  /* Bank 6 Setup Time from AOE asserted to Read or Write asserted = 1 cycle */
#define B6ST_2                 0x00000020  /* Bank 6 Setup Time from AOE asserted to Read or Write asserted = 2 cycles */
#define B6ST_3                 0x00000030  /* Bank 6 Setup Time from AOE asserted to Read or Write asserted = 3 cycles */
#define B6ST_4                 0x00000000  /* Bank 6 Setup Time from AOE asserted to Read or Write asserted = 4 cycles */
#define B6HT_1                 0x00000040  /* Bank 6 Hold Time from Read or Write deasserted to AOE deasserted = 1 cycle */
#define B6HT_2                 0x00000080  /* Bank 6 Hold Time from Read or Write deasserted to AOE deasserted = 2 cycles */
#define B6HT_3                 0x000000C0  /* Bank 6 Hold Time from Read or Write deasserted to AOE deasserted = 3 cycles */
#define B6HT_4                 0x00000000  /* Bank 6 Hold Time from Read or Write deasserted to AOE deasserted = 4 cycles */
#define B6RAT_1                0x00000100  /* Bank 6 Read Access Time = 1 cycle */
#define B6RAT_2                0x00000200  /* Bank 6 Read Access Time = 2 cycles */
#define B6RAT_3                0x00000300  /* Bank 6 Read Access Time = 3 cycles */
#define B6RAT_4                0x00000400  /* Bank 6 Read Access Time = 4 cycles */
#define B6RAT_5                0x00000500  /* Bank 6 Read Access Time = 5 cycles */
#define B6RAT_6                0x00000600  /* Bank 6 Read Access Time = 6 cycles */
#define B6RAT_7                0x00000700  /* Bank 6 Read Access Time = 7 cycles */
#define B6RAT_8                0x00000800  /* Bank 6 Read Access Time = 8 cycles */
#define B6RAT_9                0x00000900  /* Bank 6 Read Access Time = 9 cycles */
#define B6RAT_10               0x00000A00  /* Bank 6 Read Access Time = 10 cycles */
#define B6RAT_11               0x00000B00  /* Bank 6 Read Access Time = 11 cycles */
#define B6RAT_12               0x00000C00  /* Bank 6 Read Access Time = 12 cycles */
#define B6RAT_13               0x00000D00  /* Bank 6 Read Access Time = 13 cycles */
#define B6RAT_14               0x00000E00  /* Bank 6 Read Access Time = 14 cycles */
#define B6RAT_15               0x00000F00  /* Bank 6 Read Access Time = 15 cycles */
#define B6WAT_1                0x00001000  /* Bank 6 Write Access Time = 1 cycle */
#define B6WAT_2                0x00002000  /* Bank 6 Write Access Time = 2 cycles */
#define B6WAT_3                0x00003000  /* Bank 6 Write Access Time = 3 cycles */
#define B6WAT_4                0x00004000  /* Bank 6 Write Access Time = 4 cycles */
#define B6WAT_5                0x00005000  /* Bank 6 Write Access Time = 5 cycles */
#define B6WAT_6                0x00006000  /* Bank 6 Write Access Time = 6 cycles */
#define B6WAT_7                0x00007000  /* Bank 6 Write Access Time = 7 cycles */
#define B6WAT_8                0x00008000  /* Bank 6 Write Access Time = 8 cycles */
#define B6WAT_9                0x00009000  /* Bank 6 Write Access Time = 9 cycles */
#define B6WAT_10               0x0000A000  /* Bank 6 Write Access Time = 10 cycles */
#define B6WAT_11               0x0000B000  /* Bank 6 Write Access Time = 11 cycles */
#define B6WAT_12               0x0000C000  /* Bank 6 Write Access Time = 12 cycles */
#define B6WAT_13               0x0000D000  /* Bank 6 Write Access Time = 13 cycles */
#define B6WAT_14               0x0000E000  /* Bank 6 Write Access Time = 14 cycles */
#define B6WAT_15               0x0000F000  /* Bank 6 Write Access Time = 15 cycles */
#define B7RDYEN                0x00000001  /* Bank 7 RDY enable, 0=disable, 1=enable */
#define B7RDYPOL               0x00000002  /* Bank 7 RDY Active high, 0=active low, 1=active high */
#define B7TT_1                 0x00000004  /* Bank 7 Transition Time from Read to Write = 1 cycle */
#define B7TT_2                 0x00000008  /* Bank 7 Transition Time from Read to Write = 2 cycles */
#define B7TT_3                 0x0000000C  /* Bank 7 Transition Time from Read to Write = 3 cycles */
#define B7TT_4                 0x00000000  /* Bank 7 Transition Time from Read to Write = 4 cycles */
#define B7ST_1                 0x00000010  /* Bank 7 Setup Time from AOE asserted to Read or Write asserted = 1 cycle */
#define B7ST_2                 0x00000020  /* Bank 7 Setup Time from AOE asserted to Read or Write asserted = 2 cycles */
#define B7ST_3                 0x00000030  /* Bank 7 Setup Time from AOE asserted to Read or Write asserted = 3 cycles */
#define B7ST_4                 0x00000000  /* Bank 7 Setup Time from AOE asserted to Read or Write asserted = 4 cycles */
#define B7HT_1                 0x00000040  /* Bank 7 Hold Time from Read or Write deasserted to AOE deasserted = 1 cycle */
#define B7HT_2                 0x00000080  /* Bank 7 Hold Time from Read or Write deasserted to AOE deasserted = 2 cycles */
#define B7HT_3                 0x000000C0  /* Bank 7 Hold Time from Read or Write deasserted to AOE deasserted = 3 cycles */
#define B7HT_4                 0x00000000  /* Bank 7 Hold Time from Read or Write deasserted to AOE deasserted = 4 cycles */
#define B7RAT_1                0x00000100  /* Bank 7 Read Access Time = 1 cycle */
#define B7RAT_2                0x00000200  /* Bank 7 Read Access Time = 2 cycles */
#define B7RAT_3                0x00000300  /* Bank 7 Read Access Time = 3 cycles */
#define B7RAT_4                0x00000400  /* Bank 7 Read Access Time = 4 cycles */
#define B7RAT_5                0x00000500  /* Bank 7 Read Access Time = 5 cycles */
#define B7RAT_6                0x00000600  /* Bank 7 Read Access Time = 6 cycles */
#define B7RAT_7                0x00000700  /* Bank 7 Read Access Time = 7 cycles */
#define B7RAT_8                0x00000800  /* Bank 7 Read Access Time = 8 cycles */
#define B7RAT_9                0x00000900  /* Bank 7 Read Access Time = 9 cycles */
#define B7RAT_10               0x00000A00  /* Bank 7 Read Access Time = 10 cycles */
#define B7RAT_11               0x00000B00  /* Bank 7 Read Access Time = 11 cycles */
#define B7RAT_12               0x00000C00  /* Bank 7 Read Access Time = 12 cycles */
#define B7RAT_13               0x00000D00  /* Bank 7 Read Access Time = 13 cycles */
#define B7RAT_14               0x00000E00  /* Bank 7 Read Access Time = 14 cycles */
#define B7RAT_15               0x00000F00  /* Bank 7 Read Access Time = 15 cycles */
#define B7WAT_1                0x00001000  /* Bank 7 Write Access Time = 1 cycle */
#define B7WAT_2                0x00002000  /* Bank 7 Write Access Time = 2 cycles */
#define B7WAT_3                0x00003000  /* Bank 7 Write Access Time = 3 cycles */
#define B7WAT_4                0x00004000  /* Bank 7 Write Access Time = 4 cycles */
#define B7WAT_5                0x00005000  /* Bank 7 Write Access Time = 5 cycles */
#define B7WAT_6                0x00006000  /* Bank 7 Write Access Time = 6 cycles */
#define B7WAT_7                0x00007000  /* Bank 7 Write Access Time = 7 cycles */
#define B7WAT_8                0x00008000  /* Bank 7 Write Access Time = 8 cycles */
#define B7WAT_9                0x00009000  /* Bank 7 Write Access Time = 9 cycles */
#define B7WAT_10               0x0000A000  /* Bank 7 Write Access Time = 10 cycles */
#define B7WAT_11               0x0000B000  /* Bank 7 Write Access Time = 11 cycles */
#define B7WAT_12               0x0000C000  /* Bank 7 Write Access Time = 12 cycles */
#define B7WAT_13               0x0000D000  /* Bank 7 Write Access Time = 13 cycles */
#define B7WAT_14               0x0000E000  /* Bank 7 Write Access Time = 14 cycles */
#define B7WAT_15               0x0000F000  /* Bank 7 Write Access Time = 15 cycles */

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* __DEF_BF535_H */
