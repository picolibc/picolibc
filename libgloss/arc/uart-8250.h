/*
 * uart-8250.h -- polling driver for 32-bit 8250 UART.
 * Header defines _uart_8250_setup().
 *
 * Copyright (c) 2024 Synopsys Inc.
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

#ifndef _UART_8250_H
#define _UART_8250_H

#include <stdint.h>

int _uart_8250_setup (void *base, int aux_mapped, uint32_t clk, uint32_t baud);

#endif /* ! _UART_8250_H */
