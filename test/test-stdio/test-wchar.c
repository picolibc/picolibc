/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024, Synopsys Inc.
 * Copyright © 2024, Solid Sands B.V.
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
#include <wchar.h>
#include <unistd.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "WCHAR.DAT"
#endif

const wchar_t *string = L"Hello\n";

int
main(void)
{
    FILE          *file;
    const wchar_t *ref;
    wint_t         res;

    /* Create testfile.dat in write mode */
    file = fopen(TEST_FILE_NAME, "w");
    if (file == NULL)
        return 1;

    /* Write wide characters to the file */
    fputws(string, file);
    fclose(file);

    /* Open testfile.dat in read mode */
    file = fopen(TEST_FILE_NAME, "r");

    for (ref = string; *ref != L'\0'; ref++) {

        /* Read wide-char by wide-char from the file */
        res = fgetwc(file);
        if ((wchar_t)res != *ref) {
            printf("Test Failed: Failed to read wide character from file\n");
            unlink(TEST_FILE_NAME);
            return 1;
        }
    }

    if (fgetwc(file) != WEOF) {
        printf("Test Failed: Failed to check if end-of-file reached\n");
        unlink(TEST_FILE_NAME);
        return 1;
    }

    printf("Test Passed: Wide characters were written & read successfuly\n");
    unlink(TEST_FILE_NAME);
    return 0;
}
