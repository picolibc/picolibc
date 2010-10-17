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
** Copyright (C) 2008 Analog Devices Inc., All Rights Reserved.
**
************************************************************************************
**
** This include file contains a list of macro "defines" to enable the programmer
** to use symbolic names for register-access and bit-manipulation.
**
**/
#ifndef _DEF_BF518_H
#define _DEF_BF518_H

/* Include all Core registers and bit definitions */
#include <def_LPBlackfin.h>

/* SYSTEM & MMR ADDRESS DEFINITIONS FOR ADSP-BF518 */

/* Include defBF51x_base.h for the set of #defines that are common to all ADSP-BF51x processors */
#include <defBF51x_base.h>

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_19_4:"macros not strictly following 19.4")
#pragma diag(suppress:misra_rule_19_7:"Allow function-like macros")
#endif /* _MISRA_RULES */

/* The following are the #defines needed by ADSP-BF518 that are not in the common header */
/* 10/100 Ethernet Controller	(0xFFC03000 - 0xFFC031FF) */

#define EMAC_OPMODE             0xFFC03000       /* Operating Mode Register                              */
#define EMAC_ADDRLO             0xFFC03004       /* Address Low (32 LSBs) Register                       */
#define EMAC_ADDRHI             0xFFC03008       /* Address High (16 MSBs) Register                      */
#define EMAC_HASHLO             0xFFC0300C       /* Multicast Hash Table Low (Bins 31-0) Register        */
#define EMAC_HASHHI             0xFFC03010       /* Multicast Hash Table High (Bins 63-32) Register      */
#define EMAC_STAADD             0xFFC03014       /* Station Management Address Register                  */
#define EMAC_STADAT             0xFFC03018       /* Station Management Data Register                     */
#define EMAC_FLC                0xFFC0301C       /* Flow Control Register                                */
#define EMAC_VLAN1              0xFFC03020       /* VLAN1 Tag Register                                   */
#define EMAC_VLAN2              0xFFC03024       /* VLAN2 Tag Register                                   */
#define EMAC_WKUP_CTL           0xFFC0302C       /* Wake-Up Control/Status Register                      */
#define EMAC_WKUP_FFMSK0        0xFFC03030       /* Wake-Up Frame Filter 0 Byte Mask Register            */
#define EMAC_WKUP_FFMSK1        0xFFC03034       /* Wake-Up Frame Filter 1 Byte Mask Register            */
#define EMAC_WKUP_FFMSK2        0xFFC03038       /* Wake-Up Frame Filter 2 Byte Mask Register            */
#define EMAC_WKUP_FFMSK3        0xFFC0303C       /* Wake-Up Frame Filter 3 Byte Mask Register            */
#define EMAC_WKUP_FFCMD         0xFFC03040       /* Wake-Up Frame Filter Commands Register               */
#define EMAC_WKUP_FFOFF         0xFFC03044       /* Wake-Up Frame Filter Offsets Register                */
#define EMAC_WKUP_FFCRC0        0xFFC03048       /* Wake-Up Frame Filter 0,1 CRC-16 Register             */
#define EMAC_WKUP_FFCRC1        0xFFC0304C       /* Wake-Up Frame Filter 2,3 CRC-16 Register             */

#define EMAC_SYSCTL             0xFFC03060       /* EMAC System Control Register                         */
#define EMAC_SYSTAT             0xFFC03064       /* EMAC System Status Register                          */
#define EMAC_RX_STAT            0xFFC03068       /* RX Current Frame Status Register                     */
#define EMAC_RX_STKY            0xFFC0306C       /* RX Sticky Frame Status Register                      */
#define EMAC_RX_IRQE            0xFFC03070       /* RX Frame Status Interrupt Enables Register           */
#define EMAC_TX_STAT            0xFFC03074       /* TX Current Frame Status Register                     */
#define EMAC_TX_STKY            0xFFC03078       /* TX Sticky Frame Status Register                      */
#define EMAC_TX_IRQE            0xFFC0307C       /* TX Frame Status Interrupt Enables Register           */

#define EMAC_MMC_CTL            0xFFC03080       /* MMC Counter Control Register                         */
#define EMAC_MMC_RIRQS          0xFFC03084       /* MMC RX Interrupt Status Register                     */
#define EMAC_MMC_RIRQE          0xFFC03088       /* MMC RX Interrupt Enables Register                    */
#define EMAC_MMC_TIRQS          0xFFC0308C       /* MMC TX Interrupt Status Register                     */
#define EMAC_MMC_TIRQE          0xFFC03090       /* MMC TX Interrupt Enables Register                    */

/* EMAC PTP (IEEE 1588) */

