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
#include <stdbool.h>

/* Validate C locale ctype data */

#define boolname(b)     ((b) ? "true" : "false")
#define iscat(name)     is ## name
#define TEST(name)   do {                                               \
        if (!!iscat(name)(c) != !!name) {                               \
            printf("character %#2x '%c' is%s is %s should be %s\n",     \
                   c, c, #name, boolname(iscat(name)(c)), boolname(name)); \
            error = 1;                                                  \
        }                                                               \
    } while(0)

int main(void) {

    int c;
    int error = 0;

    for (c = 0; c < 128; c++) {
        /* Direct values */
        bool blank = c == ' ' || c == '\t';
        bool cntrl = c < ' ' || c >= 127;
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

        TEST(alnum);
        TEST(alpha);
        TEST(blank);
        TEST(cntrl);
        TEST(digit);
        TEST(graph);
        TEST(lower);
        TEST(print);
        TEST(punct);
        TEST(space);
        TEST(upper);
        TEST(xdigit);
    }
    return error;
}
