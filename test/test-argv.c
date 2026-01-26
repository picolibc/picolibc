/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

  * Neither the name of Qualcomm Technologies, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Basic test to verify argv/argc parsing via semihosting. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef EXPECT_PROG
#define EXPECT_PROG "test-argv"
#endif
static const char *expect_prog = EXPECT_PROG;
static const char *expect_args[] = { "hello", "world" };

#define EXPECT_NARG (1 + (int)(sizeof(expect_args) / sizeof(expect_args[0])))

int
main(int argc, char **argv)
{
#if defined(__HEXAGON_ARCH__)
    unsigned i;
    int      errors = 0;

    if (argc != EXPECT_NARG) {
        printf("argc is %d expect %d\n", argc, (int)EXPECT_NARG);
        errors = 1;
    }

    /* Check program name tail matches 'test-argv' */
    const char *tail = strrchr(argv[0], '/');
    tail = tail ? tail + 1 : argv[0];
    if (strcmp(tail, expect_prog) != 0) {
        printf("argv[0] tail is '%s' expect '%s'\n", tail, expect_prog);
        errors = 1;
    }

    /* Check remaining arguments */
    for (i = 1; (int)i < argc; i++) {
        const char *exp = expect_args[i - 1];
        if (strcmp(argv[i], exp) != 0) {
            printf("argv[%u] is '%s' expect '%s'\n", i, argv[i], exp);
            errors = 1;
        }
    }

    return errors;
#else
    printf("Hexagon-only argv test; skipping\n");
    (void)argc;
    (void)argv;
    (void)expect_prog;
    (void)expect_args;
    return 77;
#endif
}
