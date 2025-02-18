/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 Keith Packard
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
#include <uchar.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <errno.h>

#define MAX_C8  16
#define MAX_C16 16
#define MAX_C32 16
#define MAX_MB  16

const struct {
    char8_t     c8[MAX_C8];
    char        mb[MAX_MB];
    unsigned    err;
} test_c8[] = {
    { .c8 = {0x61}, .mb = "a" },
    { .c8 = {0x0}, .mb = "" },
#if !defined(__PICOLIBC__) || defined(_MB_CAPABLE)
    { .c8 = "\xF4\x8F\xBF\xBF", .mb = "\xF4\x8F\xBF\xBF" },

    { .c8 = "㌰", .mb = "㌰" },
    { .c8 = "🚀", .mb = "🚀" },
    { .c8 = "\xC2\x80", .mb = "\xC2\x80" },

    /* overlong two-byte encoding */
    { .c8= "\xC0\x80", .mb = "\xC0\x80", .err = 0 + 1 },
    /* overlong three-byte encoding */
    { .c8 = "\xE0\x9F\xBF", .mb = "\xE0\x9F\xBF", .err = 1 + 1 },
    /* overlong four-byte encoding */
    { .c8 = "\xF0\x8F\xBF\xBF", .mb = "\xF0\x8F\xBF\xBF", .err = 1 + 1 },
    /* lowest surrogate 0xd800 */
    { .c8 = "\xED\xA0\x80", .mb = "\xED\xA0\x80", .err = 1 + 1 },
    /* highest surrogate 0xdfff */
    { .c8 = "\xED\xBF\xBF", .mb = "\xED\xBF\xBF", .err = 1 + 1 },
    /* missing second byte */
    { .c8 = "\xC2", .mb = "\xC2", .err = 1 + 1 },
    { .c8 = "\xE0", .mb = "\xE0", .err = 1 + 1 },
    { .c8 = "\xF0", .mb = "\xF0", .err = 1 + 1 },
    /* missing third byte */
    { .c8 = "\xE0\xA0", .mb = "\xE0\xA0", .err = 2 + 1 },
    { .c8 = "\xF0\x90", .mb = "\xF0\x90", .err = 2 + 1 },
    /* missing fourth byte */
    { .c8 = "\xF0\x90\x80", .mb = "\xF0\x90\x80", .err = 3 + 1 },
    /* too big 0x110000 */
    { .c8 = "\xF4\x90\x80\x80", .mb = "\xF4\x90\x80\x80", .err = 1 + 1 },
    /* too big 0x140000 */
    { .c8 = "\xF5\x90\x80\x80", .mb = "\xF5\x90\x80\x80", .err = 0 + 1 },
#endif
};

#define NTEST_C8 sizeof(test_c8)/sizeof(test_c8[0])

const struct {
    char16_t    c16[MAX_C16];
    char        mb[MAX_MB];
    unsigned    err;
} test_c16[] = {
    { .c16 = { 0x0061 }, .mb = "a" },
    { .c16 = { 0x0000 }, .mb = "" },
#if !defined(__PICOLIBC__) || defined(_MB_CAPABLE)
    { .c16 = { 0x3330 }, .mb = "㌰" },
    { .c16 = { 0xd83d, 0xde80 }, .mb = "🚀" },

    /* Missing low surrogate */
    { .c16 = { 0xd83d, 0x0000 }, .mb = "", .err = 1 + 1 },

    /* Missing high surrogate */
    { .c16 = { 0xde80, 0x0000 }, .mb = "ʀ", .err = 0 + 1 },

    /* Extra high surrogate */
    { .c16 = { 0xd83d, 0xda80 }, .mb = "🚀", .err = 1 + 1 },

    /* High surrogate, non-surrogate, low surrogate */
    { .c16 = { 0xd83d, 0x3330, 0xde80 }, .mb = "🚀", .err = 1 + 1 },
#endif
};

#define NTEST_C16 sizeof(test_c16)/sizeof(test_c16[0])

