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

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "ACCESS.TXT"
#endif

#define check(condition, message)                    \
    do {                                             \
        if (!(condition)) {                          \
            printf("%s: %s\n", message, #condition); \
            (void)remove(TEST_FILE_NAME);            \
            exit(1);                                 \
        }                                            \
    } while (0)

#ifdef __PICOLIBC__
__typeof(access) __fake_access __weak;
#define not_fake(sym) ((sym) != __fake_##sym)
#else
#define not_fake(sym) 1
#endif

int
main(void)
{
    int fd;
    int ret;

#ifndef TESTS_ENABLE_POSIX_IO
    printf("POSIX I/O not enabled, skipping\n");
    return 77;
#endif

    if (!not_fake(access)) {
        printf("fake access, skipping\n");
        return 77;
    }

    /* Clean up any leftover files from previous test runs */
    (void)remove(TEST_FILE_NAME);

    /* Check that a non-existent file is not accessible */
    ret = access(TEST_FILE_NAME, F_OK);
    check(ret != 0, "access non-existent file should fail");

    /* Create a test file */
    fd = open(TEST_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    check(fd >= 0, "open");
    close(fd);

    /* Check that the file now exists */
    ret = access(TEST_FILE_NAME, F_OK);
    check(ret == 0, "access existing file F_OK");

    /* Check read access */
    ret = access(TEST_FILE_NAME, R_OK);
    check(ret == 0, "access existing file R_OK");

    /* Clean up */
    (void)remove(TEST_FILE_NAME);

    printf("access test passed\n");
    exit(0);
}
