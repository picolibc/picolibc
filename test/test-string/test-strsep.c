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

/* strsep is a BSD extension */
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>

/*
 * strsep returns every field, including empty ones at the start, in
 * the middle and at the end, then NULL with *stringp set to NULL.
 */

static int ret;

static void
split(const char *input, const char *delim, const char * const *want)
{
    char  buf[64];
    char *s = buf, *field;
    int   i;

    snprintf(buf, sizeof(buf), "%s", input);
    for (i = 0; (field = strsep(&s, delim)) != NULL; i++) {
        if (want[i] == NULL) {
            printf("strsep(\"%s\") gave an extra field \"%s\"\n", input, field);
            ret = 1;
            return;
        }
        if (strcmp(field, want[i]) != 0) {
            printf("strsep(\"%s\") field %d is \"%s\", not \"%s\"\n", input, i, field, want[i]);
            ret = 1;
        }
    }
    if (want[i] != NULL) {
        printf("strsep(\"%s\") stopped before field %d \"%s\"\n", input, i, want[i]);
        ret = 1;
    }
    if (s != NULL) {
        printf("strsep(\"%s\") left the pointer set\n", input);
        ret = 1;
    }
}

#define FIELDS(...) ((const char * const[]) { __VA_ARGS__, NULL })

int
main(void)
{
    char *s = NULL;

    split("a:b:c", ":", FIELDS("a", "b", "c"));
    split("a:", ":", FIELDS("a", ""));
    split(":a", ":", FIELDS("", "a"));
    split("a::b", ":", FIELDS("a", "", "b"));
    split(":", ":", FIELDS("", ""));
    split("", ":", FIELDS(""));
    split("a,b;c", ",;", FIELDS("a", "b", "c"));
    split("abc", "", FIELDS("abc"));

    if (strsep(&s, ":") != NULL) {
        printf("strsep with a null pointer\n");
        ret = 1;
    }
    if (ret == 0)
        printf("success\n");
    return ret;
}
