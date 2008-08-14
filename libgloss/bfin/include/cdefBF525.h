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
** Copyright (C) 2008 Analog Devices, Inc.
**
************************************************************************************
**
** This include file contains a list of macro "defines" to enable the programmer
** to use symbolic names for the ADSP-BF525 peripherals.
**
************************************************************************************
** System MMR Register Map
************************************************************************************/

#ifndef _CDEF_BF525_H
#define _CDEF_BF525_H

/* include all Core registers and bit definitions */
#include <defBF525.h>

/* include core specific register pointer definitions */
#include <cdef_LPBlackfin.h>

/* SYSTEM & MMR ADDRESS DEFINITIONS FOR ADSP-BF525 */

/* include cdefBF52x_base.h for the set of #defines that are common to all ADSP-BF52x processors */
#include <cdefBF52x_base.h>

/* The following are the #defines needed by ADSP-BF525 that are not in the common header */

/* USB Control Registers */

#define                        pUSB_FADDR ((volatile unsigned short *)USB_FADDR)
#define                        pUSB_POWER ((volatile unsigned short *)USB_POWER)
#define                       pUSB_INTRTX ((volatile unsigned short *)USB_INTRTX)
#define                       pUSB_INTRRX ((volatile unsigned short *)USB_INTRRX)
#define                      pUSB_INTRTXE ((volatile unsigned short *)USB_INTRTXE)
#define                      pUSB_INTRRXE ((volatile unsigned short *)USB_INTRRXE)
#define                      pUSB_INTRUSB ((volatile unsigned short *)USB_INTRUSB)
#define                     pUSB_INTRUSBE ((volatile unsigned short *)USB_INTRUSBE)
#define                        pUSB_FRAME ((volatile unsigned short *)USB_FRAME)
#define                        pUSB_INDEX ((volatile unsigned short *)USB_INDEX)
#define                     pUSB_TESTMODE ((volatile unsigned short *)USB_TESTMODE)
#define                     pUSB_GLOBINTR ((volatile unsigned short *)USB_GLOBINTR)
#define                   pUSB_GLOBAL_CTL ((volatile unsigned short *)USB_GLOBAL_CTL)

/* USB Packet Control Registers */

#define                pUSB_TX_MAX_PACKET ((volatile unsigned short *)USB_TX_MAX_PACKET)
#define                         pUSB_CSR0 ((volatile unsigned short *)USB_CSR0)
#define                        pUSB_TXCSR ((volatile unsigned short *)USB_TXCSR)
#define                pUSB_RX_MAX_PACKET ((volatile unsigned short *)USB_RX_MAX_PACKET)
#define                        pUSB_RXCSR ((volatile unsigned short *)USB_RXCSR)
#define                       pUSB_COUNT0 ((volatile unsigned short *)USB_COUNT0)
#define                      pUSB_RXCOUNT ((volatile unsigned short *)USB_RXCOUNT)
#define                       pUSB_TXTYPE ((volatile unsigned short *)USB_TXTYPE)
#define                    pUSB_NAKLIMIT0 ((volatile unsigned short *)USB_NAKLIMIT0)
#define                   pUSB_TXINTERVAL ((volatile unsigned short *)USB_TXINTERVAL)
#define                       pUSB_RXTYPE ((volatile unsigned short *)USB_RXTYPE)
#define                   pUSB_RXINTERVAL ((volatile unsigned short *)USB_RXINTERVAL)
#define                      pUSB_TXCOUNT ((volatile unsigned short *)USB_TXCOUNT)

/* USB Endpoint FIFO Registers */

#define                     pUSB_EP0_FIFO ((volatile unsigned short *)USB_EP0_FIFO)
#define                     pUSB_EP1_FIFO ((volatile unsigned short *)USB_EP1_FIFO)
#define                     pUSB_EP2_FIFO ((volatile unsigned short *)USB_EP2_FIFO)
#define                     pUSB_EP3_FIFO ((volatile unsigned short *)USB_EP3_FIFO)
#define                     pUSB_EP4_FIFO ((volatile unsigned short *)USB_EP4_FIFO)
#define                     pUSB_EP5_FIFO ((volatile unsigned short *)USB_EP5_FIFO)
#define                     pUSB_EP6_FIFO ((volatile unsigned short *)USB_EP6_FIFO)
#define                     pUSB_EP7_FIFO ((volatile unsigned short *)USB_EP7_FIFO)

/* USB OTG Control Registers */

