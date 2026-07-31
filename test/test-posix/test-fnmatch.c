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

#include <fnmatch.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>

static int
y(char const *pattern, char const *string, int flags)
{
    return fnmatch(pattern, string, flags) == 0;
}
static int
n(char const *pattern, char const *string, int flags)
{
    return fnmatch(pattern, string, flags) == FNM_NOMATCH;
}

static int
fail(int val, int line)
{
    printf("Failure type %d at line %d\n", val, line);
    return val;
}

int
main(void)
{
    char const       *Apat = 'A' < '\\' ? "[A-\\\\]" : "[\\\\-A]";
    char const       *apat = 'a' < '\\' ? "[a-\\\\]" : "[\\\\-a]";
    static char const A_1[] = { 'A' - 1, 0 };
    static char const A01[] = { 'A' + 1, 0 };
    static char const a_1[] = { 'a' - 1, 0 };
    static char const a01[] = { 'a' + 1, 0 };
    static char const bs_1[] = { '\\' - 1, 0 };
    static char const bs01[] = { '\\' + 1, 0 };
    int               result = 0;
    /* ==== Start of tests in the "C" locale ==== */
    /* These are sanity checks. They all succeed on current platforms.  */
    if (!n("a*", "", 0))
        result |= fail(1, __LINE__);
    if (!y("a*", "abc", 0))
        result |= fail(1, __LINE__);
    if (!n("d*/*1", "d/s/1", FNM_PATHNAME))
        result |= fail(1, __LINE__);
    if (!y("a\\bc", "abc", 0))
        result |= fail(1, __LINE__);
    if (!n("a\\bc", "abc", FNM_NOESCAPE))
        result |= fail(1, __LINE__);
    if (!y("*x", ".x", 0))
        result |= fail(1, __LINE__);
    if (!n("*x", ".x", FNM_PERIOD))
        result |= fail(1, __LINE__);
    if (!y(Apat, "\\", 0))
        result |= fail(2, __LINE__);
    if (!y(Apat, "A", 0))
        result |= fail(2, __LINE__);
    if (!y(apat, "\\", 0))
        result |= fail(2, __LINE__);
    if (!y(apat, "a", 0))
        result |= fail(2, __LINE__);
    if (!(n(Apat, A_1, 0) == ('A' < '\\')))
        result |= fail(2, __LINE__);
    if (!(n(apat, a_1, 0) == ('a' < '\\')))
        result |= fail(2, __LINE__);
    if (!(y(Apat, A01, 0) == ('A' < '\\')))
        result |= fail(2, __LINE__);
    if (!(y(apat, a01, 0) == ('a' < '\\')))
        result |= fail(2, __LINE__);
    if (!(y(Apat, bs_1, 0) == ('A' < '\\')))
        result |= fail(2, __LINE__);
    if (!(y(apat, bs_1, 0) == ('a' < '\\')))
        result |= fail(2, __LINE__);
    if (!(n(Apat, bs01, 0) == ('A' < '\\')))
        result |= fail(2, __LINE__);
    if (!(n(apat, bs01, 0) == ('a' < '\\')))
        result |= fail(2, __LINE__);
    /* glibc bug <https://sourceware.org/PR12378>
       exists in glibc 2.12, fixed in glibc 2.13.  */
    if (!y("[/b", "[/b", 0)) /*"]]"*/
        result |= fail(4, __LINE__);
    /* This test fails on FreeBSD 13.2, NetBSD 10.0, Cygwin 3.4.6.  */
    if (!y("[[:alnum:]]", "a", 0))
        result |= fail(8, __LINE__);
    /* ==== End of tests in the "C" locale ==== */
    /* ==== Start of tests that require a specific locale ==== */
    /* This test fails on Solaris 11.4.  */
    if (setlocale(LC_ALL, "en_US.UTF-8") != NULL) {
        if (!n("[!a-z]", "", 0))
            result |= fail(16, __LINE__);
    }
#ifndef __PICOLIBC__
    /* This test requires working multi-byte support in fnmatch */
    if (setlocale(LC_ALL, "C.UTF-8") != NULL) {
        if (!y("x?y", "x\303\274y", 0))
            result |= fail(32, __LINE__);
    }
#endif
    /* ==== End of tests that require a specific locale ==== */
    return result;
}
