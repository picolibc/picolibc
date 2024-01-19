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

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if !defined(__PICOLIBC__) || defined(TINY_STDIO) || defined(_WANT_IO_LONG_LONG)

unsigned long long
naive_atou(const char *buf)
{
    unsigned long long v = 0;
    int sign = 1;
    char c;

    while ((c = *buf++) == ' ')
        ;
    do {
        assert('0' <= c && c <= '9');
        v = v * 10 + (c - '0');
        c = *buf++;
    } while(c);
    return v * sign;
}

long long
naive_atol(const char *buf)
{
    long long v = 0;
    int sign = 1;
    char c;

    while ((c = *buf++) == ' ')
        ;
    switch (c) {
    case '+':
        sign = 1;
        c = *buf++;
        break;
    case '-':
        sign = -1;
        c = *buf++;
        break;
    }
    do {
        assert('0' <= c && c <= '9');
        v = v * 10 + (c - '0');
        c = *buf++;
    } while(c);
    return v * sign;
}

char *
naive_utoa(char *buf, unsigned long long v)
{
    buf += 21;
    *--buf = '\0';
    do {
        *--buf = v % 10 + '0';
        v /= 10;
    } while (v);
    return buf;
}

char *
naive_ltoa(char *buf, long long v)
{
    int sign = 0;
    buf += 21;
    *--buf = '\0';
    if (v < 0) {
        sign = 1;
        v = -v;
    }
    do {
        *--buf = v % 10 + '0';
        v /= 10;
    } while (v);
    if (sign)
        *--buf = '-';
    return buf;
}

static int
check(long long x)
{
    long long   scanned, uscanned, u;
    char        buf[64], naive_buf[64];
    char        *naive;
    int         ret = 0;

    sprintf(buf, "%lld", x);
    naive = naive_ltoa(naive_buf, x);
    sscanf(naive, "%lld", &scanned);
    if (strcmp(buf, naive) != 0) {
        printf("sprintf '%s' naive '%s'\n", buf, naive);
        ret = 1;
    }
    if (scanned != x) {
        printf("sscanf '%lld' actual '%lld'\n", scanned, x);
        ret = 1;
    }

    u = (unsigned long long) x;
    sprintf(buf, "%llu", u);
    naive = naive_utoa(naive_buf, u);
    sscanf(naive, "%llu", &uscanned);
    if (strcmp(buf, naive) != 0) {
        printf("unsigned sprintf '%s' naive '%s'\n", buf, naive);
        ret = 1;
    }
    if (scanned != x) {
        printf("unsigned sscanf '%llu' actual '%llu'\n", uscanned, u);
        ret = 1;
    }
    return ret;
}

static long long
randval(void)
{
    unsigned long        a, b, c;

    a = random();
    b = random();
    c = random();
    return ((unsigned long long) a << 33) ^ ((unsigned long long) b << 15) ^ ((unsigned long long) c);
}

#ifdef __MSP430__
#define RAND_LOOPS 1000ll
#define SMALL_MIN -1024ll
#define SMALL_MAX 1024ll
#else
#define RAND_LOOPS 100000ll
#define SMALL_MIN -65536
#define SMALL_MAX 65536
#endif

int main(void)
{
    long long   x;
    int         ret = 0;
    long long   t;

    for (x = SMALL_MIN; x <= SMALL_MAX; x++)
        ret |= check(x);

    for (t = 0; t < RAND_LOOPS; t++) {
        x = randval();
        ret |= check(x);
    }
    return ret;
}

#else

int main(void)
{
    printf("skipping long long I/O test\n");
    return 77;
}

#endif