#define EMAC_PTP_CTL		  0xffc030a0	 /* PTP Block Control 						   */
#define EMAC_PTP_IE		  0xffc030a4   	 /* PTP Block Interrupt Enable 				   */
#define EMAC_PTP_ISTAT		  0xffc030a8       /* PTP Block Interrupt Status 				   */
#define EMAC_PTP_FOFF		  0xffc030ac       /* PTP Filter offset Register 				   */
#define EMAC_PTP_FV1		  0xffc030b0       /* PTP Filter Value Register 1 				   */
#define EMAC_PTP_FV2		  0xffc030b4       /* PTP Filter Value Register 2 				   */
#define EMAC_PTP_FV3		  0xffc030b8       /* PTP Filter Value Register 3 				   */
#define EMAC_PTP_ADDEND		  0xffc030bc       /* PTP Addend for Frequency Compensation 		   */
#define EMAC_PTP_ACCR		  0xffc030c0   	 /* PTP Accumulator for Frequency Compensation 		   */
#define EMAC_PTP_OFFSET		  0xffc030c4   	 /* PTP Time Offset Register 					   */
#define EMAC_PTP_TIMELO		  0xffc030c8   	 /* PTP Precision Clock Time Low 				   */
#define EMAC_PTP_TIMEHI		  0xffc030cc   	 /* PTP Precision Clock Time High 				   */
#define EMAC_PTP_RXSNAPLO	  0xffc030d0   	 /* PTP Receive Snapshot Register Low 			   */
#define EMAC_PTP_RXSNAPHI	  0xffc030d4   	 /* PTP Receive Snapshot Register High 			   */
#define EMAC_PTP_TXSNAPLO	  0xffc030d8   	 /* PTP Transmit Snapshot Register Low 			   */
#define EMAC_PTP_TXSNAPHI	  0xffc030dc   	 /* PTP Transmit Snapshot Register High 			   */
#define EMAC_PTP_ALARMLO	  0xffc030e0   	 /* PTP Alarm time Low 						   */
#define EMAC_PTP_ALARMHI	  0xffc030e4   	 /* PTP Alarm time High 					   */
#define EMAC_PTP_ID_OFF		  0xffc030e8   	 /* PTP Capture ID offset register 				   */
#define EMAC_PTP_ID_SNAP	  0xffc030ec   	 /* PTP Capture ID register 					   */
#define EMAC_PTP_PPS_STARTLOP	  0xffc030f0   	 /* PPS Start Time Low 						   */
#define EMAC_PTP_PPS_STARTHIP	  0xffc030f4   	 /* PPS Start Time High						   */
#define EMAC_PTP_PPS_PERIOD	  0xffc030f8   	 /* PPS Count Register						   */

#define EMAC_RXC_OK             0xFFC03100       /* RX Frame Successful Count                            */
#define EMAC_RXC_FCS            0xFFC03104       /* RX Frame FCS Failure Count                           */
#define EMAC_RXC_ALIGN          0xFFC03108       /* RX Alignment Error Count                             */
#define EMAC_RXC_OCTET          0xFFC0310C       /* RX Octets Successfully Received Count                */
#define EMAC_RXC_DMAOVF         0xFFC03110       /* Internal MAC Sublayer Error RX Frame Count           */
#define EMAC_RXC_UNICST         0xFFC03114       /* Unicast RX Frame Count                               */
#define EMAC_RXC_MULTI          0xFFC03118       /* Multicast RX Frame Count                             */
#define EMAC_RXC_BROAD          0xFFC0311C       /* Broadcast RX Frame Count                             */
#define EMAC_RXC_LNERRI         0xFFC03120       /* RX Frame In Range Error Count                        */
#define EMAC_RXC_LNERRO         0xFFC03124       /* RX Frame Out Of Range Error Count                    */
#define EMAC_RXC_LONG           0xFFC03128       /* RX Frame Too Long Count                              */
#define EMAC_RXC_MACCTL         0xFFC0312C       /* MAC Control RX Frame Count                           */
#define EMAC_RXC_OPCODE         0xFFC03130       /* Unsupported Op-Code RX Frame Count                   */
#define EMAC_RXC_PAUSE          0xFFC03134       /* MAC Control Pause RX Frame Count                     */
#define EMAC_RXC_ALLFRM         0xFFC03138       /* Overall RX Frame Count                               */
#define EMAC_RXC_ALLOCT         0xFFC0313C       /* Overall RX Octet Count                               */
#define EMAC_RXC_TYPED          0xFFC03140       /* Type/Length Consistent RX Frame Count                */
#define EMAC_RXC_SHORT          0xFFC03144       /* RX Frame Fragment Count - Byte Count x < 64          */
#define EMAC_RXC_EQ64           0xFFC03148       /* Good RX Frame Count - Byte Count x = 64              */
#define EMAC_RXC_LT128          0xFFC0314C       /* Good RX Frame Count - Byte Count  64 < x < 128       */
#define EMAC_RXC_LT256          0xFFC03150       /* Good RX Frame Count - Byte Count 128 <= x < 256      */
#define EMAC_RXC_LT512          0xFFC03154       /* Good RX Frame Count - Byte Count 256 <= x < 512      */
#define EMAC_RXC_LT1024         0xFFC03158       /* Good RX Frame Count - Byte Count 512 <= x < 1024     */
#define EMAC_RXC_GE1024         0xFFC0315C       /* Good RX Frame Count - Byte Count x >= 1024           */

