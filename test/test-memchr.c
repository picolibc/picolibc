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

#define _GNU_SOURCE
#include <string.h>
#include <stdint.h>
#include <stdio.h>

const char haystack_mid[] = "hello world";
#define BALE 128
char haystack[BALE];

static int
check (void *(*func)(const void *, int, size_t),
       const char *name,
       int off,
       int len,
       int needle,
       int expect)
{
    char *ptr = func(haystack + off, needle, len);
    int result = -1;
    if (ptr)
        result = ptr - (haystack + off);
    if (result != expect) {
        printf("%s(%s, 0x%x). Got %d expect %d\n",
               name, haystack, needle, result, expect);
        return 1;
    }
    return 0;
}

int
main(void)
{
    int ret = 0;
    int l_off, r_off;

    /*
     * Paste the search string into the middle of a block with various
     * alignment and length values
     */
    for (l_off = 0; l_off < 32; l_off++) {
        for (r_off = 0; r_off < 32; r_off++) {
            int len = BALE - l_off - r_off;
            memset(haystack, 'x', sizeof(haystack));
            memcpy(haystack + l_off, haystack_mid, sizeof(haystack_mid));
            ret |= check(memchr, "memchr", l_off, len, 'h', 0);
            ret |= check(memrchr, "memrchr", l_off, len, 'h', 0);
            ret |= check(memchr, "memchr", l_off, len, 'l', 2);
            ret |= check(memrchr, "memrchr", l_off, len, 'l', 9);
            ret |= check(memchr, "memchr", l_off, len, '\0', 11);
            ret |= check(memrchr, "memrchr", l_off, len, '\0', 11);
        }
    }
    return ret;
}
