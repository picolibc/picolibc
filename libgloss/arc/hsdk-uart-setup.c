/*
 * hsdk-setup.c -- provide _setup_low_level() to initialize UART.
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
 *
 */

#include "uart-8250.h"

/* Setup UART parameters.  */
int
_setup_low_level (void)
{
  void * const uart_base = (void *) 0xf0005000;
  const int uart_aux_mapped = 0;
  const uint32_t uart_clock = 33333333;
  const uint32_t uart_baud = 115200;

  _uart_8250_setup (uart_base, uart_aux_mapped, uart_clock, uart_baud);

  return 0;
}