#define EMAC_TXC_OK             0xFFC03180       /* TX Frame Successful Count                             */
#define EMAC_TXC_1COL           0xFFC03184       /* TX Frames Successful After Single Collision Count     */
#define EMAC_TXC_GT1COL         0xFFC03188       /* TX Frames Successful After Multiple Collisions Count  */
#define EMAC_TXC_OCTET          0xFFC0318C       /* TX Octets Successfully Received Count                 */
#define EMAC_TXC_DEFER          0xFFC03190       /* TX Frame Delayed Due To Busy Count                    */
#define EMAC_TXC_LATECL         0xFFC03194       /* Late TX Collisions Count                              */
#define EMAC_TXC_XS_COL         0xFFC03198       /* TX Frame Failed Due To Excessive Collisions Count     */
#define EMAC_TXC_DMAUND         0xFFC0319C       /* Internal MAC Sublayer Error TX Frame Count            */
#define EMAC_TXC_CRSERR         0xFFC031A0       /* Carrier Sense Deasserted During TX Frame Count        */
#define EMAC_TXC_UNICST         0xFFC031A4       /* Unicast TX Frame Count                                */
#define EMAC_TXC_MULTI          0xFFC031A8       /* Multicast TX Frame Count                              */
#define EMAC_TXC_BROAD          0xFFC031AC       /* Broadcast TX Frame Count                              */
#define EMAC_TXC_XS_DFR         0xFFC031B0       /* TX Frames With Excessive Deferral Count               */
#define EMAC_TXC_MACCTL         0xFFC031B4       /* MAC Control TX Frame Count                            */
#define EMAC_TXC_ALLFRM         0xFFC031B8       /* Overall TX Frame Count                                */
#define EMAC_TXC_ALLOCT         0xFFC031BC       /* Overall TX Octet Count                                */
#define EMAC_TXC_EQ64           0xFFC031C0       /* Good TX Frame Count - Byte Count x = 64               */
#define EMAC_TXC_LT128          0xFFC031C4       /* Good TX Frame Count - Byte Count  64 < x < 128        */
#define EMAC_TXC_LT256          0xFFC031C8       /* Good TX Frame Count - Byte Count 128 <= x < 256       */
#define EMAC_TXC_LT512          0xFFC031CC       /* Good TX Frame Count - Byte Count 256 <= x < 512       */
#define EMAC_TXC_LT1024         0xFFC031D0       /* Good TX Frame Count - Byte Count 512 <= x < 1024      */
#define EMAC_TXC_GE1024         0xFFC031D4       /* Good TX Frame Count - Byte Count x >= 1024            */
#define EMAC_TXC_ABORT          0xFFC031D8       /* Total TX Frames Aborted Count                         */

/* Listing for IEEE-Supported Count Registers */

#define FramesReceivedOK                EMAC_RXC_OK        /* RX Frame Successful Count                            */
#define FrameCheckSequenceErrors        EMAC_RXC_FCS       /* RX Frame FCS Failure Count                           */
#define AlignmentErrors                 EMAC_RXC_ALIGN     /* RX Alignment Error Count                             */
#define OctetsReceivedOK                EMAC_RXC_OCTET     /* RX Octets Successfully Received Count                */
#define FramesLostDueToIntMACRcvError   EMAC_RXC_DMAOVF    /* Internal MAC Sublayer Error RX Frame Count           */
#define UnicastFramesReceivedOK         EMAC_RXC_UNICST    /* Unicast RX Frame Count                               */
#define MulticastFramesReceivedOK       EMAC_RXC_MULTI     /* Multicast RX Frame Count                             */
#define BroadcastFramesReceivedOK       EMAC_RXC_BROAD     /* Broadcast RX Frame Count                             */
#define InRangeLengthErrors             EMAC_RXC_LNERRI    /* RX Frame In Range Error Count                        */
#define OutOfRangeLengthField           EMAC_RXC_LNERRO    /* RX Frame Out Of Range Error Count                    */
#define FrameTooLongErrors              EMAC_RXC_LONG      /* RX Frame Too Long Count                              */
#define MACControlFramesReceived        EMAC_RXC_MACCTL    /* MAC Control RX Frame Count                           */
#define UnsupportedOpcodesReceived      EMAC_RXC_OPCODE    /* Unsupported Op-Code RX Frame Count                   */
#define PAUSEMACCtrlFramesReceived      EMAC_RXC_PAUSE     /* MAC Control Pause RX Frame Count                     */
#define FramesReceivedAll               EMAC_RXC_ALLFRM    /* Overall RX Frame Count                               */
#define OctetsReceivedAll               EMAC_RXC_ALLOCT    /* Overall RX Octet Count                               */
#define TypedFramesReceived             EMAC_RXC_TYPED     /* Type/Length Consistent RX Frame Count                */
#define FramesLenLt64Received           EMAC_RXC_SHORT     /* RX Frame Fragment Count - Byte Count x < 64          */
#define FramesLenEq64Received           EMAC_RXC_EQ64      /* Good RX Frame Count - Byte Count x = 64              */
#define FramesLen65_127Received         EMAC_RXC_LT128     /* Good RX Frame Count - Byte Count  64 < x < 128       */
#define FramesLen128_255Received        EMAC_RXC_LT256     /* Good RX Frame Count - Byte Count 128 <= x < 256      */
#define FramesLen256_511Received        EMAC_RXC_LT512     /* Good RX Frame Count - Byte Count 256 <= x < 512      */
#define FramesLen512_1023Received       EMAC_RXC_LT1024    /* Good RX Frame Count - Byte Count 512 <= x < 1024     */
#define FramesLen1024_MaxReceived       EMAC_RXC_GE1024    /* Good RX Frame Count - Byte Count x >= 1024           */

