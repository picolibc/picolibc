/* or1k_uart.c -- UART implementation for OpenRISC 1000.
 *
 *Copyright (c) 2014 Authors
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

#include "include/or1k-support.h"
#include "or1k_uart.h"

#include <stdint.h>

void (*_or1k_uart_read_cb)(char c);

void _or1k_uart_interrupt_handler(uint32_t data)
{
	_or1k_uart_read_cb(REG8(RB));
}

int _or1k_uart_init(void)
{
	uint16_t divisor;

	// Is uart present?
	if (!_or1k_board_uart_base) {
			return -1;
	}

	// Reset the callback function
	_or1k_uart_read_cb = 0;

	// Calculate and set divisor
	divisor = _or1k_board_clk_freq / (_or1k_board_uart_baud * 16);
	REG8(LCR) = LCR_DLA;
	REG8(DLB1) = divisor & 0xff;
	REG8(DLB2) = divisor >> 8;

	// Set line control register:
	//  - 8 bits per character
	//  - 1 stop bit
	//  - No parity
	//  - Break disabled
	//  - Disallow access to divisor latch
	REG8(LCR) = LCR_BPC_8;

	// Reset FIFOs and set trigger level to 14 bytes
	REG8(FCR) = FCR_CLRRECV | FCR_CLRTMIT | FCR_TRIG_14;

	// Disable all interrupts
	REG8(IER) = 0;

	return 0;
}

void _or1k_uart_write(char c)
{
	while (!REG8(LSR) & LSR_TFE) {}

	REG8(THR) = c;
}

void or1k_uart_set_read_cb(void (*cb)(char c))
{
	_or1k_uart_read_cb = cb;

	// Enable interrupt
	REG8(IER) = 1 << IER_RDAI;

	or1k_interrupt_handler_add(_or1k_board_uart_IRQ,
			_or1k_uart_interrupt_handler, 0);
	or1k_interrupt_enable(_or1k_board_uart_IRQ);
}
