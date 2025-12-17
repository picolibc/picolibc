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

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define SEED    42
#define HAY_MAX 2048

static uint8_t hay[HAY_MAX];

/* Generate random uint8_t */
static uint8_t
rand_byte(void)
{
    return rand() & 0xff;
}

static int
rand_int(void)
{
    int      i;
    unsigned r = 0;

    for (i = 0; i < __INT_WIDTH__; i += 8) {
        r |= (unsigned)rand_byte() << i;
    }
    return (int)r;
}

/* Generate random size_t */
static size_t
rand_size_t(void)
{
    size_t r = 0;
    size_t i;

    for (i = 0; i < SIZE_WIDTH; i += 8) {
        r |= (size_t)rand_byte() << i;
    }
    return r;
}

/* Generate random number between 0 and max inclusive */
static size_t
rand_range(size_t max)
{
    size_t mask = ~(size_t)0;
    size_t ret;

    if (max == 0)
        return 0;
    while (mask >> 1 > max)
        mask >>= 1;
    for (;;) {
        ret = rand_size_t() & mask;
        if (ret <= max)
            return ret;
    }
}

static size_t
rand_size(size_t max)
{
    for (;;) {
        size_t ret = rand_range(max);
        if (ret)
            return ret;
    }
}

static size_t
rand_pos(size_t max)
{
    return rand_range(max);
}

#ifdef __MSP430__
/* MSP430 emulator is rather slow */
#define LOOPS 4
#else
#define LOOPS 32
#endif

static void
generate_hay(uint8_t *dst, size_t size)
{
    while (size) {
        *dst++ = rand_byte();
        size--;
    }
}

static void
fixup_hay(uint8_t *hay, size_t hay_size, uint8_t needle)
{
    while (hay_size) {
        if (*hay == needle)
            *hay ^= 0x55;
        hay++;
        hay_size--;
    }
}

int
main(void)
{
    static size_t hay_start, hay_size;
    int           ret = 0;
    int           hay_size_loop;
    int           needle_pos_loop;
    size_t        needle_pos_1, needle_pos_2;

    srand(SEED);

    /* Check long alignments for hay */
    for (hay_start = 0; hay_start < sizeof(long); hay_start++) {
        printf("loop %zd of %zd\n", hay_start, sizeof(long));

        /* Generate LOOPS random hay sizes */
        for (hay_size_loop = 0; hay_size_loop < LOOPS; hay_size_loop++) {
            hay_size = rand_size(HAY_MAX - hay_start - 1);

            /* Generate LOOPS random needle locations */
            for (needle_pos_loop = 0; needle_pos_loop < LOOPS; needle_pos_loop++) {
                needle_pos_1 = rand_pos(hay_size - 1);
                needle_pos_2 = rand_pos(hay_size - 1);
                uint8_t *hay_cur = &hay[hay_start];
                int      needle = rand_int();

                if (needle_pos_2 < needle_pos_1) {
                    size_t t = needle_pos_1;
                    needle_pos_1 = needle_pos_2;
                    needle_pos_2 = t;
                }
#if 0
                printf("hay_start %zu hay_size %zu needle %d\n",
                       hay_start, hay_size, needle);
#endif

                /* Set up the data */
                memset(hay, 0, HAY_MAX);
                generate_hay(hay_cur, hay_size);

                /*
                 * Make sure the needle doesn't already exist in hay by
                 * adjusting both
                 */
                fixup_hay(hay_cur, hay_size, (uint8_t)needle);

                /* Place the needle in the haystack */
                hay_cur[needle_pos_1] = (uint8_t)needle;
                hay_cur[needle_pos_2] = (uint8_t)needle;

                uint8_t *result;

                result = memchr(hay_cur, needle, hay_size);

                if (result != hay_cur + needle_pos_1) {
                    if (!result)
                        printf("memchr expected needle at %zu got NULL\n", needle_pos_1);
                    else
                        printf("memchr expected needle at %zu got %zu\n", needle_pos_1,
                               result - hay_cur);
                    printf("    hay_start %zu hay_size %zu needle_pos_1 %zu needle_pos_2 %zu "
                           "needle %d\n",
                           hay_start, hay_size, needle_pos_1, needle_pos_2, needle);
                    ret = 1;
                }

                result = memrchr(hay_cur, needle, hay_size);

                if (result != hay_cur + needle_pos_2) {
                    if (!result)
                        printf("memrchr expected needle at %zu got NULL\n", needle_pos_2);
                    else
                        printf("memrchr expected needle at %zu got %zu\n", needle_pos_2,
                               result - hay_cur);
                    printf("    hay_start %zu hay_size %zu needle_pos_1 %zu needle_pos_2 %zu "
                           "needle %d\n",
                           hay_start, hay_size, needle_pos_1, needle_pos_2, needle);
                    ret = 1;
                }

                /*
                 * Introduce a single bit error in the hay
                 * so that it no longer matches anywhere
                 */
                hay_cur[needle_pos_1] ^= 1;
                hay_cur[needle_pos_2] ^= 2;

                result = memchr(hay_cur, needle, hay_size);

                if (result != NULL) {
                    printf("memchr expected no needle, got %zu\n", result - hay_cur);
                    printf("    hay_start %zu hay_size %zu needle_pos_1 %zu needle_pos_2 %zu "
                           "needle %d\n",
                           hay_start, hay_size, needle_pos_1, needle_pos_2, needle);
                    ret = 1;
                }

                result = memrchr(hay_cur, needle, hay_size);

                if (result != NULL) {
                    printf("memrchr expected no needle, got %zu\n", result - hay_cur);
                    printf("    hay_start %zu hay_size %zu needle_pos_1 %zu needle_pos_2 %zu "
                           "needle %d\n",
                           hay_start, hay_size, needle_pos_1, needle_pos_2, needle);
                    ret = 1;
                }
            }
        }
    }
    return ret;
}
