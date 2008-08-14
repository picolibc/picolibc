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
** cdefBF542.h
**
** Copyright (C) 2008 Analog Devices, Inc.
**
************************************************************************************
**
** This include file contains a list of macro "defines" to enable the programmer
** to use symbolic names for the ADSP-BF542 peripherals.
**
************************************************************************************
** System MMR Register Map
************************************************************************************/

#ifndef _CDEF_BF542_H
#define _CDEF_BF542_H

/* include all Core registers and bit definitions */
#include <defBF542.h>

/* include core specific register pointer definitions */
#include <cdef_LPBlackfin.h>

/* SYSTEM & MMR ADDRESS DEFINITIONS FOR ADSP-BF542 */

/* include cdefBF54x_base.h for the set of #defines that are common to all ADSP-BF54x processors */
#include <cdefBF54x_base.h>

/* The following are the #defines needed by ADSP-BF542 that are not in the common header */

/* ATAPI Registers */

#define                    pATAPI_CONTROL ((volatile unsigned short *)ATAPI_CONTROL)
#define                     pATAPI_STATUS ((volatile unsigned short *)ATAPI_STATUS)
#define                   pATAPI_DEV_ADDR ((volatile unsigned short *)ATAPI_DEV_ADDR)
#define                  pATAPI_DEV_TXBUF ((volatile unsigned short *)ATAPI_DEV_TXBUF)
#define                  pATAPI_DEV_RXBUF ((volatile unsigned short *)ATAPI_DEV_RXBUF)
#define                   pATAPI_INT_MASK ((volatile unsigned short *)ATAPI_INT_MASK)
#define                 pATAPI_INT_STATUS ((volatile unsigned short *)ATAPI_INT_STATUS)
#define                   pATAPI_XFER_LEN ((volatile unsigned short *)ATAPI_XFER_LEN)
#define                pATAPI_LINE_STATUS ((volatile unsigned short *)ATAPI_LINE_STATUS)
#define                   pATAPI_SM_STATE ((volatile unsigned short *)ATAPI_SM_STATE)
#define                  pATAPI_TERMINATE ((volatile unsigned short *)ATAPI_TERMINATE)
#define                 pATAPI_PIO_TFRCNT ((volatile unsigned short *)ATAPI_PIO_TFRCNT)
#define                 pATAPI_DMA_TFRCNT ((volatile unsigned short *)ATAPI_DMA_TFRCNT)
#define               pATAPI_UMAIN_TFRCNT ((volatile unsigned short *)ATAPI_UMAIN_TFRCNT)
#define             pATAPI_UDMAOUT_TFRCNT ((volatile unsigned short *)ATAPI_UDMAOUT_TFRCNT)
#define                  pATAPI_REG_TIM_0 ((volatile unsigned short *)ATAPI_REG_TIM_0)
#define                  pATAPI_PIO_TIM_0 ((volatile unsigned short *)ATAPI_PIO_TIM_0)
#define                  pATAPI_PIO_TIM_1 ((volatile unsigned short *)ATAPI_PIO_TIM_1)
#define                pATAPI_MULTI_TIM_0 ((volatile unsigned short *)ATAPI_MULTI_TIM_0)
#define                pATAPI_MULTI_TIM_1 ((volatile unsigned short *)ATAPI_MULTI_TIM_1)
#define                pATAPI_MULTI_TIM_2 ((volatile unsigned short *)ATAPI_MULTI_TIM_2)
#define                pATAPI_ULTRA_TIM_0 ((volatile unsigned short *)ATAPI_ULTRA_TIM_0)
#define                pATAPI_ULTRA_TIM_1 ((volatile unsigned short *)ATAPI_ULTRA_TIM_1)
#define                pATAPI_ULTRA_TIM_2 ((volatile unsigned short *)ATAPI_ULTRA_TIM_2)
#define                pATAPI_ULTRA_TIM_3 ((volatile unsigned short *)ATAPI_ULTRA_TIM_3)

/* SDH Registers */

