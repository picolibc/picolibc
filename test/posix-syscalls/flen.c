/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2020 Keith Packard
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

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "SEMIFLEN.TXT"
#endif

#define TEST_STRING     "hello, world"
#define TEST_STRING_LEN 12

int
main(void)
{
    int          fd;
    ssize_t      written;
    int          code = 0;
    int          ret;
    struct stat  st;

    fd = open(TEST_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
      if (errno == ENOSYS) {
        printf("open not implemented, skipping test\n");
        exit(77);
      }
        printf("open %s failed: %d\n", TEST_FILE_NAME, errno);
        exit(1);
    }
    written = write(fd, TEST_STRING, TEST_STRING_LEN);
    if (written != TEST_STRING_LEN) {
        printf("write failed %ld %d\n", (long)written, errno);
        code = 2;
        goto bail1;
    }
    ret = close(fd);
    fd = -1;
    if (ret != 0) {
        printf("close failed %d %d\n", ret, errno);
        code = 3;
        goto bail1;
    }
    fd = open(TEST_FILE_NAME, O_RDONLY);
    if (fd < 0) {
        printf("open %s failed: %d\n", TEST_FILE_NAME, errno);
        code = 4;
        goto bail1;
    }
    ret = fstat(fd, &st);
    if (ret != 0) {
        if (errno == ENOSYS) {
          printf("fstat not implemented, skipping test\n");
          close(fd);
          unlink(TEST_FILE_NAME);
          exit(77);
        }
        printf("fstat failed %d %d\n", ret, errno);
        code = 5;
        goto bail1;
    }
    if (st.st_size != TEST_STRING_LEN) {
        printf("fstat size failed %ld != %d\n", (long)st.st_size, TEST_STRING_LEN);
        code = 6;
        goto bail1;
    }
bail1:
    if (fd >= 0)
        (void)close(fd);
    (void)unlink(TEST_FILE_NAME);
    exit(code);
}
