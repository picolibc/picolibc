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

#include <wctype.h>
#include <ctype.h>
#include <stdio.h>

/*
 * Verify that iswctype matches the assicated iswxxx and isxxx functions
 * for ASCII and EOF
 */

static struct {
    const char *name;
    int         (*func)(int);
    int         (*wfunc)(wint_t);
    wctype_t    desc;
} wctypes[] = {
    { .name = "alnum",  .func = isalnum,  .wfunc = iswalnum  },
    { .name = "alpha",  .func = isalpha,  .wfunc = iswalpha  },
    { .name = "blank",  .func = isblank,  .wfunc = iswblank  },
    { .name = "cntrl",  .func = iscntrl,  .wfunc = iswcntrl  },
    { .name = "digit",  .func = isdigit,  .wfunc = iswdigit  },
    { .name = "graph",  .func = isgraph,  .wfunc = iswgraph  },
    { .name = "lower",  .func = islower,  .wfunc = iswlower  },
    { .name = "print",  .func = isprint,  .wfunc = iswprint  },
    { .name = "punct",  .func = ispunct,  .wfunc = iswpunct  },
    { .name = "space",  .func = isspace,  .wfunc = iswspace  },
    { .name = "upper",  .func = isupper,  .wfunc = iswupper  },
    { .name = "xdigit", .func = isxdigit, .wfunc = iswxdigit },
};

#define NUM_WCTYPES (sizeof(wctypes) / sizeof(wctypes[0]))

int
main(void)
{
    size_t t;
    int    c;
    wint_t wc;
    int    ret = 0;

    for (t = 0; t < NUM_WCTYPES; t++)
        wctypes[t].desc = wctype(wctypes[t].name);
    for (c = 0; c <= 0x80; c++) {
        if (c == 0x80) {
            c = EOF;
            wc = WEOF;
        } else
            wc = (wint_t)c;
        for (t = 0; t < NUM_WCTYPES; t++) {
            int wfunc_ret = !!wctypes[t].wfunc(wc);
            int func_ret = !!wctypes[t].func(c);
            int wctype_ret = !!iswctype(wc, wctypes[t].desc);
            if (wfunc_ret != func_ret || wfunc_ret != wctype_ret) {
                printf("c: %#x is%s %d isw%s %d iswctype(%s) %d\n", c, wctypes[t].name, wfunc_ret,
                       wctypes[t].name, func_ret, wctypes[t].name, wctype_ret);
                ret = 1;
            }
        }
        if (c == EOF)
            break;
    }
    return ret;
}
