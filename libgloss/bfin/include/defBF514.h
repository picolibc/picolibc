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
** Copyright (C) 2006-2008 Analog Devices Inc., All Rights Reserved.
**
************************************************************************************
**
** This include file contains a list of macro "defines" to enable the programmer
** to use symbolic names for register-access and bit-manipulation.
**
**/
#ifndef _DEF_BF514_H
#define _DEF_BF514_H

/* Include all Core registers and bit definitions */
#include <def_LPBlackfin.h>

/* SYSTEM & MMR ADDRESS DEFINITIONS FOR ADSP-BF514 */

/* Include defBF51x_base.h for the set of #defines that are common to all ADSP-BF51x processors */
#include <defBF51x_base.h>

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_19_4:"macros violate rule 19.4")
#endif /* _MISRA_RULES */

/* The following are the #defines needed by ADSP-BF514 that are not in the common header */

/* RSI Registers */

#define RSI_PWR_CONTROL			0xFFC03800		/* RSI Power Control Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_PWR_CTL  			RSI_PWR_CONTROL	/* SDH Power Control */
#define RSI_CLK_CONTROL			0xFFC03804		/* RSI Clock Control Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_CLK_CTL  			RSI_CLK_CONTROL	/* SDH Clock Control */
#define RSI_ARGUMENT			0xFFC03808		/* RSI Argument Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_ARGUMENT  			RSI_ARGUMENT	/* SDH Argument */
#define RSI_COMMAND			0xFFC0380C		/* RSI Command Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_COMMAND  			RSI_COMMAND      	/* SDH Command */
#define RSI_RESP_CMD			0xFFC03810		/* RSI Response Command Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_RESP_CMD  			RSI_RESP_CMD     	/* SDH Response Command */
#define RSI_RESPONSE0			0xFFC03814		/* RSI Response Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_RESPONSE0  			RSI_RESPONSE0    	/* SDH Response0 */
#define RSI_RESPONSE1			0xFFC03818		/* RSI Response Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_RESPONSE1  			RSI_RESPONSE1    	/* SDH Response1 */
#define RSI_RESPONSE2			0xFFC0381C	/* RSI Response Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_RESPONSE2  			RSI_RESPONSE2    	/* SDH Response2 */
#define RSI_RESPONSE3			0xFFC03820		/* RSI Response Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_RESPONSE3  			RSI_RESPONSE3    	/* SDH Response3 */
#define RSI_DATA_TIMER			0xFFC03824		/* RSI Data Timer Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_DATA_TIMER 			RSI_DATA_TIMER  	/* SDH Data Timer */
#define RSI_DATA_LGTH			0xFFC03828		/* RSI Data Length Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_DATA_LGTH  			RSI_DATA_LGTH    	/* SDH Data Length */
#define RSI_DATA_CONTROL		0xFFC0382C		/* RSI Data Control Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_DATA_CTL  			RSI_DATA_CONTROL 	/* SDH Data Control */
#define RSI_DATA_CNT			0xFFC03830		/* RSI Data Counter Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_DATA_CNT  			RSI_DATA_CNT     	/* SDH Data Counter */
#define RSI_STATUS			0xFFC03834		/* RSI Status Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_STATUS  			RSI_STATUS       	/* SDH Status */
#define RSI_STATUSCL			0xFFC03838		/* RSI Status Clear Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_STATUS_CLR  		RSI_STATUSCL     	/* SDH Status Clear */
#define RSI_MASK0				0xFFC0383C		/* RSI Interrupt 0 Mask Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_MASK0  			RSI_MASK0        	/* SDH Interrupt0 Mask */
#define RSI_MASK1				0xFFC03840		/* RSI Interrupt 1 Mask Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_MASK1  			RSI_MASK1        	/* SDH Interrupt1 Mask */
#define RSI_FIFO_CNT			0xFFC03848		/* RSI FIFO Counter Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_FIFO_CNT  			RSI_FIFO_CNT     	/* SDH FIFO Counter */
#define RSI_CEATA_CONTROL		0xFFC0384C		/* RSI CEATA Register */
#define RSI_FIFO				0xFFC03880		/* RSI Data FIFO Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_FIFO  			RSI_FIFO         	/* SDH Data FIFO */
#define RSI_ESTAT				0xFFC038C0		/* RSI Exception Status Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_E_STATUS  			RSI_ESTAT        	/* SDH Exception Status */
#define RSI_EMASK				0xFFC038C4		/* RSI Exception Mask Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_E_MASK  			RSI_EMASK        	/* SDH Exception Mask */
#define RSI_CONFIG			0xFFC038C8		/* RSI Configuration Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_CFG  				RSI_CONFIG       	/* SDH Configuration */
#define RSI_RD_WAIT_EN			0xFFC038CC		/* RSI Read Wait Enable Register */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_RD_WAIT_EN  		RSI_RD_WAIT_EN   	/* SDH Read Wait Enable */
#define RSI_PID0				0xFFC038D0		/* RSI Peripheral ID Register 0 */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_PID0  			RSI_PID0         	/* SDH Peripheral Identification0 */
#define RSI_PID1				0xFFC038D4		/* RSI Peripheral ID Register 1 */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_PID1  			RSI_PID1         	/* SDH Peripheral Identification1 */
#define RSI_PID2				0xFFC038D8		/* RSI Peripheral ID Register 2 */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_PID2  			RSI_PID2         	/* SDH Peripheral Identification2 */
#define RSI_PID3				0xFFC038DC		/* RSI Peripheral ID Register 3 */
/* legacy register name (below) provided for backwards code compatibility */
#define SDH_PID3  			RSI_PID3         	/* SDH Peripheral Identification3 */
/* RSI Registers */



