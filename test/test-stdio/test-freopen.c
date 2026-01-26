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
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>

#ifdef TESTS_ENABLE_POSIX_IO

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "FREOPEN.TXT"
#endif

#define ANOTHER_FILE_NAME "2" TEST_FILE_NAME

#define check(condition, message)                    \
    do {                                             \
        if (!(condition)) {                          \
            printf("%s: %s\n", message, #condition); \
            if (f)                                   \
                fclose(f);                           \
            (void)remove(TEST_FILE_NAME);            \
            (void)remove(ANOTHER_FILE_NAME);         \
            exit(1);                                 \
        }                                            \
    } while (0)

#define MESSAGE "hello, world\n"

static void
check_contents(const char *filename, int repeats)
{
    FILE *f;
    char *s;
    int   r;
    int   c;

    f = fopen(filename, "r");
    check(f != NULL, "fopen r");
    for (r = 0; r < repeats; r++) {
        for (s = MESSAGE; *s; s++) {
            c = getc(f);
            check((char)c == *s, "contents");
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

    f = freopen(ANOTHER_FILE_NAME, "w", f);
    check(f != NULL, "freopen w");
    fputs(MESSAGE, f);
    fclose(f);

    check_contents(TEST_FILE_NAME, 1);
    check_contents(ANOTHER_FILE_NAME, 1);

    (void)remove(TEST_FILE_NAME);
    (void)remove(ANOTHER_FILE_NAME);

    exit(0);
}

#else

int
main(void)
{
    printf("POSIX I/O testing disabled, skipping.\n");
    return 77;
}

#endif
