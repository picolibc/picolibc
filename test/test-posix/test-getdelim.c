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
#include <string.h>

static const char testdata[] = "foo\nbar\n";

int
main(void)
{
    FILE *in = fmemopen((void *)testdata, sizeof(testdata) - 1, "r");
    int   result = 0;
    if (!in)
        return 1;
    {
        /* Test result for a NULL buffer and a zero size.
           Based on a test program from Karl Heuer.  */
        char  *line = NULL;
        size_t siz = 0;
        int    len = getdelim(&line, &siz, '\n', in);
        if (!(len == 4 && line && strcmp(line, "foo\n") == 0)) {
            printf("failed NULL buffer and zero size test\n");
            result = 1;
        }
        free(line);
    }
    {
        /* Test result for a NULL buffer and a non-zero size.
           This crashes on FreeBSD 8.0.  */
        char  *line = NULL;
        size_t siz = (size_t)(~0) / 4;
        if (getdelim(&line, &siz, '\n', in) == -1) {
            printf("failed NULL buffer and non-zero size test\n");
            result = 1;
        }
        free(line);
    }
    {
        char  *line = NULL;
        size_t siz = 0;
        /* Test result for a NULL buffer and zero size at EOF */
        if (getdelim(&line, &siz, '\n', in) != -1 || getdelim(&line, &siz, '\n', in) != -1) {
            printf("failed NULL buffer and zero size at EOF test\n");
            result = 1;
        }
        free(line);
    }

    fclose(in);

    {
        static const char input[] = "\n";
        FILE             *one = fmemopen((void *)input, sizeof(input) - 1, "r");
        char             *line = malloc(2);
        size_t            siz = 1;
        ssize_t           len;

        if (!one || !line)
            return 1;
        line[0] = 'x';
        line[1] = 'x';
        len = getdelim(&line, &siz, '\n', one);
        if (!(len == 1 && siz >= 2 && line[0] == '\n' && line[1] == '\0')) {
            printf("failed non-NULL buffer and zero size test. len %zd siz %zd line '%s'\n", len,
                   siz, line);
            result = 1;
        }
        free(line);
        fclose(one);
    }
    {
        static const char input[] = "123456789012345\n";
        FILE             *exact = fmemopen((void *)input, sizeof(input) - 1, "r");
        char             *line = malloc(sizeof(input));
        size_t            siz = sizeof(input) - 1;
        int               len;
        if (!exact || !line) {
            printf("test setup failure\n");
            return 1;
        }
        memset(line, 'x', sizeof(input));
        len = getdelim(&line, &siz, '\n', exact);
        if (!(len == (int)sizeof(input) - 1 && siz >= sizeof(input)
              && memcmp(line, input, sizeof(input)) == 0)) {
            printf("failed exact non-null buffer and non-zero size test\n");
            result = 1;
        }
        free(line);
        fclose(exact);
    }
    {
        static const char input[] = "tail";
        FILE             *partial = fmemopen((void *)input, sizeof(input) - 1, "r");
        char             *line = malloc(8);
        size_t            siz = 8;
        int               len;
        if (!partial || !line)
            return 1;
        memset(line, 'x', siz);
        len = getdelim(&line, &siz, '\n', partial);
        if (!(len == (int)sizeof(input) - 1 && strcmp(line, input) == 0)) {
            printf("failed over size non-null buffer and non-zero size test\n");
            result = 1;
        }
        free(line);
        fclose(partial);
    }
    return result;
}
