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

#define _XOPEN_SOURCE 700
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

/*
 * wcwidth and wcswidth (XSI): 0 for the null character, -1 for
 * characters that aren't printable in the locale, column counts
 * otherwise. The UTF-8 checks run where a C.UTF-8 locale exists.
 */

#define check(condition, message)                      \
    do {                                               \
        if (!(condition)) {                            \
            printf("%s: %s\n", (message), #condition); \
            exit(1);                                   \
        }                                              \
    } while (0)

static void
portable(void)
{
    wint_t c;

    check(wcwidth(L'\0') == 0, "wcwidth of the null character");
    for (c = 0x20; c < 0x7f; c++)
        check(wcwidth((wchar_t)c) == 1, "wcwidth of a printable ASCII character");
    for (c = 1; c < 0x20; c++)
        check(wcwidth((wchar_t)c) == -1, "wcwidth of a control character");
    check(wcwidth(0x7f) == -1, "wcwidth of DEL");

    check(wcswidth(L"", 5) == 0, "wcswidth of an empty string");
    check(wcswidth(L"abc", 5) == 3, "wcswidth stops at the null");
    check(wcswidth(L"abc", 2) == 2, "wcswidth stops at n");
    check(wcswidth(L"a\tb", 3) == -1, "wcswidth of a control character");
    check(wcswidth(L"ab\n", 2) == 2, "wcswidth ignores what follows n");
}

int
main(void)
{
    portable();

    /* The C locale has no printable characters outside the portable set */
    check(wcwidth(0xe9) == -1, "wcwidth of U+00E9 in the C locale");
    check(wcwidth(0x4e00) == -1, "wcwidth of U+4E00 in the C locale");

    if (setlocale(LC_CTYPE, "C.UTF-8") != NULL) {
        wchar_t wide[] = { L'a', 0x4e00, 0xe9, 0 };

        portable();
        check(wcwidth(0xe9) == 1, "wcwidth of U+00E9");
        check(wcwidth(0x4e00) == 2, "wcwidth of a wide character");
        check(wcwidth(0x0301) == 0, "wcwidth of a combining character");
        check(wcswidth(wide, 8) == 4, "wcswidth of mixed widths");
        check(setlocale(LC_CTYPE, "C") != NULL, "back to the C locale");
    } else {
        printf("no C.UTF-8 locale, skipping the UTF-8 checks\n");
    }

    printf("success\n");
    return 0;
}
