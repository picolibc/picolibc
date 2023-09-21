/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 Keith Packard
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

#include <string.h>
#include <stdint.h>
#include <stdio.h>

const char haystack[] = "hello world";

#define check(func, needle, expect) do {                                \
    char *ptr = func(haystack, needle);                                 \
    int result = -1;                                                    \
    if (ptr)                                                            \
        result = ptr - haystack;                                        \
    if (result != expect) {                                             \
        printf("%s(%s, 0x%x). Got %d expect %d\n",                      \
               #func, haystack, needle, result, expect);                \
        ret++;                                                          \
    }                                                                   \
    } while(0)

#if INT_MAX == INT16_MAX
#define many_check(func, needle, expect) do { \
        check(func, needle, expect);                    \
        check(func, needle | 0xff00, expect);           \
        check(func, needle | 0x0100, expect);           \
        check(func, needle | 0x8000, expect);           \
    } while(0)
#else
#define many_check(func, needle, expect) do { \
        check(func, needle, expect);                    \
        check(func, needle | 0xffffff00, expect);       \
        check(func, needle | 0x00000100, expect);       \
        check(func, needle | 0x80000000, expect);       \
    } while(0)
#endif

int
main(void)
{
    int ret = 0;

    many_check(strchr, 'h', 0);
    many_check(strrchr, 'h', 0);
    many_check(strchr, 'l', 2);
    many_check(strrchr, 'l', 9);
    many_check(strchr, '\0', 11);
    many_check(strrchr, '\0', 11);
    return ret;
}
