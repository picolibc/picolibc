/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Artem Kulyk
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

/*
 * POSIX input-stream fflush must reposition the backend and report a
 * failed lseek. The unconsumed read-ahead bytes must not be silently
 * discarded.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

static const char data[] = "ABCDEFGH";
static size_t     backend_pos;
static int        fail_seek;

static ssize_t
cookie_read(void *cookie, char *buf, size_t n)
{
    size_t left = sizeof(data) - 1 - backend_pos;

    (void)cookie;
    if (n > left)
        n = left;
    memcpy(buf, data + backend_pos, n);
    backend_pos += n;
    return (ssize_t)n;
}

static int
cookie_seek(void *cookie, off_t *offset, int whence)
{
    (void)cookie;
    if (fail_seek) {
        errno = EIO;
        return -1;
    }
    if (whence == SEEK_SET)
        backend_pos = (size_t)*offset;
    else if (whence == SEEK_CUR)
        backend_pos = (size_t)((off_t)backend_pos + *offset);
    else if (whence == SEEK_END)
        backend_pos = (size_t)((off_t)(sizeof(data) - 1) + *offset);
    else
        return -1;
    *offset = (off_t)backend_pos;
    return 0;
}

static int
cookie_close(void *cookie)
{
    (void)cookie;
    return 0;
}

static cookie_io_functions_t funcs = {
    .read = cookie_read,
    .write = 0,
    .seek = cookie_seek,
    .close = cookie_close,
};

int
main(void)
{
    char  file_buf[8];
    FILE *f = fopencookie(NULL, "r", funcs);
    int   first, next, flush_ret;
    long  pos;

    if (!f) {
        perror("fopencookie");
        return 2;
    }
    if (setvbuf(f, file_buf, _IOFBF, sizeof(file_buf)) != 0) {
        perror("setvbuf");
        return 2;
    }

    first = fgetc(f);
    fail_seek = 1;
    flush_ret = fflush(f);
    fail_seek = 0;
    pos = ftell(f);
    next = fgetc(f);

    printf("first=%c fflush=%d pos=%ld next=%d\n", first, flush_ret, pos, next);
    fclose(f);

    if (first != 'A' || flush_ret != -1 || pos != 1 || next != 'B') {
        printf("failed input fflush discarded read-ahead\n");
        return 1;
    }
    return 0;
}
