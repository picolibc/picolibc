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

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "DPRINTF.TXT"
#endif

#define check(condition, message)                    \
    do {                                             \
        if (!(condition)) {                          \
            printf("%s: %s\n", message, #condition); \
            (void)remove(TEST_FILE_NAME);            \
            exit(1);                                 \
        }                                            \
    } while (0)

#define STRING_PART                                      \
    " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLM"    \
    "NOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"

static const char *strings[] = {
    "test dprintf\n",
    STRING_PART STRING_PART STRING_PART STRING_PART,
};

#define NSTRING (sizeof(strings) / sizeof(strings[0]))

int
main(void)
{
    size_t s;

    for (s = 0; s < NSTRING; s++) {
        /* Create test file */
        int fd = open(TEST_FILE_NAME, O_CREAT | O_WRONLY | O_TRUNC, 0666);
        check(fd >= 0, "create file failed");

        /* Write test data using dprintf */
        int ret = dprintf(fd, "%s", strings[s]);

        /* Verify that dprint returns expected value */
        check(ret >= 0 && (size_t)ret == strlen(strings[s]), "dprint failed");

        /* Close the test file, re-open using fopen */
        close(fd);
        FILE       *f = fopen(TEST_FILE_NAME, "r");

        static char buf[512];
        check(f != NULL, "re-open file");

        /* Read test data using fread */
        ret = fread(buf, sizeof(char), sizeof(buf) / sizeof(buf[0]), f);
        check(ret >= 0 && (size_t)ret == strlen(strings[s]), "fread failed");
        buf[ret] = '\0';

        /* Verify file contains expected data */
        check(strcmp(buf, strings[s]) == 0, "fread contents incorrect");

        /* Clean up */
        fclose(f);
        remove(TEST_FILE_NAME);
    }
    printf("test-dprintf success\n");
    return 0;
}
