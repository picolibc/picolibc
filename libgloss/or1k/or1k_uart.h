/* or1k_uart.h -- UART definitions for OpenRISC 1000.
 *
 * Copyright (c) 2014 Authors
 *
 * Contributor Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
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

/* This is the generic board support for the OpenCores UART device */

#include <stdint.h>

#include "board.h"

extern void (*_or1k_uart_read_cb)(char c);

void _or1k_uart_interrupt_handler(uint32_t data);

int _or1k_uart_init(void);
void _or1k_uart_write(char c);

#define RB  _or1k_board_uart_base + 0
#define THR _or1k_board_uart_base + 0
#define IER _or1k_board_uart_base + 1
#define IIR _or1k_board_uart_base + 2
#define FCR _or1k_board_uart_base + 2
#define LCR _or1k_board_uart_base + 3
#define MCR _or1k_board_uart_base + 4
#define LSR _or1k_board_uart_base + 5
#define MSR _or1k_board_uart_base + 6

#define DLB1 _or1k_board_uart_base + 0
#define DLB2 _or1k_board_uart_base + 1

#define IER_RDAI 0
#define IER_TEI  1
#define IER_RLSI 2
#define IER_MSI  3

#define IIR_RLS  0xC3
#define IIR_RDA  0xC2
#define IIR_TO   0xC6
#define IIR_THRE 0xC1
#define IIT_MS   0xC0

#define FCR_CLRRECV 0x1
#define FCR_CLRTMIT 0x2
#define FCR_TRIG_1  0x0
#define FCR_TRIG_4  0x40
#define FCR_TRIG_8  0x80
#define FCR_TRIG_14 0xC0

#define LCR_BPC_MASK 0x3
#define LCR_SB_MASK  0x4

#define LCR_BPC_5 0x0
#define LCR_BPC_6 0x1
#define LCR_BPC_7 0x2
#define LCR_BPC_8 0x3
#define LCR_SB_1  0x0
#define LCR_SB_2  0x4
#define LCR_PE    0x8
#define LCR_EPS   0x10
#define LCR_SP    0x20
#define LCR_BC    0x40
#define LCR_DLA   0x80

#define LSR_DR  0x0
#define LSR_OE  0x2
#define LSR_PE  0x4
#define LSR_FE  0x8
#define LSR_BI  0x10
#define LSR_TFE 0x20
#define LSR_TEI 0x40
