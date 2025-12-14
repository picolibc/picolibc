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

#include "aarch64-semihost.h"

typedef volatile uint8_t vuint8_t;
typedef volatile uint32_t vuint32_t;

struct pl011 {
    vuint32_t   dr;
    vuint32_t   rsr;
    vuint32_t   fr;
    vuint32_t   iplr;
    vuint32_t   ibrd;
    vuint32_t   fbrd;
    vuint32_t   lcrh;
    vuint32_t   cr;
    vuint32_t   ifls;
    vuint32_t   imsc;
    vuint32_t   ris;
    vuint32_t   mis;
    vuint32_t   icr;
    vuint32_t   dmacr;
};

#ifdef __GNUCLIKE_PRAGMA_DIAGNOSTIC
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
/* 'bsize' is used directly with malloc/realloc which confuses -fanalyzer */
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif

/* pl011 serial port */
#define uart    (*((struct pl011 *) 0x09000000))

/* Flag Register, UARTFR */
#define PL011_FLAG_RI   0x100
#define PL011_FLAG_TXFE 0x80
#define PL011_FLAG_RXFF 0x40
#define PL011_FLAG_TXFF 0x20
#define PL011_FLAG_RXFE 0x10
#define PL011_FLAG_DCD  0x04
#define PL011_FLAG_DSR  0x02
#define PL011_FLAG_CTS  0x01

/* Data Register, UARTDR */
#define DR_BE   (1 << 10)

/* Interrupt status bits in UARTRIS, UARTMIS, UARTIMSC */
#define INT_OE (1 << 10)
#define INT_BE (1 << 9)
#define INT_PE (1 << 8)
#define INT_FE (1 << 7)
#define INT_RT (1 << 6)
#define INT_TX (1 << 5)
#define INT_RX (1 << 4)
#define INT_DSR (1 << 3)
#define INT_DCD (1 << 2)
#define INT_CTS (1 << 1)
#define INT_RI (1 << 0)
#define INT_E (INT_OE | INT_BE | INT_PE | INT_FE)
#define INT_MS (INT_RI | INT_DSR | INT_DCD | INT_CTS)

/* Line Control Register, UARTLCR_H */
#define LCR_FEN     (1 << 4)
#define LCR_BRK     (1 << 0)

/* Control Register, UARTCR */
#define CR_OUT2     (1 << 13)
#define CR_OUT1     (1 << 12)
#define CR_RTS      (1 << 11)
#define CR_DTR      (1 << 10)
#define CR_RXE      (1 << 9)
#define CR_TXE      (1 << 8)
#define CR_LBE      (1 << 7)
#define CR_UARTEN   (1 << 0)

/* Integer Baud Rate Divider, UARTIBRD */
#define IBRD_MASK 0xffff

/* Fractional Baud Rate Divider, UARTFBRD */
#define FBRD_MASK 0x3f

int
aarch64_putc(char c, FILE *file)
{
	(void) file;
        uart.cr |= CR_TXE|CR_UARTEN;
        while ((uart.fr & PL011_FLAG_TXFF) != 0)
               ;
        uart.dr = (uint8_t) c;
	return (unsigned char) c;
}

static int
aarch64_getc(FILE *file)
{
	(void) file;
        uart.cr |= CR_RXE|CR_UARTEN;
        while ((uart.cr & PL011_FLAG_RXFE) == 0)
            ;
        return (unsigned char) uart.dr;
}

static FILE __stdio = FDEV_SETUP_STREAM(aarch64_putc, aarch64_getc, NULL, _FDEV_SETUP_RW);

#ifdef __strong_reference
#define STDIO_ALIAS(x) __strong_reference(stdin, x);
#else
#define STDIO_ALIAS(x) FILE *const x = &__stdio;
#endif

FILE *const stdin = &__stdio;
STDIO_ALIAS(stdout);
STDIO_ALIAS(stderr);
