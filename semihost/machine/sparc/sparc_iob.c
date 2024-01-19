/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2023 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sparc-semihost.h"

typedef volatile uint8_t vuint8_t;
typedef volatile uint32_t vuint32_t;

struct apbuart {
    vuint32_t   data;
    vuint32_t   status;
    vuint32_t   control;
};

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
/* 'bsize' is used directly with malloc/realloc which confuses -fanalyzer */
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif

/* leon3 serial port */
#define uart    (*((struct apbuart *) 0x80000100))

/* UART status register fields */
#define UART_DATA_READY           (1 <<  0)
#define UART_TRANSMIT_SHIFT_EMPTY (1 <<  1)
#define UART_TRANSMIT_FIFO_EMPTY  (1 <<  2)
#define UART_BREAK_RECEIVED       (1 <<  3)
#define UART_OVERRUN              (1 <<  4)
#define UART_PARITY_ERROR         (1 <<  5)
#define UART_FRAMING_ERROR        (1 <<  6)
#define UART_TRANSMIT_FIFO_HALF   (1 <<  7)
#define UART_RECEIVE_FIFO_HALF    (1 <<  8)
#define UART_TRANSMIT_FIFO_FULL   (1 <<  9)
#define UART_RECEIVE_FIFO_FULL    (1 << 10)

/* UART control register fields */
#define UART_RECEIVE_ENABLE          (1 <<  0)
#define UART_TRANSMIT_ENABLE         (1 <<  1)
#define UART_RECEIVE_INTERRUPT       (1 <<  2)
#define UART_TRANSMIT_INTERRUPT      (1 <<  3)
#define UART_PARITY_SELECT           (1 <<  4)
#define UART_PARITY_ENABLE           (1 <<  5)
#define UART_FLOW_CONTROL            (1 <<  6)
#define UART_LOOPBACK                (1 <<  7)
#define UART_EXTERNAL_CLOCK          (1 <<  8)
#define UART_RECEIVE_FIFO_INTERRUPT  (1 <<  9)
#define UART_TRANSMIT_FIFO_INTERRUPT (1 << 10)
#define UART_FIFO_DEBUG_MODE         (1 << 11)
#define UART_OUTPUT_ENABLE           (1 << 12)
#define UART_FIFO_AVAILABLE          (1 << 31)

int
sparc_putc(char c, FILE *file)
{
	(void) file;
        uart.control |= UART_TRANSMIT_ENABLE;
        while ((uart.status & UART_TRANSMIT_FIFO_FULL) != 0)
               ;
        uart.data = (uint8_t) c;
	return (unsigned char) c;
}

#ifdef TINY_STDIO

static int
sparc_getc(FILE *file)
{
	(void) file;
        uart.control |= UART_RECEIVE_ENABLE;
        while ((uart.status & UART_DATA_READY) == 0)
            ;
        return (unsigned char) uart.data;
}

static FILE __stdio = FDEV_SETUP_STREAM(sparc_putc, sparc_getc, NULL, _FDEV_SETUP_RW);

#ifdef __strong_reference
#define STDIO_ALIAS(x) __strong_reference(stdin, x);
#else
#define STDIO_ALIAS(x) FILE *const x = &__stdio;
#endif

FILE *const stdin = &__stdio;
STDIO_ALIAS(stdout);
STDIO_ALIAS(stderr);

#endif