#define FramesTransmittedOK             EMAC_TXC_OK        /* TX Frame Successful Count                            */
#define SingleCollisionFrames           EMAC_TXC_1COL      /* TX Frames Successful After Single Collision Count    */
#define MultipleCollisionFrames         EMAC_TXC_GT1COL    /* TX Frames Successful After Multiple Collisions Count */
#define OctetsTransmittedOK             EMAC_TXC_OCTET     /* TX Octets Successfully Received Count                */
#define FramesWithDeferredXmissions     EMAC_TXC_DEFER     /* TX Frame Delayed Due To Busy Count                   */
#define LateCollisions                  EMAC_TXC_LATECL    /* Late TX Collisions Count                             */
#define FramesAbortedDueToXSColls       EMAC_TXC_XS_COL    /* TX Frame Failed Due To Excessive Collisions Count    */
#define FramesLostDueToIntMacXmitError  EMAC_TXC_DMAUND    /* Internal MAC Sublayer Error TX Frame Count           */
#define CarrierSenseErrors              EMAC_TXC_CRSERR    /* Carrier Sense Deasserted During TX Frame Count       */
#define UnicastFramesXmittedOK          EMAC_TXC_UNICST    /* Unicast TX Frame Count                               */
#define MulticastFramesXmittedOK        EMAC_TXC_MULTI     /* Multicast TX Frame Count                             */
#define BroadcastFramesXmittedOK        EMAC_TXC_BROAD     /* Broadcast TX Frame Count                             */
#define FramesWithExcessiveDeferral     EMAC_TXC_XS_DFR    /* TX Frames With Excessive Deferral Count              */
#define MACControlFramesTransmitted     EMAC_TXC_MACCTL    /* MAC Control TX Frame Count                           */
#define FramesTransmittedAll            EMAC_TXC_ALLFRM    /* Overall TX Frame Count                               */
#define OctetsTransmittedAll            EMAC_TXC_ALLOCT    /* Overall TX Octet Count                               */
#define FramesLenEq64Transmitted        EMAC_TXC_EQ64      /* Good TX Frame Count - Byte Count x = 64              */
#define FramesLen65_127Transmitted      EMAC_TXC_LT128     /* Good TX Frame Count - Byte Count  64 < x < 128       */
#define FramesLen128_255Transmitted     EMAC_TXC_LT256     /* Good TX Frame Count - Byte Count 128 <= x < 256      */
#define FramesLen256_511Transmitted     EMAC_TXC_LT512     /* Good TX Frame Count - Byte Count 256 <= x < 512      */
#define FramesLen512_1023Transmitted    EMAC_TXC_LT1024    /* Good TX Frame Count - Byte Count 512 <= x < 1024     */
#define FramesLen1024_MaxTransmitted    EMAC_TXC_GE1024    /* Good TX Frame Count - Byte Count x >= 1024           */
#define TxAbortedFrames                 EMAC_TXC_ABORT     /* Total TX Frames Aborted Count                        */


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



/***********************************************************************************
** System MMR Register Bits And Macros
**
** Disclaimer:	All macros are intended to make C and Assembly code more readable.
**				Use these macros carefully, as any that do left shifts for field
**				depositing will result in the lower order bits being destroyed.  Any
**				macro that shifts left to properly position the bit-field should be
**				used as part of an OR to initialize a register and NOT as a dynamic
**				modifier UNLESS the lower order bits are saved and ORed back in when
**				the macro is used.
*************************************************************************************/

/************************  ETHERNET 10/100 CONTROLLER MASKS  ************************/

/* EMAC_OPMODE Masks */

#define	RE                 0x00000001     /* Receiver Enable                                    */
#define	ASTP               0x00000002     /* Enable Automatic Pad Stripping On RX Frames        */
#define	HU                 0x00000010     /* Hash Filter Unicast Address                        */
#define	HM                 0x00000020     /* Hash Filter Multicast Address                      */
#define	PAM                0x00000040     /* Pass-All-Multicast Mode Enable                     */
#define	PR                 0x00000080     /* Promiscuous Mode Enable                            */
#define	IFE                0x00000100     /* Inverse Filtering Enable                           */
#define	DBF                0x00000200     /* Disable Broadcast Frame Reception                  */
#define	PBF                0x00000400     /* Pass Bad Frames Enable                             */
#define	PSF                0x00000800     /* Pass Short Frames Enable                           */
#define	RAF                0x00001000     /* Receive-All Mode                                   */
#define	TE                 0x00010000     /* Transmitter Enable                                 */
#define	DTXPAD             0x00020000     /* Disable Automatic TX Padding                       */
#define	DTXCRC             0x00040000     /* Disable Automatic TX CRC Generation                */
#define	DC                 0x00080000     /* Deferral Check                                     */
#define	BOLMT              0x00300000     /* Back-Off Limit                                     */
#define	BOLMT_10           0x00000000     /*		10-bit range                            */
#define	BOLMT_8            0x00100000     /*		8-bit range                             */
#define	BOLMT_4            0x00200000     /*		4-bit range                             */
#define	BOLMT_1            0x00300000     /*		1-bit range                             */
#define	DRTY               0x00400000     /* Disable TX Retry On Collision                      */
#define	LCTRE              0x00800000     /* Enable TX Retry On Late Collision                  */
#define	RMII               0x01000000     /* RMII/MII* Mode                                     */
#define	RMII_10            0x02000000     /* Speed Select for RMII Port (10MBit/100MBit*)       */
#define	FDMODE             0x04000000     /* Duplex Mode Enable (Full/Half*)                    */
#define	LB                 0x08000000     /* Internal Loopback Enable                           */
#define	DRO                0x10000000     /* Disable Receive Own Frames (Half-Duplex Mode)      */