/* ********************************************************** */
/*     SINGLE BIT MACRO PAIRS (bit mask and negated one)      */
/*     and MULTI BIT READ MACROS                              */
/* ********************************************************** */

/* Bit masks for RSI_PWR_CONTROL */
#define                    PWR_ON  0x3        		/* Power On */
#define                RSI_CMD_OD  0x40       		/* Open Drain Output */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                 SD_CMD_OD  RSI_CMD_OD 		/* Open Drain Output */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                nSD_CMD_OD  0x0
/* legacy bit mask (below) provided for backwards code compatibility */
#if 0
#define                       TBD  0x3c       		/* TBD */
#endif
/* legacy bit mask (below) provided for backwards code compatibility */
#define                   ROD_CTL  0x80
/* legacy bit mask (below) provided for backwards code compatibility */
#define                  nROD_CTL  0x80


/* Bit masks for RSI_CLK_CONTROL */
#define                    CLKDIV  0xff           	/* MC_CLK Divisor */
#define                    CLK_EN  0x100          	/* MC_CLK Bus Clock Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                     CLK_E  CLK_EN         	/* MC_CLK Bus Clock Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                    nCLK_E  0x0
#define                 PWR_SV_EN  0x200          	/* Power Save Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                  PWR_SV_E  PWR_SV_EN      	/* Power Save Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                 nPWR_SV_E  0x0
#define             CLKDIV_BYPASS  0x400          	/* Bypass Divisor */
/* legacy bit mask (below) provided for backwards code compatibility */
#define            nCLKDIV_BYPASS  0x0
#define                  BUS_MODE  0x1800           /* Bus width selection */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                  WIDE_BUS  0x0800    	/* Wide Bus Mode Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                 nWIDE_BUS  0x0


/* Bit masks for RSI_COMMAND */
#define                   CMD_IDX  0x3f           	/* Command Index */
#define                CMD_RSP_EN  0x40           	/* Response */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                   CMD_RSP  CMD_RSP_EN     	/* Response */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                  nCMD_RSP  0x0
#define               CMD_LRSP_EN  0x80           	/* Long Response */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                 CMD_L_RSP  CMD_LRSP_EN    	/* Long Response */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                nCMD_L_RSP  0x0
#define                CMD_INT_EN  0x100          	/* Command Interrupt */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                 CMD_INT_E  CMD_INT_EN     	/* Command Interrupt */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                nCMD_INT_E  0x0
#define               CMD_PEND_EN  0x200          	/* Command Pending */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                CMD_PEND_E  CMD_PEND_EN		/* Command Pending */
/* legacy bit mask (below) provided for backwards code compatibility */
#define               nCMD_PEND_E  0x0
#define                    CMD_EN  0x400			/* Command Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                     CMD_E  CMD_EN          	/* Command Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                    nCMD_E  0x0


/* Bit masks for RSI_RESP_CMD */
#define                  RESP_CMD  0x3f       		/* Response Command */

/* Bit masks for RSI_DATA_LGTH */
#define               DATA_LENGTH  0xffff       	/* Data Length */