#define                      pSDH_PWR_CTL ((volatile unsigned short *)SDH_PWR_CTL)
#define                      pSDH_CLK_CTL ((volatile unsigned short *)SDH_CLK_CTL)
#define                     pSDH_ARGUMENT ((volatile unsigned long *)SDH_ARGUMENT)
#define                      pSDH_COMMAND ((volatile unsigned short *)SDH_COMMAND)
#define                     pSDH_RESP_CMD ((volatile unsigned short *)SDH_RESP_CMD)
#define                    pSDH_RESPONSE0 ((volatile unsigned long *)SDH_RESPONSE0)
#define                    pSDH_RESPONSE1 ((volatile unsigned long *)SDH_RESPONSE1)
#define                    pSDH_RESPONSE2 ((volatile unsigned long *)SDH_RESPONSE2)
#define                    pSDH_RESPONSE3 ((volatile unsigned long *)SDH_RESPONSE3)
#define                   pSDH_DATA_TIMER ((volatile unsigned long *)SDH_DATA_TIMER)
#define                    pSDH_DATA_LGTH ((volatile unsigned short *)SDH_DATA_LGTH)
#define                     pSDH_DATA_CTL ((volatile unsigned short *)SDH_DATA_CTL)
#define                     pSDH_DATA_CNT ((volatile unsigned short *)SDH_DATA_CNT)
#define                       pSDH_STATUS ((volatile unsigned long *)SDH_STATUS)
#define                   pSDH_STATUS_CLR ((volatile unsigned short *)SDH_STATUS_CLR)
#define                        pSDH_MASK0 ((volatile unsigned long *)SDH_MASK0)
#define                        pSDH_MASK1 ((volatile unsigned long *)SDH_MASK1)
#define                     pSDH_FIFO_CNT ((volatile unsigned short *)SDH_FIFO_CNT)
#define                         pSDH_FIFO ((volatile unsigned long *)SDH_FIFO)
#define                     pSDH_E_STATUS ((volatile unsigned short *)SDH_E_STATUS)
#define                       pSDH_E_MASK ((volatile unsigned short *)SDH_E_MASK)
#define                          pSDH_CFG ((volatile unsigned short *)SDH_CFG)
#define                   pSDH_RD_WAIT_EN ((volatile unsigned short *)SDH_RD_WAIT_EN)
#define                         pSDH_PID0 ((volatile unsigned short *)SDH_PID0)
#define                         pSDH_PID1 ((volatile unsigned short *)SDH_PID1)
#define                         pSDH_PID2 ((volatile unsigned short *)SDH_PID2)
#define                         pSDH_PID3 ((volatile unsigned short *)SDH_PID3)
#define                         pSDH_PID4 ((volatile unsigned short *)SDH_PID4)
#define                         pSDH_PID5 ((volatile unsigned short *)SDH_PID5)
#define                         pSDH_PID6 ((volatile unsigned short *)SDH_PID6)
#define                         pSDH_PID7 ((volatile unsigned short *)SDH_PID7)

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

/* USB Endpoint 1 Control Registers */

#define               pUSB_EP_NI0_TXCOUNT ((volatile unsigned short *)USB_EP_NI0_TXCOUNT)
#define                pUSB_EP_NI1_TXMAXP ((volatile unsigned short *)USB_EP_NI1_TXMAXP)
#define                 pUSB_EP_NI1_TXCSR ((volatile unsigned short *)USB_EP_NI1_TXCSR)
#define                pUSB_EP_NI1_RXMAXP ((volatile unsigned short *)USB_EP_NI1_RXMAXP)
#define                 pUSB_EP_NI1_RXCSR ((volatile unsigned short *)USB_EP_NI1_RXCSR)
#define               pUSB_EP_NI1_RXCOUNT ((volatile unsigned short *)USB_EP_NI1_RXCOUNT)
#define                pUSB_EP_NI1_TXTYPE ((volatile unsigned short *)USB_EP_NI1_TXTYPE)
#define            pUSB_EP_NI1_TXINTERVAL ((volatile unsigned short *)USB_EP_NI1_TXINTERVAL)
#define                pUSB_EP_NI1_RXTYPE ((volatile unsigned short *)USB_EP_NI1_RXTYPE)
#define            pUSB_EP_NI1_RXINTERVAL ((volatile unsigned short *)USB_EP_NI1_RXINTERVAL)

