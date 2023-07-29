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
#include "mips-semihost.h"

#define UART0   0x3f8

typedef volatile uint8_t vuint8_t;

#define REG_SPACING     8

#if REG_SPACING
#define __cat(a,b)      a ## b
#define reg_pad(n)      uint8_t __cat(pad, n)[REG_SPACING-1];
#else
#define reg_pad(n)
#endif


struct uart_8250 {
    vuint8_t     data;
    reg_pad(0)
    vuint8_t     ier;
    reg_pad(1)
    vuint8_t     iir;
    reg_pad(2)
    vuint8_t     lcr;
    reg_pad(3)

    vuint8_t     mcr;
    reg_pad(4)
    vuint8_t     lsr;
    reg_pad(5)
    vuint8_t     msr;
    reg_pad(6)
    vuint8_t     scr;
    reg_pad(7)
};

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
/* 'bsize' is used directly with malloc/realloc which confuses -fanalyzer */
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif

/* malta serial port */
#define uart    (*((struct uart_8250 *) 0xbf000900))

#define UART_LSR_FIFOE		0x80 /* Fifo error */
#define UART_LSR_TEMT		0x40 /* Transmitter empty */
#define UART_LSR_THRE		0x20 /* Transmit-hold-register empty */
#define UART_LSR_BI		0x10 /* Break interrupt indicator */
#define UART_LSR_FE		0x08 /* Frame error indicator */
#define UART_LSR_PE		0x04 /* Parity error indicator */
#define UART_LSR_OE		0x02 /* Overrun error indicator */
#define UART_LSR_DR		0x01 /* Receiver data ready */

int
mips_putc(char c, FILE *file)
{
	(void) file;
        while ((uart.lsr & (UART_LSR_TEMT|UART_LSR_THRE)) != (UART_LSR_TEMT|UART_LSR_THRE))
               ;
        uart.data = (uint8_t) c;
	return (unsigned char) c;
}

#ifdef TINY_STDIO

static int
mips_getc(FILE *file)
{
	(void) file;
	return EOF;
}

static FILE __stdio = FDEV_SETUP_STREAM(mips_putc, mips_getc, NULL, _FDEV_SETUP_RW);

#ifdef __strong_reference
#define STDIO_ALIAS(x) __strong_reference(stdin, x);
#else
#define STDIO_ALIAS(x) FILE *const x = &__stdio;
#endif

FILE *const stdin = &__stdio;
STDIO_ALIAS(stdout);
STDIO_ALIAS(stderr);

#endif
