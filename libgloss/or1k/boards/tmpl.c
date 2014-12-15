/* tmpl.c -- Template for new boards.
 *
 * Copyright (c) 2014 Authors
 *
 * Contributor Stefan Wallentowitz <stefan.wallentowitz@saunalahti.fi>
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

// TODO: set memory base here
unsigned long __attribute__((weak)) _or1k_board_mem_base = 0x0;

// TODO: set memory size here
unsigned long __attribute__((weak)) _or1k_board_mem_size = 0x0;

// TODO: set board clock frequency here
unsigned long __attribute__((weak)) _or1k_board_clk_freq = 0x0;

// TODO: UART configuration
unsigned long __attribute__((weak)) _or1k_board_uart_base = 0x0;
unsigned long __attribute__((weak)) _or1k_board_uart_baud = 0x0;
unsigned long __attribute__((weak)) _or1k_board_uart_IRQ = 0x0;

// TODO: Board exit function, default: loop
void __attribute__((weak)) _or1k_board_exit(void) {
	while (1) {}
}

// TODO: Board initialization
void __attribute__((weak)) _or1k_board_init(void) {
	return;
}