#define                  pUSB_OTG_DEV_CTL ((volatile unsigned short *)USB_OTG_DEV_CTL)
#define                 pUSB_OTG_VBUS_IRQ ((volatile unsigned short *)USB_OTG_VBUS_IRQ)
#define                pUSB_OTG_VBUS_MASK ((volatile unsigned short *)USB_OTG_VBUS_MASK)

/* USB Phy Control Registers */

#define                     pUSB_LINKINFO ((volatile unsigned short *)USB_LINKINFO)
#define                        pUSB_VPLEN ((volatile unsigned short *)USB_VPLEN)
#define                      pUSB_HS_EOF1 ((volatile unsigned short *)USB_HS_EOF1)
#define                      pUSB_FS_EOF1 ((volatile unsigned short *)USB_FS_EOF1)
#define                      pUSB_LS_EOF1 ((volatile unsigned short *)USB_LS_EOF1)

/* (APHY_CNTRL is for ADI usage only) */

#define                   pUSB_APHY_CNTRL ((volatile unsigned short *)USB_APHY_CNTRL)

/* (APHY_CALIB is for ADI usage only) */

#define                   pUSB_APHY_CALIB ((volatile unsigned short *)USB_APHY_CALIB)

#define                  pUSB_APHY_CNTRL2 ((volatile unsigned short *)USB_APHY_CNTRL2)

/* (PHY_TEST is for ADI usage only) */

#define                     pUSB_PHY_TEST ((volatile unsigned short *)USB_PHY_TEST)

#define                  pUSB_PLLOSC_CTRL ((volatile unsigned short *)USB_PLLOSC_CTRL)
#define                   pUSB_SRP_CLKDIV ((volatile unsigned short *)USB_SRP_CLKDIV)

/* USB Endpoint 0 Control Registers */

#define                pUSB_EP_NI0_TXMAXP ((volatile unsigned short *)USB_EP_NI0_TXMAXP)
#define                 pUSB_EP_NI0_TXCSR ((volatile unsigned short *)USB_EP_NI0_TXCSR)
#define                pUSB_EP_NI0_RXMAXP ((volatile unsigned short *)USB_EP_NI0_RXMAXP)
#define                 pUSB_EP_NI0_RXCSR ((volatile unsigned short *)USB_EP_NI0_RXCSR)
#define               pUSB_EP_NI0_RXCOUNT ((volatile unsigned short *)USB_EP_NI0_RXCOUNT)
#define                pUSB_EP_NI0_TXTYPE ((volatile unsigned short *)USB_EP_NI0_TXTYPE)
#define            pUSB_EP_NI0_TXINTERVAL ((volatile unsigned short *)USB_EP_NI0_TXINTERVAL)
#define                pUSB_EP_NI0_RXTYPE ((volatile unsigned short *)USB_EP_NI0_RXTYPE)
#define            pUSB_EP_NI0_RXINTERVAL ((volatile unsigned short *)USB_EP_NI0_RXINTERVAL)
#define               pUSB_EP_NI0_TXCOUNT ((volatile unsigned short *)USB_EP_NI0_TXCOUNT)

/* USB Endpoint 1 Control Registers */

#define                pUSB_EP_NI1_TXMAXP ((volatile unsigned short *)USB_EP_NI1_TXMAXP)
#define                 pUSB_EP_NI1_TXCSR ((volatile unsigned short *)USB_EP_NI1_TXCSR)
#define                pUSB_EP_NI1_RXMAXP ((volatile unsigned short *)USB_EP_NI1_RXMAXP)
#define                 pUSB_EP_NI1_RXCSR ((volatile unsigned short *)USB_EP_NI1_RXCSR)
#define               pUSB_EP_NI1_RXCOUNT ((volatile unsigned short *)USB_EP_NI1_RXCOUNT)
#define                pUSB_EP_NI1_TXTYPE ((volatile unsigned short *)USB_EP_NI1_TXTYPE)
#define            pUSB_EP_NI1_TXINTERVAL ((volatile unsigned short *)USB_EP_NI1_TXINTERVAL)
#define                pUSB_EP_NI1_RXTYPE ((volatile unsigned short *)USB_EP_NI1_RXTYPE)
#define            pUSB_EP_NI1_RXINTERVAL ((volatile unsigned short *)USB_EP_NI1_RXINTERVAL)
#define               pUSB_EP_NI1_TXCOUNT ((volatile unsigned short *)USB_EP_NI1_TXCOUNT)

/* USB Endpoint 2 Control Registers */

