/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 Keith Packard
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

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>

#ifdef __arm__
void *__aeabi_memset(void *dest, size_t n, int c);
void *__aeabi_memset4(void *dest, size_t n, int c);
void *__aeabi_memset8(void *dest, size_t n, int c);
void __aeabi_memclr(void *dest, size_t n);
void __aeabi_memclr4(void *dest, size_t n);
void __aeabi_memclr8(void *dest, size_t n);
#endif

static uint8_t crc_table[256];

static uint8_t
crc8(uint8_t c)
{
    int i;
    for (i = 0; i < 8; i++) {
        if (c & 1)
            c = 0x31 ^ (c >> 1);
        else
            c >>= 1;
    }
    return c;
}

static void
init_crc(void)
{
    int i;
    for (i = 0; i < 256; i++)
        crc_table[i] = crc8((uint8_t) i);
}

static char
expect(size_t pos)
{
    unsigned int i;
    uint8_t     c = 0xff;
    for (i = 0; i < sizeof(size_t); i++)
        c ^= crc_table[(uint8_t) (pos >> (i * 8))];
    return (char) c;
}

size_t
check(char *label, void *buf, size_t size, size_t start, size_t end, int c)
{
    size_t      p;
    size_t      error = 0;
    char        *b = buf;

    for (p = 0; p < size; p++) {
        char g = b[p];
        char e = c;
        if (p < start || end <= p)
            e = expect(p);
        if (g != e) {
            fprintf(stderr, "%s(%zu, %zu):%zu expect 0x%02x got 0x%02x\n",
                    label, start, end, p, (uint8_t) e, (uint8_t) g);
            error++;
        }
    }
    return error;
}

size_t
randrange(size_t max)
{
    size_t      pot;
    size_t      rnd;

    for (pot = 1; pot && pot < max; pot <<= 1)
        ;

    for (;;) {
        rnd = random() & (pot - 1);
        if (rnd < max)
            return rnd;
    }
}


#define MAX_BUF         1024

#define NEND            64
#define NSTART          64

static char buf[MAX_BUF];

static void
fill(void)
{
    size_t      p;

    for (p = 0; p < MAX_BUF; p++)
        buf[p] = expect(p);
}

static size_t
test(size_t start, size_t end, int c)
{
    size_t error = 0;
    char *b = buf;
    fill();
    memset(b + start, c, end - start);
    error += check("memset", buf, MAX_BUF, start, end, c);

    fill();
    bzero(b + start, end - start);
    error += check("bzero", buf, MAX_BUF, start, end, 0);

#ifdef __arm__
    fill();
    __aeabi_memset(b + start, end - start, c);
    error += check("__aeabi_memset", buf, MAX_BUF, start, end, c);

    fill();
    __aeabi_memclr(b + start, end - start);
    error += check("__aeabi_memclr", buf, MAX_BUF, start, end, 0);

    start &= ~3;
    end &= ~3;

    fill();
    __aeabi_memset4(b + start, end - start, c);
    error += check("__aeabi_memset4", buf, MAX_BUF, start, end, c);

    fill();
    __aeabi_memclr4(b + start, end - start);
    error += check("__aeabi_memclr4", buf, MAX_BUF, start, end, 0);

    start &= ~7;
    end &= ~7;

    fill();
    __aeabi_memset8(b + start, end - start, c);
    error += check("__aeabi_memset8", buf, MAX_BUF, start, end, c);

    fill();
    __aeabi_memclr8(b + start, end - start);
    error += check("__aeabi_memclr8", buf, MAX_BUF, start, end, 0);
#endif

    return error;

}

int
main(void)
{
    size_t      error;
    size_t      end;
    size_t      start;

    int         nend;
    int         nstart;
    int         ret = 0;

    init_crc();
    for (nend = 0; nend < NEND; nend++) {
        end = randrange(MAX_BUF + 1);
        for (nstart = 0; nstart < NSTART; nstart++) {
            start = randrange(end + 1);
            int c = randrange(INT_MAX);
            error = test(start, end, c);
            if (error)
                ret = 1;
        }
    }

    return ret;
}