/* Bit masks for RSI_DATA_CONTROL */
#define                   DATA_EN  0x1            	/* Data Transfer Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                     DTX_E  DATA_EN        	/* Data Transfer Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                    nDTX_E  0x0
#define                  DATA_DIR  0x2            	/* Data Transfer Direction */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                   DTX_DIR  DATA_DIR      	/* Data Transfer Direction */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                  nDTX_DIR  0x0
#define                 DATA_MODE  0x4            	/* Data Transfer Mode */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                  DTX_MODE  DATA_MODE      	/* Data Transfer Mode */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                 nDTX_MODE  0x0
#define               DATA_DMA_EN  0x8            	/* Data Transfer DMA Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                 DTX_DMA_E  0x8            	/* Data Transfer DMA Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                nDTX_DMA_E  0x0
#define             DATA_BLK_LGTH  0xf0           	/* Data Transfer Block Length */
/* legacy bit mask (below) provided for backwards code compatibility */
#define              DTX_BLK_LGTH  0xf0           	/* Data Transfer Block Length */
#define 		             CEATA_EN  0x100	      /* CE-ATA operation mode enable */
#define 		         CEATA_CCS_EN  0x200	      /* CE-ATA CCS mode enable */

/* Bit masks for RSI_DATA_CNT */
#define               DATA_COUNT  0xffff       		/* Data Count */

/* Bit masks for RSI_STATUS */
#define              CMD_CRC_FAIL  0x1        		/* CMD CRC Fail */
/* legacy bit mask (below) provided for backwards code compatibility */
#define             nCMD_CRC_FAIL  0x0
#define              DAT_CRC_FAIL  0x2        		/* Data CRC Fail */
/* legacy bit mask (below) provided for backwards code compatibility */
#define             nDAT_CRC_FAIL  0x0
#define               CMD_TIMEOUT  0x4        		/* CMD Time Out */
/* legacy bit mask (below) provided for backwards code compatibility */
#define              nCMD_TIMEOUT  0x0
#define               DAT_TIMEOUT  0x8        		/* Data Time Out */
/* legacy bit mask (below) provided for backwards code compatibility */
#define              nDAT_TIMEOUT  0x0
#define               TX_UNDERRUN  0x10       		/* Transmit Underrun */
/* legacy bit mask (below) provided for backwards code compatibility */
#define              nTX_UNDERRUN  0x0
#define                RX_OVERRUN  0x20       		/* Receive Overrun */
/* legacy bit mask (below) provided for backwards code compatibility */
#define               nRX_OVERRUN  0x0
#define              CMD_RESP_END  0x40       		/* CMD Response End */
/* legacy bit mask (below) provided for backwards code compatibility */
#define             nCMD_RESP_END  0x0
#define                  CMD_SENT  0x80       		/* CMD Sent */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                 nCMD_SENT  0x0
#define                   DAT_END  0x100      		/* Data End */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                  nDAT_END  0x0
#define             START_BIT_ERR  0x200      		/* Start Bit Error */
/* legacy bit mask (below) provided for backwards code compatibility */
#define            nSTART_BIT_ERR  0x0
#define               DAT_BLK_END  0x400      		/* Data Block End */
/* legacy bit mask (below) provided for backwards code compatibility */
#define              nDAT_BLK_END  0x0
#define                   CMD_ACT  0x800      		/* CMD Active */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                  nCMD_ACT  0x0
#define                    TX_ACT  0x1000     		/* Transmit Active */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                   nTX_ACT  0x0
#define                    RX_ACT  0x2000     		/* Receive Active */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                   nRX_ACT  0x0
#define              TX_FIFO_STAT  0x4000     		/* Transmit FIFO Status */
/* legacy bit mask (below) provided for backwards code compatibility */
#define             nTX_FIFO_STAT  0x0
#define              RX_FIFO_STAT  0x8000     		/* Receive FIFO Status */
/* legacy bit mask (below) provided for backwards code compatibility */
#define             nRX_FIFO_STAT  0x0
#define              TX_FIFO_FULL  0x10000    		/* Transmit FIFO Full */
/* legacy bit mask (below) provided for backwards code compatibility */
#define             nTX_FIFO_FULL  0x0
#define              RX_FIFO_FULL  0x20000    		/* Receive FIFO Full */
/* legacy bit mask (below) provided for backwards code compatibility */
#define             nRX_FIFO_FULL  0x0
#define              TX_FIFO_ZERO  0x40000    		/* Transmit FIFO Empty */
/* legacy bit mask (below) provided for backwards code compatibility */
#define             nTX_FIFO_ZERO  0x0
#define               RX_DAT_ZERO  0x80000    		/* Receive FIFO Empty */
/* legacy bit mask (below) provided for backwards code compatibility */
#define              nRX_DAT_ZERO  0x0
#define                TX_DAT_RDY  0x100000   		/* Transmit Data Available */
/* legacy bit mask (below) provided for backwards code compatibility */
#define               nTX_DAT_RDY  0x0
#define               RX_FIFO_RDY  0x200000   		/* Receive Data Available */
/* legacy bit mask (below) provided for backwards code compatibility */
#define              nRX_FIFO_RDY  0x0

