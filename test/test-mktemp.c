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
#include <string.h>

#define check(condition, message) do {                  \
        if (!(condition)) {                             \
            printf("%s: %s\n", message, #condition);    \
            if (f)                                      \
                fclose(f);                              \
            (void) remove(template);                    \
            exit(1);                                    \
        }                                               \
    } while(0)

static const char NAME_TEMPLATE[] = "foo-XXXXXX";

static const char NAME_TEMPLATE_EXT[] = "foo-XXXXXX.txt";

#define EXT_LEN                 4

#define MESSAGE "hello, world\n"

void
check_contents(char *template,int repeats)
{
    FILE *f;
    char *s;
    int r;
    int c;

    f = fopen(template, "r");
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

/* Allow testing mktemp which is deprecated (for good reason) */
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

int
main(void)
{
    char        template[sizeof(NAME_TEMPLATE_EXT)];
    char        *s_ret;
    FILE        *f = NULL;
    int         i_ret;

    /* Create temp file */
    strcpy(template, NAME_TEMPLATE);
    s_ret = mktemp(template);
    check(s_ret == template, "mktemp");

    /* Make sure we can create a file, write contents and read them back */
    f = fopen(template, "w");
    check(f != NULL, "fopen w");
    fputs(MESSAGE, f);
    fclose(f);
    f = NULL;
    check_contents(template, 1);
    (void) remove(template);

    /* Create temp file */
    strcpy(template, NAME_TEMPLATE);
    i_ret = mkstemp(template);
    check(i_ret >= 0, "mkstemp");

    /* Make sure we can create a file, write contents and read them back */
    f = fdopen(i_ret, "w");
    check(f != NULL, "fopen w");
    fputs(MESSAGE, f);
    fclose(f);
    f = NULL;
    check_contents(template, 1);
    (void) remove(template);

    /* Create temp file with extension */
    strcpy(template, NAME_TEMPLATE_EXT);
    i_ret = mkstemps(template, EXT_LEN);
    check(i_ret >= 0, "mkstemps");

    /* Make sure the name still ends with the extension */
    check(strcmp(template + sizeof(NAME_TEMPLATE_EXT) - EXT_LEN,
                 NAME_TEMPLATE_EXT + sizeof(NAME_TEMPLATE_EXT) - EXT_LEN) == 0, "extension");
    f = fdopen(i_ret, "w");
    check(f != NULL, "fopen w");
    fputs(MESSAGE, f);
    fclose(f);
    check_contents(template, 1);
    (void) remove(template);

    exit(0);
}
