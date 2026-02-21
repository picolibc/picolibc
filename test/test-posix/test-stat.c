/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "FSTAT.TXT"
#endif

#define check(condition, message)                    \
    do {                                             \
        if (!(condition)) {                          \
            printf("%s: %s\n", message, #condition); \
            (void)remove(TEST_FILE_NAME);            \
            exit(1);                                 \
        }                                            \
    } while (0)

#define MESSAGE "hello, world\n"

#ifdef __PICOLIBC__
__typeof(stat)         __fake_stat __weak;
__typeof(fstat)        __fake_fstat __weak;
__typeof(gettimeofday) __fake_gettimeofday __weak;
#define not_fake(sym) ((sym) != __fake_##sym)
#else
#define not_fake(sym) 1
#endif

static void
check_stat(struct stat *statbuf)
{
    struct timeval now;

    if (not_fake(gettimeofday)) {
        gettimeofday(&now, NULL);

        check(statbuf->st_atime != 0, "zero access time");
        check(statbuf->st_ctime != 0, "zero create time");
        check(statbuf->st_mtime != 0, "zero modify time");

        check(statbuf->st_atime <= now.tv_sec, "future access time");
        check(statbuf->st_ctime <= now.tv_sec, "future create time");
        check(statbuf->st_mtime <= now.tv_sec, "future modify time");
    } else {
        printf("fake gettimeofday, skipping time checks\n");
    }

    /*
     * These elements are left at zero if the value isn't known
     *
     * check(statbuf->st_dev != 0, "zero device");
     * check(statbuf->st_ino != 0, "zero inode");
     * check(statbuf->st_blocks != 0, "zero blocks");
     */

    check(statbuf->st_rdev == 0, "non-zero rdev");
    check(statbuf->st_nlink <= 1, "nlink != 1");
    check(statbuf->st_size == (off_t)strlen(MESSAGE), "size != sizeof(MESSAGE)");
}

static void
check_contents(void)
{
    struct stat statbuf;
    int         ret;
    int         fd;

    if (not_fake(stat)) {
        memset(&statbuf, 0xcc, sizeof(statbuf));
        ret = stat(TEST_FILE_NAME, &statbuf);

        check(ret == 0, "stat " TEST_FILE_NAME);
        check_stat(&statbuf);
    } else {
        printf("fake stat, skipping\n");
    }

    if (not_fake(fstat)) {
        fd = open(TEST_FILE_NAME, 0);

        check(fd >= 0, "open " TEST_FILE_NAME);

        memset(&statbuf, 0xcc, sizeof(statbuf));
        ret = fstat(fd, &statbuf);

        close(fd);

        check(ret == 0, "stat " TEST_FILE_NAME);
        check_stat(&statbuf);
    } else {
        printf("fake fstat, skipping\n");
    }
}

int
main(void)
{
    FILE *f;

#ifndef TESTS_ENABLE_POSIX_IO
    printf("POSIX I/O not enabled, skipping\n");
    return 77;
#endif

    /* Make sure we can create a file, write contents and read them back */
    f = fopen(TEST_FILE_NAME, "w");
    check(f != NULL, "fopen w");
    fputs(MESSAGE, f);
    fclose(f);
    check_contents();

    (void)remove(TEST_FILE_NAME);

    exit(0);
}
