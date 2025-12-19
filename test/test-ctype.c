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

#include <stdio.h>
#include <ctype.h>
#include <wctype.h>
#include <wchar.h>
#include <stdbool.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

/* Validate C locale ctype data */

#define boolname(b)     ((b) ? "true" : "false")
#define iscat(name)     is ## name
#define iswcat(name)     isw ## name
#define TEST_ASCII(name)   do {                                         \
        if (!!iscat(name)(c) != !!name) {                               \
            printf("character %#2x '%c' is%s is %s should be %s\n",     \
                   c, c, #name, boolname(iscat(name)(c)), boolname(name)); \
            error = 1;                                                  \
        }                                                               \
    } while(0)

#define TEST_WC(name) do {                                              \
        if (!!iscat(name)(c) != !!iswcat(name)((wint_t)(wc))) {         \
                printf("%s: character %#2x %#5lx '%c' is%s = %s isw%s = %s\n",    \
                       locales[l], c, (unsigned long) wc, c, #name, boolname(iscat(name)(c)), #name, \
                       boolname(iswcat(name)((wint_t) wc)));            \
                error = 1;                                              \
        }                                                               \
    } while(0)

#ifdef __MB_EXTENDED_CHARSETS_ISO
#define HAVE_ISO_CHARSETS
#endif

#ifdef __MB_EXTENDED_CHARSETS_WINDOWS
#define HAVE_WINDOWS_CHARSETS
#endif

#ifdef __MB_EXTENDED_CHARSETS_JIS
#define HAVE_JIS_CHARSETS
#endif

static const char *const locales[] = {
    "C",
#define C_LOCALE        0
#ifdef HAVE_ISO_CHARSETS
    "C.ISO-8859-1",
    "C.ISO-8859-2",
    "C.ISO-8859-3",
    "C.ISO-8859-4",
    "C.ISO-8859-5",
    "C.ISO-8859-6",
    "C.ISO-8859-7",
    "C.ISO-8859-8",
    "C.ISO-8859-9",
    "C.ISO-8859-10",
    "C.ISO-8859-11",
    "C.ISO-8859-13",
    "C.ISO-8859-14",
    "C.ISO-8859-15",
    "C.ISO-8859-16",
#endif
#ifdef HAVE_WINDOWS_CHARSETS
    "C.GEORGIAN-PS", "C.PT154", "C.KOI8-T", "C.CP437", "C.CP737", "C.CP775",
    "C.CP850", "C.CP852", "C.CP855", "C.CP857", "C.CP858", "C.CP862", "C.CP866",
    "C.CP874", "C.CP1125", "C.CP1250", "C.CP1251", "C.CP1252", "C.CP1253",
    "C.CP1254", "C.CP1256", "C.CP1257", "C.KOI8-R", "C.KOI8-U",
#endif
#ifdef HAVE_JIS_CHARSETS
    "C.SHIFT-JIS",
    "C.EUC-JP",
#endif
};

#define NUM_LOCALE sizeof(locales)/sizeof(locales[0])

int main(void) {

    int c;
    char ch[5];
    wchar_t wc;
    int error = 0;
    unsigned l;
    size_t mb_ret;

    for (l = 0; l < NUM_LOCALE; l++) {
        if (setlocale(LC_ALL, locales[l]) == NULL) {
            printf("%s: setlocale failed\n", locales[l]);
            error = 1;
            continue;
        }
        for (c = 0; c < 0x100; c++) {
            /* Direct values */
            bool blank = c == ' ' || c == '\t';
            bool cntrl = c < ' ' || c == 127;
            bool digit = '0' <= c && c <= '9';
            bool graph = ' ' < c && c < 127;
            bool lower = 'a' <= c && c <= 'z';
            bool print = ' ' <= c && c < 127;
            bool space = c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
            bool upper = 'A' <= c && c <= 'Z';
            bool xdigit = ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');

            /* Derived values */
            bool alpha = upper || lower;
            bool alnum = alpha || digit;

            bool punct = print && !space && !alnum;

            mbstate_t ps;

            if (c < 0x80) {
                TEST_ASCII(alnum);
                TEST_ASCII(alpha);
                TEST_ASCII(blank);
                TEST_ASCII(cntrl);
                TEST_ASCII(digit);
                TEST_ASCII(graph);
                TEST_ASCII(lower);
                TEST_ASCII(print);
                TEST_ASCII(punct);
                TEST_ASCII(space);
                TEST_ASCII(upper);
                TEST_ASCII(xdigit);
            }

            ch[0] = c;
            wc = 0;
            memset(&ps, 0, sizeof(ps));
            mb_ret = mbrtowc(&wc, ch, 1, &ps);

            if (c >= 0x80 && l == C_LOCALE) {
                if (mb_ret != (size_t) -1) {
                    printf("mbrtowc %#2x returned %#5lx in %s\n", c, (unsigned long) wc, locales[l]);
                    error = 1;
                }
                continue;
            } else {
                if (mb_ret == (size_t) -1) {
                    if (c == 0xa0 && !strncmp(locales[l], "C.ISO", 5))
                    {
                        printf("mbrtowc %#2x failed in %s\n", c, locales[l]);
                        error = 1;
                    }
                    continue;
                }
                if (mb_ret == (size_t) -2) {
                    if (!strcmp(locales[l], "C.SHIFT-JIS") || !strcmp(locales[l], "C.EUC-JP"))
                        continue;
                }
                if (mb_ret == 0 && c == 0)
                    continue;
                if (mb_ret != 1) {
                    printf("mbrtowc %02x returned %zd in %s\n", c, mb_ret, locales[l]);
                    continue;
                }
            }

            TEST_WC(alnum);
            TEST_WC(alpha);
            TEST_WC(blank);
            TEST_WC(cntrl);
            TEST_WC(digit);
            TEST_WC(graph);
            TEST_WC(lower);
            TEST_WC(print);
            TEST_WC(punct);
            TEST_WC(space);
            TEST_WC(upper);
            TEST_WC(xdigit);
        }
    }
    return error;
}
