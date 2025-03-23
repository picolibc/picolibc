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
#include <wchar.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <errno.h>

#define MAX_WC  16
#define MAX_MB  16

const struct {
    wchar_t     wc[MAX_WC];
    char        mb[MAX_MB];
    unsigned    err;
} test[] = {

#if __SIZEOF_WCHAR_T__ == 2
    { .wc = { 0x0061 }, .mb = "a" },
    { .wc = { 0x0000 }, .mb = "" },
#if !defined(__PICOLIBC__) || defined(__MB_CAPABLE)
    { .wc = { 0x3330 }, .mb = "㌰" },
    { .wc = { 0xd83d, 0xde80 }, .mb = "🚀" },                   /* 0x01f680 */
    { .wc = { 0xdbc2, 0xdd7f }, .mb = "\xf4\x80\xa5\xbf" },     /* 0x10097f */
    { .wc = { 0xd800, 0xdc00 }, .mb = "\xf0\x90\x80\x80" },     /* 0x010000 */
    { .wc = { 0xdbff, 0xdfff }, .mb = "\xf4\x8f\xbf\xbf" },     /* 0x10ffff */

    /* Missing low surrogate */
    { .wc = { 0xd83d, 0x0000 }, .mb = "", .err = 1 + 1 },

    /* Missing high surrogate */
    { .wc = { 0xde80, 0x0000 }, .mb = "ʀ", .err = 0 + 1 },

    /* Extra high surrogate */
    { .wc = { 0xd83d, 0xda80 }, .mb = "🚀", .err = 1 + 1 },

    /* High surrogate, non-surrogate, low surrogate */
    { .wc = { 0xd83d, 0x3330, 0xde80 }, .mb = "🚀", .err = 1 + 1 },
#endif

#else

    { .wc = { 0x00000061 }, .mb = "a" },
    { .wc = { 0x00000000 }, .mb = "" },
#if !defined(__PICOLIBC__) || defined(__MB_CAPABLE)
    { .wc = { 0x00003330 }, .mb = "㌰" },
    { .wc = { 0x0001f680 }, .mb = "🚀" },

#ifndef __GLIBC__
    /*
     * Unicode value out of range.
     *
     * Glibc doesn't report this as an error, instead it silently
     * drops the value (!).
     */
    { .wc = { 0x00110000 }, .mb = "", .err = 0 + 1 },
#endif

    /* High surrogate value */
    { .wc = { 0x0000d83d }, .mb = "", .err = 0 + 1 },

    /* Low surrogate value */
    { .wc = { 0x0000de80 }, .mb = "", .err = 0 + 1 },
#endif /* !defined(__PICOLIBC__) || defined(__MB_CAPABLE) */

#endif /* else __SIZEOF_WCHAR_T__ == 2 */
};

#define NTEST (sizeof(test)/sizeof(test[0]))

int main(void)
{
    unsigned i;
    unsigned j;
    unsigned k;
    int status = 0;
    mbstate_t mbstate;

#if !defined(__PICOLIBC__) || defined(__MB_CAPABLE)
    if (!setlocale(LC_CTYPE, "C.UTF-8")) {
        printf("setlocale(LC_CTYPE, \"C.UTF-8\") failed\n");
        return 1;
    }
#endif

    for (i = 0; i < NTEST; i++) {
        char    mb[MAX_MB];
        size_t  ret = 0;
        size_t  off = 0;
        memset(mb, 0, sizeof(mb));
        memset(&mbstate, 0, sizeof(mbstate));
        for (j = 0; test[i].wc[j] != 0; j++) {
            ret = wcrtomb(mb + off, test[i].wc[j], &mbstate);
            if (ret == (size_t) -1) {
                if (test[i].err != 0 && test[i].err == j + 1)
                    break;
                printf("wcrtomb %d failed at char %d\n", i, j);
                status = 1;
                break;
            } else if (test[i].err == j + 1) {
                printf("wcrtomb %d unexpected success wc[%u] is %#lx: ", i, j, (unsigned long) test[i].wc[j]);
                for (k = 0; test[i].wc[k] != 0; k++)
                    printf(" 0x%lx", (unsigned long) test[i].wc[k]);
                printf("\n");
                status = 1;
                break;
            }
            off += ret;
        }
        if (test[i].err == 0) {
            mb[off] = '\0';
            printf("wcrtomb '%ls' '%s'\n", test[i].wc, mb);
            if (strcmp(mb, test[i].mb)) {
                printf("test %d: expected '%s' got '%s'\n", i, test[i].mb, mb);
                status = 1;
            }
        } else if (ret == (size_t) -1) {
            printf("expected wcrtomb error at %d: ", j);
            for (k = 0; test[i].wc[k] != 0; k++)
                printf(" 0x%lx", (unsigned long) test[i].wc[k]);
            printf("\n");
        }

        wchar_t wc[MAX_WC];
        off = 0;
        ret = 0;
        j = 0;
        memset(wc, 0, sizeof(wc));
        memset(&mbstate, 0, sizeof(mbstate));
        if (test[i].err == 0) {
            while (test[i].mb[j] != 0 || ret) {
                ret = mbrtowc(wc + off, &test[i].mb[j], 1, &mbstate);
                if (ret == (size_t) -1)
                {
                    printf("mbrtowc %d failed at byte %d\n", i, j);
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
            for (j = 0; j < MAX_WC; j++) {
                if (wc[j] != test[i].wc[j]) {
                    printf("mbrtowc %d[%d]: expected 0x%08lx got 0x%08lx\n",
                           i, j, (unsigned long) test[i].wc[j], (unsigned long) wc[j]);
                    status = 1;
                }
            }
            printf("mbrtowc '%s' '%ls'\n", test[i].mb, wc);
        }
    }
    return status;
}
