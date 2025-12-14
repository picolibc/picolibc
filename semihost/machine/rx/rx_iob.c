/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
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

#include "rx_semihost.h"

typedef volatile uint8_t vuint8_t;
typedef volatile uint32_t vuint32_t;

struct sci {
    vuint8_t    smr;
    vuint8_t    brr;
    vuint8_t    scr;
    vuint8_t    tdr;

    vuint8_t    ssr;
    vuint8_t    rdr;
    vuint8_t    scmr;
    vuint8_t    semr;
};

/* SCI SCR fields */
#define SCI_SCR_CKE_0   (1 <<  0)
#define SCI_SCR_CKE_1   (2 <<  0)
#define SCI_SCR_TEIE    (1 <<  2)
#define SCI_SCR_MPIE    (1 <<  3)
#define SCI_SCR_RE      (1 <<  4)
#define SCI_SCR_TE      (1 <<  5)
#define SCI_SCR_RIE     (1 <<  6)
#define SCI_SCR_TIE     (1 <<  7)

/* SCI SSR fields */
#define SCI_SSR_MPBT    (1 << 0)
#define SCI_SSR_MPB     (1 << 1)
#define SCI_SSR_TEND    (1 << 2)
#define SCI_SSR_PER     (1 << 3)
#define SCI_SSR_FER     (1 << 4)
#define SCI_SSR_ORER    (1 << 5)
#define SCI_SSR_RDRF    (1 << 6)
#define SCI_SSR_TDRE    (1 << 7)

#ifdef __GNUCLIKE_PRAGMA_DIAGNOSTIC
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
/* 'bsize' is used directly with malloc/realloc which confuses -fanalyzer */
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif

/* rx62n serial port */
#define SCI0    (*((struct sci *) 0x00088240))

int
rx_putc(char c, FILE *file)
{
	(void) file;
        SCI0.scr |= SCI_SCR_TE;
        while ((SCI0.ssr & SCI_SSR_TEND) == 0)
               ;
        SCI0.tdr = (uint8_t) c;
	return (unsigned char) c;
}

static int
rx_getc(FILE *file)
{
	(void) file;
        SCI0.scr |= SCI_SCR_RE;
        while ((SCI0.ssr & SCI_SSR_RDRF) == 0)
            ;
        return (unsigned char) SCI0.rdr;
}

static FILE __stdio = FDEV_SETUP_STREAM(rx_putc, rx_getc, NULL, _FDEV_SETUP_RW);

#ifdef __strong_reference
#define STDIO_ALIAS(x) __strong_reference(stdin, x);
#else
#define STDIO_ALIAS(x) FILE *const x = &__stdio;
#endif

FILE *const stdin = &__stdio;
STDIO_ALIAS(stdout);
STDIO_ALIAS(stderr);
