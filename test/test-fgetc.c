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
#define TEST_FILE_NAME "FGETC.TXT"
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


int
main(void)
{
    FILE *f;
    int ch;

    /* Make sure we can create a file */
    f = fopen(TEST_FILE_NAME, "w");
    check(f != NULL, "Error opening file");

    // Get error indicator before reading
    check(ferror(f) == 0, "Error occured before reading");

    // Attempt to read from the file
    ch = fgetc(f);
    check(ch == EOF, "fgetc returns non EOF");

    // Get error indicator after reading
    check(ferror(f) != 0, "After reading from non readable stream, the error flag of straem f is not set");

    // Close the file
    fclose(f);

    (void) remove(TEST_FILE_NAME);

    exit(0);
}
