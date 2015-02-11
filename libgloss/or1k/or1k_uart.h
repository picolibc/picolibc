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

/* This is the generic board support for the OpenCores UART device, internal
 * header. */

#include <stdint.h>

#include "board.h"

/**
 * Registered callback function
 */
extern void (*_or1k_uart_read_cb)(char c);

/**
 * The UART interrupt handler
 */
void _or1k_uart_interrupt_handler(uint32_t data);

/**
 * Initialize UART
 */
int _or1k_uart_init(void);

/**
 * Write character to UART
 */
void _or1k_uart_write(char c);
