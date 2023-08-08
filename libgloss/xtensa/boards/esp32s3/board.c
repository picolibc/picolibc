/*
 * Copyright (C) 2023 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdint.h>
#include <soc/uart.h>

void board_init(void)
{
    WRITE_REGISTER(UART0_CLKDIV_REG, UART0_CLKDIV_VAL);
}

void board_uart_write_char(char c)
{
    /* wait until txfifo_cnt == 0 */
    while (UART0_TXFIFO_CNT) {
        ;
    }
    if (c == '\n') {
        WRITE_REGISTER(UART0_TX_ADDR, '\r');
    }
    WRITE_REGISTER(UART0_TX_ADDR, c);
}
