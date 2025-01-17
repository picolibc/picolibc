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
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <limits.h>

static struct {
    char n;
    int (*f)(wint_t);
} funcs[] = {
    { .n = 'A', iswalnum },
    { .n = 'a', iswalpha },
    { .n = 'b', iswblank },
    { .n = 'c', iswcntrl },
    { .n = 'd', iswdigit },
    { .n = 'g', iswgraph },
    { .n = 'l', iswlower },
    { .n = 'p', iswprint },
    { .n = '!', iswpunct },
    { .n = 's', iswspace },
    { .n = 'u', iswupper },
    { .n = 'x', iswxdigit },
};

#define NFUNC sizeof(funcs)/sizeof(funcs[0])

static const char *locales[] = {
    "C",
#ifdef HAVE_UTF_CHARSETS
    "C.UTF-8",
#endif
#ifdef HAVE_ISO_CHARSETS
    "C.ISO-8859-2",
#endif
#ifdef HAVE_WINDOWS_CHARSETS
    "C.CP1125",
#endif
#ifdef HAVE_JIS_CHARSETS
    "C.SJIS",
#endif
};

#define NUM_LOCALE sizeof(locales)/sizeof(locales[0])

#if __SIZEOF_WCHAR_T__ == 2
#define LAST_CHAR 0xffff
#else
#define LAST_CHAR 0xe01ef
#endif

int main(int argc, char **argv)
{
    wchar_t     c;
    unsigned    f;
    FILE        *out = stdout;
    unsigned    i;
    const char  *encode;
    unsigned    prev_mask;
    unsigned    this_mask;

    if (argc > 1) {
        out = fopen(argv[1], "w");
        if (!out) {
            perror(argv[1]);
            exit(1);
        }
    }

    for (i = 0; i < NUM_LOCALE; i++) {
        if (setlocale(LC_ALL, locales[i]) == NULL) {
            printf("invalid locale %s\n", locales[i]);
            continue;
        }
        encode = locales[i];
        prev_mask = ~0;
        for (c = 0x0000; ; c++) {
            this_mask = 0;
            for (f = 0; f < NFUNC; f++)
                if (funcs[f].f(c))
                    this_mask |= (1 << f);
            if (this_mask != prev_mask) {
                fprintf(out, "%-12s 0x%04lx ", encode, (unsigned long) c);
                for (f = 0; f < NFUNC; f++)
                    fprintf(out, "%c", (this_mask & (1 << f)) ? funcs[f].n : '.');
                prev_mask = this_mask;
                fprintf(out, "\n");
            }
            if (c == LAST_CHAR)
                break;
        }
    }
    fflush(out);
    return 0;
}