/* EMAC_STAADD Masks */

#define	STABUSY            0x00000001     /* Initiate Station Mgt Reg Access / STA Busy Stat    */
#define	STAOP              0x00000002     /* Station Management Operation Code (Write/Read*)    */
#define	STADISPRE          0x00000004     /* Disable Preamble Generation                        */
#define	STAIE              0x00000008     /* Station Mgt. Transfer Done Interrupt Enable        */
#define	REGAD              0x000007C0     /* STA Register Address                               */
#define	PHYAD              0x0000F800     /* PHY Device Address                                 */

#ifdef _MISRA_RULES
#define	SET_REGAD(x) (((x)&0x1Fu)<<  6 )   /* Set STA Register Address                           */
#define	SET_PHYAD(x) (((x)&0x1Fu)<< 11 )   /* Set PHY Device Address                             */
#else
#define	SET_REGAD(x) (((x)&0x1F)<<  6 )   /* Set STA Register Address                           */
#define	SET_PHYAD(x) (((x)&0x1F)<< 11 )   /* Set PHY Device Address                             */
#endif /* _MISRA_RULES */

/* EMAC_STADAT Mask */

#define	STADATA            0x0000FFFF     /* Station Management Data                            */

/* EMAC_FLC Masks */

#define	FLCBUSY            0x00000001     /* Send Flow Ctrl Frame / Flow Ctrl Busy Status       */
#define	FLCE               0x00000002     /* Flow Control Enable                                */
#define	PCF                0x00000004     /* Pass Control Frames                                */
#define	BKPRSEN            0x00000008     /* Enable Backpressure                                */
#define	FLCPAUSE           0xFFFF0000     /* Pause Time                                         */

#ifdef _MISRA_RULES
#define	SET_FLCPAUSE(x) (((x)&0xFFFFu)<< 16) /* Set Pause Time                                   */
#else
#define	SET_FLCPAUSE(x) (((x)&0xFFFF)<< 16) /* Set Pause Time                                   */
#endif /* _MISRA_RULES */

/* EMAC_WKUP_CTL Masks */

#define	CAPWKFRM           0x00000001    /* Capture Wake-Up Frames                              */
#define	MPKE               0x00000002    /* Magic Packet Enable                                 */
#define	RWKE               0x00000004    /* Remote Wake-Up Frame Enable                         */
#define	GUWKE              0x00000008    /* Global Unicast Wake Enable                          */
#define	MPKS               0x00000020    /* Magic Packet Received Status                        */
#define	RWKS               0x00000F00    /* Wake-Up Frame Received Status, Filters 3:0          */

/* EMAC_WKUP_FFCMD Masks */

#define	WF0_E              0x00000001    /* Enable Wake-Up Filter 0                              */
#define	WF0_T              0x00000008    /* Wake-Up Filter 0 Addr Type (Multicast/Unicast*)      */
#define	WF1_E              0x00000100    /* Enable Wake-Up Filter 1                              */
#define	WF1_T              0x00000800    /* Wake-Up Filter 1 Addr Type (Multicast/Unicast*)      */
#define	WF2_E              0x00010000    /* Enable Wake-Up Filter 2                              */
#define	WF2_T              0x00080000    /* Wake-Up Filter 2 Addr Type (Multicast/Unicast*)      */
#define	WF3_E              0x01000000    /* Enable Wake-Up Filter 3                              */
#define	WF3_T              0x08000000    /* Wake-Up Filter 3 Addr Type (Multicast/Unicast*)      */

/* EMAC_WKUP_FFOFF Masks */

#define	WF0_OFF            0x000000FF    /* Wake-Up Filter 0 Pattern Offset                      */
#define	WF1_OFF            0x0000FF00    /* Wake-Up Filter 1 Pattern Offset                      */
#define	WF2_OFF            0x00FF0000    /* Wake-Up Filter 2 Pattern Offset                      */
#define	WF3_OFF            0xFF000000    /* Wake-Up Filter 3 Pattern Offset                      */

#ifdef _MISRA_RULES
#define	SET_WF0_OFF(x) (((x)&0xFFu)<<  0 ) /* Set Wake-Up Filter 0 Byte Offset                    */
#define	SET_WF1_OFF(x) (((x)&0xFFu)<<  8 ) /* Set Wake-Up Filter 1 Byte Offset                    */
#define	SET_WF2_OFF(x) (((x)&0xFFu)<< 16 ) /* Set Wake-Up Filter 2 Byte Offset                    */
#define	SET_WF3_OFF(x) (((x)&0xFFu)<< 24 ) /* Set Wake-Up Filter 3 Byte Offset                    */
#else
#define	SET_WF0_OFF(x) (((x)&0xFF)<<  0 ) /* Set Wake-Up Filter 0 Byte Offset                    */
#define	SET_WF1_OFF(x) (((x)&0xFF)<<  8 ) /* Set Wake-Up Filter 1 Byte Offset                    */
#define	SET_WF2_OFF(x) (((x)&0xFF)<< 16 ) /* Set Wake-Up Filter 2 Byte Offset                    */
#define	SET_WF3_OFF(x) (((x)&0xFF)<< 24 ) /* Set Wake-Up Filter 3 Byte Offset                    */
#endif /* _MISRA_RULES */

