/* board.h -- board variable definitions for OpenRISC 1000.
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

#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdint.h>

extern void* _or1k_board_mem_base;
extern uint32_t _or1k_board_mem_size;
extern uint32_t _or1k_board_clk_freq;

extern uint32_t _or1k_board_uart_base;
extern uint32_t  _or1k_board_uart_baud;
extern uint32_t _or1k_board_uart_IRQ;

extern void _or1k_board_exit(void);
extern void _or1k_board_init_early(void);
extern void _or1k_board_init(void);

#endif // __BOARD_H__