#define                pUSB_EP_NI2_TXMAXP ((volatile unsigned short *)USB_EP_NI2_TXMAXP)
#define                 pUSB_EP_NI2_TXCSR ((volatile unsigned short *)USB_EP_NI2_TXCSR)
#define                pUSB_EP_NI2_RXMAXP ((volatile unsigned short *)USB_EP_NI2_RXMAXP)
#define                 pUSB_EP_NI2_RXCSR ((volatile unsigned short *)USB_EP_NI2_RXCSR)
#define               pUSB_EP_NI2_RXCOUNT ((volatile unsigned short *)USB_EP_NI2_RXCOUNT)
#define                pUSB_EP_NI2_TXTYPE ((volatile unsigned short *)USB_EP_NI2_TXTYPE)
#define            pUSB_EP_NI2_TXINTERVAL ((volatile unsigned short *)USB_EP_NI2_TXINTERVAL)
#define                pUSB_EP_NI2_RXTYPE ((volatile unsigned short *)USB_EP_NI2_RXTYPE)
#define            pUSB_EP_NI2_RXINTERVAL ((volatile unsigned short *)USB_EP_NI2_RXINTERVAL)
#define               pUSB_EP_NI2_TXCOUNT ((volatile unsigned short *)USB_EP_NI2_TXCOUNT)

/* USB Endpoint 3 Control Registers */

#define                pUSB_EP_NI3_TXMAXP ((volatile unsigned short *)USB_EP_NI3_TXMAXP)
#define                 pUSB_EP_NI3_TXCSR ((volatile unsigned short *)USB_EP_NI3_TXCSR)
#define                pUSB_EP_NI3_RXMAXP ((volatile unsigned short *)USB_EP_NI3_RXMAXP)
#define                 pUSB_EP_NI3_RXCSR ((volatile unsigned short *)USB_EP_NI3_RXCSR)
#define               pUSB_EP_NI3_RXCOUNT ((volatile unsigned short *)USB_EP_NI3_RXCOUNT)
#define                pUSB_EP_NI3_TXTYPE ((volatile unsigned short *)USB_EP_NI3_TXTYPE)
#define            pUSB_EP_NI3_TXINTERVAL ((volatile unsigned short *)USB_EP_NI3_TXINTERVAL)
#define                pUSB_EP_NI3_RXTYPE ((volatile unsigned short *)USB_EP_NI3_RXTYPE)
#define            pUSB_EP_NI3_RXINTERVAL ((volatile unsigned short *)USB_EP_NI3_RXINTERVAL)
#define               pUSB_EP_NI3_TXCOUNT ((volatile unsigned short *)USB_EP_NI3_TXCOUNT)

/* USB Endpoint 4 Control Registers */

#define                pUSB_EP_NI4_TXMAXP ((volatile unsigned short *)USB_EP_NI4_TXMAXP)
#define                 pUSB_EP_NI4_TXCSR ((volatile unsigned short *)USB_EP_NI4_TXCSR)
#define                pUSB_EP_NI4_RXMAXP ((volatile unsigned short *)USB_EP_NI4_RXMAXP)
#define                 pUSB_EP_NI4_RXCSR ((volatile unsigned short *)USB_EP_NI4_RXCSR)
#define               pUSB_EP_NI4_RXCOUNT ((volatile unsigned short *)USB_EP_NI4_RXCOUNT)
#define                pUSB_EP_NI4_TXTYPE ((volatile unsigned short *)USB_EP_NI4_TXTYPE)
#define            pUSB_EP_NI4_TXINTERVAL ((volatile unsigned short *)USB_EP_NI4_TXINTERVAL)
#define                pUSB_EP_NI4_RXTYPE ((volatile unsigned short *)USB_EP_NI4_RXTYPE)
#define            pUSB_EP_NI4_RXINTERVAL ((volatile unsigned short *)USB_EP_NI4_RXINTERVAL)
#define               pUSB_EP_NI4_TXCOUNT ((volatile unsigned short *)USB_EP_NI4_TXCOUNT)

/* USB Endpoint 5 Control Registers */