/* Set ALL Offsets */
#define	SET_WF_OFFS(x0,x1,x2,x3) (SET_WF0_OFF((x0))|SET_WF1_OFF((x1))|SET_WF2_OFF((x2))|SET_WF3_OFF((x3)))

/* EMAC_WKUP_FFCRC0 Masks */

#define	WF0_CRC           0x0000FFFF    /* Wake-Up Filter 0 Pattern CRC                           */
#define	WF1_CRC           0xFFFF0000    /* Wake-Up Filter 1 Pattern CRC                           */

#ifdef _MISRA_RULES
#define	SET_WF0_CRC(x) (((x)&0xFFFFu)<<   0 ) /* Set Wake-Up Filter 0 Target CRC                   */
#define	SET_WF1_CRC(x) (((x)&0xFFFFu)<<  16 ) /* Set Wake-Up Filter 1 Target CRC                   */
#else
#define	SET_WF0_CRC(x) (((x)&0xFFFF)<<   0 ) /* Set Wake-Up Filter 0 Target CRC                   */
#define	SET_WF1_CRC(x) (((x)&0xFFFF)<<  16 ) /* Set Wake-Up Filter 1 Target CRC                   */
#endif /* _MISRA_RULES */

/* EMAC_WKUP_FFCRC1 Masks */

#define	WF2_CRC           0x0000FFFF    /* Wake-Up Filter 2 Pattern CRC                           */
#define	WF3_CRC           0xFFFF0000    /* Wake-Up Filter 3 Pattern CRC                           */

#ifdef _MISRA_RULES
#define	SET_WF2_CRC(x) (((x)&0xFFFFu)<<   0 ) /* Set Wake-Up Filter 2 Target CRC                   */
#define	SET_WF3_CRC(x) (((x)&0xFFFFu)<<  16 ) /* Set Wake-Up Filter 3 Target CRC                   */
#else
#define	SET_WF2_CRC(x) (((x)&0xFFFF)<<   0 ) /* Set Wake-Up Filter 2 Target CRC                   */
#define	SET_WF3_CRC(x) (((x)&0xFFFF)<<  16 ) /* Set Wake-Up Filter 3 Target CRC                   */
#endif /* _MISRA_RULES */

/* EMAC_SYSCTL Masks */

#define	PHYIE             0x00000001    /* PHY_INT Interrupt Enable                               */
#define	RXDWA             0x00000002    /* Receive Frame DMA Word Alignment (Odd/Even*)           */
#define	RXCKS             0x00000004    /* Enable RX Frame TCP/UDP Checksum Computation           */
#define	MDCDIV            0x00003F00    /* SCLK:MDC Clock Divisor [MDC=SCLK/(2*(N+1))]            */

#ifdef _MISRA_RULES
#define	SET_MDCDIV(x) (((x)&0x3Fu)<< 8)   /* Set MDC Clock Divisor                                 */
#else
#define	SET_MDCDIV(x) (((x)&0x3F)<< 8)   /* Set MDC Clock Divisor                                 */
#endif /* _MISRA_RULES */

/* EMAC_SYSTAT Masks */

#define	PHYINT            0x00000001    /* PHY_INT Interrupt Status                               */
#define	MMCINT            0x00000002    /* MMC Counter Interrupt Status                           */
#define	RXFSINT           0x00000004    /* RX Frame-Status Interrupt Status                       */
#define	TXFSINT           0x00000008    /* TX Frame-Status Interrupt Status                       */
#define	WAKEDET           0x00000010    /* Wake-Up Detected Status                                */
#define	RXDMAERR          0x00000020    /* RX DMA Direction Error Status                          */
#define	TXDMAERR          0x00000040    /* TX DMA Direction Error Status                          */
#define	STMDONE           0x00000080    /* Station Mgt. Transfer Done Interrupt Status            */

/* EMAC_RX_STAT, EMAC_RX_STKY, and EMAC_RX_IRQE Masks */

#define	RX_FRLEN          0x000007FF    /* Frame Length In Bytes                                  */
#define	RX_COMP           0x00001000    /* RX Frame Complete                                      */
#define	RX_OK             0x00002000    /* RX Frame Received With No Errors                       */
#define	RX_LONG           0x00004000    /* RX Frame Too Long Error                                */
#define	RX_ALIGN          0x00008000    /* RX Frame Alignment Error                               */
#define	RX_CRC            0x00010000    /* RX Frame CRC Error                                     */
#define	RX_LEN            0x00020000    /* RX Frame Length Error                                  */
#define	RX_FRAG           0x00040000    /* RX Frame Fragment Error                                */
#define	RX_ADDR           0x00080000    /* RX Frame Address Filter Failed Error                   */
#define	RX_DMAO           0x00100000    /* RX Frame DMA Overrun Error                             */
#define	RX_PHY            0x00200000    /* RX Frame PHY Error                                     */
#define	RX_LATE           0x00400000    /* RX Frame Late Collision Error                          */
#define	RX_RANGE          0x00800000    /* RX Frame Length Field Out of Range Error               */
#define	RX_MULTI          0x01000000    /* RX Multicast Frame Indicator                           */
#define	RX_BROAD          0x02000000    /* RX Broadcast Frame Indicator                           */
#define	RX_CTL            0x04000000    /* RX Control Frame Indicator                             */
#define	RX_UCTL           0x08000000    /* Unsupported RX Control Frame Indicator                 */
#define	RX_TYPE           0x10000000    /* RX Typed Frame Indicator                               */
#define	RX_VLAN1          0x20000000    /* RX VLAN1 Frame Indicator                               */
#define	RX_VLAN2          0x40000000    /* RX VLAN2 Frame Indicator                               */
#define	RX_ACCEPT         0x80000000    /* RX Frame Accepted Indicator                            */