/* Bit masks for RSI_STATCL */
#define         CMD_CRC_FAIL_STAT  0x1       		/* CMD CRC Fail Status */
/* legacy bit mask (below) provided for backwards code compatibility */
#define        nCMD_CRC_FAIL_STAT  0x0
#define         DAT_CRC_FAIL_STAT  0x2        		/* Data CRC Fail Status */
/* legacy bit mask (below) provided for backwards code compatibility */
#define        nDAT_CRC_FAIL_STAT  0x0
#define          CMD_TIMEOUT_STAT  0x4        		/* CMD Time Out Status */
/* legacy bit mask (below) provided for backwards code compatibility */
#define         nCMD_TIMEOUT_STAT  0x0
#define          DAT_TIMEOUT_STAT  0x8        		/* Data Time Out status */
/* legacy bit mask (below) provided for backwards code compatibility */
#define         nDAT_TIMEOUT_STAT  0x0
#define          TX_UNDERRUN_STAT  0x10       		/* Transmit Underrun Status */
/* legacy bit mask (below) provided for backwards code compatibility */
#define         nTX_UNDERRUN_STAT  0x0
#define           RX_OVERRUN_STAT  0x20       		/* Receive Overrun Status */
/* legacy bit mask (below) provided for backwards code compatibility */
#define          nRX_OVERRUN_STAT  0x0
#define         CMD_RESP_END_STAT  0x40       		/* CMD Response End Status */
/* legacy bit mask (below) provided for backwards code compatibility */
#define        nCMD_RESP_END_STAT  0x0
#define             CMD_SENT_STAT  0x80       		/* CMD Sent Status */
/* legacy bit mask (below) provided for backwards code compatibility */
#define            nCMD_SENT_STAT  0x0
#define              DAT_END_STAT  0x100      		/* Data End Status */
/* legacy bit mask (below) provided for backwards code compatibility */
#define             nDAT_END_STAT  0x0
#define        START_BIT_ERR_STAT  0x200      		/* Start Bit Error Status */
/* legacy bit mask (below) provided for backwards code compatibility */
#define       nSTART_BIT_ERR_STAT  0x0
#define          DAT_BLK_END_STAT  0x400      		/* Data Block End Status */
/* legacy bit mask (below) provided for backwards code compatibility */
#define         nDAT_BLK_END_STAT  0x0

/* Bit masks for RSI_MASKx */
#define         CMD_CRC_FAIL_MASK  0x1        		/* CMD CRC Fail Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define        nCMD_CRC_FAIL_MASK  0x0
#define         DAT_CRC_FAIL_MASK  0x2        		/* Data CRC Fail Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define        nDAT_CRC_FAIL_MASK  0x0
#define          CMD_TIMEOUT_MASK  0x4        		/* CMD Time Out Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define         nCMD_TIMEOUT_MASK  0x0
#define          DAT_TIMEOUT_MASK  0x8        		/* Data Time Out Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define         nDAT_TIMEOUT_MASK  0x0
#define          TX_UNDERRUN_MASK  0x10       		/* Transmit Underrun Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define         nTX_UNDERRUN_MASK  0x0
#define           RX_OVERRUN_MASK  0x20       		/* Receive Overrun Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define          nRX_OVERRUN_MASK  0x0
#define         CMD_RESP_END_MASK  0x40       		/* CMD Response End Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define        nCMD_RESP_END_MASK  0x0
#define             CMD_SENT_MASK  0x80       		/* CMD Sent Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define            nCMD_SENT_MASK  0x0
#define              DAT_END_MASK  0x100      		/* Data End Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define             nDAT_END_MASK  0x0
#define        START_BIT_ERR_MASK  0x200      		/* Start Bit Error Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define       nSTART_BIT_ERR_MASK  0x0
#define          DAT_BLK_END_MASK  0x400      		/* Data Block End Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define         nDAT_BLK_END_MASK  0x0
#define              CMD_ACT_MASK  0x800      		/* CMD Active Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define             nCMD_ACT_MASK  0x0
#define               TX_ACT_MASK  0x1000     		/* Transmit Active Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define              nTX_ACT_MASK  0x0
#define               RX_ACT_MASK  0x2000     		/* Receive Active Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define              nRX_ACT_MASK  0x0
#define         TX_FIFO_STAT_MASK  0x4000     		/* Transmit FIFO Status Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define        nTX_FIFO_STAT_MASK  0x0
#define         RX_FIFO_STAT_MASK  0x8000     		/* Receive FIFO Status Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define        nRX_FIFO_STAT_MASK  0x0
#define         TX_FIFO_FULL_MASK  0x10000    		/* Transmit FIFO Full Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define        nTX_FIFO_FULL_MASK  0x0
#define         RX_FIFO_FULL_MASK  0x20000    		/* Receive FIFO Full Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define        nRX_FIFO_FULL_MASK  0x0
#define         TX_FIFO_ZERO_MASK  0x40000    		/* Transmit FIFO Empty Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define        nTX_FIFO_ZERO_MASK  0x0
#define          RX_DAT_ZERO_MASK  0x80000    		/* Receive FIFO Empty Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define         nRX_DAT_ZERO_MASK  0x0
#define           TX_DAT_RDY_MASK  0x100000   		/* Transmit Data Available Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define          nTX_DAT_RDY_MASK  0x0
#define          RX_FIFO_RDY_MASK  0x200000   		/* Receive Data Available Mask */
/* legacy bit mask (below) provided for backwards code compatibility */
#define         nRX_FIFO_RDY_MASK  0x0

