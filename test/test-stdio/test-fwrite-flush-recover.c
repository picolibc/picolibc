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
 * When fwrite fills the buffer and the resulting flush fails, the
 * buffer state must remain valid: a later write after clearerr must
 * land inside the buffer and be flushed alone.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

#define BUF_SIZE   16
#define GUARD_SIZE 16
#define GUARD_BYTE 0x5a

static int    fail_writes;
static int    write_calls;
static size_t write_len;
static char   write_data[BUF_SIZE];

static ssize_t
cookie_write(void *cookie, const char *buf, size_t n)
{
    (void)cookie;
    write_calls++;
    write_len = n;
    if (fail_writes)
        return -1;
    if (n <= sizeof(write_data))
        memcpy(write_data, buf, n);
    return (ssize_t)n;
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
    .seek = 0,
    .close = cookie_close,
};

/* Guard bytes on either side of the stdio buffer catch stray stores */
static struct {
    unsigned char before[GUARD_SIZE];
    char          buf[BUF_SIZE];
    unsigned char after[GUARD_SIZE];
} file_buf;

static int
check_guard(void)
{
    int i;

    for (i = 0; i < GUARD_SIZE; i++)
        if (file_buf.before[i] != GUARD_BYTE || file_buf.after[i] != GUARD_BYTE)
            return 0;
    return 1;
}

int
main(void)
{
    char   data[10];
    FILE  *f = fopencookie(NULL, "w", funcs);
    size_t first, second;
    int    put_ret, flush_ret, guard_ok;

    if (!f) {
        perror("fopencookie");
        return 2;
    }
    memset(file_buf.before, GUARD_BYTE, sizeof(file_buf.before));
    memset(file_buf.after, GUARD_BYTE, sizeof(file_buf.after));
    if (setvbuf(f, file_buf.buf, _IOFBF, sizeof(file_buf.buf)) != 0) {
        perror("setvbuf");
        return 2;
    }

    memset(data, 'a', sizeof(data));
    fail_writes = 1;

    /* Fits in the buffer */
    first = fwrite(data, 1, sizeof(data), f);

    /* Fills the buffer, flush fails */
    second = fwrite(data, 1, sizeof(data), f);

    if (!ferror(f)) {
        printf("fwrite flush failure did not set ferror\n");
        return 1;
    }

    /* Recover and write one byte */
    clearerr(f);
    fail_writes = 0;
    write_calls = 0;
    write_len = 0;
    put_ret = fputc('x', f);
    flush_ret = fflush(f);
    guard_ok = check_guard();

    printf("first=%zu second=%zu put=%d flush=%d calls=%d len=%zu guard=%d\n", first, second,
           put_ret, flush_ret, write_calls, write_len, guard_ok);
    fclose(f);

    if (first != sizeof(data) || second >= sizeof(data)) {
        printf("wrong fwrite return values\n");
        return 1;
    }
    if (!guard_ok) {
        printf("stdio wrote outside of the buffer\n");
        return 1;
    }
    if (put_ret != 'x' || flush_ret != 0 || write_calls != 1 || write_len != 1
        || write_data[0] != 'x') {
        printf("buffer state corrupted after failed flush\n");
        return 1;
    }
    return 0;
}