/*  EMAC_TX_STAT, EMAC_TX_STKY, and EMAC_TX_IRQE Masks  */

#define	TX_COMP           0x00000001    /* TX Frame Complete                                      */
#define	TX_OK             0x00000002    /* TX Frame Sent With No Errors                           */
#define	TX_ECOLL          0x00000004    /* TX Frame Excessive Collision Error                     */
#define	TX_LATE           0x00000008    /* TX Frame Late Collision Error                          */
#define	TX_DMAU           0x00000010    /* TX Frame DMA Underrun Error (STAT)                     */
#define	TX_MACE           0x00000010    /* Internal MAC Error Detected (STKY and IRQE)            */
#define	TX_EDEFER         0x00000020    /* TX Frame Excessive Deferral Error                      */
#define	TX_BROAD          0x00000040    /* TX Broadcast Frame Indicator                           */
#define	TX_MULTI          0x00000080    /* TX Multicast Frame Indicator                           */
#define	TX_CCNT           0x00000F00    /* TX Frame Collision Count                               */
#define	TX_DEFER          0x00001000    /* TX Frame Deferred Indicator                            */
#define	TX_CRS            0x00002000    /* TX Frame Carrier Sense Not Asserted Error              */
#define	TX_LOSS           0x00004000    /* TX Frame Carrier Lost During TX Error                  */
#define	TX_RETRY          0x00008000    /* TX Frame Successful After Retry                        */
#define	TX_FRLEN          0x07FF0000    /* TX Frame Length (Bytes)                                */

/* EMAC_MMC_CTL Masks */
#define	RSTC              0x00000001    /* Reset All Counters                                     */
#define	CROLL             0x00000002    /* Counter Roll-Over Enable                               */
#define	CCOR              0x00000004    /* Counter Clear-On-Read Mode Enable                      */
#define	MMCE              0x00000008    /* Enable MMC Counter Operation                           */

/* EMAC_MMC_RIRQS and EMAC_MMC_RIRQE Masks */
#define	RX_OK_CNT         0x00000001    /* RX Frames Received With No Errors                      */
#define	RX_FCS_CNT        0x00000002    /* RX Frames W/Frame Check Sequence Errors                */
#define	RX_ALIGN_CNT      0x00000004    /* RX Frames With Alignment Errors                        */
#define	RX_OCTET_CNT      0x00000008    /* RX Octets Received OK                                  */
#define	RX_LOST_CNT       0x00000010    /* RX Frames Lost Due To Internal MAC RX Error            */
#define	RX_UNI_CNT        0x00000020    /* Unicast RX Frames Received OK                          */
#define	RX_MULTI_CNT      0x00000040    /* Multicast RX Frames Received OK                        */
#define	RX_BROAD_CNT      0x00000080    /* Broadcast RX Frames Received OK                        */
#define	RX_IRL_CNT        0x00000100    /* RX Frames With In-Range Length Errors                  */
#define	RX_ORL_CNT        0x00000200    /* RX Frames With Out-Of-Range Length Errors              */
#define	RX_LONG_CNT       0x00000400    /* RX Frames With Frame Too Long Errors                   */
#define	RX_MACCTL_CNT     0x00000800    /* MAC Control RX Frames Received                         */
#define	RX_OPCODE_CTL     0x00001000    /* Unsupported Op-Code RX Frames Received                 */
#define	RX_PAUSE_CNT      0x00002000    /* PAUSEMAC Control RX Frames Received                    */
#define	RX_ALLF_CNT       0x00004000    /* All RX Frames Received                                 */
#define	RX_ALLO_CNT       0x00008000    /* All RX Octets Received                                 */
#define	RX_TYPED_CNT      0x00010000    /* Typed RX Frames Received                               */
#define	RX_SHORT_CNT      0x00020000    /* RX Frame Fragments (< 64 Bytes) Received               */
#define	RX_EQ64_CNT       0x00040000    /* 64-Byte RX Frames Received                             */
#define	RX_LT128_CNT      0x00080000    /* 65-127-Byte RX Frames Received                         */
#define	RX_LT256_CNT      0x00100000    /* 128-255-Byte RX Frames Received                        */
#define	RX_LT512_CNT      0x00200000    /* 256-511-Byte RX Frames Received                        */
#define	RX_LT1024_CNT     0x00400000    /* 512-1023-Byte RX Frames Received                       */
#define	RX_GE1024_CNT     0x00800000    /* 1024-Max-Byte RX Frames Received                       */

/* EMAC_MMC_TIRQS and EMAC_MMC_TIRQE Masks  */

