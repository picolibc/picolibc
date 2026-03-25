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
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "RENAME.TXT"
#endif

#define TEST_FILE_NAME_1 "A" TEST_FILE_NAME
#define TEST_FILE_NAME_2 "B" TEST_FILE_NAME

#define check(condition, message)                    \
    do {                                             \
        if (!(condition)) {                          \
            printf("%s: %s\n", message, #condition); \
            (void)remove(TEST_FILE_NAME_1);          \
            (void)remove(TEST_FILE_NAME_2);          \
            exit(1);                                 \
        }                                            \
    } while (0)

#define MESSAGE "hello, world\n"

#ifdef __PICOLIBC__
__typeof(rename) __fake_rename __weak;
#define not_fake(sym) ((sym) != __fake_##sym)
#else
#define not_fake(sym) 1
#endif

int
main(void)
{
    FILE *f;
    char  buf[256];
    int   ret;

#ifndef TESTS_ENABLE_POSIX_IO
    printf("POSIX I/O not enabled, skipping\n");
    return 77;
#endif

    if (!not_fake(rename)) {
        printf("fake rename, skipping\n");
        return 77;
    }

    /* Clean up any leftover files from previous test runs */
    (void)remove(TEST_FILE_NAME_1);
    (void)remove(TEST_FILE_NAME_2);

    /* Create a test file with some content */
    f = fopen(TEST_FILE_NAME_1, "w");
    check(f != NULL, "fopen w " TEST_FILE_NAME_1);
    fputs(MESSAGE, f);
    fclose(f);

    /* Verify the file exists and can be read */
    f = fopen(TEST_FILE_NAME_1, "r");
    check(f != NULL, "fopen r " TEST_FILE_NAME_1);
    fgets(buf, sizeof(buf), f);
    fclose(f);
    check(strcmp(buf, MESSAGE) == 0, "file contents match");

    /* Rename the file */
    ret = rename(TEST_FILE_NAME_1, TEST_FILE_NAME_2);
    check(ret == 0, "rename " TEST_FILE_NAME_1 " -> " TEST_FILE_NAME_2);

    /* Verify the old file no longer exists */
    f = fopen(TEST_FILE_NAME_1, "r");
    check(f == NULL, "old file should not exist");

    /* Verify the new file exists and has the same content */
    f = fopen(TEST_FILE_NAME_2, "r");
    check(f != NULL, "fopen r " TEST_FILE_NAME_2);
    memset(buf, 0, sizeof(buf));
    fgets(buf, sizeof(buf), f);
    fclose(f);
    check(strcmp(buf, MESSAGE) == 0, "renamed file contents match");

    /* Test renaming to an existing file (should succeed and replace) */
    f = fopen(TEST_FILE_NAME_1, "w");
    check(f != NULL, "fopen w " TEST_FILE_NAME_1 " (second time)");
    fputs("different content\n", f);
    fclose(f);

    ret = rename(TEST_FILE_NAME_1, TEST_FILE_NAME_2);
    check(ret == 0, "rename over existing file");

    /* Verify the new content replaced the old */
    f = fopen(TEST_FILE_NAME_2, "r");
    check(f != NULL, "fopen r " TEST_FILE_NAME_2 " (after replace)");
    memset(buf, 0, sizeof(buf));
    fgets(buf, sizeof(buf), f);
    fclose(f);
    check(strcmp(buf, "different content\n") == 0, "replaced file contents match");

    /* Clean up */
    (void)remove(TEST_FILE_NAME_1);
    (void)remove(TEST_FILE_NAME_2);

    printf("rename test passed\n");
    exit(0);
}
