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

/*
 * Test that lseek() rejects offsets that exceed the 32-bit range
 * supported by the Hexagon semihosting ABI (SYS_SEEK).
 *
 * The Hexagon semihost lseek() computes the final absolute offset and
 * checks it against INT32_MAX / INT32_MIN before issuing the trap.
 * When the offset is out of range it must return -1 and set errno to
 * EINVAL without invoking the semihost call.
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "LSEEKOVERFLOW.TXT"
#endif

#define MESSAGE "hello, world\n"

#define check(condition, message)                    \
    do {                                             \
        if (!(condition)) {                          \
            printf("%s: %s\n", message, #condition); \
            (void)remove(TEST_FILE_NAME);            \
            exit(1);                                 \
        }                                            \
    } while (0)

int
main(void)
{
    /* Right now test is aimed for Hexagon only */
#ifndef __HEXAGON_ARCH__
    return 77;
#endif

#ifndef TESTS_ENABLE_POSIX_IO
    printf("POSIX I/O not enabled, skipping\n");
    return 77;
#endif
    int   fd;
    off_t ret;

    /* Create a small file so we have a valid fd to work with. */
    fd = open(TEST_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    check(fd >= 0, "open for write");
    check(write(fd, MESSAGE, strlen(MESSAGE)) == (ssize_t)strlen(MESSAGE), "write");
    check(close(fd) == 0, "close after write");

    fd = open(TEST_FILE_NAME, O_RDONLY);
    check(fd >= 0, "open for read");

    /*
     * Test 1: SEEK_SET with an offset larger than INT32_MAX.
     * The final absolute offset equals the supplied value, which
     * overflows the 32-bit semihost ABI.
     */
    errno = 0;
    off_t more_than_32 = INT32_MAX;
    more_than_32 += 1;
    ret = lseek(fd, more_than_32, SEEK_SET);
    check(ret == (off_t)-1, "SEEK_SET overflow should return -1");
    check(errno == EINVAL, "SEEK_SET overflow should set errno to EINVAL");

    /*
     * Test 2: SEEK_SET with a negative offset below INT32_MIN.
     * Negative absolute offsets are also out of range for the ABI.
     */
    errno = 0;
    ret = lseek(fd, (off_t)INT32_MIN - 1, SEEK_SET);
    check(ret == (off_t)-1, "SEEK_SET underflow should return -1");
    check(errno == EINVAL, "SEEK_SET underflow should set errno to EINVAL");

    (void)close(fd);
    (void)remove(TEST_FILE_NAME);
    return 0;
}