/* Bit masks for RSI_FIFO_CNT */
#define                FIFO_COUNT  0x7fff     		/* FIFO Count */

/* Bit masks for RSI_CEATA_CONTROL */
#define		  CEATA_TX_CCSD  0x1	    		/* Send CE-ATA CCSD sequence */

/* Bit masks for RSI_ESTAT */
#define              SDIO_INT_DET  0x2        		/* SDIO Int Detected */
/* legacy bit mask (below) provided for backwards code compatibility */
#define             nSDIO_INT_DET  0x0
#define               SD_CARD_DET  0x10       		/* SD Card Detect */
/* legacy bit mask (below) provided for backwards code compatibility */
#define              nSD_CARD_DET  0x0
#define             CEATA_INT_DET  0x20

/* Bit masks for RSI_EMASK */
#define         SDIO_INT_DET_MASK  0x2                /* Mask SDIO Int Detected */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                  SDIO_MSK  SDIO_INT_DET_MASK  /* Mask SDIO Int Detected */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                 nSDIO_MSK  0x0
#define          SD_CARD_DET_MASK  0x10               /* Mask Card Detect */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                  SCD_MASK  SD_CARD_DET_MASK   /* Mask Card Detect */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                  nSCD_MSK  0x0
#define        CEATA_INT_DET_MASK  0x20


/* Bit masks for RSI_CFG */
/* Left in for backwards compatibility */
#define                RSI_CLK_EN  0x1
/* legacy bit mask (below) provided for backwards code compatibility */
#define                   CLKS_EN  RSI_CLK_EN        	/* Clocks Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                  nCLKS_EN  0x0
#define                  SDIO4_EN  0x4        		/* SDIO 4-Bit Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                      SD4E  SDIO4_EN        	/* SDIO 4-Bit Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                     nSD4E  0x0
#define                     MW_EN  0x8        		/* Moving Window Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                       MWE  MW_EN        	/* Moving Window Enable */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                      nMWE  0x0
#define                   RSI_RST  0x10       		/* SDMMC Reset */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                    SD_RST  RSI_RST       	/* SDMMC Reset */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                   nSD_RST  0x0
#define                    PU_DAT  0x20       		/* Pull-up SD_DAT */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                 PUP_SDDAT  PU_DAT       	/* Pull-up SD_DAT */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                nPUP_SDDAT  0x0
#define                   PU_DAT3  0x40       		/* Pull-up SD_DAT3 */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                PUP_SDDAT3  PU_DAT3       	/* Pull-up SD_DAT3 */
/* legacy bit mask (below) provided for backwards code compatibility */
#define               nPUP_SDDAT3  0x0
#define                   PD_DAT3  0x80       		/* Pull-down SD_DAT3 */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                 PD_SDDAT3  PD_DAT3       	/* Pull-down SD_DAT3 */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                nPD_SDDAT3  0x0


/* Bit masks for RSI_RD_WAIT_EN */
#define			 SDIO_RWR  0x1             	/* Read Wait Request */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                       RWR  SDIO_RWR        	/* Read Wait Request */
/* legacy bit mask (below) provided for backwards code compatibility */
#define                      nRWR  0x0

/* Bit masks for RSI_PIDx */
#define                   RSI_PID  0xff			/* RSI Peripheral ID */
#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* _DEF_BF514_H */