/* USB Endpoint 2 Control Registers */

#define               pUSB_EP_NI1_TXCOUNT ((volatile unsigned short *)USB_EP_NI1_TXCOUNT)
#define                pUSB_EP_NI2_TXMAXP ((volatile unsigned short *)USB_EP_NI2_TXMAXP)
#define                 pUSB_EP_NI2_TXCSR ((volatile unsigned short *)USB_EP_NI2_TXCSR)
#define                pUSB_EP_NI2_RXMAXP ((volatile unsigned short *)USB_EP_NI2_RXMAXP)
#define                 pUSB_EP_NI2_RXCSR ((volatile unsigned short *)USB_EP_NI2_RXCSR)
#define               pUSB_EP_NI2_RXCOUNT ((volatile unsigned short *)USB_EP_NI2_RXCOUNT)
#define                pUSB_EP_NI2_TXTYPE ((volatile unsigned short *)USB_EP_NI2_TXTYPE)
#define            pUSB_EP_NI2_TXINTERVAL ((volatile unsigned short *)USB_EP_NI2_TXINTERVAL)
#define                pUSB_EP_NI2_RXTYPE ((volatile unsigned short *)USB_EP_NI2_RXTYPE)
#define            pUSB_EP_NI2_RXINTERVAL ((volatile unsigned short *)USB_EP_NI2_RXINTERVAL)

/* USB Endpoint 3 Control Registers */

#define               pUSB_EP_NI2_TXCOUNT ((volatile unsigned short *)USB_EP_NI2_TXCOUNT)
#define                pUSB_EP_NI3_TXMAXP ((volatile unsigned short *)USB_EP_NI3_TXMAXP)
#define                 pUSB_EP_NI3_TXCSR ((volatile unsigned short *)USB_EP_NI3_TXCSR)
#define                pUSB_EP_NI3_RXMAXP ((volatile unsigned short *)USB_EP_NI3_RXMAXP)
#define                 pUSB_EP_NI3_RXCSR ((volatile unsigned short *)USB_EP_NI3_RXCSR)
#define               pUSB_EP_NI3_RXCOUNT ((volatile unsigned short *)USB_EP_NI3_RXCOUNT)
#define                pUSB_EP_NI3_TXTYPE ((volatile unsigned short *)USB_EP_NI3_TXTYPE)
#define            pUSB_EP_NI3_TXINTERVAL ((volatile unsigned short *)USB_EP_NI3_TXINTERVAL)
#define                pUSB_EP_NI3_RXTYPE ((volatile unsigned short *)USB_EP_NI3_RXTYPE)
#define            pUSB_EP_NI3_RXINTERVAL ((volatile unsigned short *)USB_EP_NI3_RXINTERVAL)

/* USB Endpoint 4 Control Registers */

#define               pUSB_EP_NI3_TXCOUNT ((volatile unsigned short *)USB_EP_NI3_TXCOUNT)
#define                pUSB_EP_NI4_TXMAXP ((volatile unsigned short *)USB_EP_NI4_TXMAXP)
#define                 pUSB_EP_NI4_TXCSR ((volatile unsigned short *)USB_EP_NI4_TXCSR)
#define                pUSB_EP_NI4_RXMAXP ((volatile unsigned short *)USB_EP_NI4_RXMAXP)
#define                 pUSB_EP_NI4_RXCSR ((volatile unsigned short *)USB_EP_NI4_RXCSR)
#define               pUSB_EP_NI4_RXCOUNT ((volatile unsigned short *)USB_EP_NI4_RXCOUNT)
#define                pUSB_EP_NI4_TXTYPE ((volatile unsigned short *)USB_EP_NI4_TXTYPE)
#define            pUSB_EP_NI4_TXINTERVAL ((volatile unsigned short *)USB_EP_NI4_TXINTERVAL)
#define                pUSB_EP_NI4_RXTYPE ((volatile unsigned short *)USB_EP_NI4_RXTYPE)
#define            pUSB_EP_NI4_RXINTERVAL ((volatile unsigned short *)USB_EP_NI4_RXINTERVAL)

