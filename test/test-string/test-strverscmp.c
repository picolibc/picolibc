/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * strverscmp (a GNU extension): digit runs compare as numbers, and a
 * run with leading zeros is a fraction, ordered before integers and
 * with more leading zeros smaller. This is the order the glibc manual
 * gives, each element before the next.
 */

static const char * const order[] = {
    "000", "00", "01", "010", "09", "0", "1", "9", "10",
};

#define N_ORDER (sizeof(order) / sizeof(order[0]))

static const struct {
    const char *l, *r;
    int         sign;
} pairs[] = {
    { "",        "",       0  },
    { "abc",     "abc",    0  },
    { "a",       "b",      -1 },
    { "a0",      "a",      1  },
    { "0a",      "0",      1  },
    { "item9",   "item10", -1 },
    { "item010", "item09", -1 },
    { "v1.2.10", "v1.2.9", 1  },
    { "x.001",   "x.01",   -1 },
};

#define N_PAIRS (sizeof(pairs) / sizeof(pairs[0]))

static int
sign(int v)
{
    return (v > 0) - (v < 0);
}

int
main(void)
{
    size_t i, j;
    int    ret = 0;

    for (i = 0; i < N_ORDER; i++)
        for (j = 0; j < N_ORDER; j++) {
            int want = (i > j) - (i < j);
            int got = sign(strverscmp(order[i], order[j]));

            if (got != want) {
                printf("strverscmp(\"%s\", \"%s\") = %d, want %d\n", order[i], order[j], got, want);
                ret = 1;
            }
        }
    for (i = 0; i < N_PAIRS; i++) {
        int got = sign(strverscmp(pairs[i].l, pairs[i].r));
        int rev = sign(strverscmp(pairs[i].r, pairs[i].l));

        if (got != pairs[i].sign || rev != -pairs[i].sign) {
            printf("strverscmp(\"%s\", \"%s\") = %d (reversed %d), want %d\n", pairs[i].l,
                   pairs[i].r, got, rev, pairs[i].sign);
            ret = 1;
        }
    }
    if (ret == 0)
        printf("success\n");
    return ret;
}
