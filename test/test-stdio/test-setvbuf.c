/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
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
#define TEST_FILE_NAME "SETVBUF.TXT"
#endif

#define check(condition, message, format, ...)      \
    do {                                            \
        if (!(condition)) {                         \
            printf("%s: %s ", message, #condition); \
            printf(format, ##__VA_ARGS__);          \
            if (f)                                  \
                fclose(f);                          \
            (void)remove(TEST_FILE_NAME);           \
            exit(1);                                \
        }                                           \
    } while (0)

#define MESSAGE  "hello, world\n"
#define ONE_CHAR "h"

static void
check_contents(const char *contents)
{
    FILE       *f;
    const char *s;
    int         r;
    int         c;

    f = fopen(TEST_FILE_NAME, "r");
    check(f != NULL, "fopen r", "%s\n", TEST_FILE_NAME);
    for (s = contents; *s; s++) {
        c = getc(f);
        check((char)c == *s, "contents", "got %d expect %d\n", c, *s);
    }
    c = getc(f);
    check(c == EOF, "EOF", "got %d expected EOF\n", c);
    r = fclose(f);
    f = NULL;
    check(r == 0, "fclose r", "got %d\n", r);
}

#define MY_BUFSIZ 512

static char mybuf[MY_BUFSIZ];

int
main(void)
{
    FILE *f;
    int   ret;

    /* Check _IOFBF mode */
    f = fopen(TEST_FILE_NAME, "w");
    check(f != NULL, "fopen w", "got NULL\n");
    ret = setvbuf(f, mybuf, _IOFBF, sizeof(mybuf));
    check(ret == 0, "setvbuf _IOFBF mybuf", "got %d expected 0\n", ret);
    fputs(MESSAGE, f);
    fclose(f);
    check_contents(MESSAGE);

    /* Check _IOFBF mode with library-allocated buffer */
    f = fopen(TEST_FILE_NAME, "w");
    check(f != NULL, "fopen w", "got NULL\n");
    ret = setvbuf(f, NULL, _IOFBF, MY_BUFSIZ);
    check(ret == 0, "setvbuf _IOFBF mybuf", "got %d expected 0\n", ret);
    fputs(MESSAGE, f);
    fclose(f);
    check_contents(MESSAGE);

    /* Check _IOLBF mode */
    f = fopen(TEST_FILE_NAME, "w");
    check(f != NULL, "fopen w", "got NULL\n");
    ret = setvbuf(f, mybuf, _IOLBF, sizeof(mybuf));
    check(ret == 0, "setvbuf _IOLBF mybuf", "got %d expected 0\n", ret);
    fputs(MESSAGE, f);

    /* Make sure the contents are in the file immediately */
    check_contents(MESSAGE);
    fclose(f);
    /* Make sure the contents haven't changed */
    check_contents(MESSAGE);

    /* Check _IOLBF mode with library allocated buffer */
    f = fopen(TEST_FILE_NAME, "w");
    check(f != NULL, "fopen w", "got NULL\n");
    ret = setvbuf(f, NULL, _IOLBF, MY_BUFSIZ);
    check(ret == 0, "setvbuf _IOLBF NULL", "got %d\n", ret);
    fputs(MESSAGE, f);

    /* Make sure the contents are in the file immediately */
    check_contents(MESSAGE);
    fclose(f);
    /* Make sure the contents haven't changed */
    check_contents(MESSAGE);

    /* Check _IONBF mode */
    f = fopen(TEST_FILE_NAME, "w");
    check(f != NULL, "fopen w", "got NULL\n");
    ret = setvbuf(f, NULL, _IONBF, 0);
    check(ret == 0, "setvbuf _IONBF NULL", "got %d expected 0\n", ret);
    fputs(ONE_CHAR, f);

    /* Make sure the contents are in the file immediately */
    check_contents(ONE_CHAR);
    fclose(f);
    /* Make sure the contents haven't changed */
    check_contents(ONE_CHAR);

    (void)remove(TEST_FILE_NAME);

    return 0;
}
