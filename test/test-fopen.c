/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2022 Keith Packard
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
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "FOPEN.TXT"
#endif

#define check(condition, message) do {                  \
        if (!(condition)) {                             \
            printf("%s: %s\n", message, #condition);    \
            if (f)                                      \
                fclose(f);                              \
            (void) remove(TEST_FILE_NAME);              \
            exit(1);                                    \
        }                                               \
    } while(0)

#define MESSAGE "hello, world\n"

void
check_contents(int repeats)
{
    FILE *f;
    char *s;
    int r;
    int c;

    f = fopen(TEST_FILE_NAME, "r");
    check(f != NULL, "fopen r");
    for (r = 0; r < repeats; r++) {
        for (s = MESSAGE; *s; s++) {
            c = getc(f);
            check((char) c == *s, "contents");
        }
    }
    c = getc(f);
    check(c == EOF, "EOF");
    r = fclose(f);
    f = NULL;
    check(r == 0, "fclose r");
}

int
main(void)
{
    FILE *f;

    /* Make sure we can create a file, write contents and read them back */
    f = fopen(TEST_FILE_NAME, "w");
    check(f != NULL, "fopen w");
    fputs(MESSAGE, f);
    fclose(f);
    check_contents(1);

    /* Make sure we can append contents to a file and read them back */
    f = fopen(TEST_FILE_NAME, "a");
    check(f != NULL, "fopen a");
    fputs(MESSAGE, f);
    fclose(f);
    check_contents(2);

    /* Make sure we can truncate the file */
    f = fopen(TEST_FILE_NAME, "w");
    check(f != NULL, "fopen  w 2");
    fclose(f);
    check_contents(0);

    (void) remove(TEST_FILE_NAME);

    exit(0);
}
