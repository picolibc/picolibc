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
 * ftell on a buffered stream must report the logical position,
 * including data still sitting in the write buffer.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <string.h>

static char   backend[64];
static size_t backend_pos;

static ssize_t
cookie_write(void *cookie, const char *buf, size_t n)
{
    (void)cookie;
    if (n > sizeof(backend) - backend_pos)
        n = sizeof(backend) - backend_pos;
    memcpy(backend + backend_pos, buf, n);
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
        pos = (off_t)sizeof(backend) + *offset;
    else
        pos = -1;
    if (pos < 0 || pos > (off_t)sizeof(backend)) {
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
    .read = 0,
    .write = cookie_write,
    .seek = cookie_seek,
    .close = cookie_close,
};

int
main(void)
{
    char  file_buf[16];
    FILE *f = fopencookie(NULL, "w", funcs);
    long  buffered, flushed, seeked;

    if (!f) {
        perror("fopencookie");
        return 2;
    }
    if (setvbuf(f, file_buf, _IOFBF, sizeof(file_buf)) != 0) {
        perror("setvbuf");
        return 2;
    }

    /* Data still in the buffer */
    fputs("hello", f);
    buffered = ftell(f);

    /* Data written to the backend */
    fflush(f);
    flushed = ftell(f);

    /* Reposition, then buffer more data */
    fseek(f, 2, SEEK_SET);
    fputs("LL", f);
    seeked = ftell(f);

    fclose(f);
    printf("buffered=%ld flushed=%ld seeked=%ld backend=%.5s\n", buffered, flushed, seeked,
           backend);

    if (buffered != 5 || flushed != 5 || seeked != 4) {
        printf("ftell reported the wrong position for a write stream\n");
        return 1;
    }
    if (memcmp(backend, "heLLo", 5) != 0) {
        printf("wrong backend contents\n");
        return 1;
    }
    return 0;
}
