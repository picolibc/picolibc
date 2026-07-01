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
#include <unistd.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "FFLUSH.TXT"
#endif

#define check(condition, message)                    \
    do {                                             \
        if (!(condition)) {                          \
            printf("%s: %s\n", message, #condition); \
            (void)remove(TEST_FILE_NAME);            \
            exit(1);                                 \
        }                                            \
    } while (0)

#define MESSAGE "hello"

int
main(void)
{
    FILE    *f;
    int      ret;
    char     buf[16];
    off_t    seekret;
    ssize_t  nread;
    long     stdio_pos;
    off_t    fd_pos;

#ifndef TESTS_ENABLE_POSIX_IO
    printf("POSIX I/O not enabled, skipping\n");
    return 77;
#endif

    /* Clean up any leftover files from previous test runs */
    (void)remove(TEST_FILE_NAME);

    /*
     * Test 1: fflush on a write-mode stream returns 0
     */
    f = fopen(TEST_FILE_NAME, "w");
    check(f != NULL, "fopen w");
    ret = fputs(MESSAGE, f);
    check(ret >= 0, "fputs");
    ret = fflush(f);
    check(ret == 0, "fflush write");
    fclose(f);

    /*
     * Test 2: fflush actually persists data without fclose.
     *
     * Open for read+write, write MESSAGE, then fflush.  Verify the data
     * is on the underlying fd by seeking back to 0 and doing a raw read
     * — all before fclose, so fclose cannot be responsible for the flush.
     */
    f = fopen(TEST_FILE_NAME, "w+");
    check(f != NULL, "fopen w+ round-trip");
    ret = fputs(MESSAGE, f);
    check(ret >= 0, "fputs round-trip");
    ret = fflush(f);
    check(ret == 0, "fflush round-trip");

    /* Verify data is on the fd now, before fclose */
    seekret = lseek(fileno(f), 0, SEEK_SET);
    check(seekret == 0, "lseek round-trip");
    memset(buf, 0, sizeof(buf));
    nread = read(fileno(f), buf, sizeof(buf) - 1);
    check(nread == (ssize_t)strlen(MESSAGE), "round-trip read length");
    check(strcmp(buf, MESSAGE) == 0, "round-trip data matches");

    fclose(f);

    /*
     * Test 3: fflush on a read-mode stream syncs the OS file descriptor
     * back to the stdio read position.
     *
     * The stdio layer uses a read-ahead buffer: after the first fgetc(),
     * the app has consumed 1 byte but the OS FD may be positioned further
     * ahead (at the end of the buffer fill).  fflush() discards the
     * read-ahead buffer and rewinds the FD to match ftell(), so that
     * subsequent raw POSIX reads start from the correct offset.
     */
    f = fopen(TEST_FILE_NAME, "r");
    check(f != NULL, "fopen r read-ahead");

    /* Consume one byte — app position advances to 1 */
    ret = fgetc(f);
    check(ret == MESSAGE[0], "fgetc read-ahead");

    stdio_pos = ftell(f);
    check(stdio_pos == 1, "ftell after fgetc");

    /* Confirm the fd is ahead of the logical position due to read-ahead;
     * if this fails the test is not exercising the seek-back at all. */
    fd_pos = lseek(fileno(f), 0, SEEK_CUR);
    check(fd_pos > (off_t)stdio_pos, "fd is ahead before fflush");

    /* Flush: discard read-ahead buffer, rewind FD to stdio position */
    ret = fflush(f);
    check(ret == 0, "fflush read");

    /* Raw FD position must now match the stdio position */
    fd_pos = lseek(fileno(f), 0, SEEK_CUR);
    check(fd_pos == (off_t)stdio_pos, "fd pos matches ftell after fflush");

    /* Continued stdio reads must still work correctly from position 1 */
    ret = fgetc(f);
    check(ret == MESSAGE[1], "fgetc after fflush read");

    fclose(f);

    /*
     * Test 4: fflush(stdout) returns 0
     */
    ret = fflush(stdout);
    check(ret == 0, "fflush stdout");

#ifdef __STDIO_EXIT_FLUSH
    /*
     * Test 5: fflush(NULL) flushes all open streams and persists data.
     * Only available when the library is built with stdio-exit-flush=true.
     *
     * Write to a file via stdio, call fflush(NULL) without closing the
     * stream, then verify the data was flushed by seeking the same fd
     * back to 0 and reading via raw POSIX I/O.  Using the same fd avoids
     * any semihost visibility issue that can occur when opening a second
     * fd to the same file while the first is still open.
     */
    f = fopen(TEST_FILE_NAME, "w+");
    check(f != NULL, "fopen w fflush-null");
    ret = fputs(MESSAGE, f);
    check(ret >= 0, "fputs fflush-null");

    /* Data is in the stdio buffer — not yet written to the fd */
    ret = fflush(NULL);
    check(ret == 0, "fflush NULL");

    /* Verify data was written by seeking the underlying fd to 0 and reading */
    int  fd = fileno(f);
    check(fd >= 0, "fileno fflush-null");
    seekret = lseek(fd, 0, SEEK_SET);
    check(seekret == 0, "lseek fflush-null");
    memset(buf, 0, sizeof(buf));
    nread = read(fd, buf, sizeof(buf) - 1);
    check(nread == (ssize_t)strlen(MESSAGE), "fflush-null read length");
    check(strcmp(buf, MESSAGE) == 0, "fflush-null data matches");

    fclose(f);
#endif

    (void)remove(TEST_FILE_NAME);

    printf("fflush test passed\n");
    exit(0);
}