const struct {
    char32_t    c32[MAX_C32];
    char        mb[MAX_MB];
    unsigned    err;
} test_c32[] = {
    { .c32 = { 0x00000061 }, .mb = "a" },
    { .c32 = { 0x00000000 }, .mb = "" },
#if !defined(__PICOLIBC__) || defined(_MB_CAPABLE)
    { .c32 = { 0x00003330 }, .mb = "㌰" },
    { .c32 = { 0x0001f680 }, .mb = "🚀" },

#ifndef __GLIBC__
    /*
     * Unicode value out of range.
     *
     * Glibc doesn't report this as an error, instead it silently
     * drops the value (!).
     */
    { .c32 = { 0x00110000 }, .mb = "", .err = 0 + 1 },
#endif

    /* High surrogate value */
    { .c32 = { 0x0000d83d }, .mb = "", .err = 0 + 1 },

    /* Low surrogate value */
    { .c32 = { 0x0000de80 }, .mb = "", .err = 0 + 1 },
#endif
};

#define NTEST_C32 sizeof(test_c32)/sizeof(test_c32[0])

int main(void)
{
    unsigned i;
    unsigned j;
    int status = 0;
    mbstate_t mbstate;

#if !defined(__PICOLIBC__) || defined(_MB_CAPABLE)
    if (!setlocale(LC_CTYPE, "C.UTF-8")) {
        printf("setlocale(LC_CTYPE, \"C.UTF-8\") failed\n");
        return 1;
    }
#endif

    /* utf-8 tests */
    for (i = 0; i < NTEST_C8; i++) {
        char    mb[MAX_MB];
        size_t  ret;
        size_t  off;

        off = 0;
        memset(mb, 0, sizeof(mb));
        memset(&mbstate, 0, sizeof(mbstate));
        for (j = 0; ; j++) {
            ret = c8rtomb(mb + off, test_c8[i].c8[j], &mbstate);
            if (ret == (size_t) -1) {
                if (test_c8[i].err != 0 && test_c8[i].err == j + 1)
                    break;
                printf("c8rtomb %d failed at byte %d\n", i, j);
                status = 1;
                break;
            } else if (test_c8[i].err == j + 1) {
                printf("c8rtomb %d unexpected success\n", i);
                status = 1;
                break;
            }
            off += ret;
            if (test_c8[i].c8[j] == '\0')
                break;
        }
        if (test_c8[i].err == 0) {
            mb[off] = '\0';
            if (strcmp(mb, test_c8[i].mb)) {
                printf("c8rtomb %d: expected '%s' got '%s'\n", i, test_c8[i].mb, mb);
                status = 1;
            }
        }

        char8_t c8[MAX_C8];
        off = 0;
        ret = 0;
        j = 0;
        memset(c8, 0, sizeof(c8));
        memset(&mbstate, 0, sizeof(mbstate));
        while (test_c8[i].mb[j] != 0 || ret) {
            ret = mbrtoc8(c8 + off, &test_c8[i].mb[j], 1, &mbstate);
            if (ret == (size_t) -1)
            {
                if (test_c8[i].err <= j + 1)
                    break;
                printf("mbrtoc8 %d failed at byte %d\n", i, j);
                status = 1;
                break;
            }
            switch (ret) {
            case (size_t) -2:
                j++;
                break;
            case (size_t) -3:
                off++;
                break;
            default:
                j += ret;
                off++;
                break;
            }
        }
        if (test_c8[i].err == 0) {
            for (j = 0; j < MAX_C8; j++) {
                if (c8[j] != test_c8[i].c8[j]) {
                    printf("mbrtoc8 %d[%d]: expected 0x%x got 0x%x\n",
                           i, j, test_c8[i].c8[j], c8[j]);
                    status = 1;
                }
            }
        } else if (ret != (size_t) -1) {
#ifdef __PICOLIBC__
            printf("mbrtoc8 %d unexpected success\n", i);
            status = 1;
#endif
        }
    }

    /* utf-16 tests */
    for (i = 0; i < NTEST_C16; i++) {
        char    mb[MAX_MB];
        size_t  ret;
        size_t  off = 0;
        memset(mb, 0, sizeof(mb));
        memset(&mbstate, 0, sizeof(mbstate));
        for (j = 0; test_c16[i].c16[j] != 0; j++) {
            ret = c16rtomb(mb + off, test_c16[i].c16[j], &mbstate);
            if (ret == (size_t) -1) {
                if (test_c16[i].err != 0 && test_c16[i].err == j + 1)
                    break;
                printf("c16rtomb %d failed at byte %d\n", i, j);
                status = 1;
                break;
            } else if (test_c16[i].err == j + 1) {
                printf("c16rtomb %d unexpected success\n", i);
                status = 1;
                break;
            }
            off += ret;
        }
        if (test_c16[i].err == 0) {
            mb[off] = '\0';
            if (strcmp(mb, test_c16[i].mb)) {
                printf("test %d: expected '%s' got '%s'\n", i, test_c16[i].mb, mb);
                status = 1;
            }
        }

        char16_t c16[MAX_C16];
        off = 0;
        ret = 0;
        j = 0;
        memset(c16, 0, sizeof(c16));
        memset(&mbstate, 0, sizeof(mbstate));
        if (test_c16[i].err == 0) {
            while (test_c16[i].mb[j] != 0 || ret) {
                ret = mbrtoc16(c16 + off, &test_c16[i].mb[j], 1, &mbstate);
                if (ret == (size_t) -1)
                {
                    printf("mbrtoc16 %d failed at byte %d\n", i, j);
                    status = 1;
                    break;
                }
                switch (ret) {
                case (size_t) -2:
                    j++;
                    break;
                case (size_t) -3:
                    off++;
                    break;
                default:
                    j += ret;
                    off++;
                    break;
                }
            }
            for (j = 0; j < MAX_C16; j++) {
                if (c16[j] != test_c16[i].c16[j]) {
                    printf("mbrtoc16 %d[%d]: expected 0x%x got 0x%x\n",
                           i, j, test_c16[i].c16[j], c16[j]);
                    status = 1;
                }
            }
        }
    }
    for (i = 0; i < NTEST_C32; i++) {
        char    mb[MAX_MB];
        size_t  ret;
        size_t  off = 0;
        memset(mb, 0, sizeof(mb));
        memset(&mbstate, 0, sizeof(mbstate));
        for (j = 0; test_c32[i].c32[j] != 0; j++) {
            ret = c32rtomb(mb + off, test_c32[i].c32[j], &mbstate);
            if (ret == (size_t) -1) {
                if (test_c32[i].err != 0 && test_c32[i].err == j + 1)
                    break;
                printf("c32rtomb %d failed at byte %d\n", i, j);
                status = 1;
                break;
            } else if (test_c32[i].err == j + 1) {
                printf("c32rtomb %d unexpected success c32[%u] is %#08lx\n", i, j, (unsigned long) test_c32[i].c32[j]);
                status = 1;
                break;
            }
            off += ret;
        }
        if (test_c32[i].err == 0) {
            mb[off] = '\0';
            if (strcmp(mb, test_c32[i].mb)) {
                printf("test %d: expected '%s' got '%s'\n", i, test_c32[i].mb, mb);
                status = 1;
            }
        }

        char32_t c32[MAX_C32];
        off = 0;
        ret = 0;
        j = 0;
        memset(c32, 0, sizeof(c32));
        memset(&mbstate, 0, sizeof(mbstate));
        if (test_c32[i].err == 0) {
            while (test_c32[i].mb[j] != 0 || ret) {
                ret = mbrtoc32(c32 + off, &test_c32[i].mb[j], 1, &mbstate);
                if (ret == (size_t) -1)
                {
                    printf("mbrtoc32 %d failed at byte %d\n", i, j);
                    status = 1;
                    break;
                }
                switch (ret) {
                case (size_t) -2:
                    j++;
                    break;
                case (size_t) -3:
                    off++;
                    break;
                default:
                    j += ret;
                    off++;
                    break;
                }
            }
            for (j = 0; j < MAX_C32; j++) {
                if (c32[j] != test_c32[i].c32[j]) {
                    printf("mbrtoc32 %d[%d]: expected 0x%08lx got 0x%08lx\n",
                           i, j, (unsigned long) test_c32[i].c32[j], (unsigned long) c32[j]);
                    status = 1;
                }
            }
        }
    }
    return status;
}
