/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Artem Kulyk
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

/*
 * Boundary and overlap tests for memmove: zero/one/max lengths,
 * every alignment, both overlap directions, exact overlap and
 * return pointer semantics.  Content is checked against a
 * byte-by-byte reference model.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BUF   512
#define ALIGN 8

static unsigned char model[BUF + 2 * ALIGN];
static unsigned char work[BUF + 2 * ALIGN];

static int           errors;

static void
report(const char *what, size_t n, size_t a1, size_t a2)
{
    printf("%s failed: n %zu off %zu/%zu\n", what, n, a1, a2);
    errors++;
}

static void
fill_seq(unsigned char *p, size_t n, unsigned seed)
{
    size_t i;
    for (i = 0; i < n; i++) {
        seed = seed * 1103515245u + 12345u;
        p[i] = (unsigned char)(seed >> 16);
    }
}

static void
check_region(const char *what, size_t n, size_t a1, size_t a2)
{
    if (memcmp(model, work, sizeof(model)) != 0) {
        report(what, n, a1, a2);
    }
}

/*
 * Non-overlapping memmove at all alignment pairs for a set of
 * lengths including zero, one and the buffer maximum.
 */
static void
test_move_disjoint(void)
{
    static const size_t lens[]
        = { 0, 1, 2, 3, 4, 7, 8, 15, 16, 17, 31, 32, 33, 64, 127, 128, 255, 256, 257, 511, 512 };
    size_t li, a1, a2, i;

    for (li = 0; li < sizeof(lens) / sizeof(lens[0]); li++) {
        size_t n = lens[li];
        for (a1 = 0; a1 < ALIGN; a1++) {
            for (a2 = 0; a2 < ALIGN; a2++) {
                fill_seq(work, sizeof(work), (unsigned)(n * 64 + a1 * 8 + a2));
                memcpy(model, work, sizeof(work));
                if (memmove(work + a1, work + a2 + ALIGN, n) != work + a1) {
                    report("memmove return", n, a1, a2);
                }
                memmove(model + a1, model + a2 + ALIGN, n);
                check_region("memmove disjoint", n, a1, a2);
                for (i = 0; i < sizeof(work); i++)
                    work[i] = model[i];
            }
        }
    }
}

/*
 * Overlapping memmove with dst below src (forward copy) and src below
 * dst (backward copy), at overlap distances 1..8 and all alignment
 * pairs, plus exact identity overlap.
 */
static void
test_move_overlap(void)
{
    static const size_t lens[] = { 1, 2, 3, 4, 8, 15, 16, 17, 32, 64, 128, 256, 512 };
    size_t              li, k, a, n;

    for (li = 0; li < sizeof(lens) / sizeof(lens[0]); li++) {
        n = lens[li];
        for (k = 1; k <= 8; k++) {
            if (n + k > BUF)
                continue;

            /* dst = src + k: overlapping forward copy */
            for (a = 0; a < ALIGN; a++) {
                fill_seq(work, sizeof(work), (unsigned)(n * 32 + k));
                memcpy(model, work, sizeof(work));
                if (memmove(work + a + k, work + a, n) != work + a + k) {
                    report("memmove ovw return", n, k, a);
                }
                memmove(model + a + k, model + a, n);
                check_region("memmove overlap dst=src+k", n, k, a);
            }

            /* src = dst + k: overlapping backward copy */
            for (a = 0; a < ALIGN; a++) {
                fill_seq(work, sizeof(work), (unsigned)(n * 32 + k + 1));
                memcpy(model, work, sizeof(work));
                if (memmove(work + a, work + a + k, n) != work + a) {
                    report("memmove ovb return", n, k, a);
                }
                memmove(model + a, model + a + k, n);
                check_region("memmove overlap src=dst+k", n, k, a);
            }

            /* identical pointers: must leave memory unchanged */
            for (a = 0; a < ALIGN; a++) {
                fill_seq(work, sizeof(work), (unsigned)(n + k + a));
                memcpy(model, work, sizeof(work));
                memmove(work + a, work + a, n);
                check_region("memmove self overlap", n, k, a);
            }
        }
    }
}

int
main(void)
{
    memset(work, 0, sizeof(work));
    memset(model, 0, sizeof(model));
    errors = 0;
    test_move_disjoint();
    test_move_overlap();
    if (errors)
        printf("%d errors\n", errors);
    else
        printf("ok\n");
    return errors != 0;
}
