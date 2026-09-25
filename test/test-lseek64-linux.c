/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "LSEEK64LINUX.TXT"
#endif

#define check(condition, message)                                    \
    do {                                                             \
        if (!(condition)) {                                          \
            printf("%s: %s errno=%d\n", message, #condition, errno); \
            (void)remove(TEST_FILE_NAME);                            \
            exit(1);                                                 \
        }                                                            \
    } while (0)

int
main(void)
{
#ifdef TESTS_LSEEK64_SKIP
    return 77;
#else

#ifndef TESTS_ENABLE_POSIX_IO
    printf("POSIX I/O not enabled, skipping\n");
    return 77;
#endif

    int fd = open(TEST_FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
    check(fd >= 0, "open");

    off_t large = ((off_t)1 << 32) + 1;
    off_t ret = lseek(fd, large, SEEK_SET);
    check(ret == large, "SEEK_SET above INT32_MAX");

    char ch = 'x';
    check(write(fd, &ch, 1) == 1, "write after sparse seek");

    ret = lseek(fd, 0, SEEK_END);
    check(ret == large + 1, "SEEK_END after sparse write");

    ret = lseek(fd, -1, SEEK_CUR);
    check(ret == large, "negative SEEK_CUR from 64-bit offset");

    ch = 0;
    check(read(fd, &ch, 1) == 1, "read at sparse byte");
    check(ch == 'x', "sparse byte value");

    struct stat st;
    check(fstat(fd, &st) == 0, "fstat");
    check(st.st_size == large + 1, "sparse file size");

    check(close(fd) == 0, "close");
    check(remove(TEST_FILE_NAME) == 0, "remove");
    return 0;
#endif
}
