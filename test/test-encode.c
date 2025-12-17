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

#include <ctype.h>
#include <wctype.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <limits.h>
#include <string.h>

static const char *locales[] = {
    "C",
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
    "C.GEORGIAN-PS",
    "C.PT154",
    "C.KOI8-T",
    "C.CP437",
    "C.CP737",
    "C.CP775",
    "C.CP850",
    "C.CP852",
    "C.CP855",
    "C.CP857",
    "C.CP858",
    "C.CP862",
    "C.CP866",
    "C.CP874",
    "C.CP1125",
    "C.CP1250",
    "C.CP1251",
    "C.CP1252",
    "C.CP1253",
    "C.CP1254",
    "C.CP1256",
    "C.CP1257",
    "C.KOI8-R",
    "C.KOI8-U",
#endif
#ifdef HAVE_JIS_CHARSETS
#define JIS_START (NUM_LOCALE - 2)
    "C.EUC-JP",
    "C.SHIFT-JIS",
#endif
};

#define NUM_LOCALE sizeof(locales) / sizeof(locales[0])

#if __SIZEOF_WCHAR_T__ == 2
#define LAST_CHAR 0xffff
#else
#define LAST_CHAR 0xdffff
#endif

int
main(int argc, char **argv)
{
    int         error = 0;
    int         c, lastc;
    size_t      mb_cur_max;
    char        mb[MB_LEN_MAX + 1];
    char        byte2[2];
    int         nbyte2;
    int         i2;
    size_t      mb_ret;
    wchar_t     wc;
    FILE       *out = stdout;
    unsigned    i;
    const char *encode;

    if (argc > 1) {
        out = fopen(argv[1], "w");
        if (!out) {
            perror(argv[1]);
            exit(1);
        }
    }

    for (i = 0; i < NUM_LOCALE; i++) {
        encode = locales[i];
        if (setlocale(LC_ALL, encode) == NULL) {
            printf("invalid locale %s\n", encode);
            error = 1;
            continue;
        }
        mb_cur_max = 1;
        byte2[0] = 0;
        nbyte2 = 1;
#ifdef JIS_START
        if (i >= JIS_START) {
            mb_cur_max = 2;
            if (i == JIS_START) {
                mb_cur_max = 3;
                byte2[1] = 0x8f;
                nbyte2 = 2;
            }
        }
#endif
        switch (mb_cur_max) {
        case 1:
            lastc = 0xff;
            break;
        case 2:
        case 3:
            lastc = 0xffff;
            break;
        default:
            printf("invalid MB_CUR_MAX %zd\n", mb_cur_max);
            error = 1;
            continue;
        }
        for (i2 = 0; i2 < nbyte2; i2++) {
            for (c = 0; c <= lastc; c++) {
                mbstate_t ps;
                size_t    nc;
                memset(&ps, 0, sizeof(ps));

                if (byte2[i2] != 0) {
                    mb[0] = byte2[i2];
                    mb[1] = c >> 8;
                    mb[2] = c;
                    nc = 3;
                } else {
                    if (c < 0x100) {
                        mb[0] = c;
                        nc = 1;
                    } else {
                        mb[0] = c >> 8;
                        mb[1] = c;
                        nc = 2;
                    }
                }
                unsigned long this_c = (unsigned long)(unsigned)c
                    + (unsigned long)(unsigned char)byte2[i2] * 256UL * 256UL;
                wc = 0xfffff;
                mb_ret = mbrtowc(&wc, mb, nc, &ps);
                if (mb_ret == 1 && this_c >= 0x100)
                    continue;
                if (mb_ret > mb_cur_max)
                    continue;
                if (wc == 0xfffff)
                    printf("missing %#08lx in %s ret %zd\n", this_c, encode, mb_ret);
                fprintf(out, "%-12s->wc %#08lx %#08lx\n", encode, this_c, (unsigned long)wc);
            }
        }

        for (wc = 1;; wc++) {
            mbstate_t ps;
            size_t    s;
            memset(&ps, 0, sizeof(ps));

            mb_ret = wcrtomb(mb, wc, &ps);
            if (wc == LAST_CHAR)
                break;
            if (mb_ret == (size_t)-1)
                continue;
            fprintf(out, "wc->%-12s %#07lx", encode, (unsigned long)wc);
            for (s = 0; s < mb_ret; s++)
                fprintf(out, " %02x", (unsigned char)mb[s]);
            fprintf(out, "\n");
        }
    }
    fflush(out);
    return error;
}
