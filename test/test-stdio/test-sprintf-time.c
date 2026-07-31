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
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static const char s_testData[] = "1234567890"
                                 "1234567890"
                                 "1234567890"
                                 "1234567890"
                                 "1234567890"
                                 "1234567890"
                                 "1234567890"
                                 "1234567890"
                                 "1234567890"
                                 "1234567890";

static const char s_testFmt[] = "1234567890"
                                "1234567890"
                                "1234567890"
                                "1234567890"
                                "1234567890"
                                "1234567890"
                                "1234567890"
                                "1234567890"
                                "1234567890"
                                "1234567890"
                                "%s";
static void
Test5(unsigned loops)
{
    assert(strlen(s_testData) == 100);
    assert(strlen(s_testFmt) == 102);

    // Compile this code with -fno-builtin.

    char buffer[300];

    for (unsigned i = 0; i < loops; ++i) {
        sprintf(buffer, s_testFmt, s_testData);
    }
}

int
main(int argc, char **argv)
{
    unsigned loops = 0;

    if (argc >= 2)
        loops = atoi(argv[1]);
    if (loops == 0)
        loops = 1000;
    printf("loops: %u\n", loops);
    Test5(loops);
    exit(0);
}