#define                pUSB_EP_NI5_TXMAXP ((volatile unsigned short *)USB_EP_NI5_TXMAXP)
#define                 pUSB_EP_NI5_TXCSR ((volatile unsigned short *)USB_EP_NI5_TXCSR)
#define                pUSB_EP_NI5_RXMAXP ((volatile unsigned short *)USB_EP_NI5_RXMAXP)
#define                 pUSB_EP_NI5_RXCSR ((volatile unsigned short *)USB_EP_NI5_RXCSR)
#define               pUSB_EP_NI5_RXCOUNT ((volatile unsigned short *)USB_EP_NI5_RXCOUNT)
#define                pUSB_EP_NI5_TXTYPE ((volatile unsigned short *)USB_EP_NI5_TXTYPE)
#define            pUSB_EP_NI5_TXINTERVAL ((volatile unsigned short *)USB_EP_NI5_TXINTERVAL)
#define                pUSB_EP_NI5_RXTYPE ((volatile unsigned short *)USB_EP_NI5_RXTYPE)
#define            pUSB_EP_NI5_RXINTERVAL ((volatile unsigned short *)USB_EP_NI5_RXINTERVAL)
#define               pUSB_EP_NI5_TXCOUNT ((volatile unsigned short *)USB_EP_NI5_TXCOUNT)

/* USB Endpoint 6 Control Registers */

#define                pUSB_EP_NI6_TXMAXP ((volatile unsigned short *)USB_EP_NI6_TXMAXP)
#define                 pUSB_EP_NI6_TXCSR ((volatile unsigned short *)USB_EP_NI6_TXCSR)
#define                pUSB_EP_NI6_RXMAXP ((volatile unsigned short *)USB_EP_NI6_RXMAXP)
#define                 pUSB_EP_NI6_RXCSR ((volatile unsigned short *)USB_EP_NI6_RXCSR)
#define               pUSB_EP_NI6_RXCOUNT ((volatile unsigned short *)USB_EP_NI6_RXCOUNT)
#define                pUSB_EP_NI6_TXTYPE ((volatile unsigned short *)USB_EP_NI6_TXTYPE)
#define            pUSB_EP_NI6_TXINTERVAL ((volatile unsigned short *)USB_EP_NI6_TXINTERVAL)
#define                pUSB_EP_NI6_RXTYPE ((volatile unsigned short *)USB_EP_NI6_RXTYPE)
#define            pUSB_EP_NI6_RXINTERVAL ((volatile unsigned short *)USB_EP_NI6_RXINTERVAL)
#define               pUSB_EP_NI6_TXCOUNT ((volatile unsigned short *)USB_EP_NI6_TXCOUNT)

/* USB Endpoint 7 Control Registers */

#define                pUSB_EP_NI7_TXMAXP ((volatile unsigned short *)USB_EP_NI7_TXMAXP)
#define                 pUSB_EP_NI7_TXCSR ((volatile unsigned short *)USB_EP_NI7_TXCSR)
#define                pUSB_EP_NI7_RXMAXP ((volatile unsigned short *)USB_EP_NI7_RXMAXP)
#define                 pUSB_EP_NI7_RXCSR ((volatile unsigned short *)USB_EP_NI7_RXCSR)
#define               pUSB_EP_NI7_RXCOUNT ((volatile unsigned short *)USB_EP_NI7_RXCOUNT)
#define                pUSB_EP_NI7_TXTYPE ((volatile unsigned short *)USB_EP_NI7_TXTYPE)
#define            pUSB_EP_NI7_TXINTERVAL ((volatile unsigned short *)USB_EP_NI7_TXINTERVAL)
#define                pUSB_EP_NI7_RXTYPE ((volatile unsigned short *)USB_EP_NI7_RXTYPE)
#define            pUSB_EP_NI7_RXINTERVAL ((volatile unsigned short *)USB_EP_NI7_RXINTERVAL)
#define               pUSB_EP_NI7_TXCOUNT ((volatile unsigned short *)USB_EP_NI7_TXCOUNT)

#define                pUSB_DMA_INTERRUPT ((volatile unsigned short *)USB_DMA_INTERRUPT)

/* USB Channel 0 Config Registers */

#define                  pUSB_DMA0CONTROL ((volatile unsigned short *)USB_DMA0CONTROL)
#define                  pUSB_DMA0ADDRLOW ((volatile unsigned short *)USB_DMA0ADDRLOW)
#define                 pUSB_DMA0ADDRHIGH ((volatile unsigned short *)USB_DMA0ADDRHIGH)
#define                 pUSB_DMA0COUNTLOW ((volatile unsigned short *)USB_DMA0COUNTLOW)
#define                pUSB_DMA0COUNTHIGH ((volatile unsigned short *)USB_DMA0COUNTHIGH)

/* USB Channel 1 Config Registers */

