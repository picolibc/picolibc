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

#include <wctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <limits.h>

static struct {
    char *n;
    int (*f)(wint_t);
} funcs[] = {
    { .n = "alnum", iswalnum },
    { .n = "alpha", iswalpha },
    { .n = "blank", iswblank },
    { .n = "cntrl", iswcntrl },
    { .n = "digit", iswdigit },
    { .n = "graph", iswgraph },
    { .n = "lower", iswlower },
    { .n = "print", iswprint },
    { .n = "punct", iswpunct },
    { .n = "space", iswspace },
    { .n = "upper", iswupper },
    { .n = "xdigit", iswxdigit },
};

#define NFUNC sizeof(funcs)/sizeof(funcs[0])

int main(int argc, char **argv)
{
    wchar_t     c;
    unsigned    f;
    FILE        *out = stdout;
    int         i;
    char        *encode = "ascii";
    char        mbbuf[MB_LEN_MAX];

    if (argc > 1) {
        out = fopen(argv[1], "w");
        if (!out) {
            perror(argv[1]);
            exit(1);
        }
    }

    for (i = 0; i < 2; i++) {
        for (c = 0x0000; ; c++) {
            /* Skip TAG characters as glibc allows them? */
            if (i == 0 && (c >> 7 == (0xe0000 >> 7)))
                continue;
            if(wctomb(mbbuf, c) >= 0) {
                fprintf(out, "%s 0x%04x", encode, c);
                for (f = 0; f < NFUNC; f++)
                    fprintf(out, "%6.6s ", funcs[f].f(c) ? funcs[f].n : "      ");

                fprintf(out, "\n");
            }
#if __SIZEOF_WCHAR_T__ == 2
            if (c == 0xffff)
                break;
#else
            if (c == 0xe01ef)
                break;
#endif
        }
        setlocale(LC_ALL, "C.UTF-8");
        encode = "utf-8";
    }
    fflush(out);
    return 0;
}
