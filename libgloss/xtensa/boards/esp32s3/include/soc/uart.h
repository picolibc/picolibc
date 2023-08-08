/*
 * Copyright (C) 2023 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef SOC_UART_H
#define SOC_UART_H

#include <soc/cpu.h>
#include <register_access.h>

#define UART0_BAUDRATE      115200

#define UART0_TX_ADDR       0x60000000
#define UART0_BASE          0x60000000
#define UART0_CLKDIV_REG    (UART0_BASE + 0x14)
#define UART0_STATUS        (UART0_BASE + 0x1c)
#define UART0_CLKDIV_VAL    (CPU_FREQUENCY_HZ / UART0_BAUDRATE )
#define UART0_TXFIFO_CNT    (((READ_REGISTER(UART0_STATUS)) >> 16) & 0x3ff)


void board_uart_write_char(char c);

#endif // SOC_UART_H
