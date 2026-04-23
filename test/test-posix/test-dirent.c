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
#include <string.h>
#include <limits.h>
#include <dirent.h>

#define dirent_name(ent) ((ent)->d_name)

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "DIRENT.TXT"
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
__typeof(opendir)  __fake_opendir __weak;
__typeof(readdir)  __fake_readdir __weak;
__typeof(closedir) __fake_closedir __weak;
#define not_fake(sym) ((sym) != __fake_##sym)
#else
#define not_fake(sym) 1
#endif

int
main(void)
{
    FILE          *f;
    DIR           *dir;
    struct dirent *ent;
    int            found;

#ifndef TESTS_ENABLE_POSIX_IO
    printf("POSIX I/O not enabled, skipping\n");
    return 77;
#endif

    if (!not_fake(opendir) || !not_fake(readdir) || !not_fake(closedir)) {
        printf("fake dirent ops, skipping\n");
        return 77;
    }

    /* Clean up any leftover file from a previous run */
    (void)remove(TEST_FILE_NAME);

    /* Create a test file so we have a known entry to look for */
    f = fopen(TEST_FILE_NAME, "w");
    check(f != NULL, "fopen w " TEST_FILE_NAME);
    fputs("dirent test\n", f);
    fclose(f);

    /* opendir: open the current directory */
    dir = opendir(".");
    check(dir != NULL, "opendir .");

    /* readdir: scan entries until the test file is found */
    found = 0;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(dirent_name(ent), TEST_FILE_NAME) == 0) {
            found = 1;
            break;
        }
    }

    /* closedir: close the directory handle */
    check(closedir(dir) == 0, "closedir");

    check(found, "test file not found in directory listing");

    /* Clean up */
    (void)remove(TEST_FILE_NAME);

    printf("dirent test passed\n");
    exit(0);
}
