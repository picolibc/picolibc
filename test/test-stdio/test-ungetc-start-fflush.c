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

/*
 * Input-stream fflush must discard a byte pushed back with ungetc at
 * the start of the stream and leave the stream readable from the
 * beginning, for both fmemopen and buffered streams. The file
 * position after ungetc at the start is indeterminate, so fflush
 * may report failure, but it must not lose the stream contents.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <string.h>

static const char data[] = "ABCD";
static size_t     backend_pos;

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
    off_t pos;

    (void)cookie;
    if (whence == SEEK_SET)
        pos = *offset;
    else if (whence == SEEK_CUR)
        pos = (off_t)backend_pos + *offset;
    else if (whence == SEEK_END)
        pos = (off_t)(sizeof(data) - 1) + *offset;
    else
        pos = -1;
    if (pos < 0) {
        errno = EINVAL;
        return -1;
    }
    backend_pos = (size_t)pos;
    *offset = pos;
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

static int
check(const char *name, FILE *f)
{
    int pushed, flush_ret, next;

    pushed = ungetc('X', f);
    flush_ret = fflush(f);
    next = fgetc(f);
    printf("%s: ungetc=%c fflush=%d next=%c ferror=%d\n", name, pushed, flush_ret, next, ferror(f));
    fclose(f);

    if (pushed != 'X' || (flush_ret != 0 && flush_ret != EOF) || next != 'A') {
        printf("%s: ungetc at start of stream broke fflush\n", name);
        return 1;
    }
    return 0;
}

int
main(void)
{
    char  mem[4] = { 'A', 'B', 'C', 'D' };
    char  file_buf[8];
    FILE *f;
    int   ret = 0;

    f = fmemopen(mem, sizeof(mem), "r");
    if (!f) {
        perror("fmemopen");
        return 2;
    }
    ret |= check("fmemopen", f);

    f = fopencookie(NULL, "r", funcs);
    if (!f) {
        perror("fopencookie");
        return 2;
    }
    if (setvbuf(f, file_buf, _IOFBF, sizeof(file_buf)) != 0) {
        perror("setvbuf");
        return 2;
    }
    ret |= check("bufio", f);

    return ret;
}