#define	TX_OK_CNT         0x00000001    /* TX Frames Sent OK                                      */
#define	TX_SCOLL_CNT      0x00000002    /* TX Frames With Single Collisions                       */
#define	TX_MCOLL_CNT      0x00000004    /* TX Frames With Multiple Collisions                     */
#define	TX_OCTET_CNT      0x00000008    /* TX Octets Sent OK                                      */
#define	TX_DEFER_CNT      0x00000010    /* TX Frames With Deferred Transmission                   */
#define	TX_LATE_CNT       0x00000020    /* TX Frames With Late Collisions                         */
#define	TX_ABORTC_CNT     0x00000040    /* TX Frames Aborted Due To Excess Collisions             */
#define	TX_LOST_CNT       0x00000080    /* TX Frames Lost Due To Internal MAC TX Error            */
#define	TX_CRS_CNT        0x00000100    /* TX Frames With Carrier Sense Errors                    */
#define	TX_UNI_CNT        0x00000200    /* Unicast TX Frames Sent                                 */
#define	TX_MULTI_CNT      0x00000400    /* Multicast TX Frames Sent                               */
#define	TX_BROAD_CNT      0x00000800    /* Broadcast TX Frames Sent                               */
#define	TX_EXDEF_CTL      0x00001000    /* TX Frames With Excessive Deferral                      */
#define	TX_MACCTL_CNT     0x00002000    /* MAC Control TX Frames Sent                             */
#define	TX_ALLF_CNT       0x00004000    /* All TX Frames Sent                                     */
#define	TX_ALLO_CNT       0x00008000    /* All TX Octets Sent                                     */
#define	TX_EQ64_CNT       0x00010000    /* 64-Byte TX Frames Sent                                 */
#define	TX_LT128_CNT      0x00020000    /* 65-127-Byte TX Frames Sent                             */
#define	TX_LT256_CNT      0x00040000    /* 128-255-Byte TX Frames Sent                            */
#define	TX_LT512_CNT      0x00080000    /* 256-511-Byte TX Frames Sent                            */
#define	TX_LT1024_CNT     0x00100000    /* 512-1023-Byte TX Frames Sent                           */
#define	TX_GE1024_CNT     0x00200000    /* 1024-Max-Byte TX Frames Sent                           */
#define	TX_ABORT_CNT      0x00400000    /* TX Frames Aborted                                      */


/* Bit masks for EMAC_PTP_CTL */

#define	EMAC_PTP_CTL_EN	 0x1		  /* Block Enable								*/
#define	EMAC_PTP_CTL_TL	 0x2		  /* Time Stamp Lock							*/
#define	EMAC_PTP_CTL_CKS	 0xC		  /* Clock source for the PTP_TSYNC block				*/
#define	EMAC_PTP_CTL_ASEN	 0x10		  /* Auxiliary Snapshot Enable					*/
#define	EMAC_PTP_CTL_CKDIV 0x60		  /* Divider for the selected PTP_CLK output			*/
#define	EMAC_PTP_CTL_PPSEN 0x80		  /* Pulse Per Second (PPS) Enable					*/
#define	EMAC_PTP_CTL_EFTM	 0x100	  /* Ethernet Frame type field compare mask			*/
#define	EMAC_PTP_CTL_IPVM	 0x200	  /* IP Version field compare mask					*/
#define	EMAC_PTP_CTL_IPTM	 0x400	  /* IP Type Frame field (Layer 4 protocol) compare mask	*/
#define	EMAC_PTP_CTL_UDPEM 0x800	  /* UDP Event port field compare mask				*/
#define	EMAC_PTP_CTL_PTPCM 0x1000	  /* PTP Control field compare mask					*/
#define	EMAC_PTP_CTL_CKOEN 0x2000	  /* Clock output Enable						*/

/* Bit masks for EMAC_PTP_IE */

#define	EMAC_PTP_IE_ALIE	 0x1		  /* Alarm Feature and Interrupt Enable				*/
#define	EMAC_PTP_IE_RXEIE	 0x2		  /* Receive Event Interrupt Enable					*/
#define	EMAC_PTP_IE_RXGIE	 0x4		  /* Receive General Interrupt Enable				*/
#define	EMAC_PTP_IE_TXIE	 0x8		  /* Transmit Interrupt Enable					*/
#define	EMAC_PTP_IE_TXOVE	 0x10		  /* Transmit Overrun Error Interrupt Enable			*/
#define	EMAC_PTP_IE_RXOVE	 0x20		  /* Receive Overrun Error Interrupt Enable			*/
#define	EMAC_PTP_IE_ASIE	 0x40		  /* Auxiliary Snapshot Interrupt Enable				*/

/* Bit masks for EMAC_PTP_ISTAT */

#define	EMAC_PTP_ISTAT_ALS  0x1		  /* Alarm Status								*/
#define	EMAC_PTP_ISTAT_RXEL 0x2		  /* Receive Event Interrupt Locked					*/
#define	EMAC_PTP_ISTAT_RXGL 0x4		  /* Receive General Interrupt Locked				*/
#define	EMAC_PTP_ISTAT_TXTL 0x8		  /* Transmit Snapshot Locked						*/
#define	EMAC_PTP_ISTAT_RXOV 0x10	  /* Receive Snapshot Overrun Status				*/
#define	EMAC_PTP_ISTAT_TXOV 0x20	  /* Transmit snapshot Overrun Status				*/
#define	EMAC_PTP_ISTAT_ASL  0x40	  /* Auxiliary Snapshot Interrupt Status				*/


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


/* Bit masks for SDH_CFG */

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

#endif /* _DEF_BF518_H */
