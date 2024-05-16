/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 Hana Ashour
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
#include <string.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "UNGETCFTELL.DAT"
#endif

const char *str = "This is a string";

int main(void) {

    FILE *file;
    char first;
    long position, start;

    file = fopen( TEST_FILE_NAME, "wb" );
    if( file == NULL ) return 1;
    fputs(str, file);
    fclose(file);

    file = fopen( TEST_FILE_NAME, "rb" );
    if(file == NULL) return 1;

    first = fgetc(file);
    printf("First character read: %c\n", first);

    start = ftell(file);
    printf("Position before ungetc: %ld\n", start);

    ungetc(first, file);  // Use ungetc to put the character back

    position = ftell(file);  // Check ftell position (should be 0 after ungetc if first character was read)
    printf("Position after ungetc: %ld\n", position);

    fclose(file);

    if (position == 0) {
        printf("Test passed: ungetc and ftell working as expected.\n");
        return 0;
    } else {
        printf("Test failed: Incorrect position after ungetc.\n");
        return 1;
    }
}
