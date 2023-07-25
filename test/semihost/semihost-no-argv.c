/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2020 Keith Packard
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char *expect_argv[] = { "program-name" };

#define EXPECT_NARG (sizeof(expect_argv) / sizeof(expect_argv[0]))

int
main(int argc, char **argv)
{
        unsigned c;
        int errors = 0;

        if (argc != EXPECT_NARG) {
                printf("argc is %d expect %d\n", argc, (int) EXPECT_NARG);
                errors = 1;
        }
        /* Skip argv[0] as the program name might come from the filename */
        for (c = 1; (int) c < argc; c++) {
                if (c < EXPECT_NARG) {
                        if (strcmp(argv[c], expect_argv[c]) != 0) {
                                printf("argv[%d] is '%s' expect '%s'\n",
                                       c, argv[c], expect_argv[c]);
                                errors = 1;
                        }
                }
        }
        return errors;
}
