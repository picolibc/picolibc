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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int
check(char *label, bool ok, int expect, int got)
{
    if (!ok) {
        fprintf(stderr, "%s: expect %d got %d\n", label, expect, got);
        return 1;
    }
    return 0;
}

int
check_s(char *label, bool ok, size_t expect, size_t got)
{
    if (!ok) {
        fprintf(stderr, "%s: expect %zd got %zd\n", label, expect, got);
        return 1;
    }
    return 0;
}

int
main(void)
{
    int a;
    size_t s;
    int ret = 0;

    a = putc('X', stdout);
    ret += check("putc", a == 'X', 'X', a);

    a = putchar('Y');
    ret += check("putchar", a == 'Y', 'Y', a);

    a = fputc('Z', stdout);
    ret += check("fputc", a == 'Z', 'Z', a);

    a = puts("puts");
    ret += check ("puts", a >= 0, 0, a);

    a = fputs("fputs\n", stdout);
    ret += check ("fputs", a >= 0, 0, a);

    s = fwrite("fwrite 1\n", 1, 9, stdout);
    ret += check_s ("fwrite 1", s == 9, 9, s);

    s = fwrite("fwrite 2\n", 0, 9, stdout);
    ret += check_s ("fwrite 2", s == 0, 0, s);

    s = fwrite("fwrite 3\n", 0, 0, stdout);
    ret += check_s ("fwrite 3", s == 0, 0, s);

    return ret;
}