#define                  pUSB_DMA1CONTROL ((volatile unsigned short *)USB_DMA1CONTROL)
#define                  pUSB_DMA1ADDRLOW ((volatile unsigned short *)USB_DMA1ADDRLOW)
#define                 pUSB_DMA1ADDRHIGH ((volatile unsigned short *)USB_DMA1ADDRHIGH)
#define                 pUSB_DMA1COUNTLOW ((volatile unsigned short *)USB_DMA1COUNTLOW)
#define                pUSB_DMA1COUNTHIGH ((volatile unsigned short *)USB_DMA1COUNTHIGH)

/* USB Channel 2 Config Registers */

#define                  pUSB_DMA2CONTROL ((volatile unsigned short *)USB_DMA2CONTROL)
#define                  pUSB_DMA2ADDRLOW ((volatile unsigned short *)USB_DMA2ADDRLOW)
#define                 pUSB_DMA2ADDRHIGH ((volatile unsigned short *)USB_DMA2ADDRHIGH)
#define                 pUSB_DMA2COUNTLOW ((volatile unsigned short *)USB_DMA2COUNTLOW)
#define                pUSB_DMA2COUNTHIGH ((volatile unsigned short *)USB_DMA2COUNTHIGH)

/* USB Channel 3 Config Registers */

#define                  pUSB_DMA3CONTROL ((volatile unsigned short *)USB_DMA3CONTROL)
#define                  pUSB_DMA3ADDRLOW ((volatile unsigned short *)USB_DMA3ADDRLOW)
#define                 pUSB_DMA3ADDRHIGH ((volatile unsigned short *)USB_DMA3ADDRHIGH)
#define                 pUSB_DMA3COUNTLOW ((volatile unsigned short *)USB_DMA3COUNTLOW)
#define                pUSB_DMA3COUNTHIGH ((volatile unsigned short *)USB_DMA3COUNTHIGH)

/* USB Channel 4 Config Registers */

#define                  pUSB_DMA4CONTROL ((volatile unsigned short *)USB_DMA4CONTROL)
#define                  pUSB_DMA4ADDRLOW ((volatile unsigned short *)USB_DMA4ADDRLOW)
#define                 pUSB_DMA4ADDRHIGH ((volatile unsigned short *)USB_DMA4ADDRHIGH)
#define                 pUSB_DMA4COUNTLOW ((volatile unsigned short *)USB_DMA4COUNTLOW)
#define                pUSB_DMA4COUNTHIGH ((volatile unsigned short *)USB_DMA4COUNTHIGH)

/* USB Channel 5 Config Registers */

#define                  pUSB_DMA5CONTROL ((volatile unsigned short *)USB_DMA5CONTROL)
#define                  pUSB_DMA5ADDRLOW ((volatile unsigned short *)USB_DMA5ADDRLOW)
#define                 pUSB_DMA5ADDRHIGH ((volatile unsigned short *)USB_DMA5ADDRHIGH)
#define                 pUSB_DMA5COUNTLOW ((volatile unsigned short *)USB_DMA5COUNTLOW)
#define                pUSB_DMA5COUNTHIGH ((volatile unsigned short *)USB_DMA5COUNTHIGH)

/* USB Channel 6 Config Registers */

#define                  pUSB_DMA6CONTROL ((volatile unsigned short *)USB_DMA6CONTROL)
#define                  pUSB_DMA6ADDRLOW ((volatile unsigned short *)USB_DMA6ADDRLOW)
#define                 pUSB_DMA6ADDRHIGH ((volatile unsigned short *)USB_DMA6ADDRHIGH)
#define                 pUSB_DMA6COUNTLOW ((volatile unsigned short *)USB_DMA6COUNTLOW)
#define                pUSB_DMA6COUNTHIGH ((volatile unsigned short *)USB_DMA6COUNTHIGH)

/* USB Channel 7 Config Registers */

#define                  pUSB_DMA7CONTROL ((volatile unsigned short *)USB_DMA7CONTROL)
#define                  pUSB_DMA7ADDRLOW ((volatile unsigned short *)USB_DMA7ADDRLOW)
#define                 pUSB_DMA7ADDRHIGH ((volatile unsigned short *)USB_DMA7ADDRHIGH)
#define                 pUSB_DMA7COUNTLOW ((volatile unsigned short *)USB_DMA7COUNTLOW)
#define                pUSB_DMA7COUNTHIGH ((volatile unsigned short *)USB_DMA7COUNTHIGH)

#endif /* _CDEF_BF525_H */