/* USB Endpoint 5 Control Registers */

#define               pUSB_EP_NI4_TXCOUNT ((volatile unsigned short *)USB_EP_NI4_TXCOUNT)
#define                pUSB_EP_NI5_TXMAXP ((volatile unsigned short *)USB_EP_NI5_TXMAXP)
#define                 pUSB_EP_NI5_TXCSR ((volatile unsigned short *)USB_EP_NI5_TXCSR)
#define                pUSB_EP_NI5_RXMAXP ((volatile unsigned short *)USB_EP_NI5_RXMAXP)
#define                 pUSB_EP_NI5_RXCSR ((volatile unsigned short *)USB_EP_NI5_RXCSR)
#define               pUSB_EP_NI5_RXCOUNT ((volatile unsigned short *)USB_EP_NI5_RXCOUNT)
#define                pUSB_EP_NI5_TXTYPE ((volatile unsigned short *)USB_EP_NI5_TXTYPE)
#define            pUSB_EP_NI5_TXINTERVAL ((volatile unsigned short *)USB_EP_NI5_TXINTERVAL)
#define                pUSB_EP_NI5_RXTYPE ((volatile unsigned short *)USB_EP_NI5_RXTYPE)
#define            pUSB_EP_NI5_RXINTERVAL ((volatile unsigned short *)USB_EP_NI5_RXINTERVAL)

/* USB Endpoint 6 Control Registers */

#define               pUSB_EP_NI5_TXCOUNT ((volatile unsigned short *)USB_EP_NI5_TXCOUNT)
#define                pUSB_EP_NI6_TXMAXP ((volatile unsigned short *)USB_EP_NI6_TXMAXP)
#define                 pUSB_EP_NI6_TXCSR ((volatile unsigned short *)USB_EP_NI6_TXCSR)
#define                pUSB_EP_NI6_RXMAXP ((volatile unsigned short *)USB_EP_NI6_RXMAXP)
#define                 pUSB_EP_NI6_RXCSR ((volatile unsigned short *)USB_EP_NI6_RXCSR)
#define               pUSB_EP_NI6_RXCOUNT ((volatile unsigned short *)USB_EP_NI6_RXCOUNT)
#define                pUSB_EP_NI6_TXTYPE ((volatile unsigned short *)USB_EP_NI6_TXTYPE)
#define            pUSB_EP_NI6_TXINTERVAL ((volatile unsigned short *)USB_EP_NI6_TXINTERVAL)
#define                pUSB_EP_NI6_RXTYPE ((volatile unsigned short *)USB_EP_NI6_RXTYPE)
#define            pUSB_EP_NI6_RXINTERVAL ((volatile unsigned short *)USB_EP_NI6_RXINTERVAL)

/* USB Endpoint 7 Control Registers */

#define               pUSB_EP_NI6_TXCOUNT ((volatile unsigned short *)USB_EP_NI6_TXCOUNT)
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

/* Keypad Registers */

#define                         pKPAD_CTL ((volatile unsigned short *)KPAD_CTL)
#define                    pKPAD_PRESCALE ((volatile unsigned short *)KPAD_PRESCALE)
#define                        pKPAD_MSEL ((volatile unsigned short *)KPAD_MSEL)
#define                      pKPAD_ROWCOL ((volatile unsigned short *)KPAD_ROWCOL)
#define                        pKPAD_STAT ((volatile unsigned short *)KPAD_STAT)
#define                    pKPAD_SOFTEVAL ((volatile unsigned short *)KPAD_SOFTEVAL)

#endif /* _CDEF_BF542_H */
